/*!
 ******************************************************************************
 *
 * \file
 *
 * \brief   RAJA header file for type helpers used in tests.
 *
 ******************************************************************************
 */

#ifndef _TYPE_HELPER_HPP_
#define _TYPE_HELPER_HPP_

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

#include "gtest/gtest.h"

#include <tuple>

namespace types
{


template <typename S, typename T>
struct type_cat;

template <typename... Ss, typename... Ts>
struct type_cat<std::tuple<Ss...>, std::tuple<Ts...>> {
  typedef std::tuple<Ss..., Ts...> type;
};


template <typename S, typename T>
struct product;

template <typename S, typename... Ss, typename... Ts>
struct product<std::tuple<S, Ss...>, std::tuple<Ts...>> {
  // the cartesian product of {S} and {Ts...}
  // is a list of pairs -- here: a std::tuple of 2-element std::tuples
  typedef std::tuple<std::tuple<S, Ts>...> S_cross_Ts;

  // the cartesian product of {Ss...} and {Ts...} (computed recursively)
  typedef
      typename product<std::tuple<Ss...>, std::tuple<Ts...>>::type Ss_cross_Ts;

  // concatenate both products
  typedef typename type_cat<S_cross_Ts, Ss_cross_Ts>::type type;
};

template <typename... Ss, typename... Ts, typename... Smembers>
struct product<std::tuple<std::tuple<Smembers...>, Ss...>, std::tuple<Ts...>> {
  // the cartesian product of {S} and {Ts...}
  // is a list of pairs -- here: a std::tuple of 2-element std::tuples
  typedef std::tuple<std::tuple<Smembers..., Ts>...> S_cross_Ts;

  // the cartesian product of {Ss...} and {Ts...} (computed recursively)
  typedef
      typename product<std::tuple<Ss...>, std::tuple<Ts...>>::type Ss_cross_Ts;

  // concatenate both products
  typedef typename type_cat<S_cross_Ts, Ss_cross_Ts>::type type;
};

// end the recursion
template <typename... Ts>
struct product<std::tuple<>, std::tuple<Ts...>> {
  typedef std::tuple<> type;
};
}


namespace tt
{
template <typename...>
struct concat;

template <template <class...> class T, typename U>
struct concat<T<U>> {
  using type = U;
};

template <typename T>
struct concat<T> {
  using type = T;
};

template <template <class...> class T,
          class... Front,
          class... Next,
          class... Rest>
struct concat<T<Front...>, T<Next...>, Rest...> {
  using type = typename concat<T<Front..., Next...>, Rest...>::type;
};

template <typename... Ts>
using concat_t = typename concat<Ts...>::type;

template <class T>
struct collapse {
  using type = T;
};

template <template <class...> class T, class... U>
struct collapse<T<T<U...>>> {
  using type = typename collapse<T<U...>>::type;
};

template <typename T>
using collapse_t = typename collapse<T>::type;

template <template <class> class, class>
struct apply;

template <template <class...> class L, template <class> class Fn, class... Ts>
struct apply<Fn, L<Ts...>> {
  using type = collapse_t<L<concat_t<Fn<Ts>...>>>;
};

template <template <class> class Outer, class T>
using apply_t = typename apply<Outer, T>::type;

} // closing brace for namespace tt


namespace detail
{
template <typename T>
struct ForTesting;

template <template <class...> class T, typename... Ts>
struct ForTesting<T<Ts...>> {
  using type = ::testing::Types<Ts...>;
};
}

template <typename T>
using ForTesting = typename ::detail::ForTesting<T>::type;

#endif
