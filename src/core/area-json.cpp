/***************************************
** Tsunagari Tile Engine              **
** area-json.cpp                      **
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

#include "core/area-json.h"

#include "core/area.h"
#include "core/character.h"
#include "core/entity.h"
#include "core/images.h"
#include "core/jsons.h"
#include "core/log.h"
#include "core/measure.h"
#include "core/overlay.h"
#include "core/resources.h"
#include "core/tile.h"
#include "core/window.h"
#include "core/world.h"
#include "data/data-world.h"
#include "os/c.h"
#include "util/assert.h"
#include "util/int.h"
#include "util/math2.h"
#include "util/move.h"
#include "util/optional.h"
#include "util/string2.h"
#include "util/vector.h"

#define CHECK(x)      \
    if (!(x)) {       \
        return false; \
    }

/* NOTE: In the JSON map format used by Tiled, tileset tiles start counting
         their Y-positions from 0, while layer tiles start counting from 1. I
         can't imagine why the author did this, but we have to take it into
         account.
*/

class AreaJSON : public Area {
 public:
    AreaJSON(Player* player, StringView filename) noexcept;

 private:
    //! Allocate Tile objects for one layer of map.
    void allocateMapLayer(TileGrid::LayerType type) noexcept;

    //! Parse an Area file.
    bool processDescriptor() noexcept;
    bool processMapProperties(Unique<JSONObject> obj) noexcept;
    bool processTileSet(Unique<JSONObject> obj) noexcept;
    bool processTileSetFile(Rc<JSONObject> obj,
                            StringView source,
                            int firstGid) noexcept;
    bool processTileType(Unique<JSONObject> obj,
                         Animation& graphic,
                         TiledImageID img,
                         int id) noexcept;
    bool processLayer(Unique<JSONObject> obj) noexcept;
    bool processLayerProperties(Unique<JSONObject> obj) noexcept;
    bool processLayerData(Unique<JSONArray> arr) noexcept;
    bool processObjectGroup(Unique<JSONObject> obj) noexcept;
    bool processObjectGroupProperties(Unique<JSONObject> obj) noexcept;
    bool processObject(Unique<JSONObject> obj) noexcept;
    bool splitTileFlags(StringView strOfFlags, unsigned* flags) noexcept;
    bool parseExit(StringView dest,
                   Optional<Exit>& exit,
                   bool* wwide,
                   bool* hwide) noexcept;
    bool parseARGB(StringView str,
                   unsigned char& a,
                   unsigned char& r,
                   unsigned char& g,
                   unsigned char& b) noexcept;
};


Area*
makeAreaFromJSON(Player* player, StringView filename) noexcept {
    return new AreaJSON(player, filename);
}


AreaJSON::AreaJSON(Player* player, StringView descriptor) noexcept {
    TimeMeasure m(String() << "Constructed " << descriptor << " as area-json");

    dataArea = DataWorld::instance().area(descriptor);
    this->player = player;
    this->descriptor = descriptor;

    // Add TileType #0. Not used, but Tiled's gids start from 1.
    tileGraphics.resize(1);

    ok = processDescriptor();
}

void
AreaJSON::allocateMapLayer(TileGrid::LayerType type) noexcept {
    ivec3 dim = grid.dim;

    // FIXME: Better int overflow check that multiplies x,y,z together.
    assert_(0 <= dim.y);
    assert_(0 <= dim.x);
    assert_(0 <= dim.z);

    grid.layerTypes.push_back(type);

    grid.graphics.resize(grid.graphics.size() + dim.x * dim.y);
    grid.dim.z++;
}

bool
AreaJSON::processDescriptor() noexcept {
    Rc<JSONObject> doc = JSONs::load(descriptor);

    CHECK(doc);

    CHECK(doc->hasUnsigned("width"));
    CHECK(doc->hasUnsigned("height"));

    grid.dim.x = doc->unsignedAt("width");
    grid.dim.y = doc->unsignedAt("height");
    grid.dim.z = 0;

    CHECK(doc->hasObject("properties"));
    CHECK(processMapProperties(doc->objectAt("properties")));

    CHECK(doc->hasArray("tilesets"));
    Unique<JSONArray> tilesets = doc->arrayAt("tilesets");
    CHECK(tilesets->size() > 0);

    for (size_t i = 0; i < tilesets->size(); i++) {
        CHECK(tilesets->isObject(i));
        CHECK(processTileSet(tilesets->objectAt(i)));
    }

    CHECK(doc->hasArray("layers"));
    Unique<JSONArray> layers = doc->arrayAt("layers");
    CHECK(layers->size() > 0);

    for (size_t i = 0; i < layers->size(); i++) {
        CHECK(layers->isObject(i));
        Unique<JSONObject> layer = layers->objectAt(i);

        CHECK(layer->hasString("type"));
        StringView type = layer->stringAt("type");

        if (type == "tilelayer") {
            CHECK(processLayer(move_(layer)));
        }
        else if (type == "objectgroup") {
            CHECK(processObjectGroup(move_(layer)));
        }
        else {
            Log::err(descriptor,
                     "Each layer must be a tilelayer or objectlayer");
            return false;
        }
    }

    return true;
}

