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
#include "util/move.h"
#include "util/random.h"

void
DataArea::onLoad() noexcept {}

void
DataArea::onFocus() noexcept {}

void
DataArea::onTick(time_t) noexcept {}

void
DataArea::onTurn() noexcept {}

void
DataArea::tick(time_t dt) noexcept {
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
DataArea::turn() noexcept {
    onTurn();
}

void
DataArea::playSoundEffect(StringView sound) noexcept {
    SoundID sid = Sounds::load(sound);
    PlayingSoundID psid = Sound::play(sid);
    PlayingSound::speed(psid, 1.0 + randFloat(-0.03, 0.03));
    PlayingSound::release(psid);
    Sound::release(sid);
}

void
DataArea::add(Unique<InProgress> inProgress) noexcept {
    inProgresses.push_back(move_(inProgress));
}
