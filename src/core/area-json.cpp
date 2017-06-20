/***************************************
** Tsunagari Tile Engine              **
** area-json.cpp                      **
** Copyright 2011-2013 Michael Reiley **
** Copyright 2011-2016 Paul Merrill   **
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

#include <assert.h>
#include <math.h>

#include <limits>
#include <memory>
#include <string>
#include <vector>

#include "core/area.h"
#include "core/entity.h"
#include "core/images.h"
#include "core/jsons.h"
#include "core/log.h"
#include "core/resources.h"
#include "util/string2.h"
#include "core/tile.h"
#include "core/window.h"
#include "core/world.h"
#include "util/move.h"
#include "util/optional.h"

#ifdef _WIN32
    #include "os/windows.h"
#endif

#define CHECK(x)  if (!(x)) { return false; }

/* NOTE: In the JSON map format used by Tiled, tileset tiles start counting
         their Y-positions from 0, while layer tiles start counting from 1. I
         can't imagine why the author did this, but we have to take it into
         account.
*/

class AreaJSON : public Area {
 public:
    AreaJSON(Player* player, const std::string& filename);

    //! Parse the file specified in the constructor, generating a full Area
    //! object. Must be called before use.
    virtual bool init();

 private:
    //! Allocate Tile objects for one layer of map.
    void allocateMapLayer();

    //! Parse an Area file.
    bool processDescriptor();
    bool processMapProperties(JSONObjectPtr obj);
    bool processTileSet(JSONObjectPtr obj);
    bool processTileSetFile(JSONObjectRef obj, const std::string& source,
                            int firstGid);
    bool processTileType(JSONObjectPtr obj, TileType& type,
                         Arc<TiledImage>& img, int id);
    bool processLayer(JSONObjectPtr obj);
    bool processLayerProperties(JSONObjectPtr obj);
    bool processLayerData(JSONArrayPtr arr);
    bool processObjectGroup(JSONObjectPtr obj);
    bool processObjectGroupProperties(JSONObjectPtr obj);
    bool processObject(JSONObjectPtr obj);
    bool splitTileFlags(const std::string& strOfFlags, unsigned* flags);
    bool parseExit(const std::string& dest,
                   Optional<Exit>& exit,
                   bool* wwide, bool* hwide);
    bool parseARGB(const std::string& str,
                   unsigned char& a, unsigned char& r,
                   unsigned char& g, unsigned char& b);

    std::vector<TileType*> gids;
};


Area* makeAreaFromJSON(Player* player, const std::string& filename) {
    return new AreaJSON(player, filename);
}


AreaJSON::AreaJSON(Player* player, const std::string& descriptor)
        : Area(player, descriptor) {
    // Add TileType #0. Not used, but Tiled's gids start from 1.
    gids.push_back(nullptr);
}

bool AreaJSON::init() {
    return processDescriptor();
}


void AreaJSON::allocateMapLayer() {
    assert(0 <= dim.y && dim.y <= std::numeric_limits<int>::max());
    assert(0 <= dim.x && dim.x <= std::numeric_limits<int>::max());
    assert(0 <= dim.z && dim.z + 1 <= std::numeric_limits<int>::max());

    map.push_back(grid_t((size_t)dim.y, row_t((size_t)dim.x)));
    grid_t& grid = map[(size_t)dim.z];
    for (int y = 0; y < dim.y; y++) {
        row_t& row = grid[(size_t)y];
        for (int x = 0; x < dim.x; x++) {
            Tile& tile = row[(size_t)x];
            new (&tile) Tile(this, x, y, dim.z);
        }
    }
    dim.z++;
}

bool AreaJSON::processDescriptor() {
    const JSONObjectRef doc = JSONs::instance().load(descriptor);

    CHECK(doc);

    CHECK(doc->hasUnsigned("width"));
    CHECK(doc->hasUnsigned("height"));

    dim.x = doc->unsignedAt("width");
    dim.y = doc->unsignedAt("height");
    dim.z = 0;

    CHECK(doc->hasObject("properties"));
    CHECK(processMapProperties(doc->objectAt("properties")));

    CHECK(doc->hasArray("tilesets"));
    const JSONArrayPtr tilesets = doc->arrayAt("tilesets");

    for (size_t i = 0; i < tilesets->size(); i++) {
        CHECK(tilesets->isObject(i));
        CHECK(processTileSet(tilesets->objectAt(i)));
    }

    CHECK(doc->hasArray("layers"));
    const JSONArrayPtr layers = doc->arrayAt("layers");

    for (size_t i = 0; i < layers->size(); i++) {
        CHECK(layers->isObject(i));
        JSONObjectPtr layer = layers->objectAt(i);

        CHECK(layer->hasString("type"));
        const std::string type = layer->stringAt("type");

        if (type == "tilelayer") {
            CHECK(processLayer(std::move(layer)));
        } else if (type == "objectgroup") {
            CHECK(processObjectGroup(std::move(layer)));
        } else {
            Log::err(descriptor, "Each layer must be a tilelayer or objectlayer");
            return false;
        }
    }

    return true;
}

