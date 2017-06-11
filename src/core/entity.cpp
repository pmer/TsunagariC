/***************************************
** Tsunagari Tile Engine              **
** entity.cpp                         **
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

#include <assert.h>
#include <math.h>

#include "core/area.h"
#include "core/client-conf.h"
#include "core/entity.h"
#include "core/images.h"
#include "core/jsons.h"
#include "core/log.h"
#include "core/resources.h"
#include "core/world.h"
#include "util/math2.h"
#include "util/string2.h"
#include "util/memory.h"

#define CHECK(x)  if (!(x)) { return false; }

static std::string directions[][3] = {
    {"up-left",   "up",     "up-right"},
    {"left",      "stance", "right"},
    {"down-left", "down",   "down-right"},
};


Entity::Entity()
    : dead(false),
      redraw(true),
      area(nullptr),
      r(0.0, 0.0, 0.0),
      frozen(false),
      speedMul(1.0),
      moving(false),
      phase(nullptr),
      phaseName(""),
      facing(0, 0)
{
}

Entity::~Entity()
{
}

bool Entity::init(const std::string& descriptor,
        const std::string& initialPhase)
{
    this->descriptor = descriptor;
    CHECK(processDescriptor());
    setPhase(initialPhase);
    return true;
}

void Entity::destroy()
{
    dead = true;
    if (area) {
        area->requestRedraw();
    }
}

void Entity::draw()
{
    redraw = false;
    if (!phase)
        return;

    time_t now = World::instance().time();
    Image* img = phase->frame(now);

    img->draw(
        doff.x + r.x,
        doff.y + r.y,
        r.z + area->isometricZOff(rvec2(r.x, r.y))
    );
}

bool Entity::needsRedraw(const icube& visiblePixels) const
{
    time_t now = World::instance().time();

    // Don't need to redraw
    if (!redraw && (!phase || !phase->needsRedraw(now))) {
        return false;
    }

    // Aren't on-screen
    if (visiblePixels.x2 < r.x || r.x + imgsz.x < visiblePixels.x1) {
        return false;
    }
    if (visiblePixels.y2 < r.y || r.y + imgsz.y < visiblePixels.y1) {
        return false;
    }

    return true;
}

bool Entity::isDead() const
{
    return dead;
}


void Entity::tick(time_t dt)
{
    for (auto& fn : onTickFns) {
        fn(dt);
    }
}

void Entity::turn()
{
    for (auto& fn : onTurnFns) {
        fn();
    }
}

const std::string Entity::getFacing() const
{
    return directionStr(facing);
}

bool Entity::setPhase(const std::string& name)
{
    enum SetPhaseResult res;
    res = _setPhase(name);
    if (res == PHASE_NOTFOUND) {
        res = _setPhase("stance");
        if (res == PHASE_NOTFOUND) {
            Log::err(descriptor, "phase '" + name + "' not found");
        }
    }
    return res == PHASE_CHANGED;
}

std::string Entity::getPhase() const
{
    return phaseName;
}

ivec2 Entity::getImageSize() const
{
    return imgsz;
}

void Entity::setAnimationStanding()
{
    setPhase(getFacing());
}

void Entity::setAnimationMoving()
{
    setPhase("moving " + getFacing());
}


rcoord Entity::getPixelCoord() const
{
    return r;
}

bool Entity::isMoving() const
{
    return moving;
}

Area* Entity::getArea()
{
    return area;
}

void Entity::setArea(Area* area)
{
    this->area = area;
    calcDraw();
    setSpeedMultiplier(speedMul); // Calculate new speed based on tile size.
}

double Entity::getSpeedInPixels() const
{
    double tileWidth = area->getTileDimensions().x;
    return getSpeedInTiles() * tileWidth;
}

double Entity::getSpeedInTiles() const
{
    return baseSpeed * speedMul;
}

double Entity::getSpeedMultiplier() const
{
    return speedMul;
}

void Entity::setSpeedMultiplier(double multiplier)
{
    speedMul = multiplier;
    if (area) {
        assert(area->getTileDimensions().x == area->getTileDimensions().y);
        double tilesPerMillisecond = area->getTileDimensions().x / 1000.0;
        speed = baseSpeed * speedMul * tilesPerMillisecond;
    }
}

void Entity::setFrozen(bool b)
{
    frozen = b;
}

bool Entity::getFrozen()
{
    return frozen;
}

void Entity::attach(OnTickFn fn)
{
    onTickFns.push_back(fn);
}

void Entity::attach(OnTurnFn fn)
{
    onTurnFns.push_back(fn);
}

void Entity::calcDraw()
{
    if (area) {
        ivec2 tile = area->getTileDimensions();

        // X-axis is centered on tile.
        doff.x = (tile.x - imgsz.x) / 2;
        // Y-axis is aligned with bottom of tile.
        doff.y = tile.y - imgsz.y;
    }
}

ivec2 Entity::setFacing(ivec2 facing)
{
    this->facing = ivec2(
        bound(facing.x, -1, 1),
        bound(facing.y, -1, 1)
    );
    return this->facing;
}

const std::string& Entity::directionStr(ivec2 facing) const
{
    return directions[facing.y+1][facing.x+1];
}

enum SetPhaseResult Entity::_setPhase(const std::string& name)
{
    AnimationMap::iterator it;
    it = phases.find(name);
    if (it == phases.end()) {
        return PHASE_NOTFOUND;
    }
    Animation* newPhase = &it->second;
    if (phase != newPhase) {
        time_t now = World::instance().time();
        phase = newPhase;
        phase->startOver(now);
        phaseName = name;
        redraw = true;
        return PHASE_CHANGED;
    }
    return PHASE_NOTCHANGED;
}

void Entity::setDestinationCoordinate(rcoord destCoord)
{
    // Set z right away so that we're on-level with the square we're
    // entering.
    r.z = destCoord.z;

    this->destCoord = destCoord;
    angleToDest = atan2(destCoord.y - r.y, destCoord.x - r.x);
}

void Entity::moveTowardDestination(time_t dt)
{
    if (!moving) {
        return;
    }

    redraw = true;

    double traveledPixels = speed * (double)dt;
    double toDestPixels = r.distanceTo(destCoord);
    if (toDestPixels > traveledPixels) {
        // The destination has not been reached yet.
        r.x += cos(angleToDest) * traveledPixels;
        r.y += sin(angleToDest) * traveledPixels;
    }
    else {
        // We have arrived at the destination.
        r = destCoord;
        moving = false;
        arrived();

        // If arrived() starts a new movement, rollover unused traveled
        // pixels and leave the the moving animation.
        if (moving) {
            double percent = 1.0 - toDestPixels/traveledPixels;
            time_t rem = (time_t)(percent * (double)dt);
            moveTowardDestination(rem);
        }
        else {
            setAnimationStanding();
        }
    }
}

void Entity::arrived()
{
    // for (auto& fn : onArrivedFns)
    //     fn();
}


/*
 * JSON DESCRIPTOR CODE BELOW
 */