bool
AreaJSON::processMapProperties(Unique<JSONObject> obj) noexcept {
    /*
     {
       "name": "Wooded Area"
       "music": "wind.oga",
       "loop": "xy",
       "color_overlay": "255,255,255,127"
     }
    */

    if (!obj->hasString("name")) {
        Log::err(descriptor, "Area must have \"name\" property");
    }

    name = obj->stringAt("name");

    if (obj->hasString("music")) {
        musicPath = obj->stringAt("music");
    }
    if (obj->hasString("loop")) {
        StringView directions = obj->stringAt("loop");
        grid.loopX = directions.find('x');
        grid.loopY = directions.find('y');
    }
    if (obj->hasString("color_overlay")) {
        unsigned char a, r, g, b;
        CHECK(parseARGB(obj->stringAt("color_overlay"), a, r, g, b));
        colorOverlayARGB = (uint32_t)(a << 24) + (uint32_t)(r << 16) +
                           (uint32_t)(g << 8) + (uint32_t)b;
    }

    return true;
}

/**
 * dirname
 *
 * Returns the directory component of a path, including trailing slash.  If
 * there is no directory component, return an empty string.
 */
static StringView
dirname(StringView path) noexcept {
    StringPosition slash = path.rfind('/');
    return !slash ? "" : path.substr(0, static_cast<size_t>(*slash) + 1);
}

bool
AreaJSON::processTileSet(Unique<JSONObject> obj) noexcept {
    /*
     {
       "firstgid": 1,
       "source": "tiles\/forest.png.json"
     }
    */
    CHECK(obj->hasUnsigned("firstgid"));
    const unsigned firstGid = obj->unsignedAt("firstgid");

    CHECK(obj->hasString("source"));
    String source = String() << dirname(descriptor) << obj->stringAt("source");

    // We don't handle embeded tilesets, only references to an external JSON
    // files.
    Rc<JSONObject> doc = JSONs::load(source);
    if (!doc) {
        Log::err(descriptor,
                 String() << source << ": failed to load JSON file");
        return false;
    }

    if (!processTileSetFile(doc, source, firstGid)) {
        Log::err(descriptor,
                 String() << source << ": failed to parse JSON tileset file");
        return false;
    }

    return true;
}

