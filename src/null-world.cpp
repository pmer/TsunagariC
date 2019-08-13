/********************************
** Tsunagari Tile Engine       **
** null-world.cpp              **
** Copyright 2019 Paul Merrill **
********************************/

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

#include "null-world.h"

DataWorld&
DataWorld::instance() noexcept {
    static auto globalNullDataWorld = new NullDataWorld;
    return *globalNullDataWorld;
}

NullDataWorld::NullDataWorld() noexcept {
    about.name = "Null World";
    about.author = "Paul Merrill";
    about.version = "1";

    parameters.moveMode = Conf::TILE;

    parameters.viewportResolution = {240, 160};

    parameters.input.persistDelay.initial = 300;
    parameters.input.persistDelay.consecutive = 100;

    parameters.gameStart.player.file = "null-player.json";
    parameters.gameStart.player.phase = "down";

    parameters.gameStart.area = "null-area.json";
    parameters.gameStart.coords = {0, 0, 0};
}

bool
NullDataWorld::init() noexcept {
    return true;
}
