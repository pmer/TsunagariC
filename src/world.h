/******************************
** Tsunagari Tile Engine     **
** world.h                   **
** Copyright 2011 OmegaSDG   **
******************************/

#ifndef WORLD_H
#define WORLD_H

#include <string>
#include <vector>

#include <Gosu/Gosu.hpp>

#include "common.h"

class Area;
class Entity;
class Resourcer;
class GameWindow;

//! World Class
/*!
	This class is conceptually the main class of the Tsunagari Tile Engine.
*/
class World
{
public:
	static World* getWorld();

	//! World Constructor
	World(Resourcer* rc, GameWindow* w);
	
	//! World Destructor
	~World();

	//! World Initializer
	bool init();
	
	//! Gosu Callback
	void buttonDown(const Gosu::Button btn);
	
	//! Gosu Callback
	void draw();
	
	//! Gosu Callback
	bool needsRedraw() const;

	bool loadArea(const std::string& areaName, coord_t playerPos);

private:
	bool processDescriptor();

	Resourcer* rc;
	GameWindow* wnd;
	Area* area;
	Entity* player;

	//! WorldType XML Storage Enum
	/*!
	  Stores the World type. Options are "local" (singleplayer) world, or
	  "network" (multiplayer) world.
	*/
	enum WorldType {
		LOCAL,
		NETWORK
	};
	
	//! WorldEntry XML Storage Struct
	/*!
	  Stores the World's entry point data. Includes the start
	  Area, and starting coordinates.
	*/
	struct WorldEntry {
		std::string area;
		coord_t coords;
	};

	//! WorldValues XML Storage Struct
	/*!
		Main XML storage struct for World.
	*/
	struct WorldValues {
		std::string name;
		std::string author;
		std::string playersprite;
		WorldType type;
		WorldEntry entry;
		std::vector<std::string> scripts;
	} xml;
};

#endif