bool
AreaJSON::processTileSetFile(Rc<JSONObject> obj,
                             StringView source,
                             int firstGid) noexcept {
    /*
     {
       "image": "forest.png",
       "imageheight": 304,
       "imagewidth": 272,
       "name": "forest.png",
       "tilecount": 323,
       "tileheight": 16,
       "tileproperties": {
         "29": {
           "frames": "29,58",
           "speed": "0.75"
         }
       },
       "tilewidth": 16
     }
    */

    assert_(firstGid == tileGraphics.size());

    unsigned tilex, tiley;
    unsigned pixelw, pixelh;
    unsigned width, height;

    CHECK(obj->hasString("image"));
    CHECK(obj->hasUnsigned("imageheight"));
    CHECK(obj->hasUnsigned("imagewidth"));
    CHECK(obj->hasString("name"));
    CHECK(obj->hasUnsigned("tileheight"));
    CHECK(obj->hasUnsigned("tilewidth"));

    tilex = obj->unsignedAt("tilewidth");
    tiley = obj->unsignedAt("tileheight");

    CHECK(tilex > 0 && tiley > 0);
    CHECK(tilex <= 0x7FFF && tiley <= 0x7FFF);  // Reasonable limit?

    if (grid.tileDim && grid.tileDim.x != tilex && grid.tileDim.y != tiley) {
        Log::err(descriptor,
                 "Tileset's width/height contradict earlier <layer>");
        return false;
    }
    grid.tileDim = ivec2{static_cast<int>(tilex), static_cast<int>(tiley)};

    pixelw = obj->unsignedAt("imagewidth");
    pixelh = obj->unsignedAt("imageheight");

    width = pixelw / grid.tileDim.x;
    height = pixelh / grid.tileDim.y;

    String imgSource = String() << dirname(source) << obj->stringAt("image");
    tileSets[imgSource] = TileSet{firstGid, (size_t)width, (size_t)height};

    // Load tileset image.
    TiledImageID images = Images::loadTiles(imgSource, tilex, tiley);
    if (!images) {
        Log::err(descriptor, "Tileset image not found");
        return false;
    }

    int nTiles = TiledImage::size(images);
    tileGraphics.reserve(nTiles);

    // Initialize "vanilla" tile type array.
    for (int i = 0; i < nTiles; i++) {
        ImageID image = TiledImage::getTile(images, i);
        tileGraphics.push_back(Animation(image));
    }

    if (obj->hasObject("tileproperties")) {
        // Handle explicitly declared "non-vanilla" types.

        Unique<JSONObject> tilesProperties = obj->objectAt("tileproperties");
        Vector<StringView> tileIds = tilesProperties->names();

        for (auto& id : tileIds) {
            // Must be an object... can't be an int... :)
            CHECK(tilesProperties->hasObject(id));

            Unique<JSONObject> tileProperties = tilesProperties->objectAt(id);

            // "id" is 0-based index of a tile in the current
            // tileset, if the tileset were a flat array.
            Optional<unsigned> id_ = parseUInt(id);
            if (!id_) {
                Log::err(descriptor, "Tile type id is invalid");
                return false;
            }
            if (*id_ > INT32_MAX) {
                Log::err(descriptor, "Tile type id is invalid");
                return false;
            }
            int id__ = static_cast<int>(*id_);
            if (nTiles <= id__) {
                Log::err(descriptor, "Tile type id is invalid");
                return false;
            }

            // "gid" is the global area-wide id of the tile.
            int gid = id__ + firstGid;

            Animation& graphic = tileGraphics[gid];
            if (!processTileType(move_(tileProperties),
                                 graphic,
                                 images,
                                 static_cast<int>(id__))) {
                return false;
            }
        }
    }

    return true;
}

bool
AreaJSON::processTileType(Unique<JSONObject> obj,
                          Animation& graphic,
                          TiledImageID images,
                          int id) noexcept {
    /*
      {
        "frames": "29,58",
        "speed": "0.75"
      }
    */

    // The id has already been handled by processTileSet, so we don't have
    // to worry about it.

    // If a Tile is animated, it needs both member frames and a speed.
    Vector<ImageID> framesvec;
    Optional<int> frameLen;

    int nTiles = TiledImage::size(images);

    if (obj->hasString("frames")) {
        String memtemp;
        Vector<StringView> frames;

        memtemp = obj->stringAt("frames");
        frames = splitStr(memtemp, ",");

        // Make sure the first member is this tile.
        Optional<int> firstFrame = parseInt(frames[0]);
        if (!firstFrame || *firstFrame != id) {
            Log::err(descriptor,
                     String() << "first member of tile id " << id
                              << " animation must be itself.");
            return false;
        }

        // Add frames to our animation.
        // We already have one from TileType's constructor.
        for (StringView& frame : frames) {
            Optional<unsigned> idx = parseUInt(frame);
            if (!idx) {
                Log::err(descriptor,
                         "couldn't parse frame index for animated tile");
                return false;
            }
            if (*idx > INT32_MAX) {
                Log::err(descriptor, "frame index out of bounds");
                return false;
            }

            int idx_ = static_cast<int>(*idx);

            if (nTiles <= idx_) {
                Log::err(descriptor,
                         "frame index out of range for animated tile");
                return false;
            }

            framesvec.push_back(TiledImage::getTile(images, idx_));
        }
    }
    if (obj->hasString("speed")) {
        StringView _hertz = obj->stringAt("speed");
        Optional<float> hertz = parseFloat(_hertz);
        CHECK(hertz);
        frameLen = (int)(1000.0 / *hertz);
    }

    if (framesvec.size() || frameLen) {
        if (framesvec.empty() || !frameLen) {
            Log::err(
                    descriptor,
                    "Tile type must either have both frames and speed or none");
            return false;
        }
        // Add 'now' to Animation constructor??
        time_t now = World::time();
        graphic = Animation(move_(framesvec), *frameLen);
        graphic.startOver(now);
    }

    return true;
}

