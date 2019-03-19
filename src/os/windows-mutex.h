/********************************
** Tsunagari Tile Engine       **
** windows-mutex.h             **
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

#ifndef SRC_OS_WINDOWS_MUTEX_H_
#define SRC_OS_WINDOWS_MUTEX_H_

#include "os/windows-types.h"

extern "C" {
typedef struct {
    PVOID Ptr;
} SRWLOCK, *PSRWLOCK;

#define SRWLOCK_INIT \
    { 0 }

WINBASEAPI VOID WINAPI AcquireSRWLockExclusive(PSRWLOCK SRWLock);
WINBASEAPI BOOLEAN WINAPI TryAcquireSRWLockExclusive(PSRWLOCK SRWLock);
WINBASEAPI VOID WINAPI ReleaseSRWLockExclusive(PSRWLOCK SRWLock);
}

class Mutex {
 public:
    constexpr Mutex() noexcept = default;

    inline void lock() { AcquireSRWLockExclusive(&m); }
    inline bool tryLock() { return TryAcquireSRWLockExclusive(&m) != 0; }
    inline void unlock() { ReleaseSRWLockExclusive(&m); }

    SRWLOCK m = SRWLOCK_INIT;

 private:
    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;
};

#endif  // SRC_OS_WINDOWS_MUTEX_H_
