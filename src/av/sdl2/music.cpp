/*************************************
** Tsunagari Tile Engine            **
** music.cpp                        **
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

#include "av/sdl2/music.h"

#include <limits.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include "core/measure.h"
#include "core/resources.h"
#include "util/unique.h"

static SDL2Music* globalMusicWorker = nullptr;

MusicWorker& MusicWorker::instance() {
    if (globalMusicWorker == nullptr) {
        globalMusicWorker = new SDL2Music;
    }
    return *globalMusicWorker;
}

static Rc<SDL2Song> genSong(const std::string& name) {
    Unique<Resource> r = Resources::instance().load(name);
    if (!r) {
        // Error logged.
        return Rc<SDL2Song>();
    }

    assert_(r->size() < INT_MAX);

    SDL_RWops* ops = SDL_RWFromMem(const_cast<void*>(r->data()),
                                   static_cast<int>(r->size()));

    TimeMeasure m("Constructed " + name + " as music");
    Mix_Music* music = Mix_LoadMUS_RW(ops, 1);

    // We need to keep the memory (the resource) around, so put it in a struct.
    SDL2Song* song = new SDL2Song;
    song->resource = move_(r);
    song->mix = music;

    return Rc<SDL2Song>(song);
}


SDL2Music::SDL2Music() : songs(genSong) {
    assert_(SDL_Init(SDL_INIT_AUDIO) != -1);
    assert_(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) != -1);
}

SDL2Music::~SDL2Music() {
    stop();
}

void SDL2Music::play(std::string filepath) {
    if (path == filepath) {
        if (Mix_PausedMusic()) {
            paused = 0;
            Mix_PlayMusic(currentMusic->mix.get(), -1);
        }
        return;
    }

    MusicWorker::play(move_(filepath));
    TimeMeasure m("Playing " + path);
    if (currentMusic && !Mix_PausedMusic()) {
        Mix_HaltMusic();
    }
    currentMusic = path.size() ? songs.lifetimeRequest(path)
                               : Rc<SDL2Song>();
    if (currentMusic) {
        Mix_PlayMusic(currentMusic->mix.get(), -1);
        Mix_VolumeMusic(static_cast<int>(volume * 128));
    }
}

void SDL2Music::stop() {
    MusicWorker::stop();
    if (currentMusic) {
        Mix_HaltMusic();
    }
    currentMusic = Rc<SDL2Song>();
}

bool SDL2Music::playing() {
    return currentMusic && Mix_PlayingMusic() != 0;
}

void SDL2Music::pause() {
    if (paused == 0 && currentMusic) {
        Mix_PauseMusic();
    }
    MusicWorker::pause();
}

void SDL2Music::resume() {
    MusicWorker::resume();
    if (paused == 0 && currentMusic) {
        Mix_PlayMusic(currentMusic->mix.get(), -1);
    }
}

void SDL2Music::setVolume(double volume) {
    MusicWorker::setVolume(volume);
    if (currentMusic) {
        Mix_VolumeMusic(static_cast<int>(volume * 128));
    }
}

void SDL2Music::garbageCollect() {
    songs.garbageCollect();
}