bool AreaJSON::processMapProperties(JSONObjectPtr obj) {

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
        const std::string directions = obj->stringAt("loop");
        loopX = directions.find('x') != std::string::npos;
        loopY = directions.find('y') != std::string::npos;
    }
    if (obj->hasString("color_overlay")) {
        unsigned char a, r, g, b;
        CHECK(parseARGB(obj->stringAt("color_overlay"), a, r, g, b));
        colorOverlayARGB =
            (uint32_t)(a << 24) + (uint32_t)(r << 16) +
            (uint32_t)(g <<  8) + (uint32_t)b;
    }

    return true;
}

/**
 * dirname
 *
 * Returns the directory component of a path, including trailing slash.  If
 * there is no directory component, return an empty string.
 */
static std::string dirname(const std::string& path) {
    size_t slash = path.rfind('/');
    return slash == std::string::npos ? "" : path.substr(0, slash + 1);
}

bool AreaJSON::processTileSet(JSONObjectPtr obj) {

/*
 {
   "firstgid": 1,
   "source": "tiles\/forest.png.json"
 }
*/
    CHECK(obj->hasUnsigned("firstgid"));
    const unsigned firstGid = obj->unsignedAt("firstgid");

    CHECK(obj->hasString("source"));
    const std::string source = dirname(descriptor) + obj->stringAt("source");

    // We don't handle embeded tilesets, only references to an external JSON files.
    const JSONObjectRef doc = JSONs::instance().load(source);
    if (!doc) {
        Log::err(descriptor, source + ": failed to load JSON file");
        return false;
    }

    if (!processTileSetFile(doc, source, firstGid)) {
        Log::err(descriptor, source + ": failed to parse JSON tileset file");
        return false;
    }

    return true;
}

bool AreaJSON::processTileSetFile(JSONObjectRef obj,
                                 const std::string& source,
                                 int firstGid) {

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

    TileSet* set = nullptr;
    Arc<TiledImage> img;

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

    if (tileDim && tileDim != ivec2(tilex, tiley)) {
        Log::err(descriptor,
                 "Tileset's width/height contradict earlier <layer>");
        return false;
    }
    tileDim = ivec2(tilex, tiley);

    pixelw = obj->unsignedAt("imagewidth");
    pixelh = obj->unsignedAt("imageheight");

    width = pixelw / tileDim.x;
    height = pixelh / tileDim.y;

    std::string imgSource = dirname(source) + obj->stringAt("image");
    tileSets[imgSource] = TileSet((size_t)width,
                                  (size_t)height);
    set = &tileSets[imgSource];

    // Load tileset image.
    img = Images::instance().loadTiles(imgSource, tilex, tiley);
    if (!img) {
        Log::err(descriptor, "Tileset image not found");
        return false;
    }

    // Initialize "vanilla" tile type array.
    for (size_t i = 0; i < img->size(); i++) {
        auto tileImg = (*img.get())[i];
        TileType* type = new TileType(std::move(tileImg));
        set->add(type);
        gids.push_back(type);
    }

    if (obj->hasObject("tileproperties")) {
        // Handle explicitly declared "non-vanilla" types.

        JSONObjectPtr tilesProperties = obj->objectAt("tileproperties");
        std::vector<std::string> tileIds = tilesProperties->names();

        for (auto& _id : tileIds) {
            // Must be an object... can't be an int... :)
            CHECK(tilesProperties->hasObject(_id));
            JSONObjectPtr tileProperties = tilesProperties->objectAt(_id);

            // "id" is 0-based index of a tile in the current
            // tileset, if the tileset were a flat array.
            unsigned id;
            if (!parseUInt(_id, &id)) {
                Log::err(descriptor, "Tile type id is invalid");
                return false;
            }
            if (img->size() <= id) {
                Log::err(descriptor, "Tile type id is invalid");
                return false;
            }

            // Initialize a default TileType, we'll build on that.
            TileType* type = new TileType((*img.get())[id]);
            CHECK(processTileType(std::move(tileProperties),
                                  *type, img, id));
            // "gid" is the global area-wide id of the tile.
            size_t gid = (size_t)id + (size_t)firstGid;
            delete gids[gid];  // "vanilla" type
            gids[gid] = type;
            set->set((size_t)id, type);
        }
    }

    return true;
}

