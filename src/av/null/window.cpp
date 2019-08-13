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
    TimePoint last = 0;

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
#endif
    while (true) {
        TimePoint now = SteadyClock::now();
        time_t time = ns_to_ms(now - last);

        World::update(static_cast<time_t>(time));
        DisplayList dl;
        World::draw(&dl);

        TimePoint nextFrame = last + s_to_ns(1) / 60;
        SleepFor(nextFrame - SteadyClock::now());

        last = now;
    }
#ifdef __clang__
#pragma clang diagnostic pop
#endif
}

void
GameWindow::drawRect(double, double, double, double, uint32_t) noexcept {}

void
GameWindow::scale(double, double, Function<void()>) noexcept {}
void
GameWindow::translate(double, double, Function<void()>) noexcept {}
void
GameWindow::clip(double, double, double, double, Function<void()>) noexcept {}

void
GameWindow::close() noexcept {}
