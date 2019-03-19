/********************************
** Tsunagari Tile Engine       **
** os/windows-stdio.cpp        **
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

#include "os/windows-cstdio.h"

__pragma(pack(push, 8));
extern "C" {
typedef struct {
    struct __crt_locale_data* locinfo;
    struct __crt_multibyte_data* mbcinfo;
} * _locale_t;
typedef char* va_list;

_ACRTIMP int __cdecl __stdio_common_vfprintf(unsigned __int64 _Options,
                                             FILE* _Stream,
                                             char const* _Format,
                                             _locale_t _Locale,
                                             va_list _ArgList);
_ACRTIMP int __cdecl __stdio_common_vsprintf(unsigned __int64 _Options,
                                             char* _Buffer,
                                             size_t _BufferCount,
                                             char const* _Format,
                                             _locale_t _Locale,
                                             va_list _ArgList);

#ifdef _WIN64
#    define _VA_ALIGN 8
#    define _SLOTSIZEOF(t) ((sizeof(t) + _VA_ALIGN - 1) & ~(_VA_ALIGN - 1))
#    define _APALIGN(t, ap) (((va_list)0 - (ap)) & (__alignof(t) - 1))

void __cdecl __va_start(va_list*, ...);
#    define __crt_va_start_a(ap, v) \
        ((void)(__va_start(&ap, &v, _SLOTSIZEOF(v), __alignof(v), &v)))
#    define __crt_va_arg(ap, t)                                       \
        ((sizeof(t) > (2 * sizeof(__int64)))                          \
                 ? **(t**)((ap += sizeof(__int64)) - sizeof(__int64)) \
                 : *(t*)((ap += _SLOTSIZEOF(t) + _APALIGN(t, ap)) -   \
                         _SLOTSIZEOF(t)))
#    define __crt_va_end(ap) ((void)(ap = (va_list)0))
#else
#    define _INTSIZEOF(n) ((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))
#    define __crt_va_start_a(ap, v) ((void)(ap = (va_list)&v + _INTSIZEOF(v)))
#    define __crt_va_arg(ap, t) (*(t*)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)))
#    define __crt_va_end(ap) ((void)(ap = (va_list)0))
#endif
}
__pragma(pack(pop));

#define __crt_va_start(ap, x) __crt_va_start_a(ap, x)
#define _CRT_INTERNAL_PRINTF_LEGACY_VSPRINTF_NULL_TERMINATION (1ULL << 0)

__declspec(noinline) __inline unsigned __int64* __CRTDECL
        __local_stdio_printf_options(void) {
    static unsigned __int64 _OptionsStorage;
    return &_OptionsStorage;
}

inline int __CRTDECL
_vfprintf_l(FILE* const _Stream,
            char const* const _Format,
            _locale_t const _Locale,
            va_list _ArgList) {
    return __stdio_common_vfprintf(*__local_stdio_printf_options(),
                                   _Stream,
                                   _Format,
                                   _Locale,
                                   _ArgList);
}

inline int __CRTDECL
_vsnprintf_l(char* const _Buffer,
             size_t const _BufferCount,
             char const* const _Format,
             _locale_t const _Locale,
             va_list _ArgList) {
    int const _Result = __stdio_common_vsprintf(
            *__local_stdio_printf_options() |
                    _CRT_INTERNAL_PRINTF_LEGACY_VSPRINTF_NULL_TERMINATION,
            _Buffer,
            _BufferCount,
            _Format,
            _Locale,
            _ArgList);

    return _Result < 0 ? -1 : _Result;
}

inline int __CRTDECL
_vsprintf_l(char* const _Buffer,
            char const* const _Format,
            _locale_t const _Locale,
            va_list _ArgList) {
    return _vsnprintf_l(_Buffer, (size_t)-1, _Format, _Locale, _ArgList);
}

int __CRTDECL
fprintf(FILE* const _Stream, char const* const _Format, ...) {
    int _Result;
    va_list _ArgList;
    __crt_va_start(_ArgList, _Format);
    _Result = _vfprintf_l(_Stream, _Format, nullptr, _ArgList);
    __crt_va_end(_ArgList);
    return _Result;
}

int __CRTDECL
printf(char const* const _Format, ...) {
    int _Result;
    va_list _ArgList;
    __crt_va_start(_ArgList, _Format);
    _Result = _vfprintf_l(stdout, _Format, nullptr, _ArgList);
    __crt_va_end(_ArgList);
    return _Result;
}

int __CRTDECL
sprintf(char* const _Buffer, char const* const _Format, ...) {
    int _Result;
    va_list _ArgList;
    __crt_va_start(_ArgList, _Format);
    _Result = _vsprintf_l(_Buffer, _Format, nullptr, _ArgList);
    __crt_va_end(_ArgList);
    return _Result;
}
