/******************************
** Tsunagari Tile Engine     **
** area.cpp                  **
** Copyright 2011 OmegaSDG   **
******************************/

#include <math.h>

#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Image.hpp>
#include <Gosu/Math.hpp>
#include <Gosu/Timing.hpp>

#include "area.h"
#include "common.h"
#include "entity.h"
#include "log.h"
#include "resourcer.h"
#include "window.h"
#include "world.h"
#include "xml.h"

/* NOTE: In the TMX map format used by Tiled, tileset tiles start counting
         their Y-positions from 0, while layer tiles start counting from 1. I
         can't imagine why the author did this, but we have to take it into
         account.
*/

Area::Area(Resourcer* rc,
           World* world,
           Player* player,
           const std::string& descriptor)
	: rc(rc), world(world), player(player), descriptor(descriptor),
	  onIntro(false)
{
	dim.x = dim.y = dim.z = 0;
}

Area::~Area()
{
	if (musicInst && musicInst->playing())
		musicInst->stop();
}

bool Area::init()
{
	if (!processDescriptor())
		return false;

	if (introMusic) {
		musicInst.reset(introMusic->play(1, 1, false));
		onIntro = true;
	}
	else if (mainMusic) {
		musicInst.reset(mainMusic->play(1, 1, true));
	}
	return true;
}

void Area::buttonDown(const Gosu::Button btn)
{
	if (btn == Gosu::kbRight)
		player->startMovement(icoord(1, 0, 0));
	else if (btn == Gosu::kbLeft)
		player->startMovement(icoord(-1, 0, 0));
	else if (btn == Gosu::kbUp)
		player->startMovement(icoord(0, -1, 0));
	else if (btn == Gosu::kbDown)
		player->startMovement(icoord(0, 1, 0));
}

void Area::buttonUp(const Gosu::Button btn)
{
	if (btn == Gosu::kbRight)
		player->stopMovement(icoord(1, 0, 0));
	else if (btn == Gosu::kbLeft)
		player->stopMovement(icoord(-1, 0, 0));
	else if (btn == Gosu::kbUp)
		player->stopMovement(icoord(0, -1, 0));
	else if (btn == Gosu::kbDown)
		player->stopMovement(icoord(0, 1, 0));
}

void Area::draw()
{
	Gosu::Graphics& graphics = GameWindow::getWindow().graphics();
	const Gosu::Transform trans = viewportTransform();
	graphics.pushTransform(trans);

	updateTileAnimations();
	drawTiles();
	drawEntities();

	graphics.popTransform();
}

void Area::updateTileAnimations()
{
	const int millis = GameWindow::getWindow().time();
	BOOST_FOREACH(TileSet& set, tilesets)
		BOOST_FOREACH(TileType& type, set.tileTypes)
			type.anim.updateFrame(millis);

}

void Area::drawTiles() const
{
	const icube_t tiles = visibleTiles();
	for (int z = tiles.z1; z != tiles.z2; z++) {
		const grid_t& grid = map[z];
		for (int y = tiles.y1; y != tiles.y2; y++) {
			const row_t& row = grid[y];
			for (int x = tiles.x1; x != tiles.x2; x++) {
				drawTile(row[x], x, y, z);
			}
		}
	}
}

void Area::drawTile(const Tile& tile, int x, int y, int) const
{
	const TileType* type = tile.type;
	const Gosu::Image* img = type->anim.frame();
	img->draw((double)x*img->width(), (double)y*img->height(), 0);
}

void Area::drawEntities()
{
	player->draw();
}

bool Area::needsRedraw() const
{
	if (player->needsRedraw())
		return true;

	// Do any onscreen tile types need to update their animations?
	const int millis = GameWindow::getWindow().time();
	BOOST_FOREACH(const TileSet& set, tilesets)
		BOOST_FOREACH(const TileType& type, set.tileTypes)
			if (type.anim.needsRedraw(millis) &&
					tileTypeOnScreen(type))
				return true;
	return false;
}

void Area::update(unsigned long dt)
{
	if (onIntro && !musicInst->playing()) {
		onIntro = false;
		musicInst.reset(mainMusic->play(1, 1, true));
	}
	player->update(dt);
}

icoord_t Area::getDimensions() const
{
	return dim;
}

icoord_t Area::getTileDimensions() const
{
	return tilesets[0].tileDim; // XXX only considers first tileset
}

