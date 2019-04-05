/**********************************
** Tsunagari Tile Engine         **
** unix-mutex.h                  **
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

#ifndef SRC_OS_UNIX_MUTEX_H_
#define SRC_OS_UNIX_MUTEX_H_

#include "util/assert.h"

extern "C" {
#define __PTHREAD_MUTEX_SIZE__ 56
#define _PTHREAD_MUTEX_SIG_init 0x32AAABA7
#define PTHREAD_MUTEX_INITIALIZER      \
    {                                  \
        _PTHREAD_MUTEX_SIG_init, { 0 } \
    }

struct pthread_mutex_t {
    long __sig;
    char __opaque[__PTHREAD_MUTEX_SIZE__];
};

int pthread_mutex_destroy(pthread_mutex_t*);
int pthread_mutex_lock(pthread_mutex_t*);
int pthread_mutex_unlock(pthread_mutex_t*);
}

class Mutex {
 public:
    constexpr Mutex() noexcept = default;
    inline ~Mutex() { pthread_mutex_destroy(&m); }

    inline void lock() { assert_(pthread_mutex_lock(&m) == 0); };
    inline void unlock() { assert_(pthread_mutex_unlock(&m) == 0); }

    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;

    pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
};

#endif  // SRC_OS_UNIX_MUTEX_H_
