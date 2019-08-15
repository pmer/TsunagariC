/***************************************
** Tsunagari Tile Engine              **
** inprogress-timer.h                 **
** Copyright 2014      Michael Reiley **
** Copyright 2014-2019 Paul Merrill   **
***************************************/

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

#ifndef SRC_DATA_INPROGRESS_TIMER_H_
#define SRC_DATA_INPROGRESS_TIMER_H_

#include "data/inprogress.h"

#include "util/function.h"
#include "util/int.h"
#include "util/noexcept.h"

/**
 * InProgressTimer can call a function every tick during its timer as well as
 * once its timer finishes.
 *
 * Its progress function gets called every tick with a number from 0.0 to 1.0
 * signifying the percentage of the duration that has passed.
 *
 * On the first tick where the timer has expired, progress is not called, but
 * the then function is.
 */
class InProgressTimer : public InProgress {
 public:
    typedef Function<void(float)> ProgressFn;
    typedef Function<void()> ThenFn;

    InProgressTimer(time_t duration, ThenFn then) noexcept;
    InProgressTimer(time_t duration, ProgressFn progress, ThenFn then) noexcept;

    void tick(time_t dt) noexcept;

 private:
    time_t duration, passed;
    ProgressFn progress;
    ThenFn then;
};

#endif  // SRC_DATA_INPROGRESS_TIMER_H_