bool Entity::processDescriptor() {
    JSONObjectRef doc = JSONs::instance().load(descriptor);
    if (!doc) {
        return false;
    }

    if (doc->hasDouble("speed")) {
        baseSpeed = doc->doubleAt("speed");
        setSpeedMultiplier(speedMul);  // Calculate speed from tile size.
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

bool Entity::processSprite(JSONObjectPtr sprite) {
    Arc<TiledImage> tiles;

    CHECK(sprite->hasObject("sheet"));
    CHECK(sprite->hasObject("phases"));

    JSONObjectPtr sheet = sprite->objectAt("sheet");
    CHECK(sheet->hasUnsigned("tile_width"));
    CHECK(sheet->hasUnsigned("tile_height"));
    CHECK(sheet->hasString("path"));

    imgsz.x = sheet->intAt("tile_width");
    imgsz.y = sheet->intAt("tile_height");
    std::string path = sheet->stringAt("path");
    tiles = Images::instance().loadTiles(path,
        static_cast<unsigned>(imgsz.x), static_cast<unsigned>(imgsz.y));

    return processPhases(sprite->objectAt("phases"), *tiles);
}

bool Entity::processPhases(JSONObjectPtr phases, TiledImage& tiles) {
    for (std::string& name : phases->names()) {
        CHECK(phases->hasObject(name));
        CHECK(processPhase(name, phases->objectAt(name), tiles));
    }
    return true;
}

std::vector<int> intArrayToVector(JSONArrayPtr array) {
    std::vector<int> vector;
    for (size_t i = 0; i < array->size(); i++) {
        if (array->isUnsigned(i)) {
            vector.push_back(array->intAt(i));
        }
    }
    return vector;
}

bool Entity::processPhase(std::string& name, JSONObjectPtr phase, TiledImage& tiles) {
    // Each phase requires a 'name' and a 'frame' or 'frames'. Additionally,
    // 'speed' is required if 'frames' is found.
    CHECK(phase->hasUnsigned("frame") || phase->hasArray("frames"));

    if (phase->hasUnsigned("frame")) {
        unsigned frame = phase->unsignedAt("frame");
        if (frame >= tiles.size()) {
            Log::err(descriptor,
                     "<phase> frame attribute index out of bounds");
            return false;
        }
        const auto& image = tiles[(size_t)frame];
        phases[name] = Animation(image);
    }
    else if (phase->hasArray("frames")) {
        if (!phase->hasDouble("speed")) {
            Log::err(descriptor,
                     "<phase> speed attribute must be present and "
                             "must be decimal");
            return false;
        }
        double fps = phase->doubleAt("speed");

        std::vector<int> frames = intArrayToVector(phase->arrayAt("frames"));
        std::vector<Arc<Image>> images;
        for (auto it = frames.begin(); it != frames.end(); it++) {
            int i = *it;
            if (i < 0 || (int)tiles.size() < i) {
                Log::err(descriptor,
                         "<phase> frames attribute index out of bounds");
                return false;
            }
            images.push_back(tiles[(size_t)i]);
        }

        phases[name] = Animation(std::move(images), (time_t)(1000.0 / fps));
    }
    else {
        Log::err(descriptor,
                 "<phase> frames attribute not an int or int ranges");
        return false;
    }

    return true;
}

bool Entity::processSounds(JSONObjectPtr sounds) {
    for (std::string& name : sounds->names()) {
        CHECK(sounds->hasString(name));
        CHECK(processSound(name, sounds->stringAt(name)));
    }
    return true;
}

bool Entity::processSound(std::string& name, std::string path) {
    if (path.empty()) {
        Log::err(descriptor, "sound path is empty");
        return false;
    }

    soundPaths[name] = path;
    return true;
}

bool Entity::processScripts(JSONObjectPtr scripts) {
    for (std::string& name : scripts->names()) {
        CHECK(scripts->hasString(name));
        CHECK(processScript(name, scripts->stringAt(name)));
    }
    return true;
}

bool Entity::processScript(std::string& /*name*/, std::string path) {
    if (path.empty()) {
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
bool Entity::setScript(const std::string& trigger, ScriptRef& script)
{
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
