/**********************************
** Tsunagari Tile Engine         **
** chrono.h                      **
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

#ifndef SRC_OS_CHRONO_H_
#define SRC_OS_CHRONO_H_

#include "util/assert.h"

typedef long long Duration;   // Nanoseconds.
typedef long long TimePoint;  // Nanoseconds.

constexpr Duration
ns_to_ms(Duration d) {
    return d / 1000000;
}
constexpr Duration
ns_to_s(Duration d) {
    return d / 1000000000;
}
constexpr Duration
s_to_ms(Duration d) {
    return d * 1000;
}
constexpr Duration
s_to_ns(Duration d) {
    return d * 1000000000;
}
constexpr double
ms_to_s_d(Duration d) {
    return d / 1000.0;
}
constexpr double
ns_to_s_d(Duration d) {
    return d / 1000000000.0;
}

class SteadyClock {
 public:
    static TimePoint now() noexcept;
    static TimePoint nowMS() noexcept;  // In milliseconds, not nanoseconds.
};

void SleepFor(Duration d) noexcept;

#endif  // SRC_OS_CHRONO_H_
