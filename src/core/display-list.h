/*************************************
** Tsunagari Tile Engine            **
** display-list.h                   **
** Copyright 2018-2019 Paul Merrill **
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

#ifndef SRC_CORE_DISPLAY_LIST_H_
#define SRC_CORE_DISPLAY_LIST_H_

#include "core/images.h"
#include "core/vec.h"
#include "util/vector.h"

struct DisplayItem {
    Image* image;       // TODO: Move to ImageKey.
    rvec2 destination;  // TODO: Move to float.
};

struct DisplayList {
    bool loopX, loopY;

    rvec2 padding;
    rvec2 scale;
    rvec2 scroll;
    rvec2 size;

    Vector<DisplayItem> items;

    uint32_t colorOverlayARGB;
    bool paused;  // TODO: Move to colorOverlay & overlay.
};

void displayListPresent(DisplayList* display);

#endif  // SRC_CORE_DISPLAY_LIST_H_
