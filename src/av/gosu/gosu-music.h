/***************************************
** Tsunagari Tile Engine              **
** gosu-music.h                       **
** Copyright 2011-2014 PariahSoft LLC **
** Copyright 2016      Paul Merrill   **
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

#ifndef SRC_AV_GOSU_GOSU_MUSIC_H_
#define SRC_AV_GOSU_GOSU_MUSIC_H_

#include <memory>
#include <string>

#include "cache/cache-template.cpp"
#include "cache/readercache.h"
#include "core/music-worker.h"

namespace Gosu {
    class Song;
}

class GosuMusic : public MusicWorker {
 public:
    GosuMusic();
    ~GosuMusic();

    void play(std::string filename);

    void stop();

    bool playing();
    void pause();
    void resume();

    void setVolume(double volume);

    void garbageCollect();

 private:
    std::shared_ptr<MixMusic> musicInst;

    ReaderCache<std::shared_ptr<Gosu::Song>> songs;
};

#endif  // SRC_AV_GOSU_GOSU_MUSIC_H_
