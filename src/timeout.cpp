/***************************************
** Tsunagari Tile Engine              **
** timeout.cpp                        **
** Copyright 2011-2013 PariahSoft LLC **
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

#include <list>

#include "formatter.h"
#include "timeout.h"
#include "world.h"

static std::list<Timeout*> timeouts;

void Timeout::cancel()
{
	active = false;
}

bool Timeout::isActive() const
{
	return active;
}

bool Timeout::ready(time_t now) const
{
	return now > start + delay;
}

time_t Timeout::readyTime() const
{
	return start + delay;
}

void Timeout::execute()
{
	callback->invoke();
}

std::string Timeout::repr() const
{
	time_t now = World::instance()->time();
	return Formatter("<timeout time_remaining=%dms active=%s />")
			% (start + delay - now)
			% (isActive() ? "true" : "false");
}

void updateTimeouts()
{
	time_t now = World::instance()->time();
	bool next = true;

	while (next && timeouts.size()) {
		Timeout* t = timeouts.front();
		if (!t->isActive()) {
			timeouts.pop_front();
			delete t;
		}
		else if (t->ready(now)) {
			t->execute();
			timeouts.pop_front();
			delete t;
		}
		else {
			next = false;
		}
	}
}

void exportTimeout()
{
}

