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

#include <chrono>
#include <thread>

#include "core/client-conf.h"
#include "core/display-list.h"
#include "core/world.h"
#include "util/string-view.h"

class NullGameWindow : public GameWindow {
 public:
    bool init() { return true; }

    unsigned width() const final {
        return static_cast<unsigned>(conf.windowSize.x);
    }

    unsigned height() const final {
        return static_cast<unsigned>(conf.windowSize.y);
    }

    void setCaption(StringView) final {}

    void mainLoop() final {
        using namespace std::chrono;

        time_point<steady_clock> last;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
        while (true) {
            auto now = steady_clock::now();
            double time = duration_cast<milliseconds>(now - last).count();

            World::instance().update(static_cast<time_t>(time));
            DisplayList dl;
            World::instance().draw(&dl);

            auto nextFrame = last + microseconds(1000000) / 60;
            std::this_thread::sleep_until(nextFrame);

            last = now;
        }
#pragma clang diagnostic pop
    }

    void drawRect(double, double, double, double, uint32_t) final {}

    void scale(double, double, Function<void()>) final {}
    void translate(double, double, Function<void()>) final {}
    void clip(double, double, double, double, Function<void()>) final {}

    void close() final {}
};


static NullGameWindow globalWindow;

GameWindow*
GameWindow::create() {
    return &globalWindow;
}

GameWindow&
GameWindow::instance() {
    return globalWindow;
}


time_t
GameWindow::time() {
    using namespace std::chrono;

    auto now = steady_clock::now();
    return now.time_since_epoch().count() / 1000000;
}
