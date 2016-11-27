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

#include "core/music.h"

#include "core/music-worker.h"
#include "util/dispatch-queue.h"

const DispatchQueue::QualityOfService WORKER_QOS = DispatchQueue::UTILITY;

class MusicImpl : public Music {
 public:
    void setIntro(const std::string& filepath);
    void setLoop(const std::string& filepath);
    void stop();
    void pause();
    void resume();
    void setVolume(double volume);
    void tick();
    void garbageCollect();

 private:
    DispatchQueue queue;
};

MusicImpl* globalMusic;

Music& Music::instance() {
    if (!globalMusic) {
        globalMusic = new MusicImpl;
    }
    return *globalMusic;
}

void MusicImpl::setIntro(const std::string& filename) {
    queue.async([filename]() {
        MusicWorker::instance().setIntro(filename);
    }, WORKER_QOS);
}

void MusicImpl::setLoop(const std::string& filename) {
    queue.async([filename]() {
        MusicWorker::instance().setLoop(filename);
    }, WORKER_QOS);
}

void MusicImpl::pause() {
    queue.async([]() {
        MusicWorker::instance().pause();
    }, WORKER_QOS);
}

void MusicImpl::resume() {
    queue.async([]() {
        MusicWorker::instance().resume();
    }, WORKER_QOS);
}

void MusicImpl::setVolume(double attemptedVolume) {
    queue.async([attemptedVolume]() {
        MusicWorker::instance().setVolume(attemptedVolume);
    }, WORKER_QOS);
}

void MusicImpl::stop() {
    queue.async([]() {
        MusicWorker::instance().stop();
    }, WORKER_QOS);
}

void MusicImpl::tick() {
    queue.async([]() {
        MusicWorker::instance().tick();
    }, WORKER_QOS);
}

void MusicImpl::garbageCollect() {
    queue.async([]() {
        MusicWorker::instance().garbageCollect();
    }, WORKER_QOS);
}
