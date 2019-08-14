/*************************************
** Tsunagari Tile Engine            **
** assert.h                         **
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

#ifndef SRC_UTIL_ASSERT_H_
#define SRC_UTIL_ASSERT_H_

#include "util/likely.h"
#include "util/noexcept.h"

#ifndef NDEBUG
#if __cplusplus >= 201103L || _MSC_VER >= 1900  // GCC, Clang, and Visual Studio 2015 or higher
#define assert_(expr) \
    (likely(expr) ? (void)0 : assert__(__func__, __FILE__, __LINE__, #expr))
#elif defined(_MSC_VER)  // Visual Studio 2013 or lower
#define assert_(expr) \
    (likely(expr) ? (void)0 : assert__(__FUNCTION__, __FILE__, __LINE__, #expr))
#else
#error How should I find a function's name?
#endif


void assert__(const char* func,
              const char* file,
              int line,
              const char* expr) noexcept;
#else
#define assert_(expr)
#endif

#endif  // SRC_UTIL_ASSERT_H_
