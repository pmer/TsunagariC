/*************************************
** Tsunagari Tile Engine            **
** sounds.cpp                       **
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

#include "av/sdl2/sounds.h"

#include <limits.h>

#include <SDL2/SDL.h>

#include "core/measure.h"
#include "core/resources.h"

void SDL2OpenAudio() {
    // Calling these functions more than once is okay.
    assert_(SDL_Init(SDL_INIT_AUDIO) != -1);
    assert_(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) != -1);
}

static SDL2Sounds* globalSounds = nullptr;

Sounds& Sounds::instance() {
    if (globalSounds == nullptr) {
        globalSounds = new SDL2Sounds;
    }
    return *globalSounds;
}

SDL2Sounds& SDL2Sounds::instance() {
    if (globalSounds == nullptr) {
        globalSounds = new SDL2Sounds;
    }
    return *globalSounds;
}


SDL2Sample::~SDL2Sample() {
    Mix_FreeChunk(chunk);
}


SDL2SoundInstance::SDL2SoundInstance(int channel)
        : channel(channel), state(S_PLAYING) {}

bool SDL2SoundInstance::playing() {
    return state == S_PLAYING;
}

void SDL2SoundInstance::stop() {
    assert_(state == S_PLAYING || state == S_PAUSED);
    Mix_HaltChannel(channel);
    state = S_DONE;
}

bool SDL2SoundInstance::paused() {
    return state == S_PAUSED;
}

void SDL2SoundInstance::pause() {
    assert_(state == S_PLAYING);
    Mix_Pause(channel);
    state = S_PAUSED;
}

void SDL2SoundInstance::resume() {
    assert_(state == S_PAUSED);
    Mix_Resume(channel);
    state = S_PLAYING;
}

void SDL2SoundInstance::volume(double volume) {
    Mix_Volume(channel,
               static_cast<int>(volume * 128));
}

void SDL2SoundInstance::pan(double pan) {
    auto angle = static_cast<Sint16>(pan * 90);
    assert_(Mix_SetPosition(channel, angle, 0));
}

void SDL2SoundInstance::speed(double) {
    // No-op. SDL2 doesn't support changing playback rate.
}

void SDL2SoundInstance::setDone() {
    state = S_DONE;
}


static Rc<SDL2Sample> genSample(const std::string& name) {
    Unique<Resource> r = Resources::instance().load(name);
    if (!r) {
        // Error logged.
        return Rc<SDL2Sample>();
    }

    assert_(r->size() < INT_MAX);

    SDL_RWops* ops = SDL_RWFromMem(const_cast<void*>(r->data()),
                                   static_cast<int>(r->size()));

    TimeMeasure m("Constructed " + name + " as sample");
    Mix_Chunk* chunk = Mix_LoadWAV_RW(ops, 1);

    // We need to keep the memory (the resource) around, so put it in a struct.
    auto sample = new SDL2Sample;
    sample->resource = move_(r);
    sample->chunk = chunk;

    return Rc<SDL2Sample>(sample);
}

static void channelFinished(int channel) {
    globalSounds->setDone(channel);
}

SDL2Sounds::SDL2Sounds() : samples(genSample) {
    SDL2OpenAudio();
    Mix_ChannelFinished(channelFinished);
}

Rc<SoundInstance> SDL2Sounds::play(const std::string& path) {
    auto sample = samples.lifetimeRequest(path);
    if (!sample) {
        // Error logged.
        return Rc<SoundInstance>();
    }
    int channel = Mix_PlayChannel(-1, sample->chunk, 0);
    Rc<SoundInstance> sound(new SDL2SoundInstance(channel));
    channels.reserve(channel + 1);
    for (int i = channels.size(); i <= channel + 1; i++) {
        channels.push_back();
    }
    channels[channel] = sound;
    return sound;
}

void SDL2Sounds::garbageCollect() {
    samples.garbageCollect();
}

void SDL2Sounds::setDone(int channel) {
    assert_(channels.size() > channel);

    // `channels` needs to be an Rc<SoundInstance> so the reference counter on
    // the sound can be shared with clients of the Sounds interface (from
    // core/sounds.h). Unfortunately, that means we need to reinterpret_cast the
    // Rc to get access to setDone().
    auto instance = reinterpret_cast<Rc<SDL2SoundInstance>*>(&channels[channel]);
    (*instance)->setDone();
}