bool
AreaJSON::processLayer(Unique<JSONObject> obj) noexcept {
    /*
     {
       "data": [9, 9, 9, ..., 3, 9, 9],
       "height": 33,
       "properties": {
         ...
       },
       "width": 34,
     }
    */

    CHECK(obj->hasInt("width"));
    CHECK(obj->hasInt("height"));

    const int x = obj->intAt("width");
    const int y = obj->intAt("height");

    if (grid.dim.x != x || grid.dim.y != y) {
        Log::err(descriptor, "layer x,y size != map x,y size");
        return false;
    }

    allocateMapLayer(TileGrid::LayerType::TILE_LAYER);

    CHECK(obj->hasObject("properties"));
    CHECK(processLayerProperties(obj->objectAt("properties")));

    CHECK(obj->hasArray("data"));
    CHECK(processLayerData(obj->arrayAt("data")));

    return true;
}

bool
AreaJSON::processLayerProperties(Unique<JSONObject> obj) noexcept {
    /*
     {
       "depth": "-0.5"
     }
    */

    if (!obj->hasStringFloat("depth")) {
        Log::err(descriptor, "A tilelayer must have the \"depth\" property");
        return false;
    }

    const float depth = obj->stringFloatAt("depth");

    if (grid.depth2idx.find(depth) != grid.depth2idx.end()) {
        Log::err(descriptor, "Layers cannot share a depth");
        return false;
    }

    grid.depth2idx[depth] = grid.dim.z - 1;
    grid.idx2depth.push_back(
            depth);  // Effectively idx2depth[dim.z - 1] = depth;

    return true;
}

bool
AreaJSON::processLayerData(Unique<JSONArray> arr) noexcept {
    /*
     [9, 9, 9, ..., 3, 9, 9]
    */

    const size_t z = static_cast<size_t>(grid.dim.z) - 1;

    // If we ever allow finding layers out of order.
    // assert_(0 <= z && z < dim.z);

    size_t x = 0, y = 0;

    for (size_t i = 0; i < arr->size(); i++) {
        CHECK(arr->isUnsigned(i));
        unsigned gid = arr->unsignedAt(i);

        if (gid >= tileGraphics.size()) {
            Log::err(descriptor, "Invalid tile gid");
            return false;
        }

        size_t idx = (z * grid.dim.y + y) * grid.dim.x + x;

        // A gid of zero means there is no tile at this
        // position on this layer.
        grid.graphics[idx] = gid;

        if (++x == (size_t)grid.dim.x) {
            x = 0;
            y++;
        }
    }

    return true;
}

bool
AreaJSON::processObjectGroup(Unique<JSONObject> obj) noexcept {
    /*
     {
       "name": "Prop(1)",
       "objects": [...],
       "properties": {...}
     }
    */

    CHECK(obj->hasObject("properties"));
    CHECK(processObjectGroupProperties(obj->objectAt("properties")));

    CHECK(obj->hasArray("objects"));
    Unique<JSONArray> objects = obj->arrayAt("objects");

    for (size_t i = 0; i < objects->size(); i++) {
        CHECK(objects->isObject(i));
        CHECK(processObject(objects->objectAt(i)));
    }

    return true;
}

bool
AreaJSON::processObjectGroupProperties(Unique<JSONObject> obj) noexcept {
    /*
     {
       "depth": "0.0"
     }
    */

    if (!obj->hasStringFloat("depth")) {
        Log::err(descriptor, "An objectlayer must have the \"depth\" property");
        return false;
    }

    const float depth = obj->stringFloatAt("depth");

    if (grid.depth2idx.find(depth) != grid.depth2idx.end()) {
        Log::err(descriptor, "Layers cannot share a depth");
        return false;
    }

    allocateMapLayer(TileGrid::LayerType::OBJECT_LAYER);
    grid.depth2idx[depth] = grid.dim.z - 1;
    grid.idx2depth.push_back(
            depth);  // Effectively idx2depth[dim.z - 1] = depth;

    return true;
}

