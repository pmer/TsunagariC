/***************************************
** Tsunagari Tile Engine              **
** gosu-window.h                      **
** Copyright 2011-2013 PariahSoft LLC **
** Copyright 2016      Paul Merrill   **
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

#ifndef GOSU_WINDOW_H
#define GOSU_WINDOW_H

#include <map>
#include <string>

#include <Gosu/Window.hpp>  // for Gosu::Window

#include "core/window.h"

namespace Gosu {
    class Button;
}

class GosuGameWindow : public GameWindow, public Gosu::Window
{
public:
    GosuGameWindow();
    ~GosuGameWindow() = default;

    unsigned width() const;

    unsigned height() const;

    void setCaption(const std::string& caption);

    //! Gosu Callback
    void button_down(const Gosu::Button btn);

    //! Gosu Callback
    void button_up(const Gosu::Button btn);

    //! Gosu Callback
    void draw();

    //! Gosu Callback
    bool needs_redraw() const;

    //! Gosu Callback
    void update();

    void mainLoop();

        void drawRect(double x1, double x2, double y1, double y2,
                uint32_t argb);

    void scale(double x, double y);
    void translate(double x, double y);
    void clip(double x, double y, double width, double height);

    void close();

protected:
    //! Process persistent keyboard input
    void handleKeyboardInput(time_t now);

    time_t now;
    time_t lastGCtime;

    struct keystate {
        bool consecutive, initiallyResolved;
        time_t since;
    };

    std::map<Gosu::Button, keystate> keystates;
    std::vector<KeyboardKey> gosuToTsunagariKey;
};

#endif
