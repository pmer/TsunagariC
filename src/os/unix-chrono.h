/********************************
** Tsunagari Tile Engine       **
** unix-chrono.h               **
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

#ifndef SRC_OS_UNIX_CHRONO_H_
#define SRC_OS_UNIX_CHRONO_H_

#include "util/assert.h"
#include "util/int.h"

extern "C" {
    struct timespec {
        time_t tv_sec;
        long tv_nsec;

    };
    enum clockid_t {
        CLOCK_UPTIME_RAW = 8,
    };
    int clock_gettime(clockid_t clock_id, struct timespec *tp);
    int nanosleep(const struct timespec *rqtp, struct timespec *rmtp);
    extern int * __error(void);
#define errno (*__error())
#define EINTR 4
}

typedef long long Duration;  // Nanoseconds.
typedef long long TimePoint;  // Nanoseconds.

constexpr Duration ns_to_ms(Duration d) { return d / 1000000; }
constexpr Duration ns_to_s(Duration d) { return d / 1000000000; }
constexpr Duration s_to_ns(Duration d) { return d * 1000000000; }
constexpr double ns_to_s_d(Duration d) { return d / 1000000000.0; }

class SteadyClock {
public:
    static TimePoint now() {
        struct timespec tp;
        assert_(clock_gettime(CLOCK_UPTIME_RAW, &tp) == 0);
        return TimePoint(s_to_ns(tp.tv_sec) + tp.tv_nsec);
    }
};

void SleepFor(Duration d) {
    if (d <= 0) {
        return;
    }
    long long s = ns_to_s(d);
    timespec ts;
    ts.tv_sec = static_cast<decltype(ts.tv_sec)>(s);
    ts.tv_nsec = static_cast<decltype(ts.tv_nsec)>(d - s_to_ns(s));
    while (nanosleep(&ts, &ts) == -1 && errno == EINTR);
}

#endif  // SRC_OS_UNIX_CHRONO_H_
