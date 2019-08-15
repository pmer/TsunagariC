/*************************************
** Tsunagari Tile Engine            **
** window.cpp                       **
** Copyright 2016-2019 Paul Merrill **
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

#include "core/window.h"

#include "core/client-conf.h"
#include "core/display-list.h"
#include "core/log.h"
#include "core/world.h"
#include "os/chrono.h"
#include "os/thread.h"
#include "util/string-view.h"

void
GameWindow::create() noexcept {}

time_t
GameWindow::time() noexcept {
    TimePoint now = SteadyClock::now();
    return ns_to_ms(now);
}

unsigned
GameWindow::width() noexcept {
    return static_cast<unsigned>(Conf::windowSize.x);
}

unsigned
GameWindow::height() noexcept {
    return static_cast<unsigned>(Conf::windowSize.y);
}

void
GameWindow::setCaption(StringView) noexcept {}

void
GameWindow::mainLoop() noexcept {
    DisplayList dl;

    const Duration idealFrameTime = s_to_ns(1) / 60;

    TimePoint frameStart = SteadyClock::now();
    TimePoint previousFrameStart =
        frameStart - idealFrameTime;  // Bogus initial value.

    // FIXME: Should be set to right after a frame is uploaded and we can begin
    //        drawing the next frame.
    //
    //        If we get stuck sleeping until right before the monitor refresh,
    //        then our update() and draw() functions are slow, we are at risk
    //        of dropping frames. In the pathological case, we only update the
    //        screen at 1/2 the monitor refresh rate.
    //
    //        How can we detect when a frame is uploaded so we can wake up
    //        after that point?
    //
    // NOTE:  Not too important for the null av port, but for other ports yes.
    TimePoint nextFrameStart = frameStart + idealFrameTime;

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
#endif
    while (true) {
        time_t dt = ns_to_ms(frameStart - previousFrameStart);

        World::tick(static_cast<time_t>(dt));
        World::draw(&dl);

        // Do nothing with the filled DisplayList.

        dl.items.clear();

        TimePoint frameEnd = SteadyClock::now();
        Duration timeTaken = frameEnd - frameStart;

        Duration sleepDuration = nextFrameStart - frameEnd;
        if (sleepDuration < 0) {
            sleepDuration = 0;
        }

        if (sleepDuration) {
            SleepFor(sleepDuration);
        }

        previousFrameStart = frameStart;
        frameStart = SteadyClock::now();
        nextFrameStart += idealFrameTime;

        if (frameStart > nextFrameStart) {
            int framesDropped = 0;
            while (frameStart > nextFrameStart) {
                nextFrameStart += idealFrameTime;
                framesDropped += 1;
            }
            Log::info("GameWindow",
                      String() << "Dropped " << framesDropped << " frames");
        }
    }
#ifdef __clang__
#pragma clang diagnostic pop
#endif
}

void
GameWindow::drawRect(float, float, float, float, uint32_t) noexcept {}

void
GameWindow::scale(float, float, Function<void()>) noexcept {}
void
GameWindow::translate(float, float, Function<void()>) noexcept {}
void
GameWindow::clip(float, float, float, float, Function<void()>) noexcept {}

void
GameWindow::close() noexcept {}
