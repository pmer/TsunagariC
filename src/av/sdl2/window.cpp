/*************************************
** Tsunagari Tile Engine            **
** window.cpp                       **
** Copyright 2016-2017 Paul Merrill **
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

#include <chrono>
#include <thread>

#include "av/sdl2/error.h"
#include "core/client-conf.h"
#include "core/log.h"
#include "core/measure.h"
#include "core/world.h"

#define CHECK(x)  if (!(x)) { return false; }

static SDL2GameWindow globalWindow;

GameWindow* GameWindow::create() {
    return globalWindow.init() ? &globalWindow
                               : nullptr;
}

GameWindow& GameWindow::instance() {
    return globalWindow;
}

time_t GameWindow::time() {
    std::chrono::time_point<std::chrono::steady_clock> start = globalWindow.start;
    std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

SDL_Renderer* SDL2GetRenderer() {
    return globalWindow.renderer;
}


SDL2GameWindow::SDL2GameWindow()
    : window(nullptr),
      renderer(nullptr) {}

bool SDL2GameWindow::init() {
    {
        //TimeMeasure m("Initializing SDL2");
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) < 0) {
            sdlDie("SDL2GameWindow", "SDL_Init");
            return false;
        }
    }

    {
        //TimeMeasure m("Created SDL2 window");

        int width = conf.windowSize.x;
        int height = conf.windowSize.y;

        Uint32 flags = SDL_WINDOW_HIDDEN;
        if (conf.fullscreen) {
            flags |= SDL_WINDOW_FULLSCREEN;
        }

        window = SDL_CreateWindow("Tsunagari",
                                  SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED,
                                  width, height, flags);

        if (!window) {
            sdlDie("SDL2GameWindow", "SDL_CreateWindow");
            return false;
        }
    }

    {
        //TimeMeasure m("Created SDL2 renderer");

        Uint32 flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;

        renderer = SDL_CreateRenderer(window, -1, flags);

        if (!renderer) {
            sdlDie("SDL2GameWindow", "SDL_CreateRenderer");
            return false;
        }
    }

    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);

    return true;
}

unsigned SDL2GameWindow::width() const {
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    return static_cast<unsigned>(w);
}

unsigned SDL2GameWindow::height() const {
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    return static_cast<unsigned>(h);
}

void SDL2GameWindow::setCaption(const std::string& caption) {
    SDL_SetWindowTitle(window, caption.c_str());
}

void SDL2GameWindow::mainLoop() {
    SDL_ShowWindow(window);
    while (window != nullptr) {
        handleEvents();
        World::instance().update(time());
        World::instance().draw();
        SDL_RenderPresent(renderer);
    }
}

void SDL2GameWindow::handleEvents() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        handleEvent(event);
    }
}

void SDL2GameWindow::handleEvent(const SDL_Event& event) {
    switch (event.type) {
    case SDL_KEYDOWN: {
        switch (event.key.keysym.sym) {
        case SDLK_ESCAPE:
            //exit(0);
            break;
        }
        break;
    }
    case SDL_QUIT:
        exit(0);
        break;
    }
}

void SDL2GameWindow::drawRect(double x1, double x2, double y1, double y2,
              uint32_t argb) {}

void SDL2GameWindow::scale(double x, double y, std::function<void()> op) {}
void SDL2GameWindow::translate(double x, double y, std::function<void()> op) {}
void SDL2GameWindow::clip(double x, double y, double width, double height,
                          std::function<void()> op) {}

void SDL2GameWindow::close() {
    SDL_DestroyWindow(window);
    window = nullptr;
}
