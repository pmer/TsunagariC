/********************************
** Tsunagari Tile Engine       **
** os/windows-cstdlib.h        **
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

#ifndef SRC_OS_WINDOWS_CSTDLIB_H_
#define SRC_OS_WINDOWS_CSTDLIB_H_

#include "os/windows-types.h"
#include "util/int.h"

__pragma(pack(push, 8));
extern "C" {
typedef __int64 __time64_t;

_ACRTIMP __time64_t __cdecl _time64(__time64_t* _Time);
_ACRTIMP void __cdecl free(void* _Block);
_ACRTIMP _CRTRESTRICT void* __cdecl malloc(size_t _Size);
_ACRTIMP int __cdecl rand();
_ACRTIMP void __cdecl srand(unsigned int _Seed);
}
__pragma(pack(pop));

static inline time_t __CRTDECL
time(time_t* _Time) {
    return _time64(reinterpret_cast<__time64_t*>(_Time));
}

#endif  // SRC_OS_WINDOWS_CSTDLIB_H_
