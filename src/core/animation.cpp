/***************************************
** Tsunagari Tile Engine              **
** animation.cpp                      **
** Copyright 2011-2013 Michael Reiley **
** Copyright 2011-2016 Paul Merrill   **
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

#include "animation.h"

#include <assert.h>

Animation::Animation()
    : frameTime(1),
      cycleTime(1),
      frameShowing(0) {}

Animation::Animation(Arc<Image> frame)
    : frames { std::move(frame) },
      frameTime(1),
      cycleTime(1),
      frameShowing(0) {}

Animation::Animation(std::vector<Arc<Image>> frames,
        time_t frameTime)
    : frames(std::move(frames)),
      frameTime(frameTime),
      cycleTime(1),
      frameShowing(0),
      offset(0) {
    assert(frameTime > 0);

    cycleTime = frameTime * (time_t)this->frames.size();
}

void Animation::startOver(time_t now) {
    offset = now;
    frameShowing = 0;
}

bool Animation::needsRedraw(time_t now) const {
    if (frames.size() == 1) {
        return false;
    }
    time_t pos = now - offset;
    size_t frame = (size_t)((pos % cycleTime) / frameTime);
    return frame != frameShowing;
}

Image* Animation::frame(time_t now) {
    if (frames.size() == 0) {
        return nullptr;
    }
    if (frames.size() == 1) {
        return frames[0].get();
    }

    time_t pos = now - offset;
    frameShowing = (size_t)((pos % cycleTime) / frameTime);

    return frames[frameShowing].get();
}
