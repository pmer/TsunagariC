/***************************************
** Tsunagari Tile Engine              **
** gosu-music.cpp                     **
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

#include "../client-conf.h"
#include "gosu-music.h"
#include "../reader.h"

static GosuMusic::SongRef genSong(const std::string& name)
{
	std::unique_ptr<Gosu::Buffer> buffer(Reader::readBuffer(name));
	if (!buffer)
		return GosuMusic::SongRef();
	return GosuMusic::SongRef(new Gosu::Song(buffer->frontReader()));
}


static GosuMusic globalMusic;

Music& Music::instance()
{
	return globalMusic;
}


GosuMusic::GosuMusic() : songs(genSong)
{
}

GosuMusic::~GosuMusic()
{
	if (musicInst && musicInst->playing())
		musicInst->stop();
}

bool GosuMusic::setIntro(const std::string& filename)
{
	if (Music::setIntro(filename)) {
		// Optimize XXX: Don't load until played.
		introMusic = filename.size() ? getSong(filename) : SongRef();
		return true;
	}
	else
		return false;
}

bool GosuMusic::setLoop(const std::string& filename)
{
	if (Music::setLoop(filename)) {
		// Optimize XXX: Don't load until played.
		loopMusic = filename.size() ? getSong(filename) : SongRef();
		return true;
	}
	else
		return false;
}

bool GosuMusic::setVolume(int level)
{
	if (Music::setVolume(level)) {
		if (musicInst)
			musicInst->changeVolume(level);
		return true;
	}
	else
		return false;
}

bool GosuMusic::setPaused(bool p)
{
	if (Music::setPaused(p)) {
		if (musicInst) {
			if (p)
				musicInst->pause();
			else
				musicInst->play();
		}
		return true;
	}
	else
		return false;
}

void GosuMusic::stop()
{
	Music::stop();
	if (musicInst)
		musicInst->stop();
}

void GosuMusic::tick()
{
	if (paused)
		return;

	switch (state) {
	case NOT_PLAYING:
		if (musicInst && musicInst->playing())
			musicInst->stop();
		break;
	case PLAYING_INTRO:
		if (!musicInst->playing()) {
			if (newLoop.size() && loopMusic)
				playLoop();
			else
				state = NOT_PLAYING;
		}
		break;
	case PLAYING_LOOP:
		break;
	case CHANGED_INTRO:
		if (newIntro.size() && introMusic)
			playIntro();
		else if (newLoop.size() && newLoop != curLoop)
			state = CHANGED_LOOP;
		else if (newLoop.size())
			state = PLAYING_LOOP;
		else
			state = NOT_PLAYING;
		break;
	case CHANGED_LOOP:
		if (newIntro.size() && loopMusic)
			playIntro();
		else if (newLoop.size() && loopMusic)
			playLoop();
		else
			state = NOT_PLAYING;
		break;
	}
}

void GosuMusic::playIntro()
{
	Music::playIntro();
	if (musicInst && musicInst->playing())
		musicInst->stop();
	introMusic->play(false);
	introMusic->changeVolume(conf.musicVolume / 100.0);
	musicInst = introMusic;
}

void GosuMusic::playLoop()
{
	Music::playLoop();
	if (musicInst && musicInst->playing())
		musicInst->stop();
	loopMusic->play(true);
	loopMusic->changeVolume(conf.musicVolume / 100.0);
	musicInst = loopMusic;
}

GosuMusic::SongRef GosuMusic::getSong(const std::string& name)
{
	if (!conf.audioEnabled)
		return SongRef();
	return songs.lifetimeRequest(name);
}

