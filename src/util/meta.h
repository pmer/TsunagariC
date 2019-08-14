/*************************************
** Tsunagari Tile Engine            **
** meta.h                           **
** Copyright 2017-2019 Paul Merrill **
*************************************/

// **********
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// **********

#ifndef SRC_UTIL_META_H_
#define SRC_UTIL_META_H_

#include "util/constexpr.h"

//
// template struct If
//
template<bool Cond, typename WhenTrue, typename WhenFalse> struct If_;

template<typename WhenTrue, typename WhenFalse>
struct If_<true, WhenTrue, WhenFalse> {
    typedef WhenTrue value;
};

template<typename WhenTrue, typename WhenFalse>
struct If_<false, WhenTrue, WhenFalse> {
    typedef WhenFalse value;
};

template<bool Cond, typename WhenTrue, typename WhenFalse>
using If = typename If_<Cond, WhenTrue, WhenFalse>::value;

//
// template struct EnableIf
//
struct Yes {};

template<bool Cond> struct EnableIf_;
template<> struct EnableIf_<true> { typedef Yes value; };

template<bool Cond> using EnableIf = typename EnableIf_<Cond>::value;

//
// template bool IsUnit
//
struct Unit {};

template<typename T> CONSTEXPR bool IsUnit = false;
template<> CONSTEXPR bool IsUnit<Unit> = true;

#endif  // SRC_UTIL_META_H_
