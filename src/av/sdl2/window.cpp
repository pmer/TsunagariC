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

#include "av/sdl2/window.h"

#include "av/sdl2/error.h"
#include "av/sdl2/sdl2.h"
#include "core/client-conf.h"
#include "core/display-list.h"
#include "core/log.h"
#include "core/measure.h"
#include "core/world.h"
#include "util/optional.h"

#define CHECK(x)      \
    if (!(x)) {       \
        return false; \
    }

static SDL2GameWindow globalWindow;

GameWindow*
GameWindow::create() noexcept {
    return globalWindow.init() ? &globalWindow : nullptr;
}

GameWindow&
GameWindow::instance() noexcept {
    return globalWindow;
}

SDL2GameWindow&
SDL2GameWindow::instance() noexcept {
    return globalWindow;
}

time_t
GameWindow::time() noexcept {
    TimePoint start = globalWindow.start;
    TimePoint end = SteadyClock::nowMS();
    return end - start;
}

SDL2GameWindow::SDL2GameWindow() noexcept
        : start(SteadyClock::nowMS()),
          renderer(nullptr),
          translation{0.0, 0.0},
          scaling{0.0, 0.0},
          window(nullptr),
          transform(Transform::identity()) {}

bool
SDL2GameWindow::init() noexcept {
    {
        // TimeMeasure m("Initializing SDL2");
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            sdlDie("SDL2GameWindow", "SDL_Init");
            return false;
        }
    }

    {
        TimeMeasure m("Created SDL2 window");

        int width = conf.windowSize.x;
        int height = conf.windowSize.y;

        uint32_t flags = SDL_WINDOW_HIDDEN;
        if (conf.fullscreen) {
            flags |= SDL_WINDOW_FULLSCREEN;
        }

        window = SDL_CreateWindow("Tsunagari",
                                  SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED,
                                  width,
                                  height,
                                  flags);

        if (window == nullptr) {
            sdlDie("SDL2GameWindow", "SDL_CreateWindow");
            return false;
        }
    }

    {
        TimeMeasure m("Created SDL2 renderer");

        renderer = SDL_CreateRenderer(
                window,
                -1,
                SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

        if (renderer == nullptr) {
            sdlDie("SDL2GameWindow", "SDL_CreateRenderer");
            return false;
        }

        SDL_RendererInfo info;

        if (SDL_GetRendererInfo(renderer, &info) < 0) {
            sdlDie("SDL2GameWindow", "SDL_GetRendererInfo");
            return false;
        }

        StringView name = info.name;
        bool vsync = info.flags & SDL_RENDERER_PRESENTVSYNC;

        Log::info("SDL2GameWindow",
                  String("Rendering will be done with ")
                          << name
                          << (vsync ? " with vsync" : " without vsync"));
    }

    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);

    return true;
}

unsigned
SDL2GameWindow::width() const noexcept {
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    return static_cast<unsigned>(w);
}

unsigned
SDL2GameWindow::height() const noexcept {
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    return static_cast<unsigned>(h);
}

void
SDL2GameWindow::setCaption(StringView caption) noexcept {
    SDL_SetWindowTitle(window, String(caption).null());
}

static int
getRefreshRate(SDL_Window* window) noexcept {
    // SDL_GetWindowDisplayIndex computes which display the window is on each
    // time.
    int display = SDL_GetWindowDisplayIndex(window);
    SDL_DisplayMode mode;
    SDL_GetCurrentDisplayMode(display, &mode);
    return mode.refresh_rate;
}

