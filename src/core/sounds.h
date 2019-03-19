/***************************************
** Tsunagari Tile Engine              **
** sounds.h                           **
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

#ifndef SRC_CORE_SOUNDS_H_
#define SRC_CORE_SOUNDS_H_

#include "util/rc.h"
#include "util/string.h"

class SoundInstance {
 public:
    virtual ~SoundInstance() = default;

    //! Whether the sound is currently playing.
    virtual bool playing() = 0;
    //! Stop playing the sound.  SoundInstances cannot resume from stop().
    //! Create a new one to play again.
    virtual void stop() = 0;

    //! Whether the sound is paused.
    virtual bool paused() = 0;
    //! Pause playback of the sound.
    virtual void pause() = 0;
    //! Resume playback of the sound.
    virtual void resume() = 0;

    //! Between 0.0 (silence) and 1.0 (full).
    virtual void volume(double volume) = 0;
    //! Between -1.0 (left) and 1.0 (right).
    virtual void pan(double pan) = 0;
    //! 1.0 is normal speed
    virtual void speed(double speed) = 0;

 protected:
    SoundInstance() = default;

 private:
    SoundInstance(const SoundInstance&) = delete;
    SoundInstance(SoundInstance&&) = delete;
    SoundInstance& operator=(const SoundInstance&) = delete;
    SoundInstance& operator=(SoundInstance&&) = delete;
};


class Sounds {
 public:
    //! Acquire the global Sounds object.
    static Sounds& instance();

    virtual ~Sounds() = default;

    //! Play a sound from the file at the given path.
    virtual Rc<SoundInstance> play(StringView path) = 0;

    //! Free sounds not recently played.
    virtual void garbageCollect() = 0;

 protected:
    Sounds() = default;

 private:
    Sounds(const Sounds&) = delete;
    Sounds(Sounds&&) = delete;
    Sounds& operator=(const Sounds&) = delete;
    Sounds& operator=(Sounds&&) = delete;
};

#endif  // SRC_CORE_SOUNDS_H_
