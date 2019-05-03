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

Conf conf;  // Project-wide global configuration.

// Parse and process the client config file, and set configuration defaults for
// missing options.
bool
parseConfig(StringView filename) noexcept {
    Optional<String> file = readFile(filename);

    if (!file) {
        Log::err(filename, String() << "Could not find " << filename);
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
                conf.verbosity = V_QUIET;
            }
            else if (verbosity == "normal") {
                conf.verbosity = V_NORMAL;
            }
            else if (verbosity == "verbose") {
                conf.verbosity = V_VERBOSE;
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
            conf.windowSize.x = window->intAt("width", 1, 100000);
        }
        if (window->hasUnsigned("height")) {
            conf.windowSize.y = window->intAt("height", 1, 100000);
        }
        if (window->hasBool("fullscreen")) {
            conf.fullscreen = window->boolAt("fullscreen");
        }
    }

    if (doc->hasObject("audio")) {
        Unique<JSONObject> audio = doc->objectAt("audio");

        if (audio->hasUnsigned("musicvolume")) {
            conf.musicVolume = audio->intAt("musicvolume", 0, 100);
        }
        if (audio->hasUnsigned("soundvolume")) {
            conf.soundVolume = audio->intAt("soundvolume", 0, 100);
        }
    }

    if (doc->hasObject("cache")) {
        Unique<JSONObject> cache = doc->objectAt("cache");

        if (cache->hasUnsigned("ttl")) {
            conf.cacheTTL = cache->unsignedAt("ttl") != 0;
        }
    }

    return true;
}
