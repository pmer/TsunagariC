/***************************************
** Tsunagari Tile Engine              **
** entity.h                           **
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

#ifndef SRC_CORE_ENTITY_H_
#define SRC_CORE_ENTITY_H_

#include "core/animation.h"
#include "core/images.h"
#include "core/jsons.h"
#include "core/vec.h"
#include "util/function.h"
#include "util/hashtable.h"
#include "util/string.h"
#include "util/vector.h"

class Animation;
class Area;
struct DisplayList;

enum SetPhaseResult { PHASE_NOTFOUND, PHASE_NOTCHANGED, PHASE_CHANGED };

// An Entity represents one 'thing' that will be rendered to the screen.
//
// An Entity might be a dynamic game object such as a monster, NPC, or
// item.  Entity can handle animated images that cycle through their
// frames over time. It also has the capacity to switch between a couple
// different images on demand.
//
// For example, you might have a Entity for a player character with
// animated models for walking in each possible movement direction (up,
// down, left, right) along with static standing-still images for each
// direction.
class Entity {
 public:
    Entity() = default;
    virtual ~Entity() = default;

    // Entity initializer
    virtual bool init(StringView descriptor, StringView initialPhase) noexcept;

    // Entity destroyer.
    virtual void destroy() noexcept;

    void draw(DisplayList* display) noexcept;
    bool needsRedraw(const icube& visiblePixels) const noexcept;
    bool isDead() const noexcept;

    virtual void tick(time_t dt) noexcept;
    virtual void turn() noexcept;

    // Normalize each of the X-Y axes into [-1, 0, or 1] and saves value
    // to 'facing'.
    void setFacing(ivec2 facing) noexcept;

    const StringView getFacing() const noexcept;

    // Change the graphic. Returns true if it was changed to something
    // different.
    bool setPhase(StringView name) noexcept;

    ivec2 getImageSize() const noexcept;

    void setAnimationStanding() noexcept;
    void setAnimationMoving() noexcept;


    // The offset from the upper-left of the Area to the upper-left of the
    // Tile the Entity is standing on.
    rcoord getPixelCoord() const noexcept;


    // Gets the Entity's current Area.
    Area* getArea() noexcept;

    // Specifies the Area object this entity will ask when looking for
    // nearby Tiles. Doesn't change x,y,z position.
    virtual void setArea(Area* area) noexcept;


    // Gets speed in pixels per second.
    float getSpeedInPixels() const noexcept;
    // Gets speed in tiles per second.
    float getSpeedInTiles() const noexcept;


    virtual void setFrozen(bool b) noexcept;


    typedef Function<void(time_t)> OnTickFn;
    typedef Function<void()> OnTurnFn;

    void attach(OnTickFn fn) noexcept;
    void attach(OnTurnFn fn) noexcept;

    // Script hooks.
    // ScriptRef tickScript, turnScript, tileEntryScript,
    //            tileExitScript;


 protected:
    // Precalculate various drawing measurements.
    void calcDraw() noexcept;

    // Gets a string describing a direction.
    StringView directionStr(ivec2 facing) const noexcept;

    enum SetPhaseResult _setPhase(StringView name) noexcept;

    void setDestinationCoordinate(rcoord destCoord) noexcept;

    void moveTowardDestination(time_t dt) noexcept;

    // arrived() is called when an Entity arrives at its destination.  If
    // it is ordered to begin moving again from within arrived(), then the
    // Entity’s graphics will appear as if it never stopped moving.
    virtual void arrived() noexcept;

    // JSON parsing functions used in constructing an Entity
    bool processDescriptor() noexcept;
    bool processSprite(Unique<JSONObject> sprite) noexcept;
    bool processPhases(Unique<JSONObject> phases, TiledImageID tiles) noexcept;
    bool processPhase(StringView name,
                      Unique<JSONObject> phase,
                      TiledImageID tiles) noexcept;
    bool processSounds(Unique<JSONObject> sounds) noexcept;
    bool processSound(StringView name, StringView path) noexcept;
    bool processScripts(Unique<JSONObject> scripts) noexcept;
    bool processScript(StringView name, StringView path) noexcept;
    // bool setScript(StringView trigger, ScriptRef& script) noexcept;


 protected:
    // Set to true if the Entity was destroyed this tick.
    bool dead = false;

    // Set to true if the Entity wants the screen to be redrawn.
    bool redraw = true;

    // Pointer to Area this Entity is located on.
    Area* area = nullptr;
    // Real x,y position: hold partial pixel transversal
    rcoord r = {0.0, 0.0, 0.0};
    // Drawing offset to center entity on tile.
    rcoord doff;

    String descriptor;

    bool frozen = false;

    float tilesPerSecond;
    float pixelsPerSecond;

    // True if currently moving to a new coordinate in an Area.
    bool moving = false;

    rcoord destCoord;
    float angleToDest;

    ivec2 imgsz;
    Hashmap<String, Animation> phases;
    Animation* phase = nullptr;
    String phaseName = "";
    ivec2 facing = {0, 0};

    // Map from effect name to filenames.
    //  e.g.: ["step"] = "sounds/player_step.oga"
    Hashmap<String, String> soundPaths;

    Vector<OnTickFn> onTickFns;
    Vector<OnTurnFn> onTurnFns;
};

#endif  // SRC_CORE_ENTITY_H_
