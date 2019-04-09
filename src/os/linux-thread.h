/**********************************
** Tsunagari Tile Engine         **
** linux-thread.h                **
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

#ifndef SRC_OS_LINUX_THREAD_H_
#define SRC_OS_LINUX_THREAD_H_

#include "os/c.h"
#include "os/thread.h"
#include "util/assert.h"
#include "util/function.h"
#include "util/int.h"
#include "util/move.h"

static void*
run(void* f) noexcept {
    Function<void()>* fun = reinterpret_cast<Function<void()>*>(f);
    (*fun)();
    return nullptr;
}

class Thread {
 public:
    inline explicit Thread(Function<void()> f) noexcept {
        Function<void()>* fun = new Function<void()>(move_(f));
        assert_(pthread_create(&t, nullptr, run, static_cast<void*>(fun)) == 0);
    }
    Thread(Thread&& other) noexcept : t(other.t) { other.t = 0; }
    inline ~Thread() noexcept { assert_(t == 0); }

    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;

    inline void join() noexcept {
        assert_(t != 0);
        assert_(pthread_join(t, 0) == 0);
        t = 0;
    }
    static inline unsigned hardware_concurrency() noexcept {
        long result = sysconf(_SC_NPROCESSORS_ONLN);
        if (result < 0) {
            return 0;
        }
        return static_cast<unsigned>(result);
    }

    pthread_t t = 0;
};

#endif  // SRC_OS_LINUX_THREAD_H_
