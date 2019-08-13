/***************************************
** Tsunagari Tile Engine              **
** music.cpp                          **
** Copyright 2011-2014 Michael Reiley **
** Copyright 2011-2019 Paul Merrill   **
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

#include "core/music.h"

#include "core/music-worker.h"
//#include "util/jobs.h"

void
Music::play(StringView name) noexcept {
    // String name_ = name;
    // JobsEnqueue([name_]() { MusicWorker::play(name_); });
    MusicWorker::play(name);
}

void
Music::stop() noexcept {
    // JobsEnqueue([]() { MusicWorker::stop(); });
    MusicWorker::stop();
}

void
Music::pause() noexcept {
    // JobsEnqueue([]() { MusicWorker::pause(); });
    MusicWorker::pause();
}

void
Music::resume() noexcept {
    // JobsEnqueue([]() { MusicWorker::resume(); });
    MusicWorker::resume();
}

void
Music::garbageCollect() noexcept {
    // JobsEnqueue([]() { MusicWorker::garbageCollect(); });
    MusicWorker::garbageCollect();
}
