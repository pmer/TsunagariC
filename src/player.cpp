/*********************************
** Tsunagari Tile Engine        **
** player.cpp                   **
** Copyright 2011-2012 OmegaSDG **
*********************************/

#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <Gosu/Audio.hpp>
#include <Gosu/Input.hpp>
#include <Gosu/Math.hpp>

#include "area.h"
#include "config.h"
#include "entity.h"
#include "log.h"
#include "player.h"
#include "world.h"
#include "window.h"

template<class Cont, class ValueType>
void removeValue(Cont* c, ValueType v)
{
	typename Cont::iterator it;
	for (it = c->begin(); it != c->end(); ++it) {
		if (*it == v) {
			c->erase(it);
			return;
		}
	}
}


Player::Player(Resourcer* rc, Area* area, ClientValues* conf)
	: Entity(rc, area, conf), velocity(0, 0, 0)
{
}

void Player::startMovement(icoord delta)
{
	switch (conf->moveMode) {
	case TURN:
		moveByTile(delta);
		break;
	case TILE:
		movements.push_back(delta);
		velocity = delta;
		moveByTile(velocity);
		break;
	case NOTILE:
		// TODO
		break;
	}
}

void Player::stopMovement(icoord delta)
{
	switch (conf->moveMode) {
	case TURN:
		break;
	case TILE:
		removeValue(&movements, delta);
		velocity = movements.size() ?
		           movements.back() :
			   icoord(0, 0, 0);
		if (velocity)
			moveByTile(velocity);
		break;
	case NOTILE:
		// TODO
		break;
	}
}

void Player::moveByTile(icoord delta)
{
	if (moving)
		// Support queueing moves?
		return;

	// Left CTRL allows changing facing, but disallows movement.
	const GameWindow& window = GameWindow::getWindow();
	if (window.input().down(Gosu::kbLeftControl)) {
		calculateFacing(delta.x, delta.y);
		setPhase(facing);
		redraw = true;
		return;
	}

	icoord newCoord = getTileCoords();
	newCoord += delta;

	// The tile is off the map. Turn to face the direction, but don't move.
	if (!area->inBounds(newCoord)) {
		calculateFacing(delta.x, delta.y);
		setPhase(facing);
		redraw = true;
		return;
	}

	destTile = &area->getTile(newCoord);

	// Is anything player-specific preventing us from moving?
	if (destTile->hasFlag(player_nowalk)) {
		// The tile we're trying to move onto is set as player_nowalk.
		// Turn to face the direction, but don't move.
		calculateFacing(delta.x, delta.y);
		setPhase(facing);
		redraw = true;
		return;
	}

	Entity::moveByTile(delta);
}

void Player::useTile()
{
	std::vector<icoord> tiles = frontTiles();
	BOOST_FOREACH(icoord& c, tiles) {
		Tile& t = area->getTile(c);
		t.onUseScripts(rc, this);
	}
}

void Player::preMove()
{
	Entity::preMove();

	SampleRef step = getSound("step");
	if (step)
		step->play();

}

void Player::postMove()
{
	Entity::postMove();

	// Doors
	const boost::optional<Door> door = destTile->door;
	if (door) {
		World* world = World::getWorld();
		AreaPtr newArea = world->getArea(door->area);
		if (newArea) {
			world->focusArea(newArea, door->tile);
		}
		else {
			// Roll back movement if door failed to open.
			r = fromCoord;
			Log::err("Door",
			         door->area + ": failed to load properly");
		}
	}

	icoord tile = getTileCoords();
	Log::dev("Player", boost::str(
		boost::format("location x:%d y:%d z:%d")
		  % tile.x % tile.y % (int)r.z)
	);

	// If we have a velocity, keep moving.
	if (conf->moveMode == TILE && velocity)
		moveByTile(velocity);
}

