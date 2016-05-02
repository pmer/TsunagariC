/***************************************
** Tsunagari Tile Engine              **
** character.cpp                      **
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

#include "area.h"
#include "character.h"
#include "client-conf.h"
#include "sounds.h"
#include "tile.h"

Character::Character()
    : nowalkFlags(TILE_NOWALK | TILE_NOWALK_NPC),
      nowalkExempt(0),
      fromCoord(0.0, 0.0, 0.0),
      fromTile(NULL),
      destTile(NULL),
      destExit(NULL)
{
    enterTile();
}

Character::~Character() {}

void Character::tick(time_t dt)
{
    Entity::tick(dt);

    switch (conf.moveMode) {
    case TURN:
        // Characters don't do anything on tick() for TURN mode.
        break;
    case TILE:
        moveTowardDestination(dt);
        break;
    case NOTILE:
        throw "not implemented";
    }
}

void Character::turn()
{
}

void Character::destroy()
{
    leaveTile();
    Entity::destroy();
}

icoord Character::getTileCoords_i() const
{
    return area->virt2phys(r);
}

vicoord Character::getTileCoords_vi() const
{
    return area->virt2virt(r);
}

void Character::setTileCoords(int x, int y)
{
    leaveTile();
    redraw = true;
    r = area->virt2virt(vicoord(x, y, r.z));
    enterTile();
}

void Character::setTileCoords(int x, int y, double z)
{
    leaveTile();
    redraw = true;
    r = area->virt2virt(vicoord(x, y, z));
    enterTile();
}

void Character::setTileCoords(icoord phys)
{
    leaveTile();
    redraw = true;
    r = area->phys2virt_r(phys);
    enterTile();
}

void Character::setTileCoords(vicoord virt)
{
    leaveTile();
    redraw = true;
    r = area->virt2virt(virt);
    enterTile();
}

void Character::setTileCoords(rcoord virt)
{
    leaveTile();
    redraw = true;
    r = virt;
    enterTile();
}

const Tile* Character::getTile() const
{
    return area ? area->getTile(r) : NULL;
}

Tile* Character::getTile()
{
    return area ? area->getTile(r) : NULL;
}

void Character::setArea(Area* area)
{
    leaveTile();
    Entity::setArea(area);
    enterTile();
}

void Character::moveByTile(ivec2 delta)
{
    if (moving)
        return;

    setFacing(delta);

    icoord dest = moveDest(facing);

    fromTile = getTile();
    destTile = area->getTile(dest);
    setDestinationCoordinate(area->phys2virt_r(dest));

    destExit = NULL;
    if (fromTile && fromTile->exitAt(delta))
        destExit = fromTile->exitAt(delta);
    else if (destTile && destTile->exits[EXIT_NORMAL])
        destExit = destTile->exits[EXIT_NORMAL];

    if (!canMove(dest)) {
        setAnimationStanding();
        return;
    }

    setAnimationMoving();
    moving = true;

    // Process triggers.
    runTileExitScript();
    if (fromTile)
        fromTile->runLeaveScript(this);

    // Modify tile's entity count.
    leaveTile(fromTile);
    enterTile(destTile);

    Sounds::instance().play(soundPaths["step"]);

    switch (conf.moveMode) {
    case TURN:
        // Movement is instantaneous.
        redraw = true;
        r = destCoord;
        moving = false;
        setAnimationStanding();
        arrived();
        break;
    case TILE:
    case NOTILE:
        // Movement happens in Entity::moveTowardDestination() during
        // tick().
        break;
    }
}

icoord Character::moveDest(ivec2 facing)
{
    Tile* tile = getTile();
    icoord here = getTileCoords_i();

    if (tile)
        // Handle layermod.
        return tile->moveDest(here, facing);
    else
        return here + icoord(facing.x, facing.y, 0);
}

bool Character::canMove(icoord dest)
{
    if (destExit) {
        // We can always take exits as long as we can take exits.
        // (Even if they would cause us to be out of bounds.)
        if (nowalkExempt & TILE_NOWALK_EXIT)
            return true;
    }

    bool inBounds = area->inBounds(dest);
    if (destTile && inBounds) {
        // Tile is inside map. Can we move?
        if (nowalked(*destTile))
            return false;
        if (destTile->entCnt)
            // Space is occupied by another Entity.
            return false;

        return true;
    }

    // The tile is legitimately off the map.
    return nowalkExempt & TILE_NOWALK_AREA_BOUND;
}

bool Character::nowalked(Tile& t)
{
    unsigned flags = nowalkFlags & ~nowalkExempt;
    return t.hasFlag(flags);
}

void Character::arrived()
{
    Entity::arrived();

    if (destTile) {
        double* layermod = destTile->layermods[EXIT_NORMAL];
        if (layermod) {
            r.z = *layermod;
        }
    }

    // Process triggers.
    if (destTile)
        destTile->runEnterScript(this);

    runTileEntryScript();

    // TODO: move teleportation here
    /*
     * if (onExit()) {
     *      leaveTile();
     *      moveArea(getExit());
     *      Entity::arrived();
     *      enterTile();
     * }
     */
}

void Character::leaveTile()
{
    leaveTile(getTile());
    
}

void Character::leaveTile(Tile* t)
{
    if (t)
        t->entCnt--;
}

void Character::enterTile()
{
    enterTile(getTile());
}

void Character::enterTile(Tile* t)
{
    if (t)
        t->entCnt++;
}

void Character::runTileExitScript()
{
    // if (!tileExitScript)
    //     return;
    // pythonSetGlobal("Area", area);
    // pythonSetGlobal("Character", this);
    // pythonSetGlobal("Tile", getTile());
    // tileExitScript->invoke();
}

void Character::runTileEntryScript()
{
    // if (!tileEntryScript)
    //     return;
    // pythonSetGlobal("Area", area);
    // pythonSetGlobal("Character", this);
    // pythonSetGlobal("Tile", getTile());
    // tileEntryScript->invoke();
}

