/***************************************
** Tsunagari Tile Engine              **
** client-conf.cpp                    **
** Copyright 2011-2013 Michael Reiley **
** Copyright 2011-2018 Paul Merrill   **
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

#include <iostream>

#include "config.h"
#include "core/jsons.h"
#include "nbcl/nbcl.h"
#include "util/move.h"
#include "util/string2.h"
#include "util/vector.h"

Conf conf; // Project-wide global configuration.

// Initialize and set configuration defaults.
Conf::Conf() {
    persistInit = 0;
    persistCons = 0;
}

/* Output compiled-in engine defaults. */
static void defaultsQuery() {
    std::cerr << "CLIENT_CONF_PATH:                    "
        << CLIENT_CONF_PATH << std::endl;
    std::cerr << "DEF_ENGINE_VERBOSITY:                "
        << DEF_ENGINE_VERBOSITY << std::endl;
    std::cerr << "DEF_WINDOW_WIDTH:                    "
        << DEF_WINDOW_WIDTH << std::endl;
    std::cerr << "DEF_WINDOW_HEIGHT:                   "
        << DEF_WINDOW_HEIGHT << std::endl;
    std::cerr << "DEF_WINDOW_FULLSCREEN:               "
        << DEF_WINDOW_FULLSCREEN << std::endl;
    std::cerr << "DEF_CACHE_ENABLED:                   "
        << DEF_CACHE_ENABLED << std::endl;
    std::cerr << "DEF_CACHE_TTL:                       "
        << DEF_CACHE_TTL << std::endl;
}

// Parse and process the client config file, and set configuration defaults for
// missing options.
bool parseConfig(const std::string& filename) {
    std::string file = slurp(filename);

    if (file.size() == 0) {
        Log::err(filename, "Could not find " + filename);
        return false;
    }

    JSONObjectPtr doc = JSONs::parse(move_(file));

    if (!doc) {
        Log::err(filename, "Could not parse " + filename);
        return false;
    }

    conf.verbosity = DEF_ENGINE_VERBOSITY;
    conf.windowSize = {DEF_WINDOW_WIDTH, DEF_WINDOW_HEIGHT};
    conf.fullscreen = DEF_WINDOW_FULLSCREEN;
    conf.musicVolume = 100;
    conf.soundVolume = 100;
    conf.cacheEnabled = DEF_CACHE_ENABLED;
    conf.cacheEnabled = DEF_CACHE_TTL != 0;
    conf.cacheTTL = DEF_CACHE_TTL;

    if (doc->hasObject("engine")) {
        JSONObjectPtr engine = doc->objectAt("engine");

        if (engine->hasString("verbosity")) {
            std::string verbosity = engine->stringAt("verbosity");
            if (verbosity == "quiet") {
                conf.verbosity = V_QUIET;
            } else if (verbosity == "normal") {
                conf.verbosity = V_NORMAL;
            } else if (verbosity == "verbose") {
                conf.verbosity = V_VERBOSE;
            } else {
                Log::err(filename, "Unknown value for \"engine.verbosity\", using default");
            }
        }
    }

    if (doc->hasObject("window")) {
        JSONObjectPtr window = doc->objectAt("window");

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
        JSONObjectPtr audio = doc->objectAt("audio");

        if (audio->hasUnsigned("musicvolume")) {
            conf.musicVolume = audio->intAt("musicvolume", 0, 100);
        }
        if (audio->hasUnsigned("soundvolume")) {
            conf.soundVolume = audio->intAt("soundvolume", 0, 100);
        }
    }

    if (doc->hasObject("cache")) {
        JSONObjectPtr cache = doc->objectAt("cache");

        if (cache->hasBool("enabled")) {
            conf.cacheEnabled = cache->boolAt("enabled");
        }
        if (cache->hasUnsigned("ttl")) {
            conf.cacheEnabled = !!cache->unsignedAt("ttl");
        }
    }

    return true;
}

