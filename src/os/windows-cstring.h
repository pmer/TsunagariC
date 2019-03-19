/********************************
** Tsunagari Tile Engine       **
** os/windows-cstring.h        **
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

#ifndef SRC_OS_WINDOWS_CSTRING_H_
#define SRC_OS_WINDOWS_CSTRING_H_

#include "util/int.h"

__pragma(pack(push, 8));
extern "C" {
void* __cdecl memchr(const void* buf, int chr, size_t cnt);
int __cdecl memcmp(void const* dst, void const* src, size_t size);
size_t __cdecl strlen(char const* _Str);
}
__pragma(pack(pop));

void* memmem(const void* haystack,
             size_t h_sz,
             const void* needle,
             size_t n_sz);

#endif  // SRC_OS_WINDOWS_CSTRING_H_