bool
AreaJSON::processObject(Unique<JSONObject> obj) noexcept {
    /*
     {
       "height": 16,
       "properties": {
         "onEnter": "half_speed"
         "onLeave": "normal_speed"
         "onuse": "normal_speed"
         "exit": "grassfield.area,1,1,0"
         "flags": "npc_nowalk"
       },
       "width": 16,
       "x": 256,
       "y": 272
     }
    */

    if (!obj->hasObject("properties")) {
        // Empty tile object. Odd, but acceptable.
        return true;
    }

    const size_t z = static_cast<size_t>(grid.dim.z) - 1;

    // If we ever allow finding layers out of order.
    // assert_(0 <= z && z < dim.z);

    // Gather object properties now. Assign them to tiles later.
    bool wwide[5] = {}, hwide[5] = {};  // Wide exit in width or height.

    DataArea::TileScript enterScript = nullptr,
                         leaveScript = nullptr,
                         useScript = nullptr;
    Optional<Exit> exit[5];
    Optional<float> layermods[5];
    unsigned flags = 0x0;

    Unique<JSONObject> props = obj->objectAt("properties");

    if (props->hasString("flags")) {
        CHECK(splitTileFlags(props->stringAt("flags"), &flags));
    }

    if (props->hasString("on_enter")) {
        StringView scriptName = props->stringAt("on_enter");
        enterScript = dataArea->scripts[scriptName];
    }
    if (props->hasString("on_leave")) {
        StringView scriptName = props->stringAt("on_leave");
        leaveScript = dataArea->scripts[scriptName];
    }
    if (props->hasString("on_use")) {
        StringView scriptName = props->stringAt("on_use");
        useScript = dataArea->scripts[scriptName];
    }

    if (props->hasString("exit")) {
        StringView value = props->stringAt("exit");
        CHECK(parseExit(value,
                        exit[EXIT_NORMAL],
                        &wwide[EXIT_NORMAL],
                        &hwide[EXIT_NORMAL]));
        flags |= TILE_NOWALK_NPC;
    }
    if (props->hasString("exit:up")) {
        StringView value = props->stringAt("exit:up");
        CHECK(parseExit(
                value, exit[EXIT_UP], &wwide[EXIT_UP], &hwide[EXIT_UP]));
    }
    if (props->hasString("exit:down")) {
        StringView value = props->stringAt("exit:down");
        CHECK(parseExit(
                value, exit[EXIT_DOWN], &wwide[EXIT_DOWN], &hwide[EXIT_DOWN]));
    }
    if (props->hasString("exit:left")) {
        StringView value = props->stringAt("exit:left");
        CHECK(parseExit(
                value, exit[EXIT_LEFT], &wwide[EXIT_LEFT], &hwide[EXIT_LEFT]));
    }
    if (props->hasString("exit:right")) {
        StringView value = props->stringAt("exit:right");
        CHECK(parseExit(value,
                        exit[EXIT_RIGHT],
                        &wwide[EXIT_RIGHT],
                        &hwide[EXIT_RIGHT]));
    }

    if (props->hasStringFloat("layermod")) {
        float mod = props->stringFloatAt("layermod");
        layermods[EXIT_NORMAL] = mod;
        flags |= TILE_NOWALK_NPC;
    }
    if (props->hasStringFloat("layermod:up")) {
        float mod = props->stringFloatAt("layermod:up");
        layermods[EXIT_UP] = mod;
    }
    if (props->hasStringFloat("layermod:down")) {
        float mod = props->stringFloatAt("layermod:down");
        layermods[EXIT_DOWN] = mod;
    }
    if (props->hasStringFloat("layermod:left")) {
        float mod = props->stringFloatAt("layermod:left");
        layermods[EXIT_LEFT] = mod;
    }
    if (props->hasStringFloat("layermod:right")) {
        float mod = props->stringFloatAt("layermod:right");
        layermods[EXIT_RIGHT] = mod;
    }

    // Apply these properties directly to one or more tiles in a rectangle
    // of the map. We don't keep an intermediary "object" object lying
    // around.

    CHECK(obj->hasInt("x"));
    CHECK(obj->hasInt("y"));
    CHECK(obj->hasInt("width"));
    CHECK(obj->hasInt("height"));
    const int x = obj->intAt("x") / grid.tileDim.x;
    const int y = obj->intAt("y") / grid.tileDim.y;
    const int w = obj->intAt("width") / grid.tileDim.x;
    const int h = obj->intAt("height") / grid.tileDim.y;

    CHECK(x + w <= grid.dim.x);
    CHECK(y + h <= grid.dim.y);

    // We know which Tiles are being talked about now... yay
    for (int Y = y; Y < y + h; Y++) {
        for (int X = x; X < x + w; X++) {
            icoord tile = {X, Y, static_cast<int>(z)};
            size_t hash = hash_(tile);

            grid.flags.at(tile, hash) |= flags;
            for (size_t i = 0; i < EXITS_LENGTH; i++) {
                if (exit[i]) {
                    int dx = X - x;
                    int dy = Y - y;
                    if (wwide[i]) {
                        exit[i]->coords.x += dx;
                    }
                    if (hwide[i]) {
                        exit[i]->coords.y += dy;
                    }
                    grid.exits[i].at(tile, hash) = move_(*exit[i]);
                }
            }
            for (size_t i = 0; i < EXITS_LENGTH; i++) {
                if (layermods[i]) {
                    grid.layermods[i].at(tile, hash) = *layermods[i];
                }
            }

            if (enterScript) {
                grid.scripts[TileGrid::SCRIPT_TYPE_ENTER].at(tile, hash)
                    = enterScript;
            }
            if (leaveScript) {
                grid.scripts[TileGrid::SCRIPT_TYPE_LEAVE].at(tile, hash)
                    = leaveScript;
            }
            if (useScript) {
                grid.scripts[TileGrid::SCRIPT_TYPE_USE].at(tile, hash)
                    = useScript;
            }
        }
    }

    return true;
}

