/********************************
** Tsunagari Tile Engine       **
** windows-thread.h            **
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

#ifndef SRC_OS_WINDOWS_THREAD_H_
#define SRC_OS_WINDOWS_THREAD_H_

#include "os/windows-types.h"
#include "util/function.h"
#include "util/int.h"

extern "C" {
typedef struct _SYSTEM_INFO {
    union {
        DWORD dwOemId;
        struct {
            WORD wProcessorArchitecture;
            WORD wReserved;
        } s;
    } u;
    DWORD dwPageSize;
    LPVOID lpMinimumApplicationAddress;
    LPVOID lpMaximumApplicationAddress;
    DWORD_PTR dwActiveProcessorMask;
    DWORD dwNumberOfProcessors;
    DWORD dwProcessorType;
    DWORD dwAllocationGranularity;
    WORD wProcessorLevel;
    WORD wProcessorRevision;
} SYSTEM_INFO, *LPSYSTEM_INFO;

WINBASEAPI BOOL WINAPI CloseHandle(HANDLE hObject);
WINBASEAPI VOID WINAPI GetSystemInfo(LPSYSTEM_INFO lpSystemInfo);
WINBASEAPI DWORD WINAPI WaitForSingleObjectEx(HANDLE hHandle,
                                              DWORD dwMilliseconds,
                                              BOOL bAlertable);
_ACRTIMP uintptr_t __cdecl _beginthreadex(
        void* security,
        unsigned stack_size,
        unsigned(WINAPI* start_address)(void*),
        void* arglist,
        unsigned initflag,
        unsigned* thrdaddr);

#define INFINITE 0xFFFFFFFF  // Infinite timeout.
constexpr DWORD WAIT_FAILED = 0xFFFFFFFF;
}

static unsigned WINAPI
beginthreadex_thunk(void* data) {
    auto f = static_cast<Function<void()>*>(data);
    (*f)();
    delete f;
    return 0;
}

class Thread {
 public:
    inline explicit Thread(Function<void()> f) {
        auto data = new Function<void()>(move_(f));
        id = reinterpret_cast<HANDLE>(_beginthreadex(nullptr,
                                                     0,
                                                     beginthreadex_thunk,
                                                     static_cast<void*>(data),
                                                     0,
                                                     nullptr));
        assert_(id);
    }
    Thread(Thread&& other) : id(other.id) { other.id = 0; }
    inline ~Thread() noexcept { assert_(id == 0); }

    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;

    inline void join() noexcept {
        assert_(id != 0);

        DWORD err = WaitForSingleObjectEx(id, INFINITE, false);
        (void)err;
        assert_(err != WAIT_FAILED);  // GetLastError();

        BOOL err2 = CloseHandle(id);
        (void)err;
        assert_(err2);  // GetLastError();

        id = 0;
    }

    static inline unsigned hardware_concurrency() noexcept {
        SYSTEM_INFO info;
        GetSystemInfo(&info);
        return info.dwNumberOfProcessors;
    }

    HANDLE id = 0;
};

#endif  // SRC_OS_WINDOWS_THREAD_H_
