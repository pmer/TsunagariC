/**********************************
** Tsunagari Tile Engine         **
** inprogress.cpp                **
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

#include "../log.h"

#include "inprogress.h"

InProgress::InProgress()
    : over(false)
{
}

InProgress::~InProgress() {}
void InProgress::tick(time_t) {}

bool InProgress::isOver()
{
    return over;
}


InProgressSound::InProgressSound(const std::string& sound, ThenFn then)
    : sound(Sounds::instance().play(sound)), then(then)
{
    if (!then)
        Log::err("InProgressSound", "invalid 'then'");
}

void InProgressSound::tick(time_t)
{
    if (over)
        return;

    if (!sound->playing()) {
        over = true;
        then();
    }
}

InProgressTimer::InProgressTimer(time_t duration, ProgressFn progress)
    : duration(duration), passed(0), progress(progress)
{
    if (!progress)
        Log::err("InProgressTimer", "invalid 'progress'");
}

InProgressTimer::InProgressTimer(time_t duration, ThenFn then)
    : duration(duration), passed(0), then(then)
{
    if (!then)
        Log::err("InProgressTimer", "invalid 'then'");
}

InProgressTimer::InProgressTimer(time_t duration, ProgressFn progress,
        ThenFn then)
    : duration(duration), passed(0), progress(progress), then(then)
{
    if (!progress)
        Log::err("InProgressTimer", "invalid 'progress'");
    if (!then)
        Log::err("InProgressTimer", "invalid 'then'");
}

void InProgressTimer::tick(time_t dt)
{
    if (over)
        return;

    passed += dt;

    if (passed < duration) {
        if (progress)
            // Range is [0.0, 1.0)
            progress((double)passed / (double)duration);
    }
    else {
        over = true;
        if (then)
            then();
    }
}
