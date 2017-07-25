//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2016, Lawrence Livermore National Security, LLC.
//
// Produced at the Lawrence Livermore National Laboratory
//
// LLNL-CODE-689114
//
// All rights reserved.
//
// This file is part of RAJA.
//
// For additional details, please also read RAJA/LICENSE.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the disclaimer below.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the disclaimer (as noted below) in the
//   documentation and/or other materials provided with the distribution.
//
// * Neither the name of the LLNS/LLNL nor the names of its contributors may
//   be used to endorse or promote products derived from this software without
//   specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL LAWRENCE LIVERMORE NATIONAL SECURITY,
// LLC, THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
// IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#include <cstdlib>
#include <iostream>

#include "RAJA/RAJA.hpp"
#include "RAJA/util/defines.hpp"

void checkSolution(double *C, int in_N);

template <typename T>
T* allocate(RAJA::Index_type size) {
  T* ptr;
#if defined(RAJA_ENABLE_CUDA)
  cudaMallocManaged((void**)&ptr, sizeof(T)*size,cudaMemAttachGlobal);
#else
  ptr = new T[size];
#endif
  return ptr;
}

template <typename T>
void deallocate(T* &ptr) {
  if (ptr) {
#if defined(RAJA_ENABLE_CUDA)
    cudaFree(ptr);
#else
    delete[] ptr;
#endif
    ptr = nullptr;
  }
}

/*  
  Example 2: Multiplying Two Matrices
  
  ----[Details]--------------------
  Multiplies two N x N matrices
  
  -----[RAJA Concepts]-------------
  1. Introduces nesting of forall loops (Not currently supported in CUDA)
  2. Introduces forallN variant of the RAJA loop
*/
int main(int RAJA_UNUSED_ARG(argc), char** RAJA_UNUSED_ARG(argv[]))
{

  std::cout<<"Example 2: Multiplying Two Matrices"<<std::endl;

  const int N  = 1000; 
  const int NN = N*N;

  double *A = allocate<double>(NN);
  double *B = allocate<double>(NN);
  double *C = allocate<double>(NN);

  for(int i=0; i<NN; ++i) {
    A[i] = 1.0;
    B[i] = 1.0;
  }


  std::cout<<"Standard C++ Loop"<<std::endl;
  for(int r=0; r<N; ++r) {
    for(int c=0; c<N; ++c) {

      int cId = c+r*N; double dot=0.0;

      for(int k=0; k<N; ++k) {

        int aId = k + r*N; 
        int bId = c + k*N;
        dot += A[aId]*B[bId];
      }
      
      C[cId] = dot;
    }        
  }
  checkSolution(C,N);
  std::cout<<"\n"<<std::endl;
  //======================================


  std::cout<<"RAJA: Sequential Policy - Single forall"<<std::endl;
  RAJA::forall<RAJA::seq_exec>(RAJA::RangeSegment(0,NN),[=](int i) {
      /*
        Generate column and row index
      */
      int c = i%N; int r=i/N;
      
      int cId = c+r*N; double dot=0.0;
      
      for(int k=0; k<N; ++k) {
        
        int aId = k + r*N;
        int bId = c + k*N;
        dot += A[aId]*B[bId];
      }    
      
      C[cId] = dot;
    });
  checkSolution(C,N);
  std::cout<<"\n"<<std::endl;
  //======================================

  
  std::cout<<"RAJA: Sequential Policy - Nested forall"<<std::endl;
  /*
    Forall loops may be nested under sequential and omp policies
  */
  RAJA::forall<RAJA::seq_exec>(RAJA::RangeSegment(0,N),[=](int r) {
      RAJA::forall<RAJA::seq_exec>(RAJA::RangeSegment(0,N),[=](int c) {
      
          int cId = c+r*N; double dot=0.0;

          for(int k=0; k<N; ++k) {
            
            int aId = k + r*N;
            int bId = c + k*N;
            dot += A[aId]*B[bId];
          }    
          C[cId] = dot;

        });
    });
  checkSolution(C,N);
  std::cout<<"\n"<<std::endl;
  //======================================

  std::cout<<"RAJA: Sequential Policy - forallN"<<std::endl;
  /*    
    Nested forall loops may be collapsed into a single forallN loop
  */
  RAJA::forallN< RAJA::NestedPolicy<
  RAJA::ExecList<RAJA::seq_exec,RAJA::seq_exec >>>(
  RAJA::RangeSegment(0, N),
  RAJA::RangeSegment(0, N),[=](int r, int c) {

    int cId = c+r*N; double dot=0.0;

    for(int k=0; k<N; ++k) {

      int aId = k + r*N;
      int bId = c + k*N;
      dot += A[aId]*B[bId];
    }    

    C[cId] = dot;
  });
  checkSolution(C,N);
  std::cout<<"\n"<<std::endl;
  //===========================================

#if defined(RAJA_ENABLE_OPENMP)
  std::cout<<"RAJA: OpenMP/Sequential Policy - forallN"<<std::endl;
  /*
    Here the outer loop is excuted in parallel while the inner loop 
    is executed sequentially
  */
  RAJA::forallN< RAJA::NestedPolicy<
  RAJA::ExecList<RAJA::omp_parallel_for_exec,RAJA::seq_exec >>>(
  RAJA::RangeSegment(0, N),
  RAJA::RangeSegment(0, N),[=](int r, int c) {

    int cId = c+r*N; double dot=0.0;

    for(int k=0; k<N; ++k) {

      int aId = k + r*N;
      int bId = c + k*N;
      dot += A[aId]*B[bId];
    }    
    C[cId] = dot;
  });
  checkSolution(C,N);
  std::cout<<"\n"<<std::endl;
  //===========================================
#endif

#if defined(RAJA_ENABLE_CUDA)  
  std::cout<<"RAJA: CUDA Policy - forallN"<<std::endl;
  /*
    This example illustrates creating two-dimensional thread blocks as described 
    under the CUDA nomenclature    
  */
  RAJA::forallN<RAJA::NestedPolicy<
  RAJA::ExecList<
  RAJA::cuda_threadblock_y_exec<16>,
  RAJA::cuda_threadblock_x_exec<16> >>>(
  RAJA::RangeSegment(0, N), RAJA::RangeSegment(0, N), [=] __device__ (int c, int r) {
                                           
    int cId = c+r*N; double dot=0.0;

    for(int k=0; k<N; ++k) {

      int aId = k + r*N;
      int bId = c + k*N;
      dot += A[aId]*B[bId];
    }    
    C[cId] = dot;
    
  });
  cudaDeviceSynchronize();
  checkSolution(C,N);
  std::cout<<"\n"<<std::endl;
#endif

  deallocate(A);
  deallocate(B);
  deallocate(C);
  
  return 0;
}

void checkSolution(double *C, int in_N) {

  bool eFlag = true;
  for(int id=0; id<in_N*in_N; ++id) {
    if( abs(C[id]-in_N) > 1e-9) {
      std::cout<<"Error in Result!"<<std::endl;
      eFlag = false;
      break;
    }
  }
  
  if(eFlag) {
    std::cout<<"Result is correct"<<std::endl;
  }

}
