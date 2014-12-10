/***************************************
** Tsunagari Tile Engine              **
** main.cpp                           **
** Copyright 2011-2013 PariahSoft LLC **
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

#include <Gosu/Utility.hpp>
#include <libxml/parser.h>
#include <physfs.h>

#include "client-conf.h"
#include "formatter.h"
#include "log.h"
#include "reader.h"
#include "window.h"
#include "world.h"

#include "data/world.h"

#ifdef _WIN32
  #include "os-windows.h"
#endif

#ifdef __APPLE__
  #include "os-mac.h"
#endif

/**
 * Load client config and instantiate window.
 *
 * The client config tells us our window parameters along with which World
 * we're going to load. The GameWindow class then loads and plays the game.
 */
int main(int argc, char** argv)
{
#ifdef _WIN32
	wFixConsole();
#endif

	srand((unsigned)time(NULL));

	if (!Log::init())
		return 1;

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

	/* This initializes the XML library and checks for potential
	 * ABI mismatches between the version it was compiled for and
	 * the actual shared library used.
	 */
	LIBXML_TEST_VERSION

	if (!PHYSFS_init(argv[0])) {
		Log::fatal("Main", "PHYSFS_init");
		return 1;
	}

	if (!Reader::init()) {
		Log::fatal("Main", "Reader::init");
		return 1;
	}

	GameWindow window;
	World& world = World::instance();
	DataWorld& dataWorld = DataWorld::instance();

	if (!window.init()) {
		Log::fatal("Main", "GameWindow::init");
		return 1;
	}

	if (!world.init()) {
		Log::fatal("Main", "World::init");
		return 1;
	}

	if (!dataWorld.init()) {
		Log::fatal("Main", "WorldWorld::init");
		return 1;
	}

	window.setCaption(Gosu::widen(world.getName()));

	window.show();

	PHYSFS_deinit();

	xmlCleanupParser();

	return 0;
}
