/***************************************
** Tsunagari Tile Engine              **
** client-conf.h                      **
** Copyright 2011-2013 Michael Reiley **
** Copyright 2011-2018 Paul Merrill   **
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

#include <string>

#include "core/log.h"
#include "core/vec.h"

//! Game Movement Mode
enum movement_mode_t {
    TURN,
    TILE,
    NOTILE
};

//! Engine-wide user-confurable values.
struct Conf {
    Conf();

    verbosity_t verbosity;
    movement_mode_t moveMode;
    ivec2 windowSize;
    bool fullscreen;
    int musicVolume;
    int soundVolume;
    bool cacheEnabled;
    int cacheTTL;
    int persistInit;
    int persistCons;
};
extern Conf conf;

bool parseConfig(const std::string& filename);
bool parseCommandLine(int argc, char* argv[]);

#endif
