/***************************************
** Tsunagari Tile Engine              **
** world.cpp                          **
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

#include "area-tmx.h"
#include "client-conf.h"
#include "log.h"
#include "music.h"
#include "reader.h"
#include "timeout.h"
#include "window.h"
#include "world.h"
#include "xml.h"

#include "data/world.h"

#define ASSERT(x)  if (!(x)) { return false; }

static World globalWorld;

World& World::instance()
{
	return globalWorld;
}

World::World()
	: lastTime(0), total(0), redraw(false), userPaused(false), paused(0)
{
}

World::~World()
{
}

bool World::init()
{
	ASSERT(processDescriptor()); // Try to load in descriptor.
	ASSERT(pauseInfo = Reader::getImage("resource/pause_overlay.png"));

	ASSERT(player.init(playerEntity, playerPhase));
	// pythonSetGlobal("Player", (Entity*)&player);

	view.reset(new Viewport(viewportSz));
	view->trackEntity(&player);

	Area* area = getArea(startArea);
	if (area == NULL) {
		Log::fatal("World", "failed to load initial Area");
		return false;
	}
	focusArea(area, startCoords);

	return true;
}

const std::string& World::getName() const
{
	return name;
}

time_t World::time() const
{
	return total;
}

void World::buttonDown(KeyboardKey key)
{
	switch (key) {
	case KBEscape:
		userPaused = !userPaused;
		setPaused(userPaused);
		redraw = true;
		break;
	default:
		if (!paused && keyStates.empty()) {
			area->buttonDown(key);
			// if (keydownScript)
			// 	keydownScript->invoke();
		}
		break;
	}
}

void World::buttonUp(KeyboardKey key)
{
	switch (key) {
	case KBEscape:
		break;
	default:
		if (!paused && keyStates.empty()) {
			area->buttonUp(key);
			// if (keyupScript)
			// 	keyupScript->invoke();
		}
		break;
	}
}

void World::draw()
{
	redraw = false;

	GameWindow& window = GameWindow::instance();

	if (paused) {
		unsigned ww = window.width();
		unsigned wh = window.height();
		window.drawRect(0, ww, 0, wh, 0x7F000000);

		if (userPaused) {
			unsigned iw = pauseInfo->width();
			unsigned ih = pauseInfo->height();
			double top = std::numeric_limits<double>::max();
			pauseInfo->draw(ww/2 - iw/2, wh/2 - ih/2, top);
		}
	}

	pushLetterbox();

	// Zoom and pan the Area to fit on-screen.
	rvec2 padding = view->getLetterboxOffset();
	window.translate(-padding.x, -padding.y);

	rvec2 scale = view->getScale();
	window.scale(scale.x, scale.y);

	rvec2 scroll = view->getMapOffset();
	window.translate(-scroll.x, -scroll.y);

	area->draw();
}

bool World::needsRedraw() const
{
	if (redraw)
		return true;
	if (!paused && area->needsRedraw())
		return true;
	return false;
}

void World::update(time_t now)
{
	if (lastTime == 0) {
		// There is no dt on the first update().  Don't tick.
		lastTime = now;
	}
	else {
		time_t dt = calculateDt(now);
		if (!paused) {
			total += dt;
			tick(dt);
		}
	}
}

void World::tick(unsigned long dt)
{
	updateTimeouts();
	area->tick(dt);
}

void World::turn()
{
	if (conf.moveMode == TURN) {
		updateTimeouts();
		area->turn();
	}
}

Area* World::getArea(const std::string& filename)
{
	AreaMap::iterator entry = areas.find(filename);
	if (entry != areas.end())
		return entry->second;

	Area* newArea = new AreaTMX(view.get(), &player, filename);

	if (!newArea->init())
		newArea = NULL;

	areas[filename] = newArea;

	DataArea* dataArea = DataWorld::instance().area(filename);
	if (dataArea)
		dataArea->area = newArea;

	return newArea;
}

Area* World::getFocusedArea()
{
	return area;
}

void World::focusArea(Area* area, int x, int y, double z)
{
	focusArea(area, vicoord(x, y, z));
}

void World::focusArea(Area* area, vicoord playerPos)
{
	this->area = area;
	player.setArea(area);
	player.setTileCoords(playerPos);
	view->setArea(area);
	area->focus();
}

void World::setPaused(bool b)
{
	if (!paused && !b) {
		Log::err("World", "trying to unpause, but not paused");
		return;
	}

	// If just pausing.
	if (!paused)
		storeKeys();

	paused += b ? 1 : -1;

	Music::instance().setPaused(paused != 0);

	// If finally unpausing.
	if (!paused)
		restoreKeys();
}

void World::storeKeys()
{
	keyStates.push(GameWindow::instance().getKeysDown());
}

void World::restoreKeys()
{
	BitRecord now = GameWindow::instance().getKeysDown();
	BitRecord then = keyStates.top();
	typedef std::vector<size_t> Size_tVector;
	Size_tVector diffs = now.diff(then);

	keyStates.pop();

	for (Size_tVector::iterator it = diffs.begin(); it != diffs.end(); it++) {
		KeyboardKey key = (KeyboardKey)*it;
		if (now[key])
			buttonDown(key);
		else
			buttonUp(key);
	}
}

time_t World::calculateDt(time_t now)
{
	time_t dt = now - lastTime;
	lastTime = now;
	return dt;
}

void World::pushLetterbox()
{
	GameWindow& window = GameWindow::instance();

	// Aspect ratio correction.
	rvec2 sz = view->getPhysRes();
	rvec2 lb = -1 * view->getLetterboxOffset();

	window.clip(lb.x, lb.y, sz.x - 2 * lb.x, sz.y - 2 * lb.y);
	int clips = 1;

	// Map bounds.
	rvec2 scale = view->getScale();
	rvec2 virtScroll = view->getMapOffset();
	rvec2 padding = view->getLetterboxOffset();

	rvec2 physScroll = -1 * virtScroll * scale + padding;

	bool loopX = area->loopsInX();
	bool loopY = area->loopsInY();

	if (!loopX && physScroll.x > 0) {
		// Boxes on left-right.
		window.clip(physScroll.x, 0, sz.x - 2 * physScroll.x, sz.x);
		clips++;
	}
	if (!loopY && physScroll.y > 0) {
		// Boxes on top-bottom.
		window.clip(0, physScroll.y, sz.x, sz.y - 2 * physScroll.y);
		clips++;
	}
}

bool World::processDescriptor()
{
	XMLRef doc;
	XMLNode root;

	ASSERT(doc = Reader::getXMLDoc("world.conf", "world"));
	ASSERT(root = doc->root()); // <world>

	for (XMLNode child = root.childrenNode(); child; child = child.next()) {
		if (child.is("info")) {
			ASSERT(processInfo(child));
		}
		else if (child.is("init")) {
			ASSERT(processInit(child));
		}
		else if (child.is("script")) {
			ASSERT(processScript(child));
		}
		else if (child.is("input")) {
			ASSERT(processInput(child));
		}
	}

	if (conf.moveMode == TURN &&
	   (conf.persistInit == 0 || conf.persistCons == 0)) {
		Log::fatal("world.conf", "\"input->persist\" option required in \"turn\" mode");
		return false;
	}

	return true;
}

bool World::processInfo(XMLNode node)
{
	for (node = node.childrenNode(); node; node = node.next()) {
		if (node.is("name")) {
			name = node.content();
		} else if (node.is("author")) {
			author = node.content();
		} else if (node.is("version")) {
			version = atof(node.content().c_str());
		}
	}
	return true;
}

bool World::processInit(XMLNode node)
{
	for (node = node.childrenNode(); node; node = node.next()) {
		if (node.is("area")) {
			startArea = node.content();
		}
		else if (node.is("player")) {
			playerEntity = node.attr("file");
			playerPhase = node.attr("phase");
		}
		else if (node.is("mode")) {
			std::string str = node.content();
			if (str == "turn")
				conf.moveMode = TURN;
			else if (str == "tile")
				conf.moveMode = TILE;
			else if (str == "notile")
				conf.moveMode = NOTILE;
		}
		else if (node.is("coords")) {
			if (!node.intAttr("x", &startCoords.x) ||
			    !node.intAttr("y", &startCoords.y) ||
			    !node.doubleAttr("z", &startCoords.z))
				return false;
		}
		else if (node.is("viewport")) {
			if (!node.intAttr("width", &viewportSz.x) ||
			    !node.intAttr("height", &viewportSz.y))
				return false;
		}
	}
	return true;
}

bool World::processScript(XMLNode node)
{
	for (node = node.childrenNode(); node; node = node.next()) {
		std::string filename = node.content();
		// ScriptRef script = Script::create(filename);

		// if (!script)
		// 	return false;

		// if (node.is("on_init")) {
		// 	if (!script->validate()) {
		// 		Log::err("World", "on_init: " + filename + ": invalid");
		// 		return false;
		// 	}
		// 	loadScript = script;
		// } else if (node.is("on_area_init")) {
		// 	if (!script->validate()) {
		// 		Log::err("World", "on_area_init: " + filename + ": invalid");
		// 		return false;
		// 	}
		// 	areaLoadScript = script;
		// }
	}
	return true;
}

bool World::processInput(XMLNode node)
{
	for (node = node.childrenNode(); node; node = node.next()) {
		if (node.is("persist")) {
			if (!node.intAttr("init", &conf.persistInit) ||
				!node.intAttr("cons", &conf.persistCons))
			    return false;
		}
	}
	return true;
}

void exportWorld()
{
}

