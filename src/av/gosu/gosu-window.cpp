/***************************************
** Tsunagari Tile Engine              **
** gosu-window.cpp                    **
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

#include <Gosu/Graphics.hpp>  // for Gosu::Graphics
#include <Gosu/Timing.hpp>
#include <Gosu/Utility.hpp>

#include "av/gosu/gosu-window.h"

#include "core/client-conf.h"
#include "core/world.h"

#define CHECK(x)  if (!(x)) { return false; }

// Garbage collection called every X milliseconds
#define GC_CALL_PERIOD 10 * 1000

#ifdef BACKEND_GOSU
static GosuGameWindow* globalWindow = nullptr;

GameWindow* GameWindow::create()
{
    globalWindow = new GosuGameWindow();
    return globalWindow;
}

GameWindow& GameWindow::instance()
{
    return *globalWindow;
}

time_t GameWindow::time()
{
    return (time_t)Gosu::milliseconds();
}
#endif

namespace Gosu {
    /**
     * Enable 1980s-style graphics scaling: nearest-neighbor filtering.
     * Call this function before creating any Gosu::Image.
     */
    void enableUndocumentedRetrofication() {
        extern bool undocumented_retrofication;
        undocumented_retrofication = true;
    }
}

GosuGameWindow::GosuGameWindow()
    // Gosu emulates the requested screen resolution on fullscreen,
    // but this breaks our aspect ratio-correcting letterbox.
    // Ergo we just make a window the size of the screen.
    : Gosu::Window(
        conf.fullscreen ? Gosu::screen_width() :
                          (unsigned)conf.windowSize.x,
        conf.fullscreen ? Gosu::screen_height() :
                          (unsigned)conf.windowSize.y,
        conf.fullscreen
      ),
      now(this->time()),
      lastGCtime(0)
{
    Gosu::enableUndocumentedRetrofication();

    gosuToTsunagariKey.resize(Gosu::ButtonName::NUM_BUTTONS);
    auto& keys = gosuToTsunagariKey;
    keys[Gosu::ButtonName::KB_ESCAPE] = KBEscape;
    keys[Gosu::ButtonName::KB_LEFT_SHIFT] = KBLeftShift;
    keys[Gosu::ButtonName::KB_RIGHT_SHIFT] = KBRightShift;
    keys[Gosu::ButtonName::KB_LEFT_CONTROL] = KBLeftControl;
    keys[Gosu::ButtonName::KB_RIGHT_CONTROL] = KBRightControl;
    keys[Gosu::ButtonName::KB_SPACE] = KBSpace;
    keys[Gosu::ButtonName::KB_LEFT] = KBLeftArrow;
    keys[Gosu::ButtonName::KB_RIGHT] = KBRightArrow;
    keys[Gosu::ButtonName::KB_UP] = KBUpArrow;
    keys[Gosu::ButtonName::KB_DOWN] = KBDownArrow;
}

unsigned GosuGameWindow::width() const
{
    return graphics().width();
}

unsigned GosuGameWindow::height() const
{
    return graphics().height();
}

void GosuGameWindow::setCaption(const std::string& caption)
{
    Gosu::Window::set_caption(caption);
}

void GosuGameWindow::button_down(const Gosu::Button btn)
{
    now = this->time();
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
    if (mapped) {
        emitKeyDown(mapped);
    }
}

void GosuGameWindow::button_up(const Gosu::Button btn)
{
    keystates.erase(btn);

    auto mapped = gosuToTsunagariKey[btn.id()];
    if (mapped) {
        emitKeyUp(mapped);
    }
}

void GosuGameWindow::draw()
{
    World::instance().draw();
}

bool GosuGameWindow::needs_redraw() const
{
    return World::instance().needsRedraw();
}

void GosuGameWindow::update()
{
    now = this->time();

    if (conf.moveMode == TURN) {
        handleKeyboardInput(now);
    }
    World::instance().update(now);

    if (now > lastGCtime + GC_CALL_PERIOD) {
        lastGCtime = now;
        World::instance().garbageCollect();
    }
}

void GosuGameWindow::mainLoop()
{
    show();
}

void GosuGameWindow::drawRect(double x1, double x2, double y1, double y2,
        uint32_t argb)
{
    Gosu::Color c(argb);
    double top = std::numeric_limits<double>::max();
    graphics().draw_quad(
        x1, y1, c,
        x2, y1, c,
        x2, y2, c,
        x1, y2, c,
        top
    );
}

void GosuGameWindow::scale(double x, double y)
{
    graphics().push_transform(Gosu::scale(x, y));
}

void GosuGameWindow::translate(double x, double y)
{
    graphics().push_transform(Gosu::translate(x, y));
}

void GosuGameWindow::clip(double x, double y, double width, double height)
{
    graphics().begin_clipping(x, y, width, height);
}

void GosuGameWindow::close()
{
    Gosu::Window::close();
}


void GosuGameWindow::handleKeyboardInput(time_t now)
{
    std::map<Gosu::Button, keystate>::iterator it;

    // Persistent input handling code
    for (it = keystates.begin(); it != keystates.end(); it++) {
        Gosu::Button btn = it->first;
        auto mapped = gosuToTsunagariKey[btn.id()];
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
            state.since += delay;
            World::instance().buttonDown(mapped);
            state.consecutive = true;
        }
    }
}
