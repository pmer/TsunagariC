/***************************************
** Tsunagari Tile Engine              **
** world.cpp                          **
** Copyright 2011-2015 Michael Reiley **
** Copyright 2011-2017 Paul Merrill   **
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

#include "core/world.h"

#include <limits>

#include "core/area.h"
#include "core/area-json.h"
#include "core/client-conf.h"
#include "core/images.h"
#include "core/jsons.h"
#include "core/log.h"
#include "core/music.h"
#include "core/player.h"
#include "core/resources.h"
#include "core/sounds.h"
#include "core/viewport.h"
#include "core/window.h"

#include "data/data-world.h"

#define CHECK(x)  if (!(x)) { return false; }

static World globalWorld;

World& World::instance() {
    return globalWorld;
}

World::World()
    : player(new Player),
      lastTime(0), total(0), alive(false), redraw(false),
      userPaused(false), paused(0) {}

bool World::init() {
    alive = true;

    auto& parameters = DataWorld::instance().parameters;
    auto& gameStart = parameters.gameStart;

    conf.moveMode = parameters.moveMode;

    CHECK(player->init(gameStart.player.file, gameStart.player.phase));

    Area* area = getArea(gameStart.area);
    if (area == nullptr) {
        Log::fatal("World", "failed to load initial Area");
        return false;
    }
    focusArea(area, gameStart.coords);

    Viewport::instance().setSize(parameters.viewportResolution);
    Viewport::instance().trackEntity(player.get());

    // Apply client.ini music volume now that client.ini is loaded.
    Music::instance().setVolume(1.0);

    return true;
}

time_t World::time() const {
    return total;
}

void World::buttonDown(KeyboardKey key) {
    switch (key) {
    case KBEscape:
        userPaused = !userPaused;
        setPaused(userPaused);
        redraw = true;
        break;
    default:
        if (!paused && keyStates.empty()) {
            area->buttonDown(key);
            // if (keydownScript)
            //     keydownScript->invoke();
        }
        break;
    }
}

void World::buttonUp(KeyboardKey key) {
    switch (key) {
    case KBEscape:
        break;
    default:
        if (!paused && keyStates.empty()) {
            area->buttonUp(key);
            // if (keyupScript)
            //     keyupScript->invoke();
        }
        break;
    }
}

void World::draw() {
    redraw = false;

    GameWindow& window = GameWindow::instance();

    if (paused) {
        unsigned ww = window.width();
        unsigned wh = window.height();
        window.drawRect(0, ww, 0, wh, 0x7F000000);

        if (userPaused) {
            if (!pauseInfo) {
                pauseInfo = Images::instance().load("resource/pause_overlay.png");
            }
            if (pauseInfo) {
                unsigned iw = pauseInfo->width();
                unsigned ih = pauseInfo->height();
                double top = std::numeric_limits<double>::max();
                pauseInfo->draw(ww/2 - iw/2, wh/2 - ih/2, top);
            }
        }
    }

    pushLetterbox([&] {
        uint32_t colorOverlayARGB = area->getColorOverlay();
        if ((colorOverlayARGB & 0xFF000000) != 0) {
            unsigned ww = window.width();
            unsigned wh = window.height();
            window.drawRect(0, ww, 0, wh, colorOverlayARGB);
        }

        Viewport& view = Viewport::instance();

        // Zoom and pan the Area to fit on-screen.
        rvec2 padding = view.getLetterboxOffset();
        window.translate(-padding.x, -padding.y, [&] {
            rvec2 scale = view.getScale();
            window.scale(scale.x, scale.y, [&] {
                rvec2 scroll = view.getMapOffset();
                window.translate(-scroll.x, -scroll.y, [&] {
                    area->draw();
                });
            });
        });
    });
}

bool World::needsRedraw() const {
    return redraw || (!paused && area->needsRedraw());
}

void World::update(time_t now) {
    if (lastTime == 0) {
        // There is no dt on the first update().  Don't tick.
        lastTime = now;
    } else {
        time_t dt = calculateDt(now);
        if (!paused) {
            total += dt;
            tick(dt);
        }
    }
}

void World::tick(time_t dt) {
    area->tick(dt);
}

void World::turn() {
    if (conf.moveMode == TURN) {
        area->turn();
    }
}

Area* World::getArea(const std::string& filename) {
    AreaMap::iterator entry = areas.find(filename);
    if (entry != areas.end()) {
        return entry->second;
    }

    Area* newArea = makeAreaFromJSON(player.get(), filename);

    if (!newArea->init()) {
        newArea = nullptr;
    }

    areas[filename] = newArea;

    DataArea* dataArea = DataWorld::instance().area(filename);
    if (dataArea) {
        dataArea->area = newArea;
    }

    return newArea;
}

Area* World::getFocusedArea() {
    return area;
}

void World::focusArea(Area* area, int x, int y, double z) {
    focusArea(area, vicoord(x, y, z));
}

void World::focusArea(Area* area, vicoord playerPos) {
    this->area = area;
    player->setArea(area);
    player->setTileCoords(playerPos);
    Viewport::instance().setArea(area);
    area->focus();
}

void World::setPaused(bool b) {
    if (!alive) {
        return;
    }

    if (!paused && !b) {
        Log::err("World", "trying to unpause, but not paused");
        return;
    }

    // If just pausing.
    if (!paused) {
        storeKeys();
    }

    paused += b ? 1 : -1;

    if (paused) {
        Music::instance().pause();
    }
    else {
        Music::instance().resume();
    }

    // If finally unpausing.
    if (!paused) {
        restoreKeys();
    }
}

void World::storeKeys() {
    keyStates.push(GameWindow::instance().getKeysDown());
}

void World::restoreKeys() {
    BitRecord now = GameWindow::instance().getKeysDown();
    BitRecord then = keyStates.top();
    typedef std::vector<size_t> Size_tVector;
    Size_tVector diffs = now.diff(then);

    keyStates.pop();

    for (Size_tVector::iterator it = diffs.begin(); it != diffs.end(); it++) {
        KeyboardKey key = (KeyboardKey)*it;
        if (now[key]) {
            buttonDown(key);
        } else {
            buttonUp(key);
        }
    }
}

void World::garbageCollect() {
    Images::instance().garbageCollect();
    JSONs::instance().garbageCollect();
    Music::instance().garbageCollect();
    Sounds::instance().garbageCollect();
}

time_t World::calculateDt(time_t now) {
    time_t dt = now - lastTime;
    lastTime = now;
    return dt;
}

void World::pushLetterbox(std::function<void()> op) {
    GameWindow& window = GameWindow::instance();
    Viewport& view = Viewport::instance();

    // Aspect ratio correction.
    rvec2 sz = view.getPhysRes();
    rvec2 lb = -1 * view.getLetterboxOffset();

    window.clip(lb.x, lb.y, sz.x - 2 * lb.x, sz.y - 2 * lb.y, [&] {
        // Map bounds.
        rvec2 scale = view.getScale();
        rvec2 virtScroll = view.getMapOffset();
        rvec2 padding = view.getLetterboxOffset();

        rvec2 physScroll = -1 * virtScroll * scale + padding;

        bool loopX = area->loopsInX();
        bool loopY = area->loopsInY();

        if (!loopX && physScroll.x > 0) {
            // Boxes on left-right.
            window.clip(physScroll.x, 0, sz.x - 2 * physScroll.x, sz.x, op);
        }
        if (!loopY && physScroll.y > 0) {
            // Boxes on top-bottom.
            window.clip(0, physScroll.y, sz.x, sz.y - 2 * physScroll.y, op);
        }

        op();
    });
}
