/*************************************
** Tsunagari Tile Engine            **
** music.cpp                        **
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

#include "av/sdl2/music.h"

#include "av/sdl2/error.h"
#include "av/sdl2/sounds.h"
#include "core/measure.h"
#include "core/resources.h"
#include "util/int.h"
#include "util/noexcept.h"
#include "util/unique.h"

static Rc<SDL2Song>
genSong(StringView name) noexcept {
    Optional<StringView> r = resourceLoad(name);
    if (!r) {
        // Error logged.
        return Rc<SDL2Song>();
    }

    assert_(r->size < UINT32_MAX);

    SDL_RWops* ops =
            SDL_RWFromMem(static_cast<void*>(const_cast<char*>(r->data)),
                          static_cast<int>(r->size));

    TimeMeasure m(String() << "Constructed " << name << " as music");
    Mix_Music* music = Mix_LoadMUS_RW(ops, 1);

    // We need to keep the memory (the resource) around, so put it in a struct.
    SDL2Song* song = new SDL2Song;
    song->resource = move_(r);
    song->mix = music;

    return Rc<SDL2Song>(song);
}

static bool initalized = false;
static String path;
static int paused = 0;
static Rc<SDL2Song> currentMusic;
static ReaderCache<Rc<SDL2Song>, genSong> songs;


SDL2Song::~SDL2Song() noexcept {
    Mix_FreeMusic(mix);
}


static void
init() noexcept {
    if (initalized) {
        return;
    }

    initalized = true;

    if (SDL_WasInit(SDL_INIT_AUDIO) == 0) {
        {
            TimeMeasure m("Initialized the SDL2 audio subsystem");
            if (SDL_Init(SDL_INIT_AUDIO) < 0) {
                sdlDie("SDL2Music", "SDL_Init(SDL_INIT_AUDIO)");
            }
        }

        {
            TimeMeasure m("Opened an audio device");
            if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
                sdlDie("SDL2Music", "Mix_OpenAudio");
            }
        }
    }
}

void
MusicWorker::play(StringView path_) noexcept {
    init();

    if (path == path_) {
        return;
    }

    paused = 0;
    path = path_;

    TimeMeasure m(String() << "Playing " << path);
    if (currentMusic && !Mix_PausedMusic()) {
        Mix_HaltMusic();
    }
    currentMusic = path.size() ? songs.lifetimeRequest(path) : Rc<SDL2Song>();
    if (currentMusic) {
        Mix_PlayMusic(currentMusic->mix, -1);
    }
}

void
MusicWorker::stop() noexcept {
    init();

    paused = 0;
    path = "";

    if (currentMusic) {
        Mix_HaltMusic();
    }
    currentMusic = Rc<SDL2Song>();
}

void
MusicWorker::pause() noexcept {
    init();

    if (paused == 0 && currentMusic) {
        Mix_PauseMusic();
    }

    paused++;
}

void
MusicWorker::resume() noexcept {
    init();

    paused--;

    assert_(paused >= 0);

    if (paused == 0 && currentMusic) {
        Mix_ResumeMusic();
    }
}

void
MusicWorker::garbageCollect() noexcept {
    songs.garbageCollect();
}
