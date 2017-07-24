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
#include <algorithm>
#include <iostream>
#include <initializer_list>

#include "RAJA/RAJA.hpp"
#include "RAJA/util/defines.hpp"
#include "RAJA/index/RangeSegment.hpp"


/*
  Example 5: Coloring
  Assuming a grid with the following contents
  
  grid = [1, 2, 1, 2,
          3, 4, 3, 4,
          1, 2, 1, 2,
          3, 4, 3, 4];

  This code illustrates how to create a forall loop which
  first iterates entries which hold a 1 value, then 2, etc...
  Here each number will will be treated as corresponding to a numeber.

  //--------[New Concepts]---------
  1. Constructing custom IndexSets
  2. RAJA::View
  3. RAJA::List_Segment

*/
int main(int RAJA_UNUSED_ARG(argc), char** RAJA_UNUSED_ARG(argv[]))
{

  std::cout<<"Example 5. Coloring "<<std::endl;

  int n         = 4;
  int *A        = new int[n*n];
  
  auto init = {1, 2, 1, 2, 
               3, 4, 3, 4, 
               1, 2, 1, 2,
               3, 4, 3, 4};
  
  std::copy(init.begin(), init.end(), A);
 

  //Defining a custom IndexSet
  RAJA::IndexSet colorset;

  //RAJA::View allows us introduce a two dimensional array
  RAJA::View<int,RAJA::Layout<2>> Aview(A, n, n);
  
  // buffer used for intermediate indicy storage
  RAJA::Index_type *idx = RAJA::allocate_aligned_type<RAJA::Index_type>(64, n * n / 4);


  // Iterate over each dimension (D=2 for this example)
  for (int xdim : {0, 1}) {
    for (int ydim : {0, 1}) {
      
      RAJA::Index_type count = 0;
      
      // Iterate over each extent in each dimension, incrementing by two to
      // safely advance over neighbors
      for (int xiter = xdim; xiter < n; xiter += 2) {
        for (int yiter = ydim; yiter < n; yiter += 2) {
          
          // Add the computed index to the buffer
          idx[count] = std::distance(A, std::addressof(Aview(xiter, yiter)));
          ++count;
        }
      }
      
      //RAJA::List segment - creates a list segment from a given array with a specific length.

      //Insert the indicies added from the buffer as a new ListSegment
      colorset.push_back(RAJA::ListSegment(idx, count));
    }
  }  

  // Clear temporary buffer
  RAJA::free_aligned(idx);

  
  //----[RAJA Sequantial Policy]---------
  //RAJA: Seq_segit - Sequational Segment Iteraion
  using ColorPolicy = RAJA::IndexSet::ExecPolicy<RAJA::seq_segit, RAJA::seq_exec>;  
  RAJA::forall<ColorPolicy>(colorset, [&](int idx) {
      printf("A[%d] = %d\n", idx, A[idx]);
    });  
  //==========================


  
  return 0;
}