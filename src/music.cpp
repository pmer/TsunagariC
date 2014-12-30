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
#include "formatter.h"
#include "math.h"
#include "music.h"


static void clientIniVolumeVerify()
{
	if (conf.musicVolume < 0 || 100 < conf.musicVolume)
		Log::err("Music", "Music volume not within bounds [0,100]");
}

static double clientIniVolumeApply(double volume)
{
	clientIniVolumeVerify();
	return volume * conf.musicVolume / 100.0;
}

static double clientIniVolumeUnapply(double volume)
{
	clientIniVolumeVerify();
	return volume / conf.musicVolume * 100.0;
}


Music::Music()
	: state(NOT_PLAYING), volume(1.0), pausedCount(0)
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

bool Music::paused()
{
	return pausedCount != 0;
}

void Music::pause()
{
	pausedCount++;
}

void Music::resume()
{
	if (pausedCount <= 0) {
		Log::err("Music", "unpausing, but music not paused");
		return;
	}
	pausedCount--;
}

double Music::getVolume()
{
	return clientIniVolumeUnapply(volume);
}

void Music::setVolume(double attemptedVolume)
{
	double newVolume = bound(attemptedVolume, 0.0, 1.0);
	if (attemptedVolume != newVolume) {
		Log::info("Music",
			Formatter(
				"Attempted to set volume to %, setting it to %"
			) % attemptedVolume % newVolume
		);
	}

	volume = clientIniVolumeApply(newVolume);
}

void Music::stop()
{
	state = NOT_PLAYING;
	pausedCount = 0;
	curIntro = newIntro = "";
	curLoop = newLoop = "";
}

void Music::playIntro()
{
	curIntro = newIntro;
	state = PLAYING_INTRO;
	pausedCount = 0;
}

void Music::playLoop()
{
	curLoop = newLoop;
	state = PLAYING_LOOP;
	pausedCount = 0;
}

