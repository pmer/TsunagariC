/***************************************
** Tsunagari Tile Engine              **
** area.h                             **
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

#ifndef SRC_CORE_AREA_H_
#define SRC_CORE_AREA_H_

#include "core/entity.h"
#include "core/tile-grid.h"
#include "core/tile.h"
#include "core/vec.h"
#include "core/window.h"
#include "data/data-area.h"
#include "util/hashtable.h"
#include "util/optional.h"
#include "util/string-view.h"
#include "util/string.h"

#define ISOMETRIC_ZOFF_PER_TILE 0.001

class Character;
struct DisplayList;
class NPC;
class Overlay;
class Player;

//! An Area represents one map, or screen, in a World.
/*!
    The Area class manages a three-dimensional structure of Tiles and a set
    of Entities.

    The game's Viewport must be "focused" on an Area. Only one Area can be
    focused on at a time.

    The viewport will not scroll past the edge of an Area. (At least as of
    June 2012. :)
*/
class Area {
 public:
    Area(Player* player, StringView filename);
    virtual ~Area() = default;

    //! Parse the file specified in the constructor, generating a full Area
    //! object. Must be called before use.
    virtual bool init() = 0;

    //! Prepare game state for this Area to be in focus.
    void focus();

    //! Processes keyboard input, calling the Player object when necessary.
    void buttonDown(KeyboardKey key);
    void buttonUp(KeyboardKey key);

    //! Renders all visible Tiles and Entities within this Area.
    void draw(DisplayList* display);

    //! If false, drawing might be skipped. Saves CPU cycles when idle.
    bool needsRedraw();

    //! Inform the Area that a redraw is needed.
    void requestRedraw();

    /**
     * Update the game state within this Area as if dt milliseconds had
     * passed since the last call. Updates Entities, runs scripts, and
     * checks for Tile animation updates.
     */
    void tick(time_t dt);

    /**
     * Updates Entities, runs scripts, and checks for Tile animation
     * updates.
     */
    void turn();

    uint32_t getColorOverlay();
    void setColorOverlay(uint8_t a, uint8_t r, uint8_t g, uint8_t b);

    const Tile* getTile(icoord phys) const;
    const Tile* getTile(vicoord virt) const;
    const Tile* getTile(rcoord virt) const;
    Tile* getTile(icoord phys);
    Tile* getTile(vicoord virt);
    Tile* getTile(rcoord virt);

    TileSet* getTileSet(StringView imagePath);

    //! Return the dimensions of the Tile matrix.
    ivec3 getDimensions() const;
    //! Return the pixel dimensions of a Tile graphic.
    ivec2 getTileDimensions() const;
    //! Returns a physical cubic range of Tiles that are visible on-screen.
    //! Takes actual map size into account.
    icube visibleTiles() const;

    //! Returns true if a Tile exists at the specified coordinate.
    bool inBounds(icoord phys) const;
    bool inBounds(vicoord virt) const;
    bool inBounds(rcoord virt) const;
    bool inBounds(Entity* ent) const;

    bool loopsInX() const;
    bool loopsInY() const;

    // Create an NPC and insert it into the Area.
    Rc<Character> spawnNPC(StringView descriptor,
                           vicoord coord,
                           StringView phase);
    // Create an Overlay and insert it into the Area.
    Rc<Overlay> spawnOverlay(StringView descriptor,
                             vicoord coord,
                             StringView phase);

    // Convert between virtual and physical map coordinates. Physical
    // coordinates are the physical indexes into the Tile matrix. Layer
    // depth is represented by an arbirarily chosen integer in the physical
    // system. Virtual coordinates include the correct floating-point
    // depth.
    vicoord phys2virt_vi(icoord phys) const;
    rcoord phys2virt_r(icoord phys) const;
    icoord virt2phys(vicoord virt) const;
    icoord virt2phys(rcoord virt) const;
    rcoord virt2virt(vicoord virt) const;
    vicoord virt2virt(rcoord virt) const;

    DataArea* getDataArea();

    void runEnterScript(icoord tile, Entity* triggeredBy) noexcept;
    void runLeaveScript(icoord tile, Entity* triggeredBy) noexcept;
    void runUseScript(icoord tile, Entity* triggeredBy) noexcept;

 protected:
    //! Calculate frame to show for each type of tile
    void drawTiles(DisplayList* display, const icube& tiles, int z);
    void drawEntities(DisplayList* display, const icube& tiles, int z);

 protected:
    Hashmap<String, TileSet> tileSets;

    Vector<Animation> tileGraphics;
    Vector<bool> checkedForAnimation;
    Vector<bool> tilesAnimated;

    Vector<Rc<Character>> characters;
    Vector<Rc<Overlay>> overlays;

    TileGrid grid;

    bool beenFocused;
    bool redraw;
    uint32_t colorOverlayARGB;

    DataArea* dataArea;

    Player* player;

    // The following contain filenames such that they may be loaded lazily.
    const String descriptor;

    String name;
    String author;
    Optional<String> musicPath;
};

#endif  // SRC_CORE_AREA_H_
