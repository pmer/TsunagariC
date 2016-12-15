/***************************************
** Tsunagari Tile Engine              **
** window.h                           **
** Copyright 2011-2015 Michael Reiley **
** Copyright 2011-2016 Paul Merrill   **
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

#include <map>
#include <string>

#include "util/bitrecord.h"

enum KeyboardKey {
    KBEscape = 1,
    KBLeftControl,
    KBRightControl,
    KBLeftShift,
    KBRightShift,
    KBSpace,
    KBLeftArrow,
    KBRightArrow,
    KBUpArrow,
    KBDownArrow,
    KB_SIZE,
};

//! GameWindow Class
/*!
    This class is structurally the main class of the Tsunagari Tile Engine.
    It handles input and drawing.
*/
class GameWindow {
 public:
    static GameWindow* create();
    static GameWindow& instance();

    //! Time since epoch.
    static time_t time();

    virtual ~GameWindow() = default;

    //! GameWindow Initializer
    virtual bool init() = 0;

    //! Width of the window in pixels.
    virtual unsigned width() const = 0;

    //! Height of the window in pixels.
    virtual unsigned height() const = 0;

    //! Set window manager caption.
    virtual void setCaption(const std::string& caption) = 0;

    //! Show the window and start the main loop.
    virtual void mainLoop() = 0;

    /**
     * Draws a rectangle on the screen of the specified color. Coordinates
     * are in virtual pixels.
     */
    virtual void drawRect(double x1, double x2, double y1, double y2,
        uint32_t argb) = 0;

    virtual void scale(double x, double y) = 0;
    virtual void translate(double x, double y) = 0;
    virtual void clip(double x, double y, double width, double height) = 0;

    void emitKeyDown(KeyboardKey key);
    void emitKeyUp(KeyboardKey key);
    bool isKeyDown(KeyboardKey key);
    BitRecord getKeysDown();

 protected:
    GameWindow();

    virtual void close() = 0;

    BitRecord keysDown;

};

#endif  // SRC_CORE_WINDOW_H_
