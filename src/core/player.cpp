/***************************************
** Tsunagari Tile Engine              **
** player.cpp                         **
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

#include "core/player.h"

#include "core/area.h"
#include "core/client-conf.h"
#include "core/entity.h"
#include "core/log.h"
#include "core/tile.h"
#include "core/window.h"
#include "core/world.h"

#define CHECK(x)      \
    if (!(x)) {       \
        return false; \
    }

static Player* globalPlayer = nullptr;

Player&
Player::instance() noexcept {
    return *globalPlayer;
}


Player::Player() noexcept : Character(), velocity{0, 0} {
    globalPlayer = this;
    nowalkFlags = TILE_NOWALK | TILE_NOWALK_PLAYER;
    nowalkExempt = TILE_NOWALK_EXIT;
}

void
Player::destroy() noexcept {
    Log::fatal("Player", "destroy(): Player should not be destroyed");
}

void
Player::startMovement(ivec2 delta) noexcept {
    switch (Conf::moveMode) {
    case Conf::TURN:
        moveByTile(delta);
        break;
    case Conf::TILE:
        movements.push_back(delta);
        velocity = delta;
        moveByTile(velocity);
        break;
    case Conf::NOTILE:
        // TODO
        break;
    }
}

void
Player::stopMovement(ivec2 delta) noexcept {
    switch (Conf::moveMode) {
    case Conf::TURN:
        break;
    case Conf::TILE:
        for (auto it = movements.begin(); it != movements.end(); ++it) {
            if (*it == delta) {
                movements.erase_unsorted(it);
                break;
            }
        }
        velocity = movements.size() ? movements.back() : ivec2{0, 0};
        if (velocity) {
            moveByTile(velocity);
        }
        break;
    case Conf::NOTILE:
        // TODO
        break;
    }
}

void
Player::moveByTile(ivec2 delta) noexcept {
    if (frozen) {
        return;
    }
    if (moving) {
        return;
    }

    setFacing(delta);

    // Left SHIFT allows changing facing, but disallows movement.
    if (GameWindow::keysDown[KBLeftShift] ||
        GameWindow::keysDown[KBRightShift]) {
        setAnimationStanding();
        redraw = true;
        return;
    }

    Character::moveByTile(delta);

    World::turn();
}

void
Player::useTile() noexcept {
    icoord destCoord = moveDest(facing);
    Tile* t = area->grid.getTile(destCoord);
    if (t) {
        area->runScript(TileGrid::SCRIPT_TYPE_USE, destCoord, this);
    }
}

void
Player::setFrozen(bool b) noexcept {
    if (b) {
        World::storeKeys();
    }
    else {
        World::restoreKeys();
    }

    Entity::setFrozen(b);
    if (!frozen && velocity) {
        moveByTile(velocity);
    }
}

void
Player::arrived() noexcept {
    Entity::arrived();

    if (destExit && *destExit) {
        takeExit(*(*destExit));
    }

    // If we have a velocity, keep moving.
    if (Conf::moveMode == Conf::TILE && velocity) {
        moveByTile(velocity);
    }
}

void
Player::takeExit(const Exit& exit) noexcept {
    if (!World::focusArea(exit.area, exit.coords)) {
        // Roll back movement if exit failed to open.
        setTileCoords(fromCoord);
        Log::err("Exit", String() << exit.area << ": failed to load properly");
    }
}
