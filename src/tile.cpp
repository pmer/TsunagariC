/*********************************
** Tsunagari Tile Engine        **
** tile.cpp                     **
** Copyright 2011-2012 OmegaSDG **
*********************************/

#include <stdlib.h> // for exit(1) on fatal

#include <boost/foreach.hpp>
#include <boost/format.hpp>

#include "area.h"
#include "python.h"
#include "python-optional.h"
#include "string.h"
#include "tile.h"
#include "window.h"

static int ivec2_to_dir(ivec2 v)
{
	switch (v.x) {
	case -1:
		return v.y == 0 ? EXIT_LEFT : -1;
	case 0:
		switch (v.y) {
		case -1:
			return EXIT_UP;
		case 0:
			return EXIT_NORMAL;
		case 1:
			return EXIT_DOWN;
		default:
			return -1;
		}
		break;
	case 1:
		return v.y == 0 ? EXIT_RIGHT : -1;
	default:
		return -1;
	}
}

static Exit pythonNewExit(std::string area, int x, int y, double z)
{
	return Exit(area, x, y, z);
}


/*
 * FLAGMANIP
 */
FlagManip::FlagManip(unsigned* flags)
	: flags(flags)
{
}

bool FlagManip::isNowalk() const
{
	return (*flags & TILE_NOWALK) != 0;
}

bool FlagManip::isNowalkPlayer() const
{
	return (*flags & TILE_NOWALK_PLAYER) != 0;
}

bool FlagManip::isNowalkNPC() const
{
	return (*flags & TILE_NOWALK_NPC) != 0;
}

bool FlagManip::isNowalkAreaBound() const
{
	return (*flags & TILE_NOWALK_AREA_BOUND) != 0;
}

void FlagManip::setNowalk(bool nowalk)
{
	*flags &= ~TILE_NOWALK;
	*flags |= TILE_NOWALK * nowalk;
}

void FlagManip::setNowalkPlayer(bool nowalk)
{
	*flags &= ~TILE_NOWALK_PLAYER;
	*flags |= TILE_NOWALK_PLAYER * nowalk;
}

void FlagManip::setNowalkNPC(bool nowalk)
{
	*flags &= ~TILE_NOWALK_NPC;
	*flags |= TILE_NOWALK_NPC * nowalk;
}

void FlagManip::setNowalkAreaBound(bool nowalk)
{
	*flags &= ~TILE_NOWALK_AREA_BOUND;
	*flags |= TILE_NOWALK_AREA_BOUND * nowalk;
}


Exit::Exit()
{
}

Exit::Exit(const std::string area, int x, int y, double z)
	: area(area), coords(x, y, z)
{
}


/*
 * TILEBASE
 */
TileBase::TileBase()
	: parent(NULL), flags(0x0)
{
}

FlagManip TileBase::flagManip()
{
	return FlagManip(&flags);
}

bool TileBase::hasFlag(unsigned flag) const
{
	return flags & flag || (parent && parent->hasFlag(flag));
}

TileType* TileBase::getType() const
{
	return (TileType*)parent;
}

void TileBase::setType(TileType* type)
{
	parent = type;
}

void TileBase::runEnterScript(Entity* triggeredBy)
{
	runScript(triggeredBy, enterScript);
	if (parent)
		parent->runEnterScript(triggeredBy);
}

void TileBase::runLeaveScript(Entity* triggeredBy)
{
	runScript(triggeredBy, leaveScript);
	if (parent)
		parent->runLeaveScript(triggeredBy);
}

void TileBase::runUseScript(Entity* triggeredBy)
{
	runScript(triggeredBy, useScript);
	if (parent)
		parent->runUseScript(triggeredBy);
}

void TileBase::runScript(Entity* triggeredBy, ScriptInst& script)
{
	pythonSetGlobal("Entity", triggeredBy);
	pythonSetGlobal("Tile", this);
	script.invoke();
}


/*
 * TILE
 */
Tile::Tile()
	: entCnt(0)
{
}

Tile::Tile(Area* area, int x, int y, int z)
	: TileBase(), area(area), x(x), y(y), z(z), entCnt(0)
{
	memset(exits, 0, sizeof(exits));
	memset(layermods, 0, sizeof(layermods));
}

Tile* Tile::offset(int x, int y)
{
	return area->getTile(this->x + x, this->y + y, z);
}

