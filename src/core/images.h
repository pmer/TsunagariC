/***************************************
** Tsunagari Tile Engine              **
** images.h                           **
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

#ifndef SRC_CORE_IMAGES_H_
#define SRC_CORE_IMAGES_H_

#include "util/int.h"
#include "util/markable.h"
#include "util/string-view.h"

typedef Markable<int,-1> TiledImageID;
typedef Markable<int,-1> ImageID;

class Images {
 public:
    // Load an image from the file at the given path.
    static ImageID load(StringView path) noexcept;

    // Load an image of tiles from the file at the given path. Each tile
    // with width and heigh as specified.
    static TiledImageID loadTiles(StringView path,
                                  int tileWidth,
                                  int tileHeight) noexcept;

    // Free images not recently used.
    static void prune(time_t latestPermissibleUse) noexcept;
};

class TiledImage {
 public:
    static int size(TiledImageID tiid) noexcept;

    static ImageID getTile(TiledImageID tiid, int i) noexcept;

    static void release(TiledImageID tiid) noexcept;
};

class Image {
 public:
    static void draw(ImageID iid, float x, float y, float z) noexcept;

    static int width(ImageID iid) noexcept;
    static int height(ImageID iid) noexcept;

    static void release(ImageID iid) noexcept;
};

#endif  // SRC_CORE_IMAGES_H_