// Parse and process command line options and arguments.
bool parseCommandLine(int argc, char* argv[]) {
    NBCL cmd(argc, argv);

    cmd.setStrayArgsDesc("[WORLD FILE]");

    cmd.insert("-h", "--help",         "",                "Display this help message");
    cmd.insert("-c", "--config",       "<config file>",   "Client config file to use");
    cmd.insert("-p", "--datapath",     "<file,file,...>", "Prepend zips to data path");
    cmd.insert("-q", "--quiet",        "",                "Display only fatal errors");
    cmd.insert("-n", "--normal",       "",                "Display all errors");
    cmd.insert("-v", "--verbose",      "",                "Display additional information");
    cmd.insert("-t", "--cache-ttl",    "<seconds>",       "Cache time-to-live in seconds");
    cmd.insert("-m", "--cache-size",   "<megabytes>",     "Cache size in megabytes");
    cmd.insert("-s", "--size",         "<WxH>",           "Window dimensions");
    cmd.insert("-f", "--fullscreen",   "",                "Run in fullscreen mode");
    cmd.insert("-w", "--window",       "",                "Run in windowed mode");
    cmd.insert("",   "--volume-music", "<0-100>",         "Set music volume");
    cmd.insert("",   "--volume-sound", "<0-100>",         "Set sound effects volume");
    cmd.insert("",   "--query",        "",                "Query compiled-in engine defaults");
    cmd.insert("",   "--version",      "",                "Print the engine version string");

    if (!cmd.parse()) {
        cmd.usage();
        return false;
    }

    if (cmd.check("--help")) {
        cmd.usage();
        return false;
    }

    if (cmd.check("--version")) {
        std::cout << TSUNAGARI_RELEASE_VERSION << std::endl;
        return false;
    }

    if (cmd.check("--query")) {
        defaultsQuery();
        return false;
    }

    if (cmd.check("--config")) {
        if (!parseConfig(cmd.get("--config"))) {
            return false;
        }
    }

    int verbcount = 0;
    if (cmd.check("--quiet")) {
        conf.verbosity = V_QUIET;
        verbcount++;
    }
    if (cmd.check("--normal")) {
        conf.verbosity = V_NORMAL;
        verbcount++;
    }
    if (cmd.check("--verbose")) {
        conf.verbosity = V_VERBOSE;
        verbcount++;
    }
    if (verbcount > 1) {
        Log::err("cmdline", "multiple verbosity flags on cmdline, using most verbose");
    }

    if (cmd.check("--volume-music")) {
        conf.musicVolume = parseInt100(cmd.get("--volume-music"));
    }

    if (cmd.check("--volume-sound")) {
        conf.soundVolume = parseInt100(cmd.get("--volume-sound"));
    }

    if (cmd.check("--cache-ttl")) {
        if (!parseUInt(cmd.get("--cache-ttl"), &conf.cacheTTL)) {
            Log::fatal("cmdline", "invalid argument for --cache-ttl");
            return false;
        }
        if (conf.cacheTTL == 0) {
            conf.cacheEnabled = false;
        }
    }

    if (cmd.check("--size")) {
        vector<std::string> dim = splitStr(cmd.get("--size"), "x");
        if (dim.size() != 2) {
            Log::fatal("cmdline", "invalid argument for -s/--size");
            return false;
        }
        if (!parseUInt(dim[0], &conf.windowSize.x)) {
            Log::fatal("cmdline", "invalid argument for -s/--size");
            return false;
        }
        if (!parseUInt(dim[1], &conf.windowSize.y)) {
            Log::fatal("cmdline", "invalid argument for -s/--size");
            return false;
        }
    }

    if (cmd.check("--fullscreen") && cmd.check("--window")) {
        Log::fatal("cmdline", "-f/--fullscreen and -w/--window mutually exclusive");
        return false;
    }

    if (cmd.check("--fullscreen")) {
        conf.fullscreen = true;
    }

    if (cmd.check("--window")) {
        conf.fullscreen = false;
    }

    return true;
}
