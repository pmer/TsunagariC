/**********************************
** Tsunagari Tile Engine         **
** new.h                         **
** Copyright 2019 Paul Merrill   **
**********************************/

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

#ifndef SRC_UTIL_NEW_H_
#define SRC_UTIL_NEW_H_

#include "util/int.h"

#ifndef _LIBCPP_NEW
inline void*
operator new(size_t, void* p) noexcept {
    return p;
}
#endif

__pragma(pack(push, 8)) extern "C" {
    __declspec(dllimport) __declspec(allocator) __declspec(
            restrict) void* __cdecl malloc(size_t size);
    __declspec(dllimport) void __cdecl free(void* block);
}
__pragma(pack(pop))

#endif  // SRC_UTIL_NEW_H_
