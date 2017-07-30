/***************************************
** Tsunagari Tile Engine              **
** gosu-sounds.cpp                    **
** Copyright 2011-2014 Michael Reiley **
** Copyright 2011-2017 Paul Merrill   **
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

#include "gosu-sounds.h"

#include "core/client-conf.h"
#include "core/formatter.h"
#include "core/measure.h"
#include "core/resources.h"
#include "core/sounds.h"
#include "util/assert.h"
#include "util/math2.h"
#include "util/unique.h"

#include "av/gosu/gosu-cbuffer.h"

#ifdef BACKEND_GOSU
static GosuSounds globalSounds;

Sounds& Sounds::instance() {
    return globalSounds;
}
#endif

GosuSoundInstance::GosuSoundInstance(Gosu::SampleInstance instance)
    : instance(instance) {
    volume(1.0);
}

bool GosuSoundInstance::playing() {
    return instance.playing();
}

void GosuSoundInstance::stop() {
       instance.stop();
}

bool GosuSoundInstance::paused() {
    return instance.paused();
}

void GosuSoundInstance::pause() {
    instance.pause();
}

void GosuSoundInstance::resume() {
    instance.resume();
}

void GosuSoundInstance::volume(double attemptedVolume) {
    double volume = bound(attemptedVolume, 0.0, 1.0);
    if (attemptedVolume != volume) {
        Log::info("SoundInstance",
            Formatter(
                "Attempted to set volume to %, setting it to %"
                ) % attemptedVolume % volume);
    }
    assert_(0 <= conf.soundVolume && conf.soundVolume <= 100);
    instance.change_volume(volume * conf.soundVolume / 100.0);
}

void GosuSoundInstance::pan(double attemptedPan) {
    double pan = bound(attemptedPan, 0.0, 1.0);
    if (attemptedPan != pan) {
        Log::info("SoundInstance",
            Formatter(
                "Attempted to set pan to %, setting it to %"
                ) % attemptedPan % pan);
    }
    instance.change_pan(pan);
}

void GosuSoundInstance::speed(double attemptedSpeed) {
    double speed = bound(attemptedSpeed, 0.0, 100.0);
    if (attemptedSpeed != speed) {
        Log::info("SoundInstance",
            Formatter(
                "Attempted to set speed to %, setting it to %"
                ) % attemptedSpeed % speed);
    }
    instance.change_speed(speed);
}


static std::shared_ptr<Gosu::Sample> genSample(const std::string& path) {
    Unique<Resource> r = Resources::instance().load(path);
    if (!r) {
        // Error logged.
        return std::shared_ptr<Gosu::Sample>();
    }
    GosuCBuffer buffer(r->data(), r->size());

    TimeMeasure m("Constructed " + path + " as sample");
    return std::make_shared<Gosu::Sample>(buffer.front_reader());
}

GosuSounds::GosuSounds() : samples(genSample) {}

Rc<SoundInstance> GosuSounds::play(const std::string& path) {
    auto sample = samples.lifetimeRequest(path);
    if (!sample) {
        // Error logged.
        return Rc<SoundInstance>();
    }
    return Rc<SoundInstance>(new GosuSoundInstance(sample->play()));
}

void GosuSounds::garbageCollect() {
    samples.garbageCollect();
}
