/*************************************
** Tsunagari Tile Engine            **
** sounds.h                         **
** Copyright 2016-2019 Paul Merrill **
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

#ifndef SRC_AV_SDL2_SOUNDS_H_
#define SRC_AV_SDL2_SOUNDS_H_

#include "av/sdl2/sdl2.h"
#include "cache/cache-template.h"
#include "cache/readercache.h"
#include "core/resources.h"
#include "core/sounds.h"

void SDL2OpenAudio() noexcept;

struct SDL2Sample {
    ~SDL2Sample() noexcept;

    // The Mix_Chunk needs the music data to be kept around for its lifetime.
    Optional<StringView> resource;

    Mix_Chunk* chunk;
};

class SDL2SoundInstance : public SoundInstance {
 public:
    SDL2SoundInstance(int channel) noexcept;

    bool playing() noexcept;
    void stop() noexcept;

    bool paused() noexcept;
    void pause() noexcept;
    void resume() noexcept;

    void volume(double volume) noexcept;
    void pan(double pan) noexcept;
    void speed(double speed) noexcept;

    void setDone() noexcept;

 private:
    int channel;
    enum { S_PLAYING, S_PAUSED, S_DONE } state;
};


Rc<SDL2Sample> genSample(StringView name) noexcept;

class SDL2Sounds : public Sounds {
 public:
    static SDL2Sounds& instance() noexcept;

    SDL2Sounds() noexcept;

    Rc<SoundInstance> play(StringView path) noexcept;

    void garbageCollect() noexcept;

    void setDone(int channel) noexcept;

 private:
    SDL2Sounds(const SDL2Sounds&) = delete;
    SDL2Sounds& operator=(const SDL2Sounds&) = delete;

    ReaderCache<Rc<SDL2Sample>, genSample> samples;
    Vector<Rc<SoundInstance>> channels;
};

#endif  // SRC_AV_SDL2_SOUNDS_H_
