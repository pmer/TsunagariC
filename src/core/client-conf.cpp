/***************************************
** Tsunagari Tile Engine              **
** client-conf.cpp                    **
** Copyright 2011-2013 Michael Reiley **
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

#include "core/client-conf.h"

#include "config.h"
#include "core/jsons.h"
#include "os/os.h"
#include "util/move.h"
#include "util/string.h"
#include "util/string2.h"
#include "util/vector.h"

verbosity_t Conf::verbosity = V_VERBOSE;
Conf::MovementMode Conf::moveMode;
ivec2 Conf::windowSize = {640, 480};
bool Conf::fullscreen = false;
int Conf::musicVolume = 100;
int Conf::soundVolume = 100;
time_t Conf::cacheTTL = 300;
int Conf::persistInit = 0;
int Conf::persistCons = 0;

// Parse and process the client config file, and set configuration defaults for
// missing options.
bool
Conf::parse(StringView filename) noexcept {
    Optional<String> file = readFile(filename);

    if (!file) {
        Log::fatal(filename, String() << "Could not find " << filename);
        return false;
    }

    Unique<JSONObject> doc = JSONs::parse(move_(*file));

    if (!doc) {
        Log::err(filename, String() << "Could not parse " << filename);
        return false;
    }

    if (doc->hasObject("engine")) {
        Unique<JSONObject> engine = doc->objectAt("engine");

        if (engine->hasString("verbosity")) {
            StringView verbosity = engine->stringAt("verbosity");
            if (verbosity == "quiet") {
                Conf::verbosity = V_QUIET;
            }
            else if (verbosity == "normal") {
                Conf::verbosity = V_NORMAL;
            }
            else if (verbosity == "verbose") {
                Conf::verbosity = V_VERBOSE;
            }
            else {
                Log::err(filename,
                         "Unknown value for \"engine.verbosity\", using "
                         "default");
            }
        }
    }

    if (doc->hasObject("window")) {
        Unique<JSONObject> window = doc->objectAt("window");

        if (window->hasUnsigned("width")) {
            Conf::windowSize.x = window->intAt("width", 1, 100000);
        }
        if (window->hasUnsigned("height")) {
            Conf::windowSize.y = window->intAt("height", 1, 100000);
        }
        if (window->hasBool("fullscreen")) {
            Conf::fullscreen = window->boolAt("fullscreen");
        }
    }

    if (doc->hasObject("audio")) {
        Unique<JSONObject> audio = doc->objectAt("audio");

        if (audio->hasUnsigned("musicvolume")) {
            Conf::musicVolume = audio->intAt("musicvolume", 0, 100);
        }
        if (audio->hasUnsigned("soundvolume")) {
            Conf::soundVolume = audio->intAt("soundvolume", 0, 100);
        }
    }

    if (doc->hasObject("cache")) {
        Unique<JSONObject> cache = doc->objectAt("cache");

        if (cache->hasUnsigned("ttl")) {
            Conf::cacheTTL = cache->unsignedAt("ttl") != 0;
        }
    }

    return true;
}
