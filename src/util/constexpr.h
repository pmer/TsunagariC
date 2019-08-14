/********************************
** Tsunagari Tile Engine       **
** constexpr.h                 **
** Copyright 2019 Paul Merrill **
********************************/

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

#ifndef SRC_UTIL_CONSTEXPR_H_
#define SRC_UTIL_CONSTEXPR_H_

#if __cplusplus >= 201103L || \
        _MSC_VER >= 1900  // GCC and Clang on C++11 or higher, or Visual Studio
                          // 2015 or higher
#define CONSTEXPR11 constexpr
#else
#define CONSTEXPR11
#endif

#if __cplusplus >= 201402L || \
        _MSC_VER >= 1910  // GCC and Clang on C++14 or higher, or Visual Studio
                          // 2017 or higher
#define CONSTEXPR14 constexpr
#else
#define CONSTEXPR14
#endif

#endif  // SRC_UTIL_CONSTEXPR_H_
