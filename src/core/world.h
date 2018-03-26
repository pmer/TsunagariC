/***************************************
** Tsunagari Tile Engine              **
** world.h                            **
** Copyright 2011-2013 Michael Reiley **
** Copyright 2011-2018 Paul Merrill   **
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

#ifndef SRC_CORE_WORLD_H_
#define SRC_CORE_WORLD_H_

#include <functional>
#include <stack>
#include <string>

#include "core/vec.h"
#include "core/window.h"  // for KeyboardKey
#include "util/bitrecord.h"
#include "util/arc.h"
#include "util/unique.h"

class Area;
struct DisplayList;
class Image;
class Player;

/**
 * Top class holding all data necessary to create a game. Such a collection of
 * data is called a "world". Materially, a world is just a set of graphics,
 * sound effects, music, and scripts. From the perspective from a player, each
 * world should be a separate game. Tsunagari is an engine that powers worlds.
 */
class World {
 public:
    /**
     * Get the currently open World.
     */
    static World& instance();

    World();
    ~World() = default;

    /**
     * Initialize the world for use.
     */
    bool init();

    /**
     * Syncronized time value used throughout the engine.
     */
    time_t time() const;

    /**
     * Process key presses.
     */
    void buttonDown(KeyboardKey btn);
    void buttonUp(KeyboardKey btn);

    /**
     * Draw game state to the screen.
     */
    void draw(DisplayList* display);

    /**
     * Do we need to redraw the screen?
     */
    bool needsRedraw() const;

    void update(time_t now);

    /**
     * Updates the game state within this World as if dt milliseconds had
     * passed since the last call.
     *
     *                       MOVE MODE
     *                 TURN     TILE     NOTILE
     * Area               yes      yes      yes
     * Character       no       yes      yes
     * Overlay         yes      yes      yes
     */
    void tick(time_t dt);

    /**
     * Update the game world when the turn is over (Player moves).
     *
     *                       MOVE MODE
     *                 TURN     TILE     NOTILE
     * Area               yes      no       no
     * Character       yes      no       no
     * Overlay         yes      no       no
     */
    void turn();

    /**
     * Switch the game to a new Area, moving the player to the specified
     * position in the Area.
     */
    bool focusArea(const std::string& filename, vicoord playerPos);
    void focusArea(Area* area, vicoord playerPos);

    void setPaused(bool b);

    void storeKeys();
    void restoreKeys();

    void runAreaLoadScript(Area* area);

    //! Expunge old resources cached in memory. Decisions on which are
    //! removed and which are kept are based on the global Conf struct.
    void garbageCollect();

    // ScriptRef keydownScript, keyupScript;

 protected:
    /**
     * Calculate time passed since engine state was last updated.
     */
    time_t calculateDt(time_t now);

    /**
     * Draws black borders around the screen. Used to correct the aspect
     * ratio and optimize drawing if the Area doesn't fit into the
     * Viewport.
     */
    void pushLetterbox(std::function<void()> op);

 protected:
    typedef std::map<std::string, Unique<Area>> AreaMap;

    Rc<Image> pauseInfo;

    AreaMap areas;
    Area* area;
    Unique<Player> player;

    /**
     * Last time engine state was updated. See World::update().
     */
    time_t lastTime;

    /**
     * Total unpaused game run time.
     */
    time_t total;

    bool alive;
    bool redraw;
    bool userPaused;
    int paused;

    std::stack<BitRecord> keyStates;
};

#endif  // SRC_CORE_WORLD_H_
