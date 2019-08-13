/***************************************
** Tsunagari Tile Engine              **
** world.cpp                          **
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

#include "core/world.h"

#include "core/area-json.h"
#include "core/area.h"
#include "core/character.h"
#include "core/client-conf.h"
#include "core/display-list.h"
#include "core/images.h"
#include "core/jsons.h"
#include "core/log.h"
#include "core/measure.h"
#include "core/music.h"
#include "core/overlay.h"
#include "core/player.h"
#include "core/resources.h"
#include "core/sounds.h"
#include "core/viewport.h"
#include "core/window.h"
#include "data/data-world.h"
#include "util/bitrecord.h"
#include "util/function.h"
#include "util/hashtable.h"
#include "util/rc.h"
#include "util/unique.h"
#include "util/vector.h"

class Image;
class Player;

// ScriptRef keydownScript, keyupScript;

static Rc<Image> pauseInfo;

static Hashmap<String, Unique<Area>> areas;
static Area* area = nullptr;
static Unique<Player> player = new Player;

/**
 * Last time engine state was updated. See World::update().
 */
static time_t lastTime = 0;

/**
 * Total unpaused game run time.
 */
static time_t total = 0;

static bool alive = false;
static bool redraw = false;
static bool userPaused = false;
static int paused = 0;

static Vector<BitRecord> keyStates;

static time_t
calculateDt(time_t now) noexcept {
    time_t dt = now - lastTime;
    if (dt == 0) {
        Log::info("World", "Warning: Delta time is 0");
    }
    assert_(dt >= 0);
    lastTime = now;
    return dt;
}

bool
World::init() noexcept {
    alive = true;

    auto& parameters = DataWorld::instance().parameters;
    auto& gameStart = parameters.gameStart;

    Conf::moveMode = parameters.moveMode;

    if (!player->init(gameStart.player.file, gameStart.player.phase)) {
        Log::fatal("World", "failed to load player");
        return false;
    }

    if (!focusArea(gameStart.area, gameStart.coords)) {
        Log::fatal("World", "failed to load initial Area");
        return false;
    }

    Viewport::instance().setSize(parameters.viewportResolution);
    Viewport::instance().trackEntity(player.get());

    // Apply client.ini music volume now that client.ini is loaded.
    Music::setVolume(1.0);

    return true;
}

time_t
World::time() noexcept {
    assert_(total >= 0);
    return total;
}

void
World::buttonDown(KeyboardKey key) noexcept {
    switch (key) {
    case KBEscape:
        setPaused(paused == 0);
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

void
World::buttonUp(KeyboardKey key) noexcept {
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

void
World::draw(DisplayList* display) noexcept {
    // TimeMeasure m("Drew world");

    Viewport& view = Viewport::instance();

    redraw = false;

    display->loopX = area->grid.loopX;
    display->loopY = area->grid.loopY;

    display->padding = view.getLetterboxOffset();
    display->scale = view.getScale();
    display->scroll = view.getMapOffset();
    display->size = view.getPhysRes();

    display->colorOverlayARGB = area->getColorOverlay();
    display->paused = paused > 0;

    area->draw(display);
}

bool
World::needsRedraw() noexcept {
    return redraw || (!paused && area->needsRedraw());
}

void
World::update(time_t now) noexcept {
    if (lastTime == 0) {
        // There is no dt on the first update().  Don't tick.
        lastTime = now;
    }
    else {
        time_t dt = calculateDt(now);

		if (dt == 0) {
            return;
        }

		if (paused) {
            return;
        }

		total += dt;
        assert_(total >= 0);

		tick(dt);
    }
}

void
World::tick(time_t dt) noexcept {
    area->tick(dt);
}

void
World::turn() noexcept {
    if (Conf::moveMode == Conf::TURN) {
        area->turn();
    }
}

bool
World::focusArea(StringView filename, vicoord playerPos) noexcept {
    auto entry = areas.find(filename);
    if (entry != areas.end()) {
        Area* rawArea = entry.value().get();
        focusArea(rawArea, playerPos);
        return true;
    }

    Unique<Area> newArea = makeAreaFromJSON(player.get(), filename);
    if (!newArea) {
        return false;
    }

    if (!newArea->init()) {
        return false;
    }

    DataArea* dataArea = DataWorld::instance().area(filename);
    if (!dataArea) {
        return false;
    }

    Area* rawArea = newArea.get();
    dataArea->area = rawArea;  // FIXME: Pass Area by parameter, not
                               // member variable so we can avoid this
                               // pointer.
    areas[filename] = move_(newArea);

    focusArea(rawArea, playerPos);

    return true;
}

void
World::focusArea(Area* area_, vicoord playerPos) noexcept {
    area = area_;
    player->setArea(area, playerPos);
    Viewport::instance().setArea(area);
    area->focus();
}

void
World::setPaused(bool b) noexcept {
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
        Music::pause();
    }
    else {
        Music::resume();
    }

    // If finally unpausing.
    if (!paused) {
        restoreKeys();
    }
}

void
World::storeKeys() noexcept {
    keyStates.push_back(GameWindow::instance().getKeysDown());
}

void
World::restoreKeys() noexcept {
    BitRecord now = GameWindow::instance().getKeysDown();
    BitRecord then = keyStates.back();
    Vector<size_t> diffs = now.diff(then);

    keyStates.pop_back();

    for (size_t key : diffs) {
        KeyboardKey key_ = static_cast<KeyboardKey>(key);
        if (now[key_]) {
            buttonDown(key_);
        }
        else {
            buttonUp(key_);
        }
    }
}

void
World::garbageCollect() noexcept {
    Images::garbageCollect();
    JSONs::garbageCollect();
    Music::garbageCollect();
    Sounds::instance().garbageCollect();
}
