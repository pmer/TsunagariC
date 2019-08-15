/***************************************
** Tsunagari Tile Engine              **
** tile.h                             **
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

#ifndef SRC_CORE_TILE_H_
#define SRC_CORE_TILE_H_

class Tile;
struct TileSet;

#include "core/animation.h"
#include "core/vec.h"
#include "data/data-area.h"
#include "util/move.h"
#include "util/optional.h"
#include "util/string.h"
#include "util/vector.h"

class Area;
class Entity;

//! List of possible flags that can be attached to a tile.
/*!
    Flags are attached to tiles and denote special behavior for
    the tile they are bound to.

    see AreaTMX::splitTileFlags().
*/

/**
 * TILE_NOWALK
 * Neither the player nor NPCs can walk here.
 */
#define TILE_NOWALK (unsigned)(0x001)

/**
 * TILE_NOWALK_PLAYER
 * The player cannot walk here. NPCs can, though.
 */
#define TILE_NOWALK_PLAYER ((unsigned)(0x002))

/**
 * TILE_NOWALK_NPC
 * NPCs cannot walk here. The player can, though.
 */
#define TILE_NOWALK_NPC ((unsigned)(0x004))

/**
 * TILE_NOWALK_EXIT
 * This Tile is an Exit. Please take appropriate action when entering this Tile,
 * usually by transferring to another Area.
 *
 * This flag is not carried by actual Tiles, but can instead be flipped in an
 * Entity's "exempt" flag which will be read elsewhere in the engine.
 */
#define TILE_NOWALK_EXIT ((unsigned)(0x008))

/**
 * TILE_NOWALK_AREA_BOUND
 * This Tile is at the edge of an Area. If you step here, please handle it
 * appropriately.
 *
 * (Usually if one moves off a map bound, one will either transfer to another
 * Area, or will be destroyed.)
 *
 * This flag is not carried by actual Tiles, but can instead be flipped in an
 * Entity's "exempt" flag which will be read elsewhere in the engine.
 */
#define TILE_NOWALK_AREA_BOUND ((unsigned)(0x016))


/**
 * Types of exits.
 */
enum ExitDirection {
    /**
     * An Exit that is taken upon arriving at the Tile.
     */
    EXIT_NORMAL,
    /**
     * An Exit that is taken when leaving in the upwards
     * direction from a Tile.
     */
    EXIT_UP,
    /**
     * An Exit that is taken when leaving in the downwards
     * direction from a Tile.
     */
    EXIT_DOWN,
    /**
     * An Exit that is taken when leaving to the left from
     * a Tile.
     */
    EXIT_LEFT,
    /**
     * An Exit that is taken when leaving to the right from
     * a Tile.
     */
    EXIT_RIGHT,
    EXITS_LENGTH
};

//! Convenience trigger for inter-area teleportation.
/*!
    Tiles with an exit trigger attached can teleport the player to a
    new area in the World. The Exit struct contains the destination
    area and coordinates.
*/
struct Exit {
    String area;
    vicoord coords;
};

//! Contains properties unique to this tile.
/*!
    This struct contains local tile properties for a single tile in
    the area. As opposed to global properties which apply to all
    tiles of the same type, these properties will only apply to one
    tile.
*/
class Tile {
 public:
    /**
     * Gets the correct destination for an Entity wanting to
     * move off of this tile in <code>facing</code>
     * direction.
     *
     * This call is necessary to handle layermod.
     *
     * @param area    the area containing this Tile
     * @param here    area-space coordinate for this Tile
     * @param facing  facing vector
     */
    icoord moveDest(Area* area, icoord here, ivec2 facing) const noexcept;

    const Optional<Exit>& exitAt(ivec2 dir) const noexcept;
    Optional<float> layermodAt(ivec2 dir) const noexcept;

 public:
    Optional<Exit> exits[EXITS_LENGTH];
    Optional<float> layermods[EXITS_LENGTH];
};

struct TileSet {
    int firstGid;
    size_t width;
    size_t height;

    int at(size_t x, size_t y) noexcept;
};

#endif  // SRC_CORE_TILE_H_