double Tile::getZ()
{
	vicoord vi = area->phys2virt_vi(icoord(x, y, z));
	return vi.z;
}

Exit* Tile::getNormalExit() const
{
	return exits[EXIT_NORMAL];
}

void Tile::setNormalExit(Exit exit)
{
	Exit** norm = &exits[EXIT_NORMAL];
	if (*norm)
		delete *norm;
	*norm = new Exit(exit);
}

Exit* Tile::exitAt(ivec2 dir) const
{
	int idx = ivec2_to_dir(dir);
	return idx == -1 ? NULL : exits[idx];
}

boost::optional<double> Tile::layermodAt(ivec2 dir) const
{
	int idx = ivec2_to_dir(dir);
	return idx == -1 ? boost::optional<double>() : layermods[idx];
}


/*
 * TILETYPE
 */
TileType::TileType()
	: TileBase()
{
}

TileType::TileType(ImageRef& img)
	: TileBase()
{
	anim.addFrame(img);
}

bool TileType::needsRedraw() const
{
	const int millis = GameWindow::instance().time();
	return anim.needsRedraw(millis);
}

/*
 * TILESET
 */
TileSet::TileSet()
{
}

TileSet::TileSet(int width, int height)
	: width(width), height(height)
{
}

void TileSet::add(TileType* type)
{
	types.push_back(type);
}

void TileSet::set(int idx, TileType* type)
{
	types[idx] = type;
}

TileType* TileSet::get(int x, int y)
{
	size_t i = idx(x, y);
	if (i > types.size()) {
		Log::err("TileSet", boost::str(boost::format(
			"get(%d, %d): out of bounds") % x % y));
		return NULL;
	}
	return types[i];
}

int TileSet::getWidth() const
{
	return height;
}

int TileSet::getHeight() const
{
	return width;
}

size_t TileSet::idx(int x, int y) const
{
	return y * width + x;
}


/*
 * PYTHON
 */
void exportTile()
{
	using namespace boost::python;

	class_<FlagManip> ("FlagManipulator", no_init)
		.add_property("nowalk",
			&FlagManip::isNowalk, &FlagManip::setNowalk)
		.add_property("nowalk_player",
			&FlagManip::isNowalkPlayer, &FlagManip::setNowalkPlayer)
		.add_property("nowalk_npc",
			&FlagManip::isNowalkNPC, &FlagManip::setNowalkNPC)
		.add_property("nowalk_area_bound",
			&FlagManip::isNowalkAreaBound,
			&FlagManip::setNowalkAreaBound)
		;
	class_<TileBase> ("TileBase", no_init)
		.add_property("flag", &TileBase::flagManip)
		.add_property("type",
		    make_function(
		      static_cast<TileType* (TileBase::*) () const>
		        (&TileBase::getType),
		      return_value_policy<reference_existing_object>()),
		    &TileBase::setType)
		.def_readwrite("on_enter", &TileBase::enterScript)
		.def_readwrite("on_leave", &TileBase::leaveScript)
		.def_readwrite("on_use", &TileBase::useScript)
		.def("run_enter_script", &TileBase::runEnterScript)
		.def("run_leave_script", &TileBase::runLeaveScript)
		.def("run_use_script", &TileBase::runUseScript)
		;
	class_<Tile, bases<TileBase> > ("Tile", no_init)
		.def_readonly("area", &Tile::area)
		.def_readonly("x", &Tile::x)
		.def_readonly("y", &Tile::y)
		.add_property("z", &Tile::getZ)
		.add_property("exit",
		    make_function(
		      static_cast<Exit* (Tile::*) () const>
		        (&Tile::getNormalExit),
		      return_value_policy<reference_existing_object>()),
		    &Tile::setNormalExit)
		.def_readonly("nentities", &Tile::entCnt)
		.def("offset", &Tile::offset,
		    return_value_policy<reference_existing_object>())
		;
	class_<TileType, bases<TileBase> > ("TileType", no_init)
		;
	class_<TileSet> ("TileSet", no_init)
		.add_property("width", &TileSet::getWidth)
		.add_property("height", &TileSet::getHeight)
		.def("at", &TileSet::get,
		    return_value_policy<reference_existing_object>())
		;
	class_<Exit> ("Exit", no_init)
		.def_readwrite("area", &Exit::area)
		.def_readwrite("coords", &Exit::coords)
		;
	pythonAddFunction("new_exit", pythonNewExit);
}

