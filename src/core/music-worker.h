/***************************************
** Tsunagari Tile Engine              **
** music-worker.h                     **
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

#ifndef SRC_CORE_MUSIC_WORKER_H_
#define SRC_CORE_MUSIC_WORKER_H_

#include <string>

class MusicWorker {
 public:
    static MusicWorker& instance();

    virtual ~MusicWorker() = default;

    virtual void setIntro(const std::string& filepath);
    virtual void setLoop(const std::string& filepath);

    virtual void stop();

    virtual void pause();
    virtual void resume();

    double getVolume();
    virtual void setVolume(double volume);

    virtual void tick() = 0;

    virtual void garbageCollect() = 0;

 protected:
    MusicWorker();

    void playIntro();
    void playLoop();

    enum MUSIC_STATE
    {
        NOT_PLAYING,
        PLAYING_INTRO,
        PLAYING_LOOP,
        CHANGED_INTRO,
        CHANGED_LOOP
    } state;

    double volume;
    int pausedCount;

    std::string curIntro, newIntro;
    std::string curLoop, newLoop;
};

#endif  // SRC_CORE_MUSIC_WORKER_H_
