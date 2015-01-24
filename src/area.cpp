/***************************************
** Tsunagari Tile Engine              **
** area.cpp                           **
** Copyright 2011-2014 PariahSoft LLC **
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

#include <algorithm>
#include <math.h>

#include "algorithm.h"
#include "area.h"
#include "client-conf.h"
#include "entity.h"
#include "formatter.h"
#include "log.h"
#include "images.h"
#include "math.h"
#include "music.h"
#include "npc.h"
#include "overlay.h"
#include "player.h"
#include "tile.h"
#include "viewport.h"
#include "window.h"
#include "world.h"

#include "data/data-world.h"

#define ASSERT(x)  if (!(x)) { return false; }

/* NOTE: In the TMX map format used by Tiled, tileset tiles start counting
         their Y-positions from 0, while layer tiles start counting from 1. I
         can't imagine why the author did this, but we have to take it into
         account.
*/

Area::Area(Player* player,
           const std::string& descriptor)
	: dataArea(DataWorld::instance().area(descriptor)),
	  player(player),
	  colorOverlayARGB(0),
	  dim(0, 0, 0),
	  tileDim(0, 0),
	  loopX(false), loopY(false),
	  beenFocused(false),
	  redraw(true),
	  descriptor(descriptor)
{
}

Area::~Area()
{
}

bool Area::init()
{
	// Abstract method.
	return false;
}

void Area::focus()
{
	if (!beenFocused) {
		beenFocused = true;
		if (dataArea)
			dataArea->onLoad();
	}

	if (musicIntroSet)
		Music::instance().setIntro(musicIntro);
	if (musicLoopSet)
		Music::instance().setLoop(musicLoop);

	if (dataArea)
		dataArea->onFocus();
}

void Area::buttonDown(KeyboardKey key)
{
	switch (key) {
	case KBLeftArrow:
		player->startMovement(ivec2(-1, 0));
		break;
	case KBRightArrow:
		player->startMovement(ivec2(1, 0));
		break;
	case KBUpArrow:
		player->startMovement(ivec2(0, -1));
		break;
	case KBDownArrow:
		player->startMovement(ivec2(0, 1));
		break;
	case KBSpace:
		player->useTile();
		break;
	default:
		break;
	}
}

void Area::buttonUp(KeyboardKey key)
{
	switch (key) {
	case KBLeftArrow:
		player->stopMovement(ivec2(-1, 0));
		break;
	case KBRightArrow:
		player->stopMovement(ivec2(1, 0));
		break;
	case KBUpArrow:
		player->stopMovement(ivec2(0, -1));
		break;
	case KBDownArrow:
		player->stopMovement(ivec2(0, 1));
		break;
	default:
		break;
	}
}

void Area::draw()
{
	drawTiles();
	drawEntities();
	drawColorOverlay();
	redraw = false;
}

bool Area::needsRedraw() const
{
	if (redraw)
		return true;

	const icube tiles = visibleTiles();
	const icube pixels = {
		tiles.x1 * tileDim.x,
		tiles.y1 * tileDim.y,
		tiles.z1,
		tiles.x2 * tileDim.x,
		tiles.y2 * tileDim.y,
		tiles.z2,
	};

	if (player->needsRedraw(pixels))
		return true;
	for (const auto& character : characters)
		if (character->needsRedraw(pixels))
			return true;
	for (const auto& overlay : overlays)
		if (overlay->needsRedraw(pixels))
			return true;

	// Do any on-screen tile types need to update their animations?
	for (int z = tiles.z1; z < tiles.z2; z++) {
		for (int y = tiles.y1; y < tiles.y2; y++) {
			for (int x = tiles.x1; x < tiles.x2; x++) {
				const Tile* tile = getTile(x, y, z);
				const TileType* type = tile->getType();
				if (type && type->needsRedraw())
					return true;
			}
		}
	}
	return false;
}

void Area::requestRedraw()
{
	redraw = true;
}

void Area::tick(time_t dt)
{
	if (dataArea)
		dataArea->tick(dt);

	for (auto& overlay : overlays)
		overlay->tick(dt);
	erase_if(overlays, [] (const std::shared_ptr<Overlay>& o) {
		bool dead = o->isDead();
		if (dead)
			o->setArea(NULL);
		return dead;
	});

	if (conf.moveMode != TURN) {
		player->tick(dt);

		for (auto& character : characters)
			character->tick(dt);
		erase_if(characters, [] (const std::shared_ptr<Character>& c) {
			bool dead = c->isDead();
			if (dead)
				c->setArea(NULL);
			return dead;
		});
	}

	Viewport::instance().tick(dt);
	Music::instance().tick();
}

