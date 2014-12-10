/***************************************
** Tsunagari Tile Engine              **
** window.cpp                         **
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

#include <Gosu/Graphics.hpp> // for Gosu::Graphics
#include <Gosu/Timing.hpp>
#include <Gosu/Utility.hpp>

#include "gosu-window.h"

#include "../client-conf.h"
#include "../reader.h"
#include "../world.h"

#define ASSERT(x)  if (!(x)) { return false; }

// Garbage collection called every X milliseconds
#define GC_CALL_PERIOD 10 * 1000

namespace Gosu {
	/**
	 * Enable 1980s-style graphics scaling: nearest-neighbor filtering.
	 * Call this function before creating any Gosu::Image.
	 */
	void enableUndocumentedRetrofication() {
		extern bool undocumentedRetrofication;
		undocumentedRetrofication = true;
	}
}


GameWindow* GameWindow::create()
{
	return new GosuGameWindow();
}

static GosuGameWindow* globalWindow = NULL;

GameWindow& GameWindow::instance()
{
	return *globalWindow;
}

time_t GameWindow::time()
{
	return Gosu::milliseconds();
}


GosuGameWindow::GosuGameWindow()
	// Gosu emulates the requested screen resolution on fullscreen,
	// but this breaks our aspect ratio-correcting letterbox.
	// Ergo we just make a window the size of the screen.
	: Gosu::Window(
	    conf.fullscreen ? Gosu::screenWidth() :
	                      (unsigned)conf.windowSize.x,
	    conf.fullscreen ? Gosu::screenHeight() :
	                      (unsigned)conf.windowSize.y,
	    conf.fullscreen
	  ),
	  now(Gosu::milliseconds()),
	  lastGCtime(0)
{
	globalWindow = this;
	Gosu::enableUndocumentedRetrofication();

	gosuToTsunagariKey.resize(Gosu::ButtonName::kbRangeEnd);
	auto& keys = gosuToTsunagariKey;
	keys[Gosu::ButtonName::kbEscape] = KBEscape;
	keys[Gosu::ButtonName::kbLeftShift] = KBLeftShift;
	keys[Gosu::ButtonName::kbRightShift] = KBRightShift;
	keys[Gosu::ButtonName::kbLeftControl] = KBLeftControl;
	keys[Gosu::ButtonName::kbRightControl] = KBRightControl;
	keys[Gosu::ButtonName::kbSpace] = KBSpace;
	keys[Gosu::ButtonName::kbLeft] = KBLeftArrow;
	keys[Gosu::ButtonName::kbRight] = KBRightArrow;
	keys[Gosu::ButtonName::kbUp] = KBUpArrow;
	keys[Gosu::ButtonName::kbDown] = KBDownArrow;
}

GosuGameWindow::~GosuGameWindow()
{
}

bool GosuGameWindow::init()
{
	return true;
}

int GosuGameWindow::width() const
{
	return (int)graphics().width();
}

int GosuGameWindow::height() const
{
	return (int)graphics().height();
}

void GosuGameWindow::setCaption(const std::string& caption)
{
	Gosu::Window::setCaption(Gosu::widen(caption));
}

void GosuGameWindow::buttonDown(const Gosu::Button btn)
{
	now = (int)Gosu::milliseconds();
	if (keystates.find(btn) == keystates.end()) {
		keystate& state = keystates[btn];
		state.since = now;
		state.initiallyResolved = false;
		state.consecutive = false;
	}

	// We process the initial buttonDown here so that it
	// gets handled even if we receive a buttonUp before an
	// update.
	auto mapped = gosuToTsunagariKey[btn.id()];
	if (mapped)
		emitKeyDown(mapped);
}

void GosuGameWindow::buttonUp(const Gosu::Button btn)
{
	keystates.erase(btn);

	auto mapped = gosuToTsunagariKey[btn.id()];
	if (mapped)
		emitKeyUp(mapped);
}

void GosuGameWindow::draw()
{
	World::instance().draw();
}

bool GosuGameWindow::needsRedraw() const
{
	return World::instance().needsRedraw();
}

void GosuGameWindow::update()
{
	now = time();

	if (conf.moveMode == TURN)
		handleKeyboardInput(now);
	World::instance().update(now);

	if (now > lastGCtime + GC_CALL_PERIOD) {
		lastGCtime = now;
		Reader::garbageCollect();
	}
}

void GosuGameWindow::mainLoop()
{
	show();
}

void GosuGameWindow::drawRect(double x1, double x2, double y1, double y2,
		unsigned int argb)
{
	Gosu::Color c(argb);
	double top = std::numeric_limits<double>::max();
	graphics().drawQuad(
		x1, y1, c,
		x2, y1, c,
		x2, y2, c,
		x1, y2, c,
		top
	);
}

void GosuGameWindow::scale(double x, double y)
{
	graphics().pushTransform(Gosu::scale(x, y));
}

void GosuGameWindow::translate(double x, double y)
{
	graphics().pushTransform(Gosu::translate(x, y));
}

void GosuGameWindow::clip(double x, double y, double width, double height)
{
	graphics().beginClipping(x, y, width, height);
}


void GosuGameWindow::handleKeyboardInput(time_t now)
{
	std::map<Gosu::Button, keystate>::iterator it;

	// Persistent input handling code
	for (it = keystates.begin(); it != keystates.end(); it++) {
		Gosu::Button btn = it->first;
		keystate& state = it->second;

		// If there is persistCons milliseconds of latency
		// between when a button is depressed and when we first look at
		// it here, we'll incorrectly try to fire off a second round of
		// input.
		// This can happen if an intermediary function blocks the thread
		// for a while.
		if (!state.initiallyResolved) {
			state.initiallyResolved = true;
			continue;
		}

		time_t delay = state.consecutive ?
		    conf.persistCons : conf.persistInit;
		if (now >= state.since + delay) {
			state.since = now;
			//World::instance().buttonDown(btn);
			state.consecutive = true;
		}
	}
}

