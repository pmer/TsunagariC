/***************************************
** Tsunagari Tile Engine              **
** music.h                            **
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

#ifndef SRC_CORE_MUSIC_H_
#define SRC_CORE_MUSIC_H_

#include "util/string.h"

/**
 * State manager for currently playing music. Continuously controls which music
 * will play. Immediately upon entering an Area, the currently playing music is
 * stopped and the new music started and will play in a loop.
 *
 * When switching to a new Area with the same music as the previous Area, the
 * music is left alone, if possible.
 *
 * When a new music is played, the pause state of the previous music is
 * dropped.
 */
class Music {
 public:
    //! Acquire the global Music object.
    static Music& instance() noexcept;

    virtual ~Music() = default;

    //! If the music filepath has changed, start playing it.
    virtual void play(String filepath) noexcept = 0;

    //! Stop playing music.  To begin again, set a new intro or loop.
    virtual void stop() noexcept = 0;

    //! Pause playback of music.
    virtual void pause() noexcept = 0;
    //! Resume playback of music.
    virtual void resume() noexcept = 0;

    //! Between 0.0 (silence) and 1.0 (full).
    virtual void setVolume(double volume) noexcept = 0;

    //! Free music not recently played.
    virtual void garbageCollect() noexcept = 0;

 protected:
    Music() = default;
};

#endif  // SRC_CORE_MUSIC_H_