void Area::turn()
{
	if (dataArea)
		dataArea->turn();

	player->turn();

	for (auto& character : characters)
		character->turn();
	erase_if(characters, [] (const std::shared_ptr<Character>& c) {
		bool dead = c->isDead();
		if (dead)
			c->setArea(NULL);
		return dead;
	});

	Viewport::instance().turn();
}

void Area::setColorOverlay(uint8_t a, uint8_t r, uint8_t g, uint8_t b)
{
	colorOverlayARGB = (uint32_t)(a << 24) + (uint32_t)(r << 16) +
		(uint32_t)(g << 8) + (uint32_t)b;
	redraw = true;
}



const Tile* Area::getTile(int x, int y, int z) const
{
	if (loopX)
		x = wrap(0, x, dim.x);
	if (loopY)
		y = wrap(0, y, dim.y);
	if (inBounds(x, y, z))
		return &map[(size_t)z][(size_t)y][(size_t)x];
	else
		return NULL;
}

const Tile* Area::getTile(int x, int y, double z) const
{
	return getTile(x, y, depthIndex(z));
}

const Tile* Area::getTile(icoord phys) const
{
	return getTile(phys.x, phys.y, phys.z);
}

const Tile* Area::getTile(vicoord virt) const
{
	return getTile(virt2phys(virt));
}

const Tile* Area::getTile(rcoord virt) const
{
	return getTile(virt2phys(virt));
}

Tile* Area::getTile(int x, int y, int z)
{
	if (loopX)
		x = wrap(0, x, dim.x);
	if (loopY)
		y = wrap(0, y, dim.y);
	if (inBounds(x, y, z))
		return &map[(size_t)z][(size_t)y][(size_t)x];
	else
		return NULL;
}

Tile* Area::getTile(int x, int y, double z)
{
	return getTile(x, y, depthIndex(z));
}

Tile* Area::getTile(icoord phys)
{
	return getTile(phys.x, phys.y, phys.z);
}

Tile* Area::getTile(vicoord virt)
{
	return getTile(virt2phys(virt));
}

Tile* Area::getTile(rcoord virt)
{
	return getTile(virt2phys(virt));
}

TileSet* Area::getTileSet(const std::string& imagePath)
{
	std::map<std::string, TileSet>::iterator it;
	it = tileSets.find(imagePath);
	if (it == tileSets.end()) {
		Log::err("Area", "tileset " + imagePath + " not found");
		return NULL;
	}
	return &tileSets[imagePath];
}


ivec3 Area::getDimensions() const
{
	return dim;
}

ivec2 Area::getTileDimensions() const
{
	return tileDim;
}

double Area::isometricZOff(rvec2 pos) const
{
	return pos.y / tileDim.y * ISOMETRIC_ZOFF_PER_TILE;
}

icube Area::visibleTileBounds() const
{
	rvec2 screen = Viewport::instance().getVirtRes();
	rvec2 off = Viewport::instance().getMapOffset();

	int x1 = (int)floor(off.x / tileDim.x);
	int y1 = (int)floor(off.y / tileDim.y);
	int x2 = (int)ceil((screen.x + off.x) / tileDim.x);
	int y2 = (int)ceil((screen.y + off.y) / tileDim.y);

	return icube(x1, y1, 0, x2, y2, dim.z);
}

icube Area::visibleTiles() const
{
	icube cube = visibleTileBounds();
	if (!loopX) {
		cube.x1 = std::max(cube.x1, 0);
		cube.x2 = std::min(cube.x2, dim.x);
	}
	if (!loopY) {
		cube.y1 = std::max(cube.y1, 0);
		cube.y2 = std::min(cube.y2, dim.y);
	}
	return cube;
}

bool Area::inBounds(int x, int y, int z) const
{
	return ((loopX || (0 <= x && x < dim.x)) &&
		(loopY || (0 <= y && y < dim.y)) &&
		          0 <= z && z < dim.z);
}

bool Area::inBounds(int x, int y, double z) const
{
	return inBounds(x, y, depthIndex(z));
}

bool Area::inBounds(icoord phys) const
{
	return inBounds(phys.x, phys.y, phys.z);
}

bool Area::inBounds(vicoord virt) const
{
	return inBounds(virt2phys(virt));
}

