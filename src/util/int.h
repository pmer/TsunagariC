/********************************
** Tsunagari Tile Engine       **
** int.h                       **
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

#ifndef SRC_UTIL_INT_H_
#define SRC_UTIL_INT_H_

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

#ifdef _WIN32
typedef uint64_t time_t;
#else
typedef long time_t;
#endif

#ifdef _WIN32
typedef signed long long ssize_t;
#else
typedef unsigned long size_t;
typedef signed long ssize_t;
#endif

typedef size_t uintptr_t;

static constexpr int INT_MAX = 0x7fffffff;
static constexpr int UINT_MAX = 0xffffffff;
static constexpr uint32_t UINT32_MAX = 0xffffffff;

#if defined(__APPLE__)
static constexpr size_t SIZE_MAX = 0xffffffffffffffff;
#elif defined(__linux__)
static constexpr size_t SIZE_MAX = __SIZEOF_SIZE_T__;
#endif

static constexpr double DBL_MAX = 1.7976931348623158e+308;

#endif  // SRC_UTIL_INT_H_