const Tile& Area::getTile(icoord_t c) const
{
	return map[c.z][c.y][c.x];
}

Tile& Area::getTile(icoord_t c)
{
	return map[c.z][c.y][c.x];
}

static double center(double w, double g, double p)
{
	return w>g ? (w-g)/2.0 : Gosu::boundBy(w/2.0-p, w-g, 0.0);
}

const icoord_t Area::viewportOffset() const
{
	const Gosu::Graphics& graphics = GameWindow::getWindow().graphics();
	const double tileWidth = (double)tilesets[0].tileDim.x;
	const double tileHeight = (double)tilesets[0].tileDim.y;
	const double windowWidth = (double)graphics.width() / tileWidth;
	const double windowHeight = (double)graphics.height() / tileHeight;
	const double gridWidth = (double)dim.x;
	const double gridHeight = (double)dim.y;
	const double playerX = player->getRPixel().x / tileWidth + 0.5;
	const double playerY = player->getRPixel().y / tileHeight + 0.5;

	icoord_t c;
	c.x = (int)(center(windowWidth, gridWidth, playerX) * tileWidth);
	c.y = (int)(center(windowHeight, gridHeight, playerY) * tileHeight);
	c.z = 0;

	return c;
}

const Gosu::Transform Area::viewportTransform() const
{
	const icoord_t c = viewportOffset();
	return Gosu::translate((double)c.x, (double)c.y);
}

icube_t Area::visibleTiles() const
{
	const Gosu::Graphics& graphics = GameWindow::getWindow().graphics();
	const int tileWidth = tilesets[0].tileDim.x;
	const int tileHeight = tilesets[0].tileDim.y;
	const int windowWidth = graphics.width();
	const int windowHeight = graphics.height();
	const icoord_t off = viewportOffset();

	const int x1 = -off.x / tileWidth;
	const int y1 = -off.y / tileHeight;
	const int x2 = (int)ceil((double)(windowWidth - off.x) /
		(double)tileWidth);
	const int y2 = (int)ceil((double)(windowHeight - off.y) /
		(double)tileHeight);

	// Does the entire width or height of the map fit onscreen?
	if (x1 >= 0 && y1 >= 0)
		return icube(x1, y1, 0, x2, y2, 1);
	else if (x1 >= 0)
		return icube(x1, 0, 0, x2, dim.y, 1);
	else if (y1 >= 0)
		return icube(0, y1, 0, dim.x, y2, 1);
	else
		return icube(0, 0, 0, dim.x, dim.y, 1);
}

bool Area::tileTypeOnScreen(const TileType& search) const
{
	const icube_t tiles = visibleTiles();
	for (int z = tiles.z1; z != tiles.z2; z++) {
		for (int y = tiles.y1; y != tiles.y2; y++) {
			for (int x = tiles.x1; x != tiles.x2; x++) {
				const Tile& tile = map[z][y][x];
				const TileType* type = tile.type;
				if (type == &search)
					return true;
			}
		}
	}
	return false;
}

bool Area::processDescriptor()
{
	XMLDocRef doc = rc->getXMLDoc(descriptor, "area.dtd");
	if (!doc)
		return false;

	// Iterate and process children of <map>
	xmlNode* root = xmlDocGetRootElement(doc.get()); // <map> element

	dim.x = atoi(readXmlAttribute(root, "width").c_str());
	dim.y = atoi(readXmlAttribute(root, "height").c_str());

	xmlNode* child = root->xmlChildrenNode;
	for (; child != NULL; child = child->next) {
		if (!xmlStrncmp(child->name, BAD_CAST("properties"), 11)) {
			if (!processMapProperties(child))
				return false;
		}
		else if (!xmlStrncmp(child->name, BAD_CAST("tileset"), 8)) {
			if (!processTileSet(child))
				return false;
		}
		else if (!xmlStrncmp(child->name, BAD_CAST("layer"), 6)) {
			if (!processLayer(child))
				return false;
		}
		else if (!xmlStrncmp(child->name, BAD_CAST("objectgroup"), 12)) {
			if (!processObjectGroup(child))
				return false;
		}
	}

	return true;
}

