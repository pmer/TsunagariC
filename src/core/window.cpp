/***************************************
** Tsunagari Tile Engine              **
** window.cpp                         **
** Copyright 2011-2014 PariahSoft LLC **
** Copyright 2016 Paul Merrill        **
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

#include <stdlib.h>

#include "core/window.h"
#include "core/world.h"

GameWindow::GameWindow()
    : keysDown(KB_SIZE)
{
}

GameWindow::~GameWindow()
{
}

void GameWindow::emitKeyDown(KeyboardKey key)
{
    keysDown[key]++;

    if (keysDown[KBEscape] &&
            (keysDown[KBLeftShift] || keysDown[KBRightShift])) {
        exit(0);
    }

    if (keysDown[key])
        World::instance().buttonDown(key);
}

void GameWindow::emitKeyUp(KeyboardKey key)
{
    keysDown[key]--;

    if (!keysDown[key])
        World::instance().buttonUp(key);
}

bool GameWindow::isKeyDown(KeyboardKey key)
{
    return keysDown[key] != false;
}

BitRecord GameWindow::getKeysDown()
{
    return keysDown;
}
