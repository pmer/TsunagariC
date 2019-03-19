/***************************************
** Tsunagari Tile Engine              **
** client-conf.h                      **
** Copyright 2011-2013 Michael Reiley **
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

#ifndef CLIENT_CONF_H
#define CLIENT_CONF_H

#include "core/log.h"
#include "core/vec.h"
#include "util/string-view.h"

//! Game Movement Mode
enum movement_mode_t { TURN, TILE, NOTILE };

//! Engine-wide user-confurable values.
struct Conf {
    verbosity_t verbosity = V_VERBOSE;
    movement_mode_t moveMode;
    ivec2 windowSize = {640, 480};
    bool fullscreen = false;
    int musicVolume = 100;
    int soundVolume = 100;
    int cacheTTL = 300;
    int persistInit = 0;
    int persistCons = 0;
};
extern Conf conf;

bool parseConfig(StringView filename);

#endif