bool Area::processMapProperties(xmlNode* node)
{

/*
 <properties>
  <property name="areaspec" value="1"/>
  <property name="author" value="Michael D. Reiley"/>
  <property name="name" value="Baby's First Area"/>
  <property name="intro_music" value="intro.music"/>
  <property name="main_music" value="wind.music"/>
  <property name="onLoad" value="babysfirst_init()"/>
  <property name="scripts" value="areainits.event,test.event"/>
 </properties>
*/

	xmlNode* child = node->xmlChildrenNode;
	for (; child != NULL; child = child->next) {
		std::string name = readXmlAttribute(child, "name");
		std::string value = readXmlAttribute(child, "value");
		if (!name.compare("author"))
			author = value;
		else if (!name.compare("name"))
			this->name = value;
		else if (!name.compare("intro_music"))
			introMusic = rc->getSample(value);
		else if (!name.compare("main_music"))
			mainMusic = rc->getSample(value);
		else if (!name.compare("onLoad"))
			onLoadEvents = value;
		else if (!name.compare("scripts"))
			scripts = value; // TODO split(), load
	}
	return true;
}

bool Area::processTileSet(xmlNode* node)
{

/*	
 <tileset firstgid="1" name="tiles.sheet" tilewidth="64" tileheight="64">
  <image source="tiles.sheet" width="256" height="256"/>
  <tile id="14">
   ...
  </tile>
 </tileset>
*/
	TileSet ts;
	int x = ts.tileDim.x = atoi(readXmlAttribute(node, "tilewidth").c_str());
	int y = ts.tileDim.y = atoi(readXmlAttribute(node, "tileheight").c_str());
	
	xmlNode* child = node->xmlChildrenNode;
	for (; child != NULL; child = child->next) {
		if (!xmlStrncmp(child->name, BAD_CAST("tile"), 5)) {
			unsigned id = (unsigned)atoi(readXmlAttribute(child, "id").c_str());

			// Undeclared TileTypes have default properties.
			while (ts.tileTypes.size() != id) {
				TileType tt = defaultTileType(ts);
				ts.tileTypes.push_back(tt);
			}

			// Handle explicit TileType
			if (!processTileType(child, ts))
				return false;
		}
		else if (!xmlStrncmp(child->name, BAD_CAST("image"), 6)) {
			std::string source = readXmlAttribute(child, "source");
			rc->getTiledImage(ts.tiles, source,
				(unsigned)x, (unsigned)y, true);
		}
	}

	while (ts.tiles.size()) {
		TileType tt = defaultTileType(ts);
		ts.tileTypes.push_back(tt);
	}

	tilesets.push_back(ts);
	return true;
}

TileType Area::defaultTileType(TileSet& set)
{
	TileType type;
	type.flags = 0x0;
	type.anim.addFrame(set.tiles.front());
	set.tiles.pop_front();
	return type;
}

bool Area::processTileType(xmlNode* node, TileSet& set)
{

/*
  <tile id="8">
   <properties>
    <property name="flags" value="nowalk"/>
    <property name="onEnter" value="skid();speed(2)"/>
    <property name="onLeave" value="undo()"/>
   </properties>
  </tile>
  <tile id="14">
   <properties>
    <property name="animated" value="1"/>
    <property name="size" value="2"/>
    <property name="speed" value="2"/>
   </properties>
  </tile>
*/

	// Initialize a default TileType, we'll build on that.
	TileType type = defaultTileType(set);

	unsigned id = (unsigned)atoi(readXmlAttribute(node, "id").c_str());
	unsigned expectedId = (unsigned)set.tileTypes.size();
	if (id != expectedId) {
		Log::err(descriptor, std::string("expected TileType id ") +
		         itostr(expectedId) + ", but got " +
			 itostr(id));
		return false;
	}

	xmlNode* child = node->xmlChildrenNode; // <properties>
	child = child->xmlChildrenNode; // <property>
	for (; child != NULL; child = child->next) {
		std::string name = readXmlAttribute(child, "name");
		std::string value = readXmlAttribute(child, "value");
		if (!name.compare("flags")) {
			type.flags = splitTileFlags(value.c_str());
		}
		else if (!name.compare("onEnter")) {
			if (!rc->resourceExists(value)) {
				Log::err("Resourcer", "script " + value +
						" referenced but not found");
				continue;
			}
			TileEvent e;
			e.trigger = onEnter;
			e.script = value;
			type.events.push_back(e);
			type.flags |= hasOnEnter;
		}
		else if (!name.compare("onLeave")) {
			if (!rc->resourceExists(value)) {
				Log::err("Resourcer", "script " + value +
						" referenced but not found");
				continue;
			}
			TileEvent e;
			e.trigger = onLeave;
			e.script = value;
			type.events.push_back(e);
			type.flags |= hasOnLeave;
		}
		else if (!name.compare("animated")) {
			// XXX still needed?
			// type.animated = parseBool((const char*)value);
		}
		else if (!name.compare("size")) {
			int size = atoi(value.c_str()); // atoi

			// Add size-1 more frames to our animation.
			// We already have one from defaultTileType.
			for (int i = 1; i < size; i++) {
				if (set.tiles.empty()) {
					Log::err(descriptor, "ran out of tiles"
						"/frames for animated tile");
					return false;
				}
				type.anim.addFrame(set.tiles.front());
				set.tiles.pop_front();
			}
		}
		else if (!name.compare("speed")) {
			int len = (int)(1000.0/atof(value.c_str()));
			type.anim.setFrameLen(len);
		}
	}

	set.tileTypes.push_back(type);
	return true;
}

