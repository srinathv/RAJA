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
#include "memoryManager.hpp"

#include "RAJA/RAJA.hpp"
#include "RAJA/util/defines.hpp"

const int sr = 2;  // Stencil Radius
const double PI = 3.14159265359;

struct grid_s {
  double ox, dx;
  int nx;
};


/*
  Wave Propagator
*/
template <typename T, typename fdNestedPolicy>
void Wave(T *P1, T *P2, RAJA::RangeSegment fdBounds, double ct, int nx)
{
  RAJA::forallN<fdNestedPolicy>
    (fdBounds, fdBounds, [=] RAJA_HOST_DEVICE(RAJA::Index_type ty, RAJA::Index_type tx) {  
      
      /*
        Coefficients for a fourth order stencil
      */
      double coeff[5] = {
        -1.0 / 12.0, 4.0 / 3.0, -5.0 / 2.0, 4.0 / 3.0, -1.0 / 12.0};
      
      const int id = tx + ty * nx;
      double P_old = P1[id];
      double P_curr = P2[id];

      /*
        Compute Laplacian
       */
      double lap = 0.0;
      
      for (auto r : RAJA::RangeSegment(-sr, sr + 1)) {
        const int xi = (tx + r + nx) % nx;
        const int idx = xi + nx * ty;
        lap += coeff[r + sr] * P2[idx];
        
        const int yi = (ty + r + nx) % nx;
        const int idy = tx + nx * yi;
        lap += coeff[r + sr] * P2[idy];
      }
      
      P1[id] = 2 * P_curr - P_old + ct * lap;
      
    });
}

double wave_sol(double t, double x, double y);
void set_ic(double *P1, double *P2, double t0, double t1, grid_s grid);
void compute_err(double *P, double tf, grid_s grid);

/*
  Example 4: Solver for the acoustic wave eqation

             P_tt = cc(P_xx + P_yy)

  ------[Details]----------------------
  Scheme uses a second order central diffrence discretization for time and a
  fourth order central discretization for space. Periodic boundary conditions
  are assumed.
  NOTE: The x and y dimensions are discretized identically.

  ----[RAJA Concepts]-------------------
  1. RAJA kernels are portable and a single implemenation can run 
  on various platforms

  2. RAJA MaxReduction - RAJA's implementation for computing a maximum value
  (MinReduction computes the min)

*/
int main(int RAJA_UNUSED_ARG(argc), char **RAJA_UNUSED_ARG(argv[]))
{

  printf("Example 4. Time-Domain Finite Difference Solver For The Acoustic Wave Equation \n");

  /*
    Wave speed squared
   */
  double cc = 1. / 2.0;

  /*
    Multiplier for spatial refinement
   */
  int factor = 8;

  /*
    Discretization of the domain.
    The same discretization of the x-dimension wil be used for the y-dimension
  */
  grid_s grid;
  grid.ox = -1;
  grid.dx = 0.1250 / factor;
  grid.nx = 16 * factor;
  RAJA::RangeSegment fdBounds(0, grid.nx);

  /*
    Propagate the solution until time T
   */
  double T = 0.82;


  int entries = grid.nx * grid.nx;
  double *P1 = memoryManager::allocate<double>(entries);
  double *P2 = memoryManager::allocate<double>(entries);

  /*
    Time stepping parameters
   */
  double dt, nt, time, ct;
  dt = 0.01 * (grid.dx / sqrt(cc));  // Intial step size
  nt = ceil(T / dt);                 // Total number of time steps
  dt = T / nt;                       // Final step size
  ct = (cc * dt * dt)
    / (grid.dx * grid.dx);          // Merge coefficients into a single coefficient


  /*
    Predefined Nested Policies
  */

  // Sequential
  //using fdPolicy = 
  //RAJA::NestedPolicy<RAJA::ExecList
  //<RAJA::seq_exec, RAJA::seq_exec > >;
    
  // OpenMP
  using fdPolicy =
    RAJA::NestedPolicy<RAJA::ExecList
    <RAJA::omp_collapse_nowait_exec,
    RAJA::omp_collapse_nowait_exec>,
    RAJA::OMP_Parallel<>>;

  // CUDA
  //using fdPolicy
  //= RAJA::NestedPolicy<RAJA::ExecList
  //<RAJA::cuda_threadblock_y_exec<16>,        
  //RAJA::cuda_threadblock_x_exec<16>>>;

  time = 0;
  set_ic(P1, P2, (time - dt), time, grid);
  for (int k = 0; k < nt; ++k) {

    Wave<double, fdPolicy>(P1, P2, fdBounds, ct, grid.nx);
    time += dt;

    double *Temp = P2;
    P2 = P1;
    P1 = Temp;
  }
#if defined(RAJA_ENABLE_CUDA)
  cudaDeviceSynchronize();
#endif
  compute_err(P2, time, grid);
  printf("Evolved solution to time = %f \n",time); 

  memoryManager::deallocate(P1);
  memoryManager::deallocate(P2);

  return 0;
}


/*
  Analytic Solution
  P(t,x,y) = Cos(2*PI*t)*Sin(2*PI*x)*Sin(2*PI*y)
*/
double wave_sol(double t, double x, double y)
{
  return cos(2. * PI * t) * sin(2. * PI * x) * sin(2. * PI * y);
}

/*
  Error is computed via ||P_{analytic}(:) - P_{approx}(:)||_{inf} 
*/
void compute_err(double *P, double tf, grid_s grid)
{

  RAJA::RangeSegment fdBounds(0, grid.nx);
  RAJA::ReduceMax<RAJA::seq_reduce, double> tMax(-1.0);
  using myPolicy =
    RAJA::NestedPolicy<
    RAJA::ExecList<RAJA::seq_exec, RAJA::seq_exec>>;

  RAJA::forallN<myPolicy>
    (fdBounds, fdBounds, [=](RAJA::Index_type ty, RAJA::Index_type tx) {          
      
      int id = tx + grid.nx * ty;
      double x = grid.ox + tx * grid.dx;
      double y = grid.ox + ty * grid.dx;
      double myErr = abs(P[id] - wave_sol(tf, x, y));
      
      tMax.max(myErr);  // Keeps track of the maximum value
    });
  
  double l2err = tMax;  
  printf("Max err=%lg, dx=%f \n",l2err,grid.dx);
}


/*
  Set intial condition
*/
void set_ic(double *P1, double *P2, double t0, double t1, grid_s grid)
{

  using myPolicy =
    RAJA::NestedPolicy<RAJA::ExecList<RAJA::seq_exec, RAJA::seq_exec>>;
  RAJA::RangeSegment fdBounds(0, grid.nx);
  
  RAJA::forallN<myPolicy>
    (fdBounds, fdBounds, [=](RAJA::Index_type ty, RAJA::Index_type tx) {
      
      int id = tx + ty * grid.nx;
      double x = grid.ox + tx * grid.dx;
      double y = grid.ox + ty * grid.dx;
      
      P1[id] = wave_sol(t0, x, y);
      P2[id] = wave_sol(t1, x, y);
    });
}