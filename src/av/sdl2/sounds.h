/********************************
** Tsunagari Tile Engine       **
** sounds.h                    **
** Copyright 2016 Paul Merrill **
********************************/

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

#include "core/sounds.h"

class SDL2SoundInstance : public SoundInstance {
 public:
    bool playing();
    void stop();

    bool paused();
    void pause();
    void resume();

    void volume(double volume);
    void pan(double pan);
    void speed(double speed);
};


class SDL2Sounds : public Sounds {
 public:
    SDL2Sounds() = default;

    std::shared_ptr<SoundInstance> play(const std::string& path);

    void garbageCollect();

 private:
    SDL2Sounds(const SDL2Sounds&) = delete;
    SDL2Sounds& operator=(const SDL2Sounds&) = delete;
};

#endif  // SRC_AV_SDL2_SOUNDS_H_
