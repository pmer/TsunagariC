/***************************************
** Tsunagari Tile Engine              **
** main.cpp                           **
** Copyright 2011-2013 PariahSoft LLC **
** Copyright 2016 Paul Merrill        **
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

#include <stdlib.h>
#include <time.h>

#include <physfs.h>

#include "config.h"
#include "core/client-conf.h"
#include "core/formatter.h"
#include "core/log.h"
#include "core/measure.h"
#include "core/resources.h"
#include "core/window.h"
#include "core/world.h"

#include "data/data-world.h"

#ifdef _WIN32
  #include "os/os-windows.h"
#endif

#ifdef __APPLE__
  #include "os/mac.h"
#endif

/**
 * Load client config and instantiate window.
 *
 * The client config tells us our window parameters along with which World
 * we're going to load. The GameWindow class then loads and plays the game.
 */
int main(int argc, char** argv) {
#ifdef _WIN32
    wFixConsole();
#endif

    srand((unsigned)time(nullptr));

    if (!Log::init()) {
        return 1;
    }

#ifdef __APPLE__
    macSetWorkingDirectory();
#endif

    parseConfig(CLIENT_CONF_PATH);
    if (!parseCommandLine(argc, argv)) {
        Log::fatal("Main", "parseCommandLine");
        return 1;
    }
    if (!conf.validate(CLIENT_CONF_PATH)) {
        Log::fatal("Main", "Conf::validate");
        return 1;
    }

    Log::setVerbosity(conf.verbosity);
    Log::info("Main", Formatter("Starting %") % TSUNAGARI_RELEASE_VERSION);
    Log::reportVerbosityOnStartup();

    GameWindow* window;
    {
        TimeMeasure m("Constructed window");
        window = GameWindow::create();
    }
    World& world = World::instance();
    DataWorld& dataWorld = DataWorld::instance();

    if (!window->init()) {
        Log::fatal("Main", "GameWindow::init");
        return 1;
    }
    {
        TimeMeasure m("Constructed world");
        if (!world.init()) {
            Log::fatal("Main", "World::init");
            return 1;
        }
    }
    if (!dataWorld.init()) {
        Log::fatal("Main", "DataWorld::init");
        return 1;
    }

    window->setCaption(dataWorld.about.name);

    window->mainLoop();

    // Cleanup
    delete window;

    PHYSFS_deinit();

    return 0;
}
