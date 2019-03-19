/***************************************
** Tsunagari Tile Engine              **
** music-impl.cpp                     **
** Copyright 2011-2014 Michael Riley  **
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

#include "core/music-worker.h"

#include "core/client-conf.h"
#include "util/math2.h"

static void
clientIniVolumeVerify() {
    if (conf.musicVolume < 0 || 100 < conf.musicVolume) {
        Log::err("MusicWorker", "Music volume not within bounds [0,100]");
    }
}

static double
clientIniVolumeApply(double volume) {
    clientIniVolumeVerify();
    return volume * conf.musicVolume / 100.0;
}

static double
clientIniVolumeUnapply(double volume) {
    clientIniVolumeVerify();
    return volume / conf.musicVolume * 100.0;
}


MusicWorker::MusicWorker() : volume(1.0), paused(0) {}

void
MusicWorker::play(StringView filename) {
    paused = 0;
    path = filename;
}

void
MusicWorker::stop() {
    paused = 0;
    path = "";
}

void
MusicWorker::pause() {
    paused++;
}

void
MusicWorker::resume() {
    if (paused <= 0) {
        Log::err("MusicWorker", "Unpausing, but music not paused");
        return;
    }
    paused--;
}

double
MusicWorker::getVolume() {
    return clientIniVolumeUnapply(volume);
}

void
MusicWorker::setVolume(double attemptedVolume) {
    double newVolume = bound(attemptedVolume, 0.0, 1.0);
    if (attemptedVolume != newVolume) {
        Log::info("MusicWorker",
                  String() << "Attempted to set volume to " << attemptedVolume
                           << ", setting it to " << newVolume);
    }

    volume = clientIniVolumeApply(newVolume);
}
