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

#include <sysinfoapi.h>

class Thread {
 public:
    template<class Arg> inline explicit Thread(void (*f)(Arg), Arg arg) {
        static_assert(sizeof(Arg) == sizeof(void*), "");
        pthread_create(&t,
                       nullptr,
                       reinterpret_cast<void* (*)(void*)>(f),
                       static_cast<void*>(arg));
    }
    Thread(Thread&& other) : t(other.t) { other.t = 0; }
    inline ~Thread() noexcept { assert_(t == 0); }

    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;

    inline void join() noexcept {
        assert_(id != 0);
        assert_(WaitForSingleObjectEx(t, INFINITE, FALSE) != WAIT_FAILED);  // GetLastError();
        assert_(CloseHandle(*__t));  // GetLastError();
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
