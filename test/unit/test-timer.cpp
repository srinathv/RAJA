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
// For additional details, please also read RAJA/README.
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

///
/// Source file containing tests for basic timer operation
///

#include "gtest/gtest.h"

#include "RAJA/util/Timer.hpp"

#include <iostream>
#include <sstream>
#include <string>

#include <chrono>
#include <thread>


TEST(TimerTest, No1)
{
  RAJA::Timer timer;

  timer.start("test_timer");

  {
      std::stringstream sink;
      sink << "Printing 1000 stars...\n";
      for (int i = 0; i < 1000; ++i)
          sink << "*";
      sink << std::endl;
  }

  timer.stop();

  RAJA::Timer::ElapsedType elapsed = timer.elapsed();

  EXPECT_GT(elapsed, 0.0);

  // std::cout << "Printing 1000 stars took " << elapsed << " seconds.";
}


TEST(TimerTest, No2)
{
  RAJA::Timer timer;

  timer.start("test_timer");

  for (int i = 2; i > 0; --i) {
    std::cout << i << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  timer.stop();

  RAJA::Timer::ElapsedType elapsed = timer.elapsed();

  EXPECT_GT(elapsed, 0.02);
  EXPECT_LT(elapsed, 0.05);

}
