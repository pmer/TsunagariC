/*************************************
** Tsunagari Tile Engine            **
** window.h                         **
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

#ifndef SRC_AV_SDL2_WINDOW_H_
#define SRC_AV_SDL2_WINDOW_H_

#include "av/sdl2/sdl2.h"
#include "core/vec.h"
#include "core/window.h"
#include "os/chrono.h"
#include "util/function.h"
#include "util/noexcept.h"
#include "util/transform.h"

class SDL2GameWindow {
 public:
    static SDL_Renderer* renderer;
    static rvec2 translation;
    static rvec2 scaling;
};

#endif  // SRC_AV_SDL2_WINDOW_H_
