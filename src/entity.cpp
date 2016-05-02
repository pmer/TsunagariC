/***************************************
** Tsunagari Tile Engine              **
** entity.cpp                         **
** Copyright 2011-2013 PariahSoft LLC **
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

#include <cassert>
#include <math.h>

#include "area.h"
#include "client-conf.h"
#include "entity.h"
#include "images.h"
#include "log.h"
#include "math.h"
#include "resources.h"
#include "string.h"
#include "world.h"
#include "xmls.h"

#define ASSERT(x)  if (!(x)) { return false; }

static std::string directions[][3] = {
    {"up-left",   "up",     "up-right"},
    {"left",      "stance", "right"},
    {"down-left", "down",   "down-right"},
};


Entity::Entity()
    : dead(false),
      redraw(true),
      area(NULL),
      r(0.0, 0.0, 0.0),
      frozen(false),
      speedMul(1.0),
      moving(false),
      phase(NULL),
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
    ASSERT(processDescriptor());
    setPhase(initialPhase);
    return true;
}

void Entity::destroy()
{
    dead = true;
    if (area)
        area->requestRedraw();
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
    if (!redraw && (!phase || !phase->needsRedraw(now)))
        return false;

    // Aren't on-screen
    if (visiblePixels.x2 < r.x || r.x + imgsz.x < visiblePixels.x1)
        return false;
    if (visiblePixels.y2 < r.y || r.y + imgsz.y < visiblePixels.y1)
        return false;

    return true;
}

bool Entity::isDead() const
{
    return dead;
}


void Entity::tick(time_t dt)
{
    for (auto& fn : onTickFns)
        fn(dt);
}

void Entity::turn()
{
    for (auto& fn : onTurnFns)
        fn();
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
        if (res == PHASE_NOTFOUND)
            Log::err(descriptor, "phase '" + name + "' not found");
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
        phase->startOver(now, ANIM_INFINITE_CYCLES);
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
    if (!moving)
        return;

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
 * DESCRIPTOR CODE BELOW
 */

bool Entity::processDescriptor()
{
    auto doc = XMLs::instance().load(descriptor, "entity");
    if (!doc)
        return false;
    const XMLNode root = doc->root(); // <entity>
    if (!root)
        return false;

    for (XMLNode node = root.childrenNode(); node; node = node.next()) {
        if (node.is("speed")) {
            ASSERT(node.doubleContent(&baseSpeed));
            setSpeedMultiplier(speedMul); // Calculate speed from tile size.
        } else if (node.is("sprite")) {
            ASSERT(processSprite(node.childrenNode()));
        } else if (node.is("sounds")) {
            ASSERT(processSounds(node.childrenNode()));
        } else if (node.is("scripts")) {
            ASSERT(processScripts(node.childrenNode()));
        }
    }
    return true;
}

bool Entity::processSprite(XMLNode node)
{
    std::shared_ptr<TiledImage> tiles;
    for (; node; node = node.next()) {
        if (node.is("sheet")) {
            std::string imageSheet = node.content();
            ASSERT(node.intAttr("tile_width",  &imgsz.x) &&
                   node.intAttr("tile_height", &imgsz.y));
            tiles = Images::instance().loadTiles(imageSheet, imgsz.x, imgsz.y);
            ASSERT(tiles);
        } else if (node.is("phases")) {
            ASSERT(processPhases(node.childrenNode(), tiles));
        }
    }
    return true;
}

bool Entity::processPhases(XMLNode node,
        const std::shared_ptr<TiledImage>& tiles)
{
    for (; node; node = node.next())
        if (node.is("phase"))
            ASSERT(processPhase(node, tiles));
    return true;
}

bool Entity::processPhase(const XMLNode node,
        const std::shared_ptr<TiledImage>& tiles)
{
    /* Each phase requires a 'name' and 'frames'. Additionally,
     * 'speed' is required if 'frames' has more than one member.
     */
    const std::string name = node.attr("name");
    if (name.empty()) {
        Log::err(descriptor, "<phase> name attribute is empty");
        return false;
    }

    const std::string framesStr = node.attr("frames");
    const std::string speedStr = node.attr("speed");

    if (framesStr.empty()) {
        Log::err(descriptor, "<phase> frames attribute empty");
        return false;
    }

    if (isInteger(framesStr)) {
        int frame = atoi(framesStr.c_str());
        if (frame < 0 || (int)tiles->size() < frame) {
            Log::err(descriptor,
                "<phase> frames attribute index out of bounds");
            return false;
        }
        const auto& image = (*tiles.get())[(size_t)frame];
        phases[name] = Animation(image);
    }
    else if (isRanges(framesStr)) {
        if (!isDecimal(speedStr)) {
            Log::err(descriptor,
                "<phase> speed attribute must be present and "
                "must be decimal");
        }
        double fps = atof(speedStr.c_str());

        typedef std::vector<int> IntVector;
        IntVector frames = parseRanges(framesStr);
        std::vector<std::shared_ptr<Image>> images;
        for (IntVector::iterator it = frames.begin(); it != frames.end(); it++) {
            int i = *it;
            if (i < 0 || (int)tiles->size() < i) {
                Log::err(descriptor,
                    "<phase> frames attribute index out of bounds");
                return false;
            }
            images.push_back((*tiles.get())[(size_t)i]);
        }

        phases[name] = Animation(images, (time_t)(1000.0 / fps));
    }
    else {
        Log::err(descriptor,
            "<phase> frames attribute not an int or int ranges");
        return false;
    }

    return true;
}

bool Entity::processSounds(XMLNode node)
{
    for (; node; node = node.next())
        if (node.is("sound"))
            ASSERT(processSound(node));
    return true;
}

bool Entity::processSound(const XMLNode node)
{
    const std::string name = node.attr("name");
    const std::string filename = node.content();
    if (name.empty()) {
        Log::err(descriptor, "<sound> name attribute is empty");
        return false;
    } else if (filename.empty()) {
        Log::err(descriptor, "<sound></sound> is empty");
        return false;
    }

    soundPaths[name] = filename;
    return true;
}

bool Entity::processScripts(XMLNode node)
{
    for (; node; node = node.next())
        if (node.is("script"))
            ASSERT(processScript(node));
    return true;
}

bool Entity::processScript(const XMLNode node)
{
    const std::string trigger = node.attr("trigger");
    const std::string filename = node.content();
    if (trigger.empty()) {
        Log::err(descriptor, "<script> trigger attribute is empty");
        return false;
    } else if (filename.empty()) {
        Log::err(descriptor, "<script></script> is empty");
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

