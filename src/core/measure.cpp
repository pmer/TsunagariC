/*************************************
** Tsunagari Tile Engine            **
** measure.cpp                      **
** Copyright 2016-2018 Paul Merrill **
*************************************/

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

#include "core/measure.h"

#include <chrono>
#include <string>

#include "core/log.h"


#ifdef __APPLE__
#include <sys/kdebug_signpost.h>
#include <unordered_map>

static int nextSignpost = 0;
static std::unordered_map<std::string,int> signposts;

int getSignpost(std::string description) {
    if (signposts.find(description) != signposts.end()) {
        return signposts[description];
    } else {
        Log::info("Measure", description + " is signpost " + std::to_string(nextSignpost));
        signposts[description] = nextSignpost;
        return nextSignpost++;
    }
}
#endif  // __APPLE__


struct TimeMeasureImpl {
    std::string description;
    std::chrono::time_point<std::chrono::system_clock> start;
#ifdef __APPLE__
    int signpost;
#endif
};

TimeMeasure::TimeMeasure(std::string description) {
    impl = new TimeMeasureImpl;
    impl->description = description;
    impl->start = std::chrono::system_clock::now();
#ifdef __APPLE__
    impl->signpost = getSignpost(description);
    kdebug_signpost_start(impl->signpost, 0, 0, 0, 0);
#endif
}

TimeMeasure::~TimeMeasure() {
#ifdef __APPLE__
    kdebug_signpost_end(impl->signpost, 0, 0, 0, 0);
#endif

    std::chrono::time_point<std::chrono::system_clock> end;
    end = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_seconds = end - impl->start;
    Log::info("Measure", impl->description + " took " + std::to_string(elapsed_seconds.count()) + " seconds");

    delete impl;
}
