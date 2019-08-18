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

#include "util/int.h"
#include "util/markable.h"
#include "util/string-view.h"

typedef Markable<int,-1> SoundID;
typedef Markable<int,-1> PlayingSoundID;

class Sounds {
 public:
    static SoundID load(StringView path) noexcept;

    // Free destroyed Sounds that were not recently played.
    static void prune(time_t latestPermissibleUse) noexcept;
};

class Sound {
 public:
     static PlayingSoundID play(SoundID sid) noexcept;

     static void release(SoundID sid) noexcept;
};

class PlayingSound {
 public:
    // Whether the sound is currently playing.
    static bool isPlaying(PlayingSoundID psid) noexcept;
    // Stop playing the sound. SoundInstances cannot resume from stop().
    // Create a new one to play again. Calls destroy().
    static void stop(PlayingSoundID psid) noexcept;

    // Between 0.0 (silence) and 1.0 (full).
    static void volume(PlayingSoundID psid, float volume) noexcept;
    // 1.0 is normal speed.
    static void speed(PlayingSoundID psid, float speed) noexcept;

    // Release the resources used by this PlayingSound.
    static void release(PlayingSoundID psid) noexcept;
};

#endif  // SRC_CORE_SOUNDS_H_
