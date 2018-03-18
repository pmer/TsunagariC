/***************************************
** Tsunagari Tile Engine              **
** area.h                             **
** Copyright 2011-2015 Michael Reiley **
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

#ifndef SRC_CORE_AREA_H_
#define SRC_CORE_AREA_H_

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "core/entity.h"
#include "core/tile.h"
#include "core/tile-grid.h"
#include "core/vec.h"
#include "core/window.h"

#include "data/data-area.h"

#include "util/optional.h"

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
    Area(Player* player, const std::string& filename);
    virtual ~Area() = default;

    //! Parse the file specified in the constructor, generating a full Area
    //! object. Must be called before use.
    virtual bool init();

    //! Prepare game state for this Area to be in focus.
    void focus();

    //! Processes keyboard input, calling the Player object when necessary.
    void buttonDown(KeyboardKey key);
    void buttonUp(KeyboardKey key);

    //! Renders all visible Tiles and Entities within this Area.
    void draw(DisplayList* display);

    //! If false, drawing might be skipped. Saves CPU cycles when idle.
    bool needsRedraw() const;

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

    TileSet* getTileSet(const std::string& imagePath);

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

    const std::string getDescriptor() const;

    // Create an NPC and insert it into the Area.
    Rc<NPC> spawnNPC(const std::string& descriptor, vicoord coord,
                     const std::string& phase);
    // Create an Overlay and insert it into the Area.
    Rc<Overlay> spawnOverlay(const std::string& descriptor, vicoord coord,
                             const std::string& phase);

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


 protected:
    //! Calculate frame to show for each type of tile
    void drawTiles(DisplayList* display, const icube& tiles, int z);
    void drawEntities(DisplayList* display, const icube& tiles, int z);

 protected:
    DataArea* dataArea;

    Player* player;
    uint32_t colorOverlayARGB;

    std::unordered_set<Rc<Character>> characters;
    std::unordered_set<Rc<Overlay>> overlays;

    TileGrid<Tile> grid;

    std::unordered_map<std::string, TileSet> tileSets;


    std::string name, author;
    bool beenFocused;
    bool redraw;

    // The following contain filenames such that they may be loaded lazily.
    const std::string descriptor;
    Optional<std::string> musicPath;
};

#endif  // SRC_CORE_AREA_H_