bool Area::processLayer(xmlNode* node)
{

/*
 <layer name="Tiles0" width="5" height="5">
  <properties>
   ...
  </properties>
  <data>
   <tile gid="9"/>
   <tile gid="9"/>
   <tile gid="9"/>
...
   <tile gid="3"/>
   <tile gid="9"/>
   <tile gid="9"/>
  </data>
 </layer>
*/

	int x = atoi(readXmlAttribute(node, "width").c_str());
	int y = atoi(readXmlAttribute(node, "height").c_str());

	if (dim.x != x || dim.y != y) {
		Log::err(descriptor, "layer x,y size != map x,y size");
		return false;
	}

	xmlNode* child = node->xmlChildrenNode;
	for (; child != NULL; child = child->next) {
		if (!xmlStrncmp(child->name, BAD_CAST("properties"), 11)) {
			if (!processLayerProperties(child))
				return false;
		}
		else if (!xmlStrncmp(child->name, BAD_CAST("data"), 5)) {
			if (!processLayerData(child))
				return false;
		}
	}
	return true;
}

bool Area::processLayerProperties(xmlNode* node)
{

/*
  <properties>
   <property name="layer" value="0"/>
  </properties>
*/

	xmlNode* child = node->xmlChildrenNode;
	for (; child != NULL; child = child->next) {
		std::string name = readXmlAttribute(child, "name");
		std::string value = readXmlAttribute(child, "value");
		if (!name.compare("layer")) {
			int depth = atoi(value.c_str());
			if (depth != dim.z) {
				Log::err(descriptor, "invalid layer depth");
				return false;
			}
		}
	}

	return true;
}

bool Area::processLayerData(xmlNode* node)
{

/*
  <data>
   <tile gid="9"/>
   <tile gid="9"/>
   <tile gid="9"/>
...
   <tile gid="3"/>
   <tile gid="9"/>
   <tile gid="9"/>
  </data>
*/

	row_t row;
	grid_t grid;

	row.reserve(dim.x);
	grid.reserve(dim.y);

	xmlNode* child = node->xmlChildrenNode;
	for (int i = 1; child != NULL; i++, child = child->next) {
		if (!xmlStrncmp(child->name, BAD_CAST("tile"), 5)) {
			unsigned gid = (unsigned)atoi(readXmlAttribute(child, "gid").c_str())-1;

			// XXX can only access first tileset
			TileType* type = &tilesets[0].tileTypes[gid];

			Tile t;
			t.type = type;
			t.flags = 0x0;
			type->allOfType.push_back(&t);
			row.push_back(t);
			if (row.size() % dim.x == 0) {
				grid.push_back(row);
				row.clear();
				row.reserve(dim.x);
			}
		}
	}

	map.push_back(grid);
	dim.z++;
	return true;
}

bool Area::processObjectGroup(xmlNode* node)
{

/*
 <objectgroup name="Prop0" width="5" height="5">
  <properties>
   <property name="layer" value="0"/>
  </properties>
  <object name="tile2" type="Tile" gid="7" x="64" y="320">
   <properties>
    <property name="onEnter" value="speed(0.5)"/>
    <property name="onLeave" value="undo()"/>
    <property name="door" value="grassfield.area,1,1,0"/>
    <property name="flags" value="npc_nowalk"/>
   </properties>
  </object>
 </objectgroup>
*/

	int x = atoi(readXmlAttribute(node, "width").c_str());
	int y = atoi(readXmlAttribute(node, "height").c_str());

	int zpos = -1;

	if (dim.x != x || dim.y != y) {
		Log::err(descriptor, "objectgroup x,y size != map x,y size");
		return false;
	}

	xmlNode* child = node->xmlChildrenNode;
	for (; child != NULL; child = child->next) {
		if (!xmlStrncmp(child->name, BAD_CAST("properties"), 11)) {
			if (!processObjectGroupProperties(child, &zpos))
				return false;
		}
		else if (!xmlStrncmp(child->name, BAD_CAST("object"), 7)) {
			if (zpos == -1 || !processObject(child, zpos))
				return false;
		}
	}

	return true;
}

