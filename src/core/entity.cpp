/***************************************
** Tsunagari Tile Engine              **
** entity.cpp                         **
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

#include "core/entity.h"

#include "core/area.h"
#include "core/client-conf.h"
#include "core/display-list.h"
#include "core/images.h"
#include "core/jsons.h"
#include "core/log.h"
#include "core/resources.h"
#include "core/world.h"
#include "os/c.h"
#include "util/assert.h"
#include "util/math2.h"
#include "util/move.h"
#include "util/string2.h"

#define CHECK(x)      \
    if (!(x)) {       \
        return false; \
    }

static StringView directions[][3] = {
        {"up-left", "up", "up-right"},
        {"left", "stance", "right"},
        {"down-left", "down", "down-right"},
};


bool
Entity::init(StringView descriptor, StringView initialPhase) noexcept {
    this->descriptor = descriptor;
    CHECK(processDescriptor());
    setPhase(initialPhase);
    return true;
}

void
Entity::destroy() noexcept {
    dead = true;
    if (area) {
        area->requestRedraw();
    }
}

void
Entity::draw(DisplayList* display) noexcept {
    redraw = false;
    if (!phase) {
        return;
    }

    time_t now = World::time();

    // TODO: Don't add to DisplayList if not on-screen.

    // X-axis is centered on tile.
    float maxX = (area->grid.tileDim.x + imgsz.x) / 2 + r.x;
    float minX = maxX - imgsz.x;
    // Y-axis is aligned with bottom of tile.
    float maxY = area->grid.tileDim.y + r.y;
    float minY = maxY - imgsz.y;

    display->items.push_back(DisplayItem{phase->frame(now),
                                         rvec2{minX, minY}});
}

bool
Entity::needsRedraw(const icube& visiblePixels) const noexcept {
    if (!phase) {
        // Entity is invisible.
        return false;
    }


    if (!redraw) {
        // Entity has not moved and has not changed phase.
        time_t now = World::time();
        if (!phase->needsRedraw(now)) {
            // Entity's animation does not need an update.
            return false;
        }
    }

    // At this point, we know the entity is visible, and has either moved since
    // the last frame or it wants to update its animation frame. Now we check
    // if it is on-screen.

    // X-axis is centered on tile.
    int maxX = (area->grid.tileDim.x + imgsz.x) / 2 + static_cast<int>(r.x);
    int minX = maxX - imgsz.x;
    // Y-axis is aligned with bottom of tile.
    int maxY = area->grid.tileDim.y + static_cast<int>(r.y);
    int minY = maxY - imgsz.y;

    if (visiblePixels.x2 < minX || maxX < visiblePixels.x1) {
        return false;
    }
    if (visiblePixels.y2 < minY || maxY < visiblePixels.y1) {
        return false;
    }

    return true;
}

bool
Entity::isDead() const noexcept {
    return dead;
}


void
Entity::tick(time_t dt) noexcept {
    for (auto& fn : onTickFns) {
        fn(dt);
    }
}

void
Entity::turn() noexcept {
    for (auto& fn : onTurnFns) {
        fn();
    }
}

const StringView
Entity::getFacing() const noexcept {
    return directionStr(facing);
}

bool
Entity::setPhase(StringView name) noexcept {
    enum SetPhaseResult res;
    res = _setPhase(name);
    if (res == PHASE_NOTFOUND) {
        res = _setPhase("stance");
        if (res == PHASE_NOTFOUND) {
            Log::err(descriptor,
                     String() << "phase '" << name << "' not found");
        }
    }
    return res == PHASE_CHANGED;
}

ivec2
Entity::getImageSize() const noexcept {
    return imgsz;
}

void
Entity::setAnimationStanding() noexcept {
    setPhase(getFacing());
}

void
Entity::setAnimationMoving() noexcept {
    setPhase(String() << "moving " << getFacing());
}


rcoord
Entity::getPixelCoord() const noexcept {
    return r;
}

Area*
Entity::getArea() noexcept {
    return area;
}

void
Entity::setArea(Area* area) noexcept {
    this->area = area;
    calcDraw();

    assert_(area->grid.tileDim.x == area->grid.tileDim.y);
    pixelsPerSecond = tilesPerSecond * area->grid.tileDim.x;
}

float
Entity::getSpeedInPixels() const noexcept {
    float tileWidth = static_cast<float>(area->grid.tileDim.x);
    return getSpeedInTiles() * tileWidth;
}

float
Entity::getSpeedInTiles() const noexcept {
    return tilesPerSecond;
}

void
Entity::setFrozen(bool b) noexcept {
    frozen = b;
}

void
Entity::attach(OnTickFn fn) noexcept {
    onTickFns.push_back(fn);
}

void
Entity::attach(OnTurnFn fn) noexcept {
    onTurnFns.push_back(fn);
}

void
Entity::calcDraw() noexcept {
    if (area) {
        ivec2 tile = area->grid.tileDim;

        // X-axis is centered on tile.
        doff.x = static_cast<float>((tile.x - imgsz.x) / 2);
        // Y-axis is aligned with bottom of tile.
        doff.y = static_cast<float>(tile.y - imgsz.y);
    }
}

void
Entity::setFacing(ivec2 facing) noexcept {
    assert_(facing.x == bound(facing.x, -1, 1));
    assert_(facing.y == bound(facing.y, -1, 1));
    this->facing = facing;
}

StringView
Entity::directionStr(ivec2 facing) const noexcept {
    return directions[facing.y + 1][facing.x + 1];
}

enum SetPhaseResult
Entity::_setPhase(StringView name) noexcept {
    auto it = phases.find(name);
    if (it == phases.end()) {
        return PHASE_NOTFOUND;
    }
    Animation* newPhase = &it.value();
    if (phase != newPhase) {
        time_t now = World::time();
        phase = newPhase;
        phase->startOver(now);
        phaseName = name;
        redraw = true;
        return PHASE_CHANGED;
    }
    return PHASE_NOTCHANGED;
}

void
Entity::setDestinationCoordinate(rcoord destCoord) noexcept {
    // Set z right away so that we're on-level with the square we're
    // entering.
    r.z = destCoord.z;

    this->destCoord = destCoord;
    angleToDest = static_cast<float>(atan2(destCoord.y - r.y, destCoord.x - r.x));
}

void
Entity::moveTowardDestination(time_t dt) noexcept {
    if (!moving) {
        return;
    }

    redraw = true;

    float traveledPixels = pixelsPerSecond * static_cast<float>(dt) / 1000.0f;
    float toDestPixels = r.distanceTo(destCoord);
    if (toDestPixels > traveledPixels) {
        // The destination has not been reached yet.
        r.x += static_cast<float>(cos(angleToDest)) * traveledPixels;
        r.y += static_cast<float>(sin(angleToDest)) * traveledPixels;
    }
    else {
        // We have arrived at the destination.
        r = destCoord;
        moving = false;
        arrived();

        // If arrived() starts a new movement, rollover unused traveled
        // pixels and leave the the moving animation.
        if (moving) {
            float percent = 1.0f - toDestPixels / traveledPixels;
            time_t rem = static_cast<time_t>(percent * static_cast<float>(dt));
            moveTowardDestination(rem);
        }
        else {
            setAnimationStanding();
        }
    }
}

void
Entity::arrived() noexcept {
    // for (auto& fn : onArrivedFns)
    //     fn();
}


/*
 * JSON DESCRIPTOR CODE BELOW
 */

