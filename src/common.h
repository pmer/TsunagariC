/*********************************
** Tsunagari Tile Engine        **
** common.h                     **
** Copyright 2011-2012 OmegaSDG **
*********************************/

#ifndef COMMON_H
#define COMMON_H

#include "log.h" // for message_mode_t
#include "vec.h" // for icoord

//! Game Movement Mode
enum movement_mode_t {
	TURN,
	TILE,
	NOTILE
};

/**
 * Virtual integer coordinate.
 *
 * x and y are the same as a physical integer coordinate.
 * z is a virtual layer depth within an Area.
 */
struct vicoord
{
	vicoord() {}
	vicoord(int x, int y, double z): x(x), y(y), z(z) {}

	int x, y;
	double z;
};

//! 3D cube type.
struct icube_t {
	int x1, x2;
	int y1, y2;
	int z1, z2;
};

//! cube_t constructor.
icube_t icube(int x1, int y1, int z1,
              int x2, int y2, int z2);

//! Engine-wide user-confurable values.
struct Conf {
	std::string worldFilename;
	verbosity_t verbosity;
	movement_mode_t moveMode;
	icoord windowSize;
	bool fullscreen;
	bool audioEnabled;
	bool cacheEnabled;
	int cacheTTL;
	int cacheSize;
	int persistInit;
	int persistCons;
};
extern Conf conf;

template<class T>
T wrap(T min, T value, T max)
{
	while (value < min)
		value += max;
	return value % max;
}

#endif

