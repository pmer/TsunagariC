/***************************************
** Tsunagari Tile Engine              **
** area.cpp                           **
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

#include "core/area.h"

#include "core/algorithm.h"
#include "core/client-conf.h"
#include "core/display-list.h"
#include "core/entity.h"
#include "core/images.h"
#include "core/log.h"
#include "core/music.h"
#include "core/npc.h"
#include "core/overlay.h"
#include "core/player.h"
#include "core/tile.h"
#include "core/viewport.h"
#include "core/window.h"
#include "core/world.h"
#include "data/data-world.h"
#include "os/c.h"
#include "util/assert.h"
#include "util/hashtable.h"
#include "util/math2.h"

Area::Area(Player* player, StringView descriptor)
        : dataArea(DataWorld::instance().area(descriptor)),
          player(player),
          colorOverlayARGB(0),
          beenFocused(false),
          redraw(true),
          descriptor(descriptor) {
    grid.dim = ivec3{0, 0, 0};
    grid.tileDim = ivec2{0, 0};
    grid.loopX = false;
    grid.loopY = false;
}

void
Area::focus() {
    if (!beenFocused) {
        beenFocused = true;
        if (dataArea) {
            dataArea->onLoad();
        }
    }

    if (musicPath) {
        Music::play(*musicPath);
    }

    if (dataArea) {
        dataArea->onFocus();
    }
}

void
Area::buttonDown(KeyboardKey key) {
    switch (key) {
    case KBLeftArrow:
        player->startMovement({-1, 0});
        break;
    case KBRightArrow:
        player->startMovement({1, 0});
        break;
    case KBUpArrow:
        player->startMovement({0, -1});
        break;
    case KBDownArrow:
        player->startMovement({0, 1});
        break;
    case KBSpace:
        player->useTile();
        break;
    default:
        break;
    }
}

void
Area::buttonUp(KeyboardKey key) {
    switch (key) {
    case KBLeftArrow:
        player->stopMovement({-1, 0});
        break;
    case KBRightArrow:
        player->stopMovement({1, 0});
        break;
    case KBUpArrow:
        player->stopMovement({0, -1});
        break;
    case KBDownArrow:
        player->stopMovement({0, 1});
        break;
    default:
        break;
    }
}

void
Area::draw(DisplayList* display) {
    icube tiles = visibleTiles();
    int maxZ = grid.dim.z;

    assert_(tiles.z1 == 0);
    assert_(tiles.z2 == maxZ);

    for (int z = 0; z < maxZ; z++) {
        switch (grid.layerTypes[z]) {
        case TileGrid::LayerType::TILE_LAYER:
            drawTiles(display, tiles, z);
            break;
        case TileGrid::LayerType::OBJECT_LAYER:
            drawEntities(display, tiles, z);
            break;
        }
    }

    redraw = false;
}

bool
Area::needsRedraw() {
    if (redraw) {
        return true;
    }

    const icube tiles = visibleTiles();
    const icube pixels = {
            tiles.x1 * grid.tileDim.x,
            tiles.y1 * grid.tileDim.y,
            tiles.z1,
            tiles.x2 * grid.tileDim.x,
            tiles.y2 * grid.tileDim.y,
            tiles.z2,
    };

    if (player->needsRedraw(pixels)) {
        return true;
    }
    for (const auto& character : characters) {
        if (character->needsRedraw(pixels)) {
            return true;
        }
    }
    for (const auto& overlay : overlays) {
        if (overlay->needsRedraw(pixels)) {
            return true;
        }
    }

    // Do any on-screen tile types need to update their animations?
    checkedForAnimation.resize(static_cast<size_t>(tileGraphics.size()));
    for (bool& checked : checkedForAnimation) {
        checked = false;
    }

    time_t now = World::time();

    for (int z = tiles.z1; z < tiles.z2; z++) {
        if (grid.layerTypes[z] != TileGrid::LayerType::TILE_LAYER) {
            continue;
        }
        for (int y = tiles.y1; y < tiles.y2; y++) {
            for (int x = tiles.x1; x < tiles.x2; x++) {
                int type = grid.getTileType(icoord{x, y, z});

                if (type == 0) {
                    continue;
                }

                if (checkedForAnimation[type]) {
                    continue;
                }
                checkedForAnimation[type] = true;

                if (tileGraphics[type].needsRedraw(now)) {
                    return true;
                }
            }
        }
    }
    return false;
}

void
Area::requestRedraw() {
    redraw = true;
}

void
Area::tick(time_t dt) {
    if (dataArea) {
        dataArea->tick(dt);
    }

    for (auto& overlay : overlays) {
        overlay->tick(dt);
    }
    erase_if(overlays, [](const Rc<Overlay>& o) { return o->isDead(); });

    if (Conf::moveMode != Conf::TURN) {
        player->tick(dt);

        for (auto& character : characters) {
            character->tick(dt);
        }
        erase_if(characters, [](const Rc<Character>& c) {
            bool dead = c->isDead();
            if (dead) {
                c->setArea(nullptr, {0, 0, 0.0});
            }
            return dead;
        });
    }

    Viewport::tick(dt);
}

void
Area::turn() {
    if (dataArea) {
        dataArea->turn();
    }

    player->turn();

    for (auto& character : characters) {
        character->turn();
    }
    erase_if(characters, [](const Rc<Character>& c) {
        bool dead = c->isDead();
        if (dead) {
            c->setArea(nullptr, {0, 0, 0.0});
        }
        return dead;
    });

    Viewport::turn();
}


uint32_t
Area::getColorOverlay() {
    return colorOverlayARGB;
}

void
Area::setColorOverlay(uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
    colorOverlayARGB = (uint32_t)(a << 24u) + (uint32_t)(r << 16u) +
                       (uint32_t)(g << 8u) + (uint32_t)b;
    redraw = true;
}


TileSet*
Area::getTileSet(StringView imagePath) {
    if (!tileSets.contains(imagePath)) {
        String msg;
        msg << "tileset " << imagePath << " not found";
        Log::err("Area", msg);
        return nullptr;
    }
    return &tileSets[imagePath];
}


icube
Area::visibleTiles() const {
    rvec2 screen = Viewport::getVirtRes();
    rvec2 off = Viewport::getMapOffset();

    int x1 = static_cast<int>(floor(off.x / grid.tileDim.x));
    int y1 = static_cast<int>(floor(off.y / grid.tileDim.y));
    int x2 = static_cast<int>(ceil((screen.x + off.x) / grid.tileDim.x));
    int y2 = static_cast<int>(ceil((screen.y + off.y) / grid.tileDim.y));

    if (!grid.loopX) {
        x1 = bound(x1, 0, grid.dim.x);
        x2 = bound(x2, 0, grid.dim.x);
    }
    if (!grid.loopY) {
        y1 = bound(y1, 0, grid.dim.y);
        y2 = bound(y2, 0, grid.dim.y);
    }

    return icube{x1, y1, 0, x2, y2, grid.dim.z};
}

bool
Area::inBounds(Entity* ent) const {
    return grid.inBounds(ent->getPixelCoord());
}


Rc<Character>
Area::spawnNPC(StringView descriptor, vicoord coord, StringView phase) {
    auto c = Rc<Character>(new Character);
    if (!c->init(descriptor, phase)) {
        // Error logged.
        return Rc<Character>();
    }
    c->setArea(this, coord);
    characters.push_back(c);
    return c;
}

Rc<Overlay>
Area::spawnOverlay(StringView descriptor, vicoord coord, StringView phase) {
    auto o = Rc<Overlay>(new Overlay);
    if (!o->init(descriptor, phase)) {
        // Error logged.
        return Rc<Overlay>();
    }
    o->setArea(this);
    o->teleport(coord);
    overlays.push_back(o);
    return o;
}


DataArea*
Area::getDataArea() {
    return dataArea;
}

void
Area::runScript(TileGrid::ScriptType type,
                icoord tile,
                Entity* triggeredBy) noexcept {
    Optional<DataArea::TileScript*> script =
        grid.scripts[type].tryAt(tile);
    if (script) {
        (dataArea->*(**script))(*triggeredBy, tile);
    }
}


void
Area::drawTiles(DisplayList* display, const icube& tiles, int z) {
    time_t now = World::time();

    tilesAnimated.resize(static_cast<size_t>(tileGraphics.size()));
    for (bool& animated : tilesAnimated) {
        animated = false;
    }

    display->items.reserve(display->items.size() +
                           (tiles.y2 - tiles.y1) * (tiles.x2 - tiles.x1));

    // float depth = grid.idx2depth[(size_t)z];

    int width = 16;
    int height = 16;

    for (int y = tiles.y1; y < tiles.y2; y++) {
        for (int x = tiles.x1; x < tiles.x2; x++) {
            // We are certain the Tile exists.
            int type = grid.getTileType(icoord{x, y, z});

            if (type == 0) {
                continue;
            }

            if (!tilesAnimated[type]) {
                tilesAnimated[type] = true;
                tileGraphics[type].frame(now);
            }

            Image* img = tileGraphics[type].frame();
            if (img) {
                rvec2 drawPos{float(x * width), float(y * height)};
                // drawPos.z = depth + drawPos.y / tileDimY *
                // ISOMETRIC_ZOFF_PER_TILE;
                display->items.push_back_nogrow(DisplayItem{img, drawPos});
            }
        }
    }
}

void
Area::drawEntities(DisplayList* display, const icube& tiles, int z) {
    float depth = grid.idx2depth[(size_t)z];

    for (auto& character : characters) {
        if (character->getTileCoords_i().z == z) {
            character->draw(display);
        }
    }

    for (auto& overlay : overlays) {
        if (overlay->getPixelCoord().z == depth) {
            overlay->draw(display);
        }
    }

    if (player->getTileCoords_i().z == z) {
        player->draw(display);
    }
}
