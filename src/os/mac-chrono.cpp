/********************************
** Tsunagari Tile Engine       **
** mac-chrono.cpp              **
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

#include "os/c.h"
#include "os/chrono.h"
#include "util/assert.h"
#include "util/int.h"

/*
TimePoint SteadyClock::now() noexcept {
    struct timespec tp;

    int err = clock_gettime(CLOCK_UPTIME_RAW, &tp);
    (void)err;
    assert_(err == 0);

    return TimePoint(s_to_ns(tp.tv_sec) + tp.tv_nsec);
}

TimePoint SteadyClock::nowMS() noexcept {
    struct timespec tp;

    int err = clock_gettime(CLOCK_UPTIME_RAW, &tp);
    (void)err;
    assert_(err == 0);

    return TimePoint(s_to_ms(tp.tv_sec) + ns_to_ms(tp.tv_nsec));
}
*/

typedef int kern_return_t;

struct mach_timebase_info {
    uint32_t numer;
    uint32_t denom;
};

extern "C" {
uint64_t
mach_absolute_time() noexcept;

kern_return_t
mach_timebase_info_trap(mach_timebase_info*) noexcept;
}

#define KERN_SUCCESS 0

static mach_timebase_info timebase = {0, 0};

TimePoint SteadyClock::now() noexcept {
    if (timebase.numer == 0 && timebase.denom == 0) {
        kern_return_t err = mach_timebase_info_trap(&timebase);
        assert_(err == KERN_SUCCESS);
    }

    uint64_t machTime = mach_absolute_time();
    uint64_t ns = machTime * timebase.numer / timebase.denom;

    return TimePoint(ns);
}

TimePoint SteadyClock::nowMS() noexcept {
    if (timebase.numer == 0 && timebase.denom == 0) {
        kern_return_t err = mach_timebase_info_trap(&timebase);
        assert_(err == KERN_SUCCESS);
    }

    uint64_t machTime = mach_absolute_time();
    uint64_t ns = machTime * timebase.numer / timebase.denom;

    return TimePoint(ns_to_ms(ns));
}

void SleepFor(Duration d) noexcept {
    if (d <= 0) {
        return;
    }
    long long s = ns_to_s(d);
    timespec ts;
    ts.tv_sec = static_cast<time_t>(s);
    ts.tv_nsec = static_cast<long>(d - s_to_ns(s));
    while (nanosleep(&ts, &ts) == -1 && errno == EINTR);
}
