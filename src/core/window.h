/***************************************
** Tsunagari Tile Engine              **
** window.h                           **
** Copyright 2011-2015 Michael Reiley **
** Copyright 2011-2019 Paul Merrill   **
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

#ifndef SRC_CORE_WINDOW_H_
#define SRC_CORE_WINDOW_H_

#include "core/keyboard.h"
#include "util/bitrecord.h"
#include "util/function.h"
#include "util/string-view.h"

// This class is structurally the main class of the Tsunagari Tile Engine.
// It handles input and drawing.
class GameWindow {
 public:
    static void create() noexcept;

    //! Time since epoch.
    static time_t time() noexcept;

    //! Width of the window in pixels.
    static int width() noexcept;

    //! Height of the window in pixels.
    static int height() noexcept;

    //! Set window manager caption.
    static void setCaption(StringView caption) noexcept;

    //! Show the window and start the main loop.
    static void mainLoop() noexcept;

    /**
     * Draws a rectangle on the screen of the specified color. Coordinates
     * are in virtual pixels.
     */
    static void drawRect(float x1,
                         float x2,
                         float y1,
                         float y2,
                         uint32_t argb) noexcept;

    static void scale(float x, float y, Function<void()> op) noexcept;
    static void translate(float x,
                          float y,
                          Function<void()> op) noexcept;
    static void clip(float x,
                     float y,
                     float width,
                     float height,
                     Function<void()> op) noexcept;

    static void emitKeyDown(KeyboardKey key) noexcept;
    static void emitKeyUp(KeyboardKey key) noexcept;
    static void close() noexcept;

    static BitRecord keysDown;
};

#endif  // SRC_CORE_WINDOW_H_
