/***************************************
** Tsunagari Tile Engine              **
** data-area.cpp                      **
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

#include "data/data-area.h"

#include "core/algorithm.h"
#include "core/sounds.h"
#include "data/inprogress.h"
#include "util/random.h"

void
DataArea::onLoad() {}

void
DataArea::onFocus() {}

void
DataArea::onTick(time_t) {}

void
DataArea::onTurn() {}

void
DataArea::tick(time_t dt) {
    // Only iterate over inProgresses that existed at the time of the
    // beginning of the loop.  Also, iterate by index instead of by
    // iterator because iterators are invalidated if the vector is
    // pushed_back.
    for (size_t i = 0, len = inProgresses.size(); i < len; i++) {
        auto& inProgress = inProgresses[i];
        inProgress->tick(dt);
    }
    erase_if(inProgresses, [](Unique<InProgress>& ip) { return ip->isOver(); });
    onTick(dt);
}

void
DataArea::turn() {
    onTurn();
}

void
DataArea::playSoundEffect(StringView sound) {
    Sounds::instance().play(sound)->speed(1.0 + randFloat(-0.03, 0.03));
}

void
DataArea::playSoundAndThen(StringView sound, ThenFn then) {
    inProgresses.emplace_back(new InProgressSound(sound, then));
}

void
DataArea::timerProgress(time_t duration, ProgressFn progress) {
    inProgresses.emplace_back(new InProgressTimer(duration, progress));
}

void
DataArea::timerThen(time_t duration, ThenFn then) {
    inProgresses.emplace_back(new InProgressTimer(duration, then));
}

void
DataArea::timerProgressAndThen(time_t duration,
                               ProgressFn progress,
                               ThenFn then) {
    inProgresses.emplace_back(new InProgressTimer(duration, progress, then));
}

DataArea::TileScript
DataArea::script(StringView scriptName) {
    return scripts[scriptName];
}
