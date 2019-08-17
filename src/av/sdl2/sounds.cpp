/*************************************
** Tsunagari Tile Engine            **
** sounds.cpp                       **
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

#include "av/sdl2/sounds.h"

#include "av/sdl2/error.h"
#include "av/sdl2/sdl2.h"
#include "core/measure.h"
#include "core/resources.h"
#include "util/int.h"
#include "util/noexcept.h"

static Rc<SDL2Sample>
genSample(StringView name) noexcept {
    Optional<StringView> r = resourceLoad(name);
    if (!r) {
        // Error logged.
        return Rc<SDL2Sample>();
    }

    assert_(r->size < UINT32_MAX);

    SDL_RWops* ops =
            SDL_RWFromMem(static_cast<void*>(const_cast<char*>(r->data)),
                          static_cast<int>(r->size));

    TimeMeasure m(String() << "Constructed " << name << " as sample");
    Mix_Chunk* chunk = Mix_LoadWAV_RW(ops, 1);

    // We need to keep the memory (the resource) around, so put it in a struct.
    auto sample = new SDL2Sample;
    sample->resource = move_(r);
    sample->chunk = chunk;

    return Rc<SDL2Sample>(sample);
}

static ReaderCache<Rc<SDL2Sample>, genSample> samples;
static Vector<Rc<SoundInstance>> channels;


SDL2Sample::~SDL2Sample() noexcept {
    Mix_FreeChunk(chunk);
}


SDL2SoundInstance::SDL2SoundInstance(int channel) noexcept
        : channel(channel), state(S_PLAYING) {}

bool
SDL2SoundInstance::playing() noexcept {
    return state == S_PLAYING;
}

void
SDL2SoundInstance::stop() noexcept {
    assert_(state == S_PLAYING || state == S_PAUSED);
    Mix_HaltChannel(channel);
    state = S_DONE;
}

bool
SDL2SoundInstance::paused() noexcept {
    return state == S_PAUSED;
}

void
SDL2SoundInstance::pause() noexcept {
    assert_(state == S_PLAYING);
    Mix_Pause(channel);
    state = S_PAUSED;
}

void
SDL2SoundInstance::resume() noexcept {
    assert_(state == S_PAUSED);
    Mix_Resume(channel);
    state = S_PLAYING;
}

void
SDL2SoundInstance::volume(float volume) noexcept {
    Mix_Volume(channel, static_cast<int>(volume * 128));
}

void
SDL2SoundInstance::pan(float pan) noexcept {
    auto angle = static_cast<int16_t>(pan * 90);

    int err = Mix_SetPosition(channel, angle, 0);
    (void)err;
    assert_(err == 0);
}

void
SDL2SoundInstance::speed(float) noexcept {
    // No-op. SDL2 doesn't support changing playback rate.
}

void
SDL2SoundInstance::setDone() noexcept {
    state = S_DONE;
}


static void
setDone(int channel) noexcept {
    assert_(channel >= 0);
    assert_(channels.size() > static_cast<size_t>(channel));

    // `channels` needs to be an Rc<SoundInstance> so the reference counter on
    // the sound can be shared with clients of the Sounds interface (from
    // core/sounds.h). Unfortunately, that means we need to reinterpret_cast the
    // Rc to get access to setDone().
    auto instance = reinterpret_cast<Rc<SDL2SoundInstance>*>(
            &channels[static_cast<size_t>(channel)]);
    (*instance)->setDone();
}

static void
channelFinished(int channel) noexcept {
    setDone(channel);
}

static void
init() noexcept {
    static bool initialized = false;

    if (initialized) {
        return;
    }
    initialized = true;

    if (SDL_WasInit(SDL_INIT_AUDIO) != 0) {
        return;
    }

    {
        TimeMeasure m("Initialized the SDL2 audio subsystem");
        if (SDL_Init(SDL_INIT_AUDIO) < 0) {
            sdlDie("SDL2Sounds", "SDL_Init(SDL_INIT_AUDIO)");
        }
    }

    Mix_ChannelFinished(channelFinished);
}

Rc<SoundInstance>
Sounds::play(StringView path) noexcept {
    init();

    auto sample = samples.lifetimeRequest(path);
    if (!sample) {
        // Error logged.
        return Rc<SoundInstance>();
    }

    int channel = Mix_PlayChannel(-1, sample->chunk, 0);

    if (channel < 0) {
        // Maybe there are too many sounds playing at once right now.
        return Rc<SoundInstance>();
    }

    Rc<SoundInstance> sound(new SDL2SoundInstance(channel));

    if (channels.size() <= static_cast<size_t>(channel) + 1) {
        channels.resize(static_cast<size_t>(channel + 1));
    }
    channels[static_cast<size_t>(channel)] = sound;

    return sound;
}

void
Sounds::garbageCollect() noexcept {
    samples.garbageCollect();
}