bool AreaJSON::processTileType(JSONObjectPtr obj, TileType& type,
                              Arc<TiledImage>& img, int id) {

/*
  {
    "flags": "nowalk",
    "onEnter": "skid"
    "onLeave": "no_skid"
    "onUse": "no_skid"
  }

  {
    "frames": "29,58",
    "speed": "0.75"
  }
*/

    // The id has already been handled by processTileSet, so we don't have
    // to worry about it.

    // If a Tile is animated, it needs both member frames and a speed.
    std::vector<Arc<Image>> framesvec;
    int frameLen = -1;

    if (obj->hasString("flags")) {
        std::string flags = obj->stringAt("flags");
        CHECK(splitTileFlags(flags, &type.flags));
    }
    if (obj->hasString("on_enter")) {
        std::string scriptName = obj->stringAt("on_enter");
        type.enterScript = dataArea->script(scriptName);
    }
    if (obj->hasString("on_leave")) {
        std::string scriptName = obj->stringAt("on_leave");
        type.leaveScript = dataArea->script(scriptName);
    }
    if (obj->hasString("on_use")) {
        std::string scriptName = obj->stringAt("on_use");
        type.useScript = dataArea->script(scriptName);
    }
    if (obj->hasString("frames")) {
        std::string memtemp;
        std::vector<std::string> frames;
        std::vector<std::string>::iterator it;

        memtemp = obj->stringAt("frames");
        frames = splitStr(memtemp, ",");

        // Make sure the first member is this tile.
        if (atoi(frames[0].c_str()) != id) {
            Log::err(descriptor, "first member of tile"
                                         " id " + itostr(id) +
                                 " animation must be itself.");
            return false;
        }

        // Add frames to our animation.
        // We already have one from TileType's constructor.
        for (it = frames.begin(); it < frames.end(); it++) {
            int idx = atoi(it->c_str());
            if (idx < 0 || (int)img->size() <= idx) {
                Log::err(descriptor, "frame index out "
                        "of range for animated tile");
                return false;
            }
            framesvec.push_back((*img.get())[(size_t)idx]);
        }
    }
    if (obj->hasString("speed")) {
        std::string _hertz = obj->stringAt("speed");
        double hertz;
        CHECK(parseDouble(_hertz, &hertz));
        frameLen = (int)(1000.0/hertz);
    }

    if (framesvec.size() || frameLen != -1) {
        if (framesvec.empty() || frameLen == -1) {
            Log::err(descriptor, "Tile type must either have both "
                    "frames and speed or none");
            return false;
        }
        // Add 'now' to Animation constructor??
        time_t now = World::instance().time();
        type.anim = Animation(std::move(framesvec), frameLen);
        type.anim.startOver(now);
    }

    return true;
}

bool AreaJSON::processLayer(JSONObjectPtr obj) {

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

    CHECK(obj->hasUnsigned("width"));
    CHECK(obj->hasUnsigned("height"));

    const unsigned x = obj->unsignedAt("width");
    const unsigned y = obj->unsignedAt("height");

    if (dim.x != x || dim.y != y) {
        Log::err(descriptor, "layer x,y size != map x,y size");
        return false;
    }

    allocateMapLayer();

    CHECK(obj->hasObject("properties"));
    CHECK(processLayerProperties(obj->objectAt("properties")));

    CHECK(obj->hasArray("data"));
    CHECK(processLayerData(obj->arrayAt("data")));

    return true;
}

bool AreaJSON::processLayerProperties(JSONObjectPtr obj) {

/*
 {
   "depth": "-0.5"
 }
*/

    if (!obj->hasStringDouble("depth")) {
        Log::err(descriptor, "A tilelayer must have the \"depth\" property");
        return false;
    }

    const double depth = obj->stringDoubleAt("depth");

    if (depth2idx.find(depth) != depth2idx.end()) {
        Log::err(descriptor, "Layers cannot share a depth");
        return false;
    }

    depth2idx[depth] = dim.z - 1;
    idx2depth.push_back(depth);  // Effectively idx2depth[dim.z - 1] = depth;

    return true;
}