bool
AreaJSON::splitTileFlags(StringView strOfFlags, unsigned* flags) noexcept {
    for (auto str : splitStr(strOfFlags, ",")) {
        if (str == "nowalk") {
            *flags |= TILE_NOWALK;
        }
        else if (str == "nowalk_player") {
            *flags |= TILE_NOWALK_PLAYER;
        }
        else if (str == "nowalk_npc") {
            *flags |= TILE_NOWALK_NPC;
        }
        else {
            Log::err(descriptor, String() << "Invalid tile flag: " << str);
            return false;
        }
    }

    return true;
}

/**
 * Matches regex /^\s*\d+\+?$/
 */
static bool
isIntegerOrPlus(StringView s) noexcept {
    const int space = 0;
    const int digit = 1;
    const int sign = 2;

    int state = space;

    for (char c : s) {
        if (state == space) {
            if (c == ' ') {
                continue;
            }
            else {
                state++;
            }
        }
        if (state == digit) {
            if ('0' <= c && c <= '9') {
                continue;
            }
            else {
                state++;
            }
        }
        if (state == sign) {
            return c == '+';
        }
    }
    return true;
}

bool
AreaJSON::parseExit(StringView dest,
                    Optional<Exit>& exit,
                    bool* wwide,
                    bool* hwide) noexcept {
    /*
      Format: destination area, x, y, z
      E.g.:   "babysfirst.area,1,3,0"
    */

    Vector<StringView> strs = splitStr(dest, ",");

    if (strs.size() != 4) {
        Log::err(descriptor, "exit: Invalid format");
        return false;
    }

    StringView area = strs[0];
    StringView x = strs[1];
    StringView y = strs[2];
    StringView z = strs[3];

    if (!isIntegerOrPlus(x) || !isIntegerOrPlus(y) || !isIntegerOrPlus(z)) {
        Log::err(descriptor, "exit: Invalid format");
        return false;
    }

    if (x.find('+')) {
        x = x.substr(0, x.size - 1);
    }
    if (y.find('+')) {
        y = y.substr(0, y.size - 1);
    }

    Optional<int> x_ = parseInt(x);
    Optional<int> y_ = parseInt(y);
    Optional<float> z_ = parseFloat(z);

    exit = Exit{area, *x_, *y_, *z_};

    *wwide = x.find('+');
    *hwide = y.find('+');

    return true;
}

bool
AreaJSON::parseARGB(StringView str,
                    unsigned char& a,
                    unsigned char& r,
                    unsigned char& g,
                    unsigned char& b) noexcept {
    unsigned char* channels[] = {&a, &r, &g, &b};

    Vector<StringView> strs = splitStr(str, ",");

    if (strs.size() != 4) {
        Log::err(descriptor, "invalid ARGB format");
        return false;
    }

    for (size_t i = 0; i < 4; i++) {
        Optional<int> v = parseInt(strs[i]);
        if (!v) {
            Log::err(descriptor, "invalid ARGB format");
            return false;
        }
        int v_ = *v;
        if (!(0 <= v_ && v_ < 256)) {
            Log::err(descriptor, "ARGB values must be between 0 and 255");
            return false;
        }
        *channels[i] = (unsigned char)v_;
    }

    return true;
}