bool
Entity::processDescriptor() noexcept {
    Rc<JSONObject> doc = JSONs::load(descriptor);
    if (!doc) {
        return false;
    }

    if (doc->hasFloat("speed")) {
        tilesPerSecond = doc->floatAt("speed");

        if (area) {
            assert_(area->grid.tileDim.x == area->grid.tileDim.y);
            pixelsPerSecond = tilesPerSecond * area->grid.tileDim.x;
        }
    }
    if (doc->hasObject("sprite")) {
        CHECK(processSprite(doc->objectAt("sprite")));
    }
    if (doc->hasObject("sounds")) {
        CHECK(processSounds(doc->objectAt("sounds")));
    }
    if (doc->hasObject("scripts")) {
        CHECK(processScripts(doc->objectAt("scripts")));
    }
    return true;
}

bool
Entity::processSprite(Unique<JSONObject> sprite) noexcept {
    CHECK(sprite->hasObject("sheet"));
    CHECK(sprite->hasObject("phases"));

    Unique<JSONObject> sheet = sprite->objectAt("sheet");
    CHECK(sheet->hasUnsigned("tile_width"));
    CHECK(sheet->hasUnsigned("tile_height"));
    CHECK(sheet->hasString("path"));

    imgsz.x = sheet->intAt("tile_width");
    imgsz.y = sheet->intAt("tile_height");
    StringView path = sheet->stringAt("path");
    TiledImageID tiles = Images::loadTiles(path, imgsz.x, imgsz.y);
    CHECK(tiles);

    return processPhases(sprite->objectAt("phases"), tiles);
}

