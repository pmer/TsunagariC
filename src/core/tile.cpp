/***************************************
** Tsunagari Tile Engine              **
** tile.cpp                           **
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

#include "core/tile.h"

#include "core/area.h"
#include "core/log.h"
#include "core/world.h"
#include "util/string.h"

static int
ivec2_to_dir(ivec2 v) noexcept {
    switch (v.x) {
    case -1:
        return v.y == 0 ? EXIT_LEFT : -1;
    case 0:
        switch (v.y) {
        case -1:
            return EXIT_UP;
        case 0:
            return EXIT_NORMAL;
        case 1:
            return EXIT_DOWN;
        default:
            return -1;
        }
        break;
    case 1:
        return v.y == 0 ? EXIT_RIGHT : -1;
    default:
        return -1;
    }
}


/*
 * TILE
 */
icoord
Tile::moveDest(Area* area, icoord here, ivec2 facing) const noexcept {
    icoord dest = here + icoord{facing.x, facing.y, 0};

    Optional<double> layermod = layermodAt(facing);
    if (layermod) {
        dest = area->grid.virt2phys(vicoord{dest.x, dest.y, *layermod});
    }
    return dest;
}

const Optional<Exit>&
Tile::exitAt(ivec2 dir) const noexcept {
    static Optional<Exit> empty;
    int idx = ivec2_to_dir(dir);
    return idx == -1 ? empty : exits[idx];
}

Optional<double>
Tile::layermodAt(ivec2 dir) const noexcept {
    int idx = ivec2_to_dir(dir);
    return idx == -1 ? Optional<double>() : layermods[idx];
}


/*
 * TILESET
 */
int
TileSet::at(size_t x, size_t y) noexcept {
    size_t i = y * width + x;
    assert_(firstGid <= i && i < firstGid + width * height);
    return i;
}
