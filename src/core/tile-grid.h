/***************************************
** Tsunagari Tile Engine              **
** tile-grid.h                        **
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

#ifndef SRC_CORE_TILE_GRID_H_
#define SRC_CORE_TILE_GRID_H_

#include "core/tile.h"
#include "core/vec.h"
#include "util/hashtable.h"
#include "util/vector.h"

class TileGrid {
 public:
    int getTileType(icoord phys) noexcept;
    void setTileType(icoord phys, int type) noexcept;

    Tile* getTile(icoord phys) noexcept;
    Tile* getTile(vicoord virt) noexcept;
    Tile* getTile(rcoord virt) noexcept;

    const Tile* getTile(icoord phys) const noexcept;
    const Tile* getTile(vicoord virt) const noexcept;
    const Tile* getTile(rcoord virt) const noexcept;

    //! Return the dimensions of the Tile matrix.
    ivec3 getDimensions() const noexcept;
    //! Return the pixel dimensions of a Tile graphic.
    ivec2 getTileDimensions() const noexcept;

    bool doesLoopInX() const noexcept;
    bool doesLoopInY() const noexcept;

    //! Returns true if a Tile exists at the specified coordinate.
    bool inBounds(icoord phys) const noexcept;
    bool inBounds(vicoord virt) const noexcept;
    bool inBounds(rcoord virt) const noexcept;

    // Convert between virtual and physical map coordinates. Physical
    // coordinates are the physical indexes into the Tile vector. Layer
    // depth is represented by an arbirarily chosen integer in the physical
    // system. Virtual coordinates include the correct floating-point
    // depth.
    vicoord phys2virt_vi(icoord phys) const noexcept;
    rcoord phys2virt_r(icoord phys) const noexcept;
    icoord virt2phys(vicoord virt) const noexcept;
    icoord virt2phys(rcoord virt) const noexcept;
    rcoord virt2virt(vicoord virt) const noexcept;
    vicoord virt2virt(rcoord virt) const noexcept;

 public:
    // Convert between virtual and physical map depths.
    int depthIndex(double depth) const noexcept;
    double indexDepth(int idx) const noexcept;

 public:
    //! 3-dimensional array of the tiles that make up the grid.
    Vector<int> graphics;
    Vector<Tile> objects;

    enum LayerType {
        TILE_LAYER,
        OBJECT_LAYER,
    };
    Vector<LayerType> layerTypes;

    //! 3-dimensional length of map.
    ivec3 dim;

    //! Pixel size for each tile in area. All tiles in a TileGrid must be the
    //! same size.
    ivec2 tileDim;

    //! Maps virtual float-point depths to an index in our map array.
    Hashmap<double, int> depth2idx;

    //! Maps an index in our map array to a virtual float-point depth.
    Vector<double> idx2depth;

    bool loopX, loopY;
};

#endif  // SRC_CORE_TILE_GRID_H_
