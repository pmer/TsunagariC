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

#ifndef SRC_CORE_CLIENT_CONF_H_
#define SRC_CORE_CLIENT_CONF_H_

#include "core/log.h"
#include "core/vec.h"
#include "util/int.h"
#include "util/string-view.h"

//! Engine-wide user-confurable values.
struct Conf {
    //! Game Movement Mode
    enum MovementMode { TURN, TILE, NOTILE };

	static verbosity_t verbosity;
    static MovementMode moveMode;
    static ivec2 windowSize;
    static bool fullscreen;
    static int musicVolume;
    static int soundVolume;
    static time_t cacheTTL;
    static int persistInit;
    static int persistCons;

	static bool parse(StringView filename) noexcept;
};

#endif  // SRC_CORE_CLIENT_CONF_H_
