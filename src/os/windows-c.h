/********************************
** Tsunagari Tile Engine       **
** os/windows-c.h              **
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

#ifndef SRC_OS_WINDOWS_C_H_
#define SRC_OS_WINDOWS_C_H_

#include "util/int.h"
#include "util/noexcept.h"

#define VOID void

typedef int BOOL;
typedef unsigned char BOOLEAN;
typedef unsigned char BYTE;
typedef char CHAR;
typedef unsigned long* DWORD_PTR;
typedef unsigned long DWORD;
typedef void* HANDLE;
typedef void* HWND;
typedef DWORD* LPDWORD;
typedef const char* LPCSTR;
typedef const void* LPCVOID;
typedef void* LPVOID;
typedef void* PVOID;
typedef size_t SIZE_T, *PSIZE_T;
typedef unsigned int UINT;
typedef unsigned long ULONG;
typedef unsigned short WORD;

typedef struct {
    long long QuadPart;
} LARGE_INTEGER;

#if defined(_WIN64)
typedef long long INT_PTR, *PINT_PTR;
typedef unsigned long long UINT_PTR, *PUINT_PTR;
typedef long long LONG_PTR, *PLONG_PTR;
typedef unsigned long long ULONG_PTR, *PULONG_PTR;
#else
typedef int INT_PTR, *PINT_PTR;
typedef unsigned int UINT_PTR, *PUINT_PTR;
typedef long LONG_PTR, *PLONG_PTR;
typedef unsigned long ULONG_PTR, *PULONG_PTR;
#endif

#define __CRTDECL __cdecl
#define _ACRTIMP __declspec(dllimport)
#define _ACRTIMP_ALT __declspec(dllimport)
#define _CRTRESTRICT __declspec(restrict)
#define WINAPI __stdcall
#define WINBASEAPI __declspec(dllimport)
#define WINUSERAPI __declspec(dllimport)

__pragma(pack(push, 8));
extern "C" {

typedef struct _iobuf {
    void* _Placeholder;
} FILE;

_ACRTIMP_ALT FILE* __cdecl __acrt_iob_func(unsigned) noexcept;

#define stdin (__acrt_iob_func(0))
#define stdout (__acrt_iob_func(1))
#define stderr (__acrt_iob_func(2))

typedef __int64 __time64_t;

_ACRTIMP __time64_t __cdecl _time64(__time64_t*) noexcept;
_ACRTIMP __declspec(noreturn) void __cdecl exit(int) noexcept;
_ACRTIMP void __cdecl free(void*) noexcept;
_ACRTIMP _CRTRESTRICT void* __cdecl malloc(size_t) noexcept;
_ACRTIMP int __cdecl rand() noexcept;
_ACRTIMP void __cdecl srand(unsigned int) noexcept;

void* __cdecl memchr(const void*, int, size_t) noexcept;
int __cdecl memcmp(void const*, void const*, size_t) noexcept;
void* __cdecl memcpy(void*, void const*, size_t) noexcept;
size_t __cdecl strlen(char const*) noexcept;

int __cdecl abs(int) noexcept;
double __cdecl atan2(double, double) noexcept;
_ACRTIMP double __cdecl ceil(double) noexcept;
#if defined _M_X64
_ACRTIMP float __cdecl ceilf(float) noexcept;
#else
__inline float __CRTDECL
ceilf(float _X) noexcept {
    return (float)ceil(_X);
}
#endif
double __cdecl cos(double) noexcept;
_ACRTIMP double __cdecl floor(double) noexcept;
double __cdecl sin(double) noexcept;
double __cdecl sqrt(double) noexcept;

_ACRTIMP double __cdecl strtod(char const*,
                               char**) noexcept;
_ACRTIMP long __cdecl strtol(char const*,
                             char**,
                             int) noexcept;
_ACRTIMP unsigned long __cdecl strtoul(char const*,
                                       char**,
                                       int) noexcept;
_ACRTIMP int __cdecl atoi(char const*) noexcept;

_ACRTIMP int* __cdecl _errno();
#define errno (*_errno())
}
__pragma(pack(pop));

extern "C" {
void* memmem(const void*,
             size_t,
             const void*,
             size_t) noexcept;
}

static inline time_t __CRTDECL
time(time_t* _Time) noexcept {
    return _time64(reinterpret_cast<__time64_t*>(_Time));
}

int __CRTDECL fprintf(FILE* const,
                      char const* const,
                      ...) noexcept;
int __CRTDECL printf(char const* const, ...) noexcept;
int __CRTDECL sprintf(char* const,
                      char const* const,
                      ...) noexcept;

#endif  // SRC_OS_WINDOWS_C_H_