bool Area::inBounds(rcoord virt) const
{
	return inBounds(virt2phys(virt));
}

bool Area::inBounds(Entity* ent) const
{
	return inBounds(ent->getPixelCoord());
}



bool Area::loopsInX() const
{
	return loopX;
}

bool Area::loopsInY() const
{
	return loopY;
}

const std::string Area::getDescriptor() const
{
	return descriptor;
}

std::shared_ptr<NPC> Area::spawnNPC(const std::string& descriptor,
	vicoord coord, const std::string& phase)
{
	auto c = std::make_shared<NPC>();
	if (!c->init(descriptor, phase)) {
		// Error logged.
		return std::shared_ptr<NPC>();
	}
	c->setArea(this);
	c->setTileCoords(coord);
	insert(c);
	return c;
}

std::shared_ptr<Overlay> Area::spawnOverlay(const std::string& descriptor,
	vicoord coord, const std::string& phase)
{
	auto o = std::make_shared<Overlay>();
	if (!o->init(descriptor, phase)) {
		// Error logged.
		return std::shared_ptr<Overlay>();
	}
	o->setArea(this);
	o->teleport(coord);
	insert(o);
	return o;
}

void Area::insert(std::shared_ptr<Character> c)
{
	characters.insert(c);
}

void Area::insert(std::shared_ptr<Overlay> o)
{
	overlays.insert(o);
}


vicoord Area::phys2virt_vi(icoord phys) const
{
	return vicoord(phys.x, phys.y, indexDepth(phys.z));
}

rcoord Area::phys2virt_r(icoord phys) const
{
	return rcoord(
		(double)phys.x * tileDim.x,
		(double)phys.y * tileDim.y,
		indexDepth(phys.z)
	);
}

icoord Area::virt2phys(vicoord virt) const
{
	return icoord(virt.x, virt.y, depthIndex(virt.z));
}

icoord Area::virt2phys(rcoord virt) const
{
	return icoord(
		(int)(virt.x / tileDim.x),
		(int)(virt.y / tileDim.y),
		depthIndex(virt.z)
	);
}

rcoord Area::virt2virt(vicoord virt) const
{
	return rcoord(
		(double)virt.x * tileDim.x,
		(double)virt.y * tileDim.y,
		virt.z
	);
}

vicoord Area::virt2virt(rcoord virt) const
{
	return vicoord(
		(int)virt.x / tileDim.x,
		(int)virt.y / tileDim.y,
		virt.z
	);
}

DataArea* Area::getDataArea()
{
	return dataArea;
}

int Area::depthIndex(double depth) const
{
	std::map<double, int>::const_iterator it;
	it = depth2idx.find(depth);
	if (it == depth2idx.end()) {
		Log::fatal(descriptor, Formatter(
			"attempt to access invalid layer: %") % depth);
	}
	return it->second;
}

double Area::indexDepth(int idx) const
{
	assert(0 <= idx && idx <= dim.z);
	return idx2depth[(size_t)idx];
}



void Area::drawTiles()
{
	icube tiles = visibleTiles();
	for (int z = tiles.z1; z < tiles.z2; z++) {
		assert(0 <= z && z <= dim.z);
		double depth = idx2depth[(size_t)z];
		for (int y = tiles.y1; y < tiles.y2; y++) {
			for (int x = tiles.x1; x < tiles.x2; x++) {
				Tile* tile = getTile(x, y, z);
				// We are certain the Tile exists.
				drawTile(*tile, x, y, depth);
			}
		}
	}
}

void Area::drawTile(Tile& tile, int x, int y, double depth)
{
	TileType* type = (TileType*)tile.parent;
	if (type) {
		time_t now = World::instance().time();
		Image* img = type->anim.frame(now);
		if (img) {
			rvec2 drawPos(
				double(x * (int)img->width()),
				double(y * (int)img->height())
			);
			img->draw(drawPos.x, drawPos.y,
			          depth + isometricZOff(drawPos));
		}
	}
}

void Area::drawEntities()
{
	for (auto& character : characters)
		character->draw();
	for (auto& overlay : overlays)
		overlay->draw();
	player->draw();
}

void Area::drawColorOverlay()
{
	if ((colorOverlayARGB & 0xFF000000) != 0) {
		GameWindow& window = GameWindow::instance();
		unsigned x = window.width();
		unsigned y = window.height();
		GameWindow::instance().drawRect(0, x, 0, y, colorOverlayARGB);
	}
}