void
SDL2GameWindow::mainLoop() noexcept {
    DisplayList display;

    SDL_ShowWindow(window);

    bool redrew = false;
    time_t lastTime = 0;
    bool presented = false;
    bool slept = false;

	time_t lastTen[10] = {};

    while (window != nullptr) {
        handleEvents();

        time_t now = time();
        if (now == lastTime) {
            Log::info("SDL2GameWindow",
                      String("dt will be 0")
                              << " redrew " << redrew << " presented " << redrew
                              << " slept " << slept);
        }
        lastTime = now;

        World::instance().update(time());

        if (World::instance().needsRedraw()) {
            redrew = true;
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
            SDL_RenderClear(renderer);

            display.items.clear();
            World::instance().draw(&display);
            displayListPresent(&display);

            SDL_RenderPresent(renderer);

            presented = true;
            slept = false;

            // TODO: Detect dropped frames.
        }
        else {
            redrew = false;
            // TODO: Question: How do we handle variable refresh rate screens?
            Duration frameLength{s_to_ns(1) / getRefreshRate(window)};
            SleepFor(frameLength);

            presented = false;
            slept = true;
        }
    }
}

void
SDL2GameWindow::handleEvents() noexcept {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        handleEvent(event);
    }
}

void
SDL2GameWindow::handleEvent(const SDL_Event& event) noexcept {
    KeyboardKey key;

    switch (event.type) {
    case SDL_KEYUP:
    case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
        case SDLK_ESCAPE:
            key = KBEscape;
            break;
        case SDLK_LCTRL:
            key = KBLeftControl;
            break;
        case SDLK_RCTRL:
            key = KBRightControl;
            break;
        case SDLK_LSHIFT:
            key = KBLeftShift;
            break;
        case SDLK_RSHIFT:
            key = KBRightShift;
            break;
        case SDLK_SPACE:
            key = KBSpace;
            break;
        case SDLK_LEFT:
            key = KBLeftArrow;
            break;
        case SDLK_RIGHT:
            key = KBRightArrow;
            break;
        case SDLK_UP:
            key = KBUpArrow;
            break;
        case SDLK_DOWN:
            key = KBDownArrow;
            break;
        default:
            return;
        }
        if (event.type == SDL_KEYUP) {
            emitKeyUp(key);
        }
        else if (event.type == SDL_KEYDOWN) {
            emitKeyDown(key);
        }
        break;

    case SDL_QUIT:
        SDL_DestroyWindow(window);
        exit(0);
        return;

    default:
        return;
    }
}

void
SDL2GameWindow::drawRect(double x1,
                         double x2,
                         double y1,
                         double y2,
                         uint32_t argb) noexcept {
    auto a = static_cast<uint8_t>((argb >> 24) & 0xFF);
    auto r = static_cast<uint8_t>((argb >> 16) & 0xFF);
    auto g = static_cast<uint8_t>((argb >> 8) & 0xFF);
    auto b = static_cast<uint8_t>((argb >> 0) & 0xFF);

    SDL_Rect rect{static_cast<int>(x1),
                  static_cast<int>(y1),
                  static_cast<int>(x2 - x1),
                  static_cast<int>(y2 - y1)};

    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect(renderer, &rect);
}

void
SDL2GameWindow::scale(double x, double y, Function<void()> op) noexcept {
    assert_(x == y);

    Transform prev = transform;

    auto factor = static_cast<float>(x);

    transform = Transform::scale(factor) * transform;
    updateTransform();

    op();

    transform = prev;
    updateTransform();
}

void
SDL2GameWindow::translate(double x, double y, Function<void()> op) noexcept {
    Transform prev = transform;

    transform =
            Transform::translate(static_cast<float>(x), static_cast<float>(y)) *
            transform;
    updateTransform();

    op();

    transform = prev;
    updateTransform();
}

void
SDL2GameWindow::clip(double x,
                     double y,
                     double width,
                     double height,
                     Function<void()> op) noexcept {
    op();
}

void
SDL2GameWindow::close() noexcept {
    SDL_DestroyWindow(window);
    window = nullptr;
}

void
SDL2GameWindow::updateTransform() noexcept {
    int w, h;

    SDL_GetWindowSize(window, &w, &h);

    double xScale = transform[0];
    double yScale = transform[5];
    double x = transform[12];
    double y = transform[13];

    translation = {x / xScale, y / yScale};
    scaling = {xScale, yScale};
}
