/***************************************
** Tsunagari Tile Engine              **
** gosu-music.cpp                     **
** Copyright 2011-2014 PariahSoft LLC **
** Copyright 2015-2016 Paul Merrill   **
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

#include "av/gosu/gosu-music.h"

#include <Gosu/Audio.hpp>

#include "core/measure.h"
#include "core/resources.h"
#include "util/move.h"
#include "util/unique.h"

#include "av/gosu/gosu-cbuffer.h"

static GosuMusic globalMusicWorker;

MusicWorker& MusicWorker::instance() {
    return globalMusicWorker;
}

static Rc<Gosu::Song> genSong(const std::string& name) {
    Unique<Resource> r = Resources::instance().load(name);
    if (!r) {
        // Error logged.
        return Rc<Gosu::Song>();
    }
    GosuCBuffer buffer(r->data(), r->size());

    TimeMeasure m("Constructed " + name + " as music");
    return Rc<Gosu::Song>(new Gosu::Song(buffer.front_reader()));
}


GosuMusic::GosuMusic() : songs(genSong) {}

GosuMusic::~GosuMusic() {
    stop();
}

void GosuMusic::play(std::string filepath) {
    if (path == filepath) {
        if (musicInst->paused()) {
            paused = 0;
            musicInst->play(true);
        }
        return;
    }

    MusicWorker::play(move_(filepath));
    TimeMeasure m("Playing " + path);
    if (musicInst && musicInst->playing()) {
        musicInst->stop();
    }
    musicInst = path.size() ? songs.lifetimeRequest(path)
                            : Rc<Gosu::Song>();
    musicInst->play(true);
    musicInst->change_volume(volume);
}

bool GosuMusic::playing() {
    return musicInst && musicInst->playing();
}

void GosuMusic::stop() {
    MusicWorker::stop();
    if (musicInst) {
        musicInst->stop();
    }
    musicInst = Rc<Gosu::Song>();
}

void GosuMusic::pause() {
    if (paused == 0 && musicInst) {
        musicInst->pause();
    }
    MusicWorker::pause();
}

void GosuMusic::resume() {
    MusicWorker::resume();
    if (paused == 0 && musicInst) {
        musicInst->play();
    }
}

void GosuMusic::setVolume(double level) {
    MusicWorker::setVolume(level);
    if (musicInst) {
        musicInst->change_volume(volume);
    }
}

void GosuMusic::garbageCollect() {
    songs.garbageCollect();
}
