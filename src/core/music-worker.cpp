/***************************************
** Tsunagari Tile Engine              **
** music-impl.cpp                     **
** Copyright 2011-2014 PariahSoft LLC **
** Copyright 2016 Paul Merrill        **
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
#include "core/formatter.h"
#include "util/math2.h"

static void clientIniVolumeVerify() {
    if (conf.musicVolume < 0 || 100 < conf.musicVolume) {
        Log::err("MusicWorker", "Music volume not within bounds [0,100]");
    }
}

static double clientIniVolumeApply(double volume) {
    clientIniVolumeVerify();
    return volume * conf.musicVolume / 100.0;
}

static double clientIniVolumeUnapply(double volume) {
    clientIniVolumeVerify();
    return volume / conf.musicVolume * 100.0;
}


MusicWorker::MusicWorker()
    : state(NOT_PLAYING), volume(1.0), pausedCount(0) {}

void MusicWorker::setIntro(const std::string& filename) {
    if (newIntro == filename) {
        return;
    }

    switch (state) {
    case NOT_PLAYING:
    case PLAYING_INTRO:
    case PLAYING_LOOP:
        state = CHANGED_INTRO;
    default:
        break;
    }

    newIntro = filename;
}

void MusicWorker::setLoop(const std::string& filename) {
    if (newLoop == filename) {
        return;
    }

    switch (state) {
    case NOT_PLAYING:
    case PLAYING_INTRO:
    case PLAYING_LOOP:
        state = CHANGED_LOOP;
    default:
        break;
    }

    newLoop = filename;
}

void MusicWorker::pause() {
    pausedCount++;
}

void MusicWorker::resume() {
    if (pausedCount <= 0) {
        Log::err("MusicWorker", "unpausing, but music not paused");
        return;
    }
    pausedCount--;
}

double MusicWorker::getVolume() {
    return clientIniVolumeUnapply(volume);
}

void MusicWorker::setVolume(double attemptedVolume) {
    double newVolume = bound(attemptedVolume, 0.0, 1.0);
    if (attemptedVolume != newVolume) {
        Log::info("MusicWorker",
            Formatter(
                "Attempted to set volume to %, setting it to %"
            ) % attemptedVolume % newVolume
        );
    }

    volume = clientIniVolumeApply(newVolume);
}

void MusicWorker::stop() {
    state = NOT_PLAYING;
    pausedCount = 0;
    curIntro = newIntro = "";
    curLoop = newLoop = "";
}

void MusicWorker::playIntro() {
    curIntro = newIntro;
    state = PLAYING_INTRO;
    pausedCount = 0;
}

void MusicWorker::playLoop() {
    curLoop = newLoop;
    state = PLAYING_LOOP;
    pausedCount = 0;
}