bool
Entity::processPhases(Unique<JSONObject> phases, TiledImageID tiles) noexcept {
    for (StringView name : phases->names()) {
        CHECK(phases->hasObject(name));
        CHECK(processPhase(name, phases->objectAt(name), tiles));
    }
    return true;
}

Vector<int>
intArrayToVector(Unique<JSONArray> array) noexcept {
    Vector<int> v;
    for (size_t i = 0; i < array->size(); i++) {
        if (array->isUnsigned(i)) {
            v.push_back(array->intAt(i));
        }
    }
    return v;
}

bool
Entity::processPhase(StringView name,
                     Unique<JSONObject> phase,
                     TiledImageID tiles) noexcept {
    // Each phase requires a 'name' and a 'frame' or 'frames'. Additionally,
    // 'speed' is required if 'frames' is found.
    CHECK(phase->hasUnsigned("frame") || phase->hasArray("frames"));

    int nTiles = TiledImage::size(tiles);

    if (phase->hasUnsigned("frame")) {
        unsigned frame_ = phase->unsignedAt("frame");
        if (frame_ > INT32_MAX) {
            Log::err(descriptor, "<phase> frame attribute index out of bounds");
            return false;
        }
        int frame = static_cast<int>(frame_);
        if (frame >= nTiles) {
            Log::err(descriptor, "<phase> frame attribute index out of bounds");
            return false;
        }
        ImageID image = TiledImage::getTile(tiles, frame);
        phases[name] = Animation(image);
    }
    else if (phase->hasArray("frames")) {
        if (!phase->hasFloat("speed")) {
            Log::err(descriptor,
                     "<phase> speed attribute must be present and "
                     "must be decimal");
            return false;
        }
        float fps = phase->floatAt("speed");

        Vector<int> frames = intArrayToVector(phase->arrayAt("frames"));
        Vector<ImageID> images;
        for (int i : frames) {
            if (i < 0 || nTiles < i) {
                Log::err(descriptor,
                         "<phase> frames attribute index out of bounds");
                return false;
            }
            images.push_back(TiledImage::getTile(tiles, i));
        }

        phases[name] = Animation(move_(images), (time_t)(1000.0 / fps));
    }
    else {
        Log::err(descriptor,
                 "<phase> frames attribute not an int or int ranges");
        return false;
    }

    return true;
}

bool
Entity::processSounds(Unique<JSONObject> sounds) noexcept {
    for (StringView name : sounds->names()) {
        CHECK(sounds->hasString(name));
        CHECK(processSound(name, sounds->stringAt(name)));
    }
    return true;
}

bool
Entity::processSound(StringView name, StringView path) noexcept {
    if (!path.size) {
        Log::err(descriptor, "sound path is empty");
        return false;
    }

    soundPaths[name] = path;
    return true;
}

bool
Entity::processScripts(Unique<JSONObject> scripts) noexcept {
    for (StringView name : scripts->names()) {
        CHECK(scripts->hasString(name));
        CHECK(processScript(name, scripts->stringAt(name)));
    }
    return true;
}

bool
Entity::processScript(StringView /*name*/, StringView path) noexcept {
    if (!path.size) {
        Log::err(descriptor, "script path is empty");
        return false;
    }

    // ScriptRef script = Script::create(filename);
    // if (!script || !script->validate())
    //     return false;

    // if (!setScript(trigger, script)) {
    //     Log::err(descriptor,
    //         "unrecognized script trigger: " + trigger);
    //     return false;
    // }

    return true;
}

/*
bool Entity::setScript(StringView trigger, ScriptRef& script) noexcept {
    if (trigger == "on_tick") {
        tickScript = script;
        return true;
    }
    if (trigger == "on_turn") {
        turnScript = script;
        return true;
    }
    if (trigger == "on_tile_entry") {
        tileEntryScript = script;
        return true;
    }
    if (trigger == "on_tile_exit") {
        tileExitScript = script;
        return true;
    }
    if (trigger == "on_delete") {
        deleteScript = script;
        return true;
    }
    return false;
}
*/
