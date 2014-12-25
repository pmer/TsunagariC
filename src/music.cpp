/***************************************
** Tsunagari Tile Engine              **
** music.cpp                          **
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

#include "client-conf.h"
#include "math.h"
#include "music.h"

Music::Music() : state(NOT_PLAYING), volume(100), paused(false)
{
}

Music::~Music() {}

bool Music::setIntro(const std::string& filename)
{
	if (newIntro == filename)
		return false;

	switch (state) {
	case NOT_PLAYING:
	case PLAYING_INTRO:
	case PLAYING_LOOP:
		state = CHANGED_INTRO;
	default: break;
	}

	newIntro = filename;
	return true;
}

bool Music::setLoop(const std::string& filename)
{
	if (newLoop == filename)
		return false;

	switch (state) {
	case NOT_PLAYING:
	case PLAYING_INTRO:
	case PLAYING_LOOP:
		state = CHANGED_LOOP;
	default: break;
	}

	newLoop = filename;
	return true;
}

int Music::getVolume()
{
	return volume;
}

bool Music::setVolume(int level)
{
	if (level == volume)
		return false;
	if (0 < level || level > 100) {
		Log::info("Music", "volume can only be set between 0 and 100");
		level = bound(level, 0, 100);
	}
	volume = conf.musicVolume = level;
	return true;
}

bool Music::isPaused()
{
	return paused;
}

bool Music::setPaused(bool p)
{
	if (paused == p)
		return false;
	paused = p;
	return true;
}

void Music::stop()
{
	paused = false;
	state = NOT_PLAYING;
}

void Music::tick() {}

void Music::playIntro()
{
	curIntro = newIntro;
	state = PLAYING_INTRO;
}

void Music::playLoop()
{
	curLoop = newLoop;
	state = PLAYING_LOOP;
}

