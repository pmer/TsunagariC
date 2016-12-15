/********************************
** Tsunagari Tile Engine       **
** sounds.cpp                  **
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

#include "av/sdl2/sounds.h"

static SDL2Sounds globalSounds;

Sounds& Sounds::instance() {
    return globalSounds;
}


bool SDL2SoundInstance::playing() { return false; }
void SDL2SoundInstance::stop() {}

bool SDL2SoundInstance::paused() {}
void SDL2SoundInstance::pause() {}
void SDL2SoundInstance::resume() {}

void SDL2SoundInstance::volume(double volume) {}
void SDL2SoundInstance::pan(double pan) {}
void SDL2SoundInstance::speed(double speed) {}


std::shared_ptr<SoundInstance> SDL2Sounds::play(const std::string& path) {
    return std::make_shared<SDL2SoundInstance>();
}

void SDL2Sounds::garbageCollect() {}
