/********************************
** Tsunagari Tile Engine       **
** os/windows-stdio.h          **
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

#ifndef SRC_OS_WINDOWS_CSTDIO_H_
#define SRC_OS_WINDOWS_CSTDIO_H_

#include "os/windows-types.h"

__pragma(pack(push, 8)) extern "C" {
    typedef struct _iobuf {
        void* _Placeholder;
    } FILE;

    _ACRTIMP_ALT FILE* __cdecl __acrt_iob_func(unsigned _Ix);
    int __CRTDECL fprintf(FILE* const _Stream, char const* const _Format, ...);
    int __CRTDECL printf(char const* const _Format, ...);
    int __CRTDECL sprintf(char* const _Buffer, char const* const _Format, ...);
}
__pragma(pack(pop))

#define stdin (__acrt_iob_func(0))
#define stdout (__acrt_iob_func(1))
#define stderr (__acrt_iob_func(2))

#endif  // SRC_OS_WINDOWS_CSTDIO_H_
