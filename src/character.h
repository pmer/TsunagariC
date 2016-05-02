/***************************************
** Tsunagari Tile Engine              **
** character.h                        **
** Copyright 2011-2014 PariahSoft LLC **
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

#ifndef CHARACTER_H
#define CHARACTER_H

#include <time.h>

#include "entity.h"
#include "vec.h"

class Exit;
class Tile;

class Character : public Entity
{
public:
    Character();
    virtual ~Character();

    virtual void tick(time_t dt);
    virtual void turn();

    virtual void destroy();

    //! Retrieve position within Area.
    icoord getTileCoords_i() const;
    vicoord getTileCoords_vi() const;

    //! Set location within Area.
    void setTileCoords(int x, int y);
    void setTileCoords(int x, int y, double z);
    void setTileCoords(icoord phys);
    void setTileCoords(vicoord virt);
    void setTileCoords(rcoord virt);

    //! Get the Tile that we are standing on.
    const Tile* getTile() const;
    Tile* getTile();

    void setArea(Area* area);

    //! Initiate a movement within the Area.
    void moveByTile(ivec2 delta);

protected:
    //! Indicates which coordinate we will move into if we proceed in
    //! direction specified.
    icoord moveDest(ivec2 facing);

    //! Returns true if we can move in the desired direction.
    bool canMove(icoord dest);

    bool nowalked(Tile& t);

    void arrived();

    void leaveTile();
    void leaveTile(Tile* t);
    void enterTile();
    void enterTile(Tile* t);

    void runTileExitScript();
    void runTileEntryScript();

protected:
    unsigned nowalkFlags;
    unsigned nowalkExempt;

    rcoord fromCoord;
    Tile* fromTile;
    Tile* destTile;
    Exit* destExit;
};

#endif