bool Area::processObjectGroupProperties(xmlNode* node, int* zpos)
{

/*
  <properties>
   <property name="layer" value="0"/>
  </properties>
*/

	xmlNode* child = node->xmlChildrenNode;
	for (; child != NULL; child = child->next) {
		std::string name = readXmlAttribute(child, "name");
		std::string value = readXmlAttribute(child, "value");
		if (!name.compare("layer")) {
			int layer = atoi(value.c_str());
			if (0 < layer || layer >= (int)dim.z) {
				Log::err(descriptor,
					"objectgroup must correspond with layer"
				);
				return false;
			}
			*zpos = layer;
		}
	}
	return true;
}

bool Area::processObject(xmlNode* node, int zpos)
{

/*
  <object name="tile2" type="Tile" gid="7" x="64" y="320">
   <properties>
    <property name="onEnter" value="speed(0.5)"/>
    <property name="onLeave" value="undo()"/>
    <property name="door" value="grassfield.area,1,1,0"/>
    <property name="flags" value="npc_nowalk"/>
   </properties>
  </object>
*/

	std::string type = readXmlAttribute(node, "type");
	if (type.compare("Tile")) {
		Log::err(descriptor, "object type must be Tile");
		return false;
	}

	std::string xStr = readXmlAttribute(node, "x");
	std::string yStr = readXmlAttribute(node, "y");
	// XXX we ignore the object gid... is that okay?

	// wouldn't have to access tilesets if we had tileDim ourselves
	int x = atoi(xStr.c_str()) / tilesets[0].tileDim.x;
	int y = atoi(yStr.c_str()) / tilesets[0].tileDim.y;
	y = y - 1; // bug in tiled? y is 1 too high

	// We know which Tile is being talked about now... yay
	Tile& t = map[zpos][y][x];

	xmlNode* child = node->xmlChildrenNode; // <properties>
	child = child->xmlChildrenNode; // <property>
	for (; child != NULL; child = child->next) {
		std::string name = readXmlAttribute(child, "name");
		std::string value = readXmlAttribute(child, "value");
		if (!name.compare("flags")) {
			t.flags = splitTileFlags(value.c_str());
		}
		else if (!name.compare("onEnter")) {
			if (!rc->resourceExists(value)) {
				Log::err("Resourcer", "script " + value +
						" referenced but not found");
				continue;
			}
			TileEvent e;
			e.trigger = onEnter;
			e.script = value;
			t.events.push_back(e);
			t.flags |= hasOnEnter;
		}
		else if (!name.compare("onLeave")) {
			if (!rc->resourceExists(value)) {
				Log::err("Resourcer", "script " + value +
						" referenced but not found");
				continue;
			}
			TileEvent e;
			e.trigger = onLeave;
			e.script = value;
			t.events.push_back(e);
			t.flags |= hasOnLeave;
		}
		else if (!name.compare("door")) {
			t.door.reset(parseDoor(value.c_str()));
			t.flags |= npc_nowalk;
		}
	}
	return true;
}

unsigned Area::splitTileFlags(const std::string strOfFlags)
{
	std::vector<std::string> strs;
	strs = splitStr(strOfFlags, ",");

	unsigned flags = 0x0;
	BOOST_FOREACH(const std::string& str, strs) {
		if (str == "nowalk")
			flags |= nowalk;
	}
	return flags;
}

Door Area::parseDoor(const std::string dest)
{

/*
  Format: destination Area, x, y, z
  E.g.:   "babysfirst.area,1,3,0"
*/

	std::vector<std::string> strs;
	strs = splitStr(dest, ",");

	// TODO: verify the validity of the input string... it's coming from
	// user land
	Door door;
	door.area = strs[0];
	door.tile.x = atoi(strs[1].c_str());
	door.tile.y = atoi(strs[2].c_str());
	door.tile.z = atoi(strs[3].c_str());
	return door;
}

