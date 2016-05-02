/***************************************
** Tsunagari Tile Engine              **
** gosu-music.cpp                     **
** Copyright 2011-2014 PariahSoft LLC **
** Copyright 2015 Paul Merrill        **
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

#include <Gosu/Audio.hpp>

#include "../client-conf.h"
#include "../resources.h"
#include "../time.h"

#include "gosu-cbuffer.h"
#include "gosu-music.h"

#ifdef BACKEND_GOSU
static GosuMusic globalMusic;

Music& Music::instance()
{
    return globalMusic;
}
#endif

static std::shared_ptr<Gosu::Song> genSong(const std::string& name)
{
    std::unique_ptr<Resource> r = Resources::instance().load(name);
    if (!r) {
        // Error logged.
        return std::shared_ptr<Gosu::Song>();
    }
    GosuCBuffer buffer(r->data(), r->size());

    TimeMeasure m("Constructed " + name + " as music");
    return std::shared_ptr<Gosu::Song>(
        new Gosu::Song(buffer.frontReader())
    );
}



GosuMusic::GosuMusic() : songs(genSong)
{
}

GosuMusic::~GosuMusic()
{
    if (musicInst && musicInst->playing())
        musicInst->stop();
}

bool GosuMusic::setIntro(const std::string& filepath)
{
    if (Music::setIntro(filepath)) {
        // Optimize XXX: Don't load until played.
        introMusic = filepath.size() ? songs.lifetimeRequest(filepath) :
            std::shared_ptr<Gosu::Song>();
        return true;
    }
    else
        return false;
}

bool GosuMusic::setLoop(const std::string& filepath)
{
    if (Music::setLoop(filepath)) {
        // Optimize XXX: Don't load until played.
        loopMusic = filepath.size() ? songs.lifetimeRequest(filepath) :
            std::shared_ptr<Gosu::Song>();
        return true;
    }
    else
        return false;
}

bool GosuMusic::playing()
{
    if (musicInst)
        return musicInst->playing();
    else
        return false;
}

void GosuMusic::stop()
{
    Music::stop();
    if (musicInst)
        musicInst->stop();
    musicInst = introMusic = loopMusic = std::shared_ptr<Gosu::Song>();
}

void GosuMusic::pause()
{
    Music::pause();
    if (pausedCount == 1 && musicInst)
        musicInst->pause();
}

void GosuMusic::resume()
{
    Music::resume();
    if (pausedCount == 0 && musicInst)
        musicInst->play();
}

void GosuMusic::setVolume(double level)
{
    if (musicInst)
        musicInst->changeVolume(level);
}

void GosuMusic::tick()
{
    switch (state) {
    case NOT_PLAYING:
        if (musicInst && musicInst->playing())
            musicInst->stop();
        break;
    case PLAYING_INTRO:
        if (!musicInst->playing()) {
            if (newLoop.size() && loopMusic)
                playLoop();
            else
                state = NOT_PLAYING;
        }
        break;
    case PLAYING_LOOP:
        break;
    case CHANGED_INTRO:
        if (newIntro.size() && introMusic)
            playIntro();
        else if (newLoop.size() && newLoop != curLoop)
            state = CHANGED_LOOP;
        else if (newLoop.size())
            state = PLAYING_LOOP;
        else
            state = NOT_PLAYING;
        break;
    case CHANGED_LOOP:
        if (newIntro.size() && loopMusic)
            playIntro();
        else if (newLoop.size() && loopMusic)
            playLoop();
        else
            state = NOT_PLAYING;
        break;
    }
}

void GosuMusic::playIntro()
{
    TimeMeasure m("Playing " + newIntro + " as intro");
    Music::playIntro();
    if (musicInst && musicInst->playing())
        musicInst->stop();
    introMusic->play(false);
    introMusic->changeVolume(volume);
    musicInst = introMusic;
}

void GosuMusic::playLoop()
{
    TimeMeasure m("Playing " + newLoop + " as loop");
    Music::playLoop();
    if (musicInst && musicInst->playing())
        musicInst->stop();
    loopMusic->play(true);
    loopMusic->changeVolume(volume);
    musicInst = loopMusic;
}

void GosuMusic::garbageCollect()
{
    songs.garbageCollect();
}
