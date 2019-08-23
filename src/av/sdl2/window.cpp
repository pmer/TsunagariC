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
#include "core/window.h"
#include "core/world.h"
#include "os/chrono.h"
#include "util/function.h"
#include "util/noexcept.h"
#include "util/optional.h"
#include "util/transform.h"

SDL_Renderer* SDL2GameWindow::renderer = nullptr;
rvec2 SDL2GameWindow::translation = {0.0, 0.0};
rvec2 SDL2GameWindow::scaling = {0.0, 0.0};

static TimePoint start = SteadyClock::nowMS();

static SDL_Window* window = nullptr;
static Transform transform = Transform::identity();

static int
getRefreshRate(SDL_Window* window) noexcept {
    // SDL_GetWindowDisplayIndex computes which display the window is on each
    // time.
    int display = SDL_GetWindowDisplayIndex(window);
    SDL_DisplayMode mode;
    SDL_GetCurrentDisplayMode(display, &mode);
    return mode.refresh_rate;
}

static void
handleEvent(const SDL_Event& event) noexcept {
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
            GameWindow::emitKeyUp(key);
        }
        else if (event.type == SDL_KEYDOWN) {
            GameWindow::emitKeyDown(key);
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

static void
handleEvents() noexcept {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        handleEvent(event);
    }
}

static void
updateTransform() noexcept {
    int w, h;

    SDL_GetWindowSize(window, &w, &h);

    float xScale = transform[0];
    float yScale = transform[5];
    float x = transform[12];
    float y = transform[13];

    SDL2GameWindow::translation = {x / xScale, y / yScale};
    SDL2GameWindow::scaling = {xScale, yScale};
}

time_t
GameWindow::time() noexcept {
    return SteadyClock::nowMS() - start;
}

void
GameWindow::create() noexcept {
    {
        TimeMeasure m("Initialized the SDL2 video subsystem");
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            sdlDie("SDL2GameWindow", "SDL_Init(SDL_INIT_VIDEO)");
        }
    }

    {
        TimeMeasure m("Created SDL2 window");

        int width = Conf::windowSize.x;
        int height = Conf::windowSize.y;

        uint32_t flags = SDL_WINDOW_HIDDEN;
        if (Conf::fullscreen) {
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
        }
    }

    {
        TimeMeasure m("Created SDL2 renderer");

        SDL2GameWindow::renderer = SDL_CreateRenderer(
                window,
                -1,
                SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

        if (SDL2GameWindow::renderer == nullptr) {
            sdlDie("SDL2GameWindow", "SDL_CreateRenderer");
        }
    }

    SDL_RendererInfo info;

    if (SDL_GetRendererInfo(SDL2GameWindow::renderer, &info) < 0) {
        sdlDie("SDL2GameWindow", "SDL_GetRendererInfo");
    }

    StringView name = info.name;
    bool vsync = info.flags & SDL_RENDERER_PRESENTVSYNC;

    Log::info("SDL2GameWindow",
              String("Rendering will be done with ")
                      << name
                      << (vsync ? " with vsync" : " without vsync"));

    SDL_SetRenderDrawColor(SDL2GameWindow::renderer, 0x00, 0x00, 0x00, 0xFF);
}

unsigned
GameWindow::width() noexcept {
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    return static_cast<unsigned>(w);
}

unsigned
GameWindow::height() noexcept {
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    return static_cast<unsigned>(h);
}

void
GameWindow::setCaption(StringView caption) noexcept {
    SDL_SetWindowTitle(window, String(caption).null());
}

void
GameWindow::mainLoop() noexcept {
    SDL_ShowWindow(window);

    DisplayList display;

    int refreshRate = getRefreshRate(window);
    const Duration idealFrameTime = s_to_ns(1) / refreshRate;

    // Block until the start of a frame.
    SDL_SetRenderDrawColor(SDL2GameWindow::renderer, 0, 0, 0, 0xFF);
    SDL_RenderClear(SDL2GameWindow::renderer);
    SDL_RenderPresent(SDL2GameWindow::renderer);

    TimePoint frameStart = SteadyClock::now();
    TimePoint previousFrameStart =
            frameStart - idealFrameTime;  // Bogus initial value.

    TimePoint nextFrameStart = frameStart + idealFrameTime;

    while (window != nullptr) {
        handleEvents();

        //
        // Simulate world and draw frame.
        //
        time_t dt = ns_to_ms(frameStart - previousFrameStart);

        assert_(dt >= 0);

        if (dt > 0) {
            World::tick(dt);
        } else {
            //Log::info("SDL2GameWindow", "dt == 0");
        }

        bool drew = false;
        if (World::needsRedraw()) {
            drew = true;

            World::draw(&display);

            SDL_SetRenderDrawColor(SDL2GameWindow::renderer, 0, 0, 0, 0xFF);
            SDL_RenderClear(SDL2GameWindow::renderer);
            displayListPresent(&display);
            SDL_RenderPresent(SDL2GameWindow::renderer);

            display.items.clear();
        }

        TimePoint frameEnd = SteadyClock::now();
        Duration timeTaken = frameEnd - frameStart;

        //
        // Sleep until next frame.
        //
        Duration sleepDuration = nextFrameStart - frameEnd;
        if (sleepDuration < 0) {
            sleepDuration = 0;
        }

        if (!drew && sleepDuration) {
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
}

void
GameWindow::drawRect(float x1,
                     float x2,
                     float y1,
                     float y2,
                     uint32_t argb) noexcept {
    auto a = static_cast<uint8_t>((argb >> 24) & 0xFF);
    auto r = static_cast<uint8_t>((argb >> 16) & 0xFF);
    auto g = static_cast<uint8_t>((argb >> 8) & 0xFF);
    auto b = static_cast<uint8_t>((argb >> 0) & 0xFF);

    SDL_Rect rect{static_cast<int>(x1),
                  static_cast<int>(y1),
                  static_cast<int>(x2 - x1),
                  static_cast<int>(y2 - y1)};

    SDL_SetRenderDrawColor(SDL2GameWindow::renderer, r, g, b, a);
    SDL_SetRenderDrawBlendMode(SDL2GameWindow::renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect(SDL2GameWindow::renderer, &rect);
}

void
GameWindow::scale(float x, float y, Function<void()> op) noexcept {
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
GameWindow::translate(float x, float y, Function<void()> op) noexcept {
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
GameWindow::clip(float x,
                 float y,
                 float width,
                 float height,
                 Function<void()> op) noexcept {
    op();
}

void
GameWindow::close() noexcept {
    SDL_DestroyWindow(window);
    window = nullptr;
}