bool AreaJSON::processLayerData(JSONArrayPtr arr) {

/*
 [9, 9, 9, ..., 3, 9, 9]
*/

    const int z = dim.z - 1;

    // If we ever allow finding layers out of order.
    //assert(0 <= z && z < dim.z);

    size_t x = 0, y = 0;

    for (size_t i = 0; i < arr->size(); i++) {
        CHECK(arr->isUnsigned(i));
        unsigned gid = arr->unsignedAt(i);

        if (gids.size() <= gid) {
            Log::err(descriptor, "Invalid tile gid");
            return false;
        }

        // A gid of zero means there is no tile at this
        // position on this layer.
        if (gid > 0) {
            TileType* type = gids[(size_t)gid];
            Tile& tile = map[(size_t)z][y][x];
            type->allOfType.push_back(&tile);
            tile.parent = type;
        }

        if (++x == (size_t)dim.x) {
            x = 0;
            y++;
        }
    }

    return true;
}

bool AreaJSON::processObjectGroup(JSONObjectPtr obj) {

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
    const JSONArrayPtr objects = obj->arrayAt("objects");

    for (size_t i = 0; i < objects->size(); i++) {
        CHECK(objects->isObject(i));
        CHECK(processObject(objects->objectAt(i)));
    }

    return true;
}

bool AreaJSON::processObjectGroupProperties(JSONObjectPtr obj) {

/*
 {
   "depth": "0.0"
 }
*/

    if (!obj->hasStringDouble("depth")) {
        Log::err(descriptor, "An objectlayer must have the \"depth\" property");
        return false;
    }

    const double depth = obj->stringDoubleAt("depth");

    if (depth2idx.find(depth) != depth2idx.end()) {
        Log::err(descriptor, "Layers cannot share a depth");
        return false;
    }

    allocateMapLayer();
    depth2idx[depth] = dim.z - 1;
    idx2depth.push_back(depth);  // Effectively idx2depth[dim.z - 1] = depth;

    return true;
}

bool AreaJSON::processObject(JSONObjectPtr obj) {

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

    const int z = dim.z - 1;

    // If we ever allow finding layers out of order.
    //assert(0 <= z && z < dim.z);

    // Gather object properties now. Assign them to tiles later.
    bool wwide[5], hwide[5];  // Wide exit in dimensions: width, height

    DataArea::TileScript enterScript = nullptr,
                         leaveScript = nullptr,
                         useScript = nullptr;
    Optional<Exit> exit[5];
    Optional<double> layermods[5];
    unsigned flags = 0x0;

    JSONObjectPtr props = obj->objectAt("properties");

    if (props->hasString("flags")) {
        CHECK(splitTileFlags(props->stringAt("flags"), &flags));
    }

    if (props->hasString("on_enter")) {
        const std::string scriptName = props->stringAt("on_enter");
        enterScript = dataArea->script(scriptName);
    }
    if (props->hasString("on_leave")) {
        const std::string scriptName = props->stringAt("on_leave");
        leaveScript = dataArea->script(scriptName);
    }
    if (props->hasString("on_use")) {
        const std::string scriptName = props->stringAt("on_use");
        useScript = dataArea->script(scriptName);
    }

    if (props->hasString("exit")) {
        const std::string value = props->stringAt("exit");
        CHECK(parseExit(value, exit[EXIT_NORMAL], &wwide[EXIT_NORMAL], &hwide[EXIT_NORMAL]));
        flags |= TILE_NOWALK_NPC;
    }
    if (props->hasString("exit:up")) {
        const std::string value = props->stringAt("exit:up");
        CHECK(parseExit(value, exit[EXIT_UP], &wwide[EXIT_UP], &hwide[EXIT_UP]));
    }
    if (props->hasString("exit:down")) {
        const std::string value = props->stringAt("exit:down");
        CHECK(parseExit(value, exit[EXIT_DOWN], &wwide[EXIT_DOWN], &hwide[EXIT_DOWN]));
    }
    if (props->hasString("exit:left")) {
        const std::string value = props->stringAt("exit:left");
        CHECK(parseExit(value, exit[EXIT_LEFT], &wwide[EXIT_LEFT], &hwide[EXIT_LEFT]));
    }
    if (props->hasString("exit:right")) {
        const std::string value = props->stringAt("exit:right");
        CHECK(parseExit(value, exit[EXIT_RIGHT], &wwide[EXIT_RIGHT], &hwide[EXIT_RIGHT]));
    }

    if (props->hasStringDouble("layermod")) {
        double mod = props->stringDoubleAt("layermod");
        layermods[EXIT_NORMAL] = mod;
        flags |= TILE_NOWALK_NPC;
    }
    if (props->hasStringDouble("layermod:up")) {
        double mod = props->stringDoubleAt("layermod:up");
        layermods[EXIT_UP] = mod;
    }
    if (props->hasStringDouble("layermod:down")) {
        double mod = props->stringDoubleAt("layermod:down");
        layermods[EXIT_DOWN] = mod;
    }
    if (props->hasStringDouble("layermod:left")) {
        double mod = props->stringDoubleAt("layermod:left");
        layermods[EXIT_LEFT] = mod;
    }
    if (props->hasStringDouble("layermod:right")) {
        double mod = props->stringDoubleAt("layermod:right");
        layermods[EXIT_RIGHT] = mod;
    }

    // Apply these properties directly to one or more tiles in a rectangle
    // of the map. We don't keep an intermediary "object" object lying
    // around.

    CHECK(obj->hasUnsigned("x"));
    CHECK(obj->hasUnsigned("y"));
    CHECK(obj->hasUnsigned("width"));
    CHECK(obj->hasUnsigned("height"));
    const unsigned x = obj->unsignedAt("x") / tileDim.x;
    const unsigned y = obj->unsignedAt("y") / tileDim.y;
    const unsigned w = obj->unsignedAt("width") / tileDim.x;
    const unsigned h = obj->unsignedAt("height") / tileDim.y;

    CHECK(x + w <= dim.x);
    CHECK(y + h <= dim.y);

    // We know which Tiles are being talked about now... yay
    for (unsigned Y = y; Y < y + h; Y++) {
        for (unsigned X = x; X < x + w; X++) {
            Tile& tile = map[(size_t)z][(size_t)Y][(size_t)X];

            tile.flags |= flags;
            for (size_t i = 0; i < 5; i++) {
                if (exit[i]) {
                    int dx = X - x;
                    int dy = Y - y;
                    if (wwide[i]) {
                        exit[i]->coords.x += dx;
                    }
                    if (hwide[i]) {
                        exit[i]->coords.y += dy;
                    }
                    tile.exits[i] = exit[i];
                }
            }
            for (size_t i = 0; i < 5; i++) {
                tile.layermods[i] = layermods[i];
            }
            tile.enterScript = enterScript;
            tile.leaveScript = leaveScript;
            tile.useScript = useScript;
        }
    }

    return true;
}

