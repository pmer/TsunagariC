/**********************************
** Tsunagari Tile Engine         **
** area.cpp                      **
** Copyright 2014 PariahSoft LLC **
**********************************/

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

#include "area.h"
#include "inprogress.h"

DataArea::DataArea() {}
DataArea::~DataArea() {}
void DataArea::onLoad() {}
void DataArea::onFocus() {}
void DataArea::onTick(time_t) {}
void DataArea::onTurn() {}

void DataArea::tick(time_t dt)
{
	for (auto& inProgress : inProgresses) {
		inProgress->tick(dt);
	}
	for (auto it = inProgresses.begin(); it != inProgresses.end(); ) {
		auto& inProgress = *it;
		if (inProgress->isOver())
			it = inProgresses.erase(it);
		else
			it++;
	}
	onTick(dt);
}

void DataArea::playSoundAndThen(std::string sound, ThenFn then)
{
	inProgresses.emplace_back(
		new InProgressSound(sound, then)
	);
}

void DataArea::timerProgress(time_t duration, ProgressFn progress)
{
	inProgresses.emplace_back(
		new InProgressTimer(duration, progress)
	);
}

void DataArea::timerThen(time_t duration, ThenFn then)
{
	inProgresses.emplace_back(
		new InProgressTimer(duration, then)
	);
}

void DataArea::timerProgressAndThen(time_t duration, ProgressFn progress,
	ThenFn then)
{
	inProgresses.emplace_back(
		new InProgressTimer(duration, progress, then)
	);
}

DataArea::TileScript DataArea::script(const std::string& scriptName)
{
	return scripts[scriptName];
}
