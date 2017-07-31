/***************************************
** Tsunagari Tile Engine              **
** tile.cpp                           **
** Copyright 2011-2013 PariahSoft LLC **
** Copyright 2016 Paul Merrill        **
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

#include <string.h>  // for memset

#include "core/area.h"
#include "core/formatter.h"
#include "core/log.h"
#include "core/world.h"
#include "util/move.h"
#include "util/string2.h"

static int ivec2_to_dir(ivec2 v) {
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
 * FLAGMANIP
 */
FlagManip::FlagManip(unsigned* flags) : flags(flags) {}

bool FlagManip::isNowalk() const {
    return (*flags & TILE_NOWALK) != 0;
}

bool FlagManip::isNowalkPlayer() const {
    return (*flags & TILE_NOWALK_PLAYER) != 0;
}

bool FlagManip::isNowalkNPC() const {
    return (*flags & TILE_NOWALK_NPC) != 0;
}

bool FlagManip::isNowalkExit() const {
    return (*flags & TILE_NOWALK_EXIT) != 0;
}

bool FlagManip::isNowalkAreaBound() const {
    return (*flags & TILE_NOWALK_AREA_BOUND) != 0;
}

void FlagManip::setNowalk(bool nowalk) {
    *flags &= ~TILE_NOWALK;
    *flags |= TILE_NOWALK * nowalk;
}

void FlagManip::setNowalkPlayer(bool nowalk) {
    *flags &= ~TILE_NOWALK_PLAYER;
    *flags |= TILE_NOWALK_PLAYER * nowalk;
}

void FlagManip::setNowalkNPC(bool nowalk) {
    *flags &= ~TILE_NOWALK_NPC;
    *flags |= TILE_NOWALK_NPC * nowalk;
}

void FlagManip::setNowalkExit(bool nowalk) {
    *flags &= ~TILE_NOWALK_EXIT;
    *flags |= TILE_NOWALK_EXIT * nowalk;
}

void FlagManip::setNowalkAreaBound(bool nowalk) {
    *flags &= ~TILE_NOWALK_AREA_BOUND;
    *flags |= TILE_NOWALK_AREA_BOUND * nowalk;
}


Exit::Exit(std::string area, int x, int y, double z)
    : area(move_(area)), coords(x, y, z) {}


/*
 * TILEBASE
 */
TileBase::TileBase()
    : parent(nullptr), flags(0x0),
      enterScript(nullptr), leaveScript(nullptr), useScript(nullptr) {}

FlagManip TileBase::flagManip() {
    return FlagManip(&flags);
}

bool TileBase::hasFlag(unsigned flag) const {
    return flags & flag || (parent && parent->hasFlag(flag));
}

TileType* TileBase::getType() const {
    return (TileType*)parent;
}

void TileBase::setType(TileType* type) {
    parent = type;
}


/*
 * TILE
 */
Tile::Tile() : entCnt(0) {}

Tile::Tile(Area* area, int x, int y, int z)
        : TileBase(), area(area), x(x), y(y), z(z), entCnt(0) {
    memset(exits, 0, sizeof(exits));
    memset(layermods, 0, sizeof(layermods));
}

icoord Tile::moveDest(icoord here, ivec2 facing) const {
    icoord dest = here + icoord(facing.x, facing.y, 0);

    Optional<double> layermod = layermodAt(facing);
    if (layermod)
        dest = area->virt2phys(vicoord(dest.x, dest.y, *layermod));
    return dest;
}

Tile* Tile::offset(int x, int y) const {
    return area->getTile(icoord(this->x + x, this->y + y, z));
}

double Tile::getZ() const {
    vicoord vi = area->phys2virt_vi(icoord(x, y, z));
    return vi.z;
}

Optional<Exit> Tile::getNormalExit() const {
    return exits[EXIT_NORMAL];
}

void Tile::setNormalExit(Exit exit) {
    exits[EXIT_NORMAL] = exit;
}

Optional<Exit> Tile::exitAt(ivec2 dir) const {
    int idx = ivec2_to_dir(dir);
    return idx == -1 ? Optional<Exit>() : exits[idx];
}

Optional<double> Tile::layermodAt(ivec2 dir) const {
    int idx = ivec2_to_dir(dir);
    return idx == -1 ? Optional<double>() : layermods[idx];
}

void Tile::runEnterScript(Entity* triggeredBy) {
    DataArea* dataArea = area->getDataArea();
    for (TileBase* scripted = this; scripted; scripted = scripted->parent) {
        DataArea::TileScript script = scripted->enterScript;
        if (script)
            (dataArea->*script)(*triggeredBy, *this);
    }
}

void Tile::runLeaveScript(Entity* triggeredBy) {
    DataArea* dataArea = area->getDataArea();
    for (TileBase* scripted = this; scripted; scripted = scripted->parent) {
        DataArea::TileScript script = scripted->leaveScript;
        if (script)
            (dataArea->*script)(*triggeredBy, *this);
    }
}

void Tile::runUseScript(Entity* triggeredBy) {
    DataArea* dataArea = area->getDataArea();
    for (TileBase* scripted = this; scripted; scripted = scripted->parent) {
        DataArea::TileScript script = scripted->useScript;
        if (script)
            (dataArea->*script)(*triggeredBy, *this);
    }
}


/*
 * TILETYPE
 */
TileType::TileType(const Rc<Image>& img) {
    anim = Animation(img);
}

bool TileType::needsRedraw() const {
    time_t now = World::instance().time();
    return anim.needsRedraw(now);
}

/*
 * TILESET
 */
TileSet::TileSet(size_t width, size_t height)
    : width(width), height(height) {}

void TileSet::add(TileType* type) {
    types.push_back(type);
}

void TileSet::set(size_t idx, TileType* type) {
    types[idx] = type;
}

TileType* TileSet::at(size_t x, size_t y) {
    size_t i = idx(x, y);
    if (i > types.size()) {
        Log::err("TileSet",
            Formatter("get(%, %): out of bounds") % x % y);
        return nullptr;
    }
    return types[i];
}

size_t TileSet::getWidth() const {
    return height;
}

size_t TileSet::getHeight() const {
    return width;
}

size_t TileSet::idx(size_t x, size_t y) const {
    return y * width + x;
}