bool AreaJSON::splitTileFlags(const std::string& strOfFlags, unsigned* flags) {
    for (const auto& str : splitStr(strOfFlags, ",")) {
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
            Log::err(descriptor, "Invalid tile flag: " + str);
            return false;
        }
    }

    return true;
}

/**
 * Matches regex /^\s*\d+\+?$/
 */
static bool isIntegerOrPlus(const std::string& s) {
    const int space = 0;
    const int digit = 1;
    const int sign = 2;

    int state = space;

    for (size_t i = 0; i < s.size(); i++) {
        char c = s[i];
        if (state == space) {
               if (isspace(c)) continue;
               else state++;
        }
        if (state == digit) {
            if (isdigit(c)) continue;
            else state++;
        }
        if (state == sign) {
            return c == '+';
        }
    }
    return true;
}

bool AreaJSON::parseExit(const std::string& dest,
                         Optional<Exit>& exit,
                         bool* wwide, bool* hwide) {

/*
  Format: destination area, x, y, z
  E.g.:   "babysfirst.area,1,3,0"
*/

    std::vector<std::string> strs = splitStr(dest, ",");

    if (strs.size() != 4) {
        Log::err(descriptor, "exit: Invalid format");
        return false;
    }

    std::string area = strs[0],
                x = strs[1],
                y = strs[2],
                z = strs[3];

    if (!isIntegerOrPlus(x) ||
        !isIntegerOrPlus(y) ||
        !isIntegerOrPlus(z)) {
        Log::err(descriptor, "exit: Invalid format");
        return false;
    }

    exit = Exit(area,
                atoi(x.c_str()),
                atoi(y.c_str()),
                atof(z.c_str()));

    *wwide = x.find('+') != std::string::npos;
    *hwide = y.find('+') != std::string::npos;

    return true;
}

bool AreaJSON::parseARGB(const std::string& str,
                         unsigned char& a, unsigned char& r,
                         unsigned char& g, unsigned char& b) {
    unsigned char* channels[] = { &a, &r, &g, &b };

    std::vector<std::string> strs = splitStr(str, ",");

    if (strs.size() != 4) {
        Log::err(descriptor, "invalid ARGB format");
        return false;
    }

    for (size_t i = 0; i < 4; i++) {
        std::string s = strs[i];
        if (!isInteger(s)) {
            Log::err(descriptor, "invalid ARGB format");
            return false;
        }
        int v = atoi(s.c_str());
        if (!(0 <= v && v < 256)) {
            Log::err(descriptor,
                "ARGB values must be between 0 and 255");
            return false;
        }
        *channels[i] = (unsigned char)v;
    }

    return true;
}
