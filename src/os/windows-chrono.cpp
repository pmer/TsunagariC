/********************************
** Tsunagari Tile Engine       **
** windows-chrono.cpp          **
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
#include "util/optional.h"

extern "C" {
WINBASEAPI
BOOL WINAPI QueryPerformanceFrequency(LARGE_INTEGER*);
WINBASEAPI
BOOL WINAPI QueryPerformanceCounter(LARGE_INTEGER*);
WINBASEAPI
VOID WINAPI Sleep(DWORD);
}

TimePoint
SteadyClock::now() noexcept {
    static Optional<LARGE_INTEGER> freq;

    if (!freq) {
        LARGE_INTEGER _freq;

        BOOL ok = QueryPerformanceFrequency(&_freq);
        (void)ok;
        assert_(ok);

        freq = _freq;
    }

    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);

    return TimePoint(s_to_ns(counter.QuadPart) / freq->QuadPart);
}

void
SleepFor(Duration d) noexcept {
    if (d <= 0) {
        return;
    }

    DWORD ms = static_cast<DWORD>((d + 999999) / 1000000);
    Sleep(ms);
}
