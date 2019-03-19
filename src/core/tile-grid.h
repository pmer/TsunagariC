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

#include "core/log.h"
#include "core/vec.h"
#include "util/assert.h"
#include "util/hashtable.h"
#include "util/math2.h"
#include "util/string.h"
#include "util/vector.h"

class Viewport;

template<typename T> class TileGrid {
 public:
    T* getTile(icoord phys);
    T* getTile(vicoord virt);
    T* getTile(rcoord virt);

    const T* getTile(icoord phys) const;
    const T* getTile(vicoord virt) const;
    const T* getTile(rcoord virt) const;

    //! Return the dimensions of the Tile matrix.
    ivec3 getDimensions() const;
    //! Return the pixel dimensions of a Tile graphic.
    ivec2 getTileDimensions() const;

    bool doesLoopInX() const;
    bool doesLoopInY() const;

    //! Returns true if a Tile exists at the specified coordinate.
    bool inBounds(icoord phys) const;
    bool inBounds(vicoord virt) const;
    bool inBounds(rcoord virt) const;

    // Convert between virtual and physical map coordinates. Physical
    // coordinates are the physical indexes into the Tile vector. Layer
    // depth is represented by an arbirarily chosen integer in the physical
    // system. Virtual coordinates include the correct floating-point
    // depth.
    vicoord phys2virt_vi(icoord phys) const;
    rcoord phys2virt_r(icoord phys) const;
    icoord virt2phys(vicoord virt) const;
    icoord virt2phys(rcoord virt) const;
    rcoord virt2virt(vicoord virt) const;
    vicoord virt2virt(rcoord virt) const;

 public:
    // Convert between virtual and physical map depths.
    int depthIndex(double depth) const;
    double indexDepth(int idx) const;

 public:
    //! 3-dimensional array of the tiles that make up the grid.
    vector<T> grid;

    //! 3-dimensional length of map.
    ivec3 dim;

    //! Pixel size for each tile in area. All tiles in a TileGrid must be the
    //! same size.
    ivec2 tileDim;

    //! Maps virtual float-point depths to an index in our map array.
    Hashmap<double, int> depth2idx;

    //! Maps an index in our map array to a virtual float-point depth.
    vector<double> idx2depth;

    bool loopX, loopY;
};


template<typename T>
T*
TileGrid<T>::getTile(icoord phys) {
    int x = dim.x;
    int y = dim.y;

    if (loopX) {
        phys.x = wrap(0, phys.x, dim.x);
    }

    if (loopY) {
        phys.y = wrap(0, phys.y, dim.y);
    }

    int idx = (phys.z * y + phys.y) * x + phys.x;
    return &grid[idx];
}

template<typename T>
T*
TileGrid<T>::getTile(vicoord virt) {
    return getTile(virt2phys(virt));
}

template<typename T>
T*
TileGrid<T>::getTile(rcoord virt) {
    return getTile(virt2phys(virt));
}

template<typename T>
const T*
TileGrid<T>::getTile(icoord phys) const {
    int x = dim.x;
    int y = dim.y;

    if (loopX) {
        phys.x = wrap(0, phys.x, dim.x);
    }

    if (loopY) {
        phys.y = wrap(0, phys.y, dim.y);
    }

    assert_(inBounds(phys));

    int idx = (phys.z * y + phys.y) * x + phys.x;
    return &grid[idx];
}

template<typename T>
const T*
TileGrid<T>::getTile(vicoord virt) const {
    return getTile(virt2phys(virt));
}

template<typename T>
const T*
TileGrid<T>::getTile(rcoord virt) const {
    return getTile(virt2phys(virt));
}


template<typename T>
ivec3
TileGrid<T>::getDimensions() const {
    return dim;
}

template<typename T>
ivec2
TileGrid<T>::getTileDimensions() const {
    return tileDim;
}


template<typename T>
bool
TileGrid<T>::doesLoopInX() const {
    return loopX;
}

template<typename T>
bool
TileGrid<T>::doesLoopInY() const {
    return loopY;
}


template<typename T>
bool
TileGrid<T>::inBounds(icoord phys) const {
    return (loopX || (0 <= phys.x && phys.x < dim.x)) &&
           (loopY || (0 <= phys.y && phys.y < dim.y)) &&
           (0 <= phys.z && phys.z < dim.z);
}

template<typename T>
bool
TileGrid<T>::inBounds(vicoord virt) const {
    return inBounds(virt2phys(virt));
}

template<typename T>
bool
TileGrid<T>::inBounds(rcoord virt) const {
    return inBounds(virt2phys(virt));
}


template<typename T>
vicoord
TileGrid<T>::phys2virt_vi(icoord phys) const {
    return vicoord{phys.x, phys.y, indexDepth(phys.z)};
}

template<typename T>
rcoord
TileGrid<T>::phys2virt_r(icoord phys) const {
    return rcoord{static_cast<double>(phys.x * tileDim.x),
                  static_cast<double>(phys.y * tileDim.y),
                  indexDepth(phys.z)};
}

template<typename T>
icoord
TileGrid<T>::virt2phys(vicoord virt) const {
    return icoord{static_cast<int>(virt.x),
                  static_cast<int>(virt.y),
                  depthIndex(virt.z)};
}

template<typename T>
icoord
TileGrid<T>::virt2phys(rcoord virt) const {
    return icoord{static_cast<int>(virt.x) / tileDim.x,
                  static_cast<int>(virt.y) / tileDim.y,
                  depthIndex(virt.z)};
}

template<typename T>
rcoord
TileGrid<T>::virt2virt(vicoord virt) const {
    return rcoord{static_cast<double>(virt.x * tileDim.x),
                  static_cast<double>(virt.y * tileDim.y),
                  virt.z};
}

template<typename T>
vicoord
TileGrid<T>::virt2virt(rcoord virt) const {
    return vicoord{static_cast<int>(virt.x) / tileDim.x,
                   static_cast<int>(virt.y) / tileDim.y,
                   virt.z};
}


template<typename T>
int
TileGrid<T>::depthIndex(double depth) const {
    auto it = depth2idx.find(depth);
    if (it == depth2idx.end()) {
        Log::fatal("TileGrid",
                   String() << "Attempt to access invalid layer: " << depth);
    }
    return it.value();
}

template<typename T>
double
TileGrid<T>::indexDepth(int idx) const {
    assert_(0 <= idx && idx <= dim.z);
    return idx2depth[(size_t)idx];
}

#endif  // SRC_CORE_TILE_GRID_H_
