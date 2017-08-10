/*************************************
** Tsunagari Tile Engine            **
** music.h                          **
** Copyright 2016-2017 Paul Merrill **
*************************************/

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

#ifndef SRC_AV_SDL2_MUSIC_H_
#define SRC_AV_SDL2_MUSIC_H_

#include <string>

#include "cache/cache-template.cpp"
#include "cache/readercache.h"
#include "core/music-worker.h"
#include "core/resources.h"
#include "util/rc.h"
#include "util/unique.h"

// This declaration matches exactly the one found in SDL_mixer.h so in
// music.cpp we can include both headers without the compiler complaining.
typedef struct _Mix_Music Mix_Music;

struct SDL2Song {
    // The Mix_Music needs the music data to be kept around for its lifetime.
    Unique<Resource> resource;

    Unique<Mix_Music> mix;
};

class SDL2Music : public MusicWorker {
 public:
    SDL2Music();
    ~SDL2Music();

    void play(std::string filename);

    void stop();

    bool playing();
    void pause();
    void resume();

    void setVolume(double volume);

    void garbageCollect();

 private:
    Rc<SDL2Song> currentMusic;

    ReaderCache<Rc<SDL2Song>> songs;
};

#endif  // SRC_AV_SDL2_MUSIC_H_
