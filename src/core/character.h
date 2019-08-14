/***************************************
** Tsunagari Tile Engine              **
** character.h                        **
** Copyright 2011-2014 Michael Reiley **
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

#ifndef SRC_CORE_CHARACTER_H_
#define SRC_CORE_CHARACTER_H_

#include "core/entity.h"
#include "core/tile.h"
#include "core/vec.h"
#include "util/int.h"
#include "util/optional.h"

class Tile;

class Character : public Entity {
 public:
    Character() noexcept;
    virtual ~Character() = default;

    virtual void tick(time_t dt) noexcept;
    virtual void turn() noexcept;

    virtual void destroy() noexcept;

    //! Retrieve position within Area.
    icoord getTileCoords_i() const noexcept;
    vicoord getTileCoords_vi() const noexcept;

    //! Set location within Area.
    void setTileCoords(int x, int y) noexcept;
    void setTileCoords(icoord phys) noexcept;
    void setTileCoords(vicoord virt) noexcept;
    void setTileCoords(rcoord virt) noexcept;

    //! Get the Tile that we are standing on.
    const Tile* getTile() const noexcept;
    Tile* getTile() noexcept;

    void setArea(Area* area, vicoord position) noexcept;

    //! Initiate a movement within the Area.
    void moveByTile(ivec2 delta) noexcept;

 protected:
    //! Indicates which coordinate we will move into if we proceed in
    //! direction specified.
    icoord moveDest(ivec2 facing) noexcept;

    //! Returns true if we can move in the desired direction.
    bool canMove(icoord dest) noexcept;

    bool nowalked(Tile& t) noexcept;

    void arrived() noexcept;

    void leaveTile() noexcept;
    void leaveTile(Tile* t) noexcept;
    void enterTile() noexcept;
    void enterTile(Tile* t) noexcept;

    void runTileExitScript() noexcept;
    void runTileEntryScript() noexcept;

 protected:
    unsigned nowalkFlags;
    unsigned nowalkExempt;

    rcoord fromCoord;
    Tile* fromTile;
    Tile* destTile;
    const Optional<Exit>* destExit;
};

#endif  // SRC_CORE_CHARACTER_H_
