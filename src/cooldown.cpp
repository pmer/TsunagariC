/**********************************
** Tsunagari Tile Engine         **
** cooldown.cpp                  **
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

#include "cooldown.h"
#include "log.h"

Cooldown::Cooldown()
	: duration(0), passed(0)
{
}

Cooldown::Cooldown(time_t duration)
	: duration(duration), passed(0)
{
}

void Cooldown::setDuration(time_t duration)
{
	this->duration = duration;
	passed = 0;
}

void Cooldown::advance(time_t dt)
{
	passed += dt;
}

bool Cooldown::hasExpired()
{
	return passed > duration;
}

void Cooldown::wrap()
{
	if (hasExpired()) {
		passed -= duration;
	}
	else {
		Log::err("Cooldown", "wrapping when not expired");
	}
}
