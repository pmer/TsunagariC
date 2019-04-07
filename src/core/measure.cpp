/*************************************
** Tsunagari Tile Engine            **
** measure.cpp                      **
** Copyright 2016-2019 Paul Merrill **
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

#include "core/log.h"
#include "os/chrono.h"

#ifdef __APPLE__
#    include "util/hashtable.h"

// sys/kdebug_signpost.h
extern "C" {
    int kdebug_signpost_start(uint32_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t);
    int kdebug_signpost_end(uint32_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t);
}

static uint32_t nextSignpost = 0;
static Hashmap<String, uint32_t> signposts;

uint32_t
getSignpost(String description) {
    if (signposts.find(description) != signposts.end()) {
        return signposts[description];
    }
    else {
        Log::info("Measure",
                  String() << description << " is signpost " << nextSignpost);
        signposts[move_(description)] = nextSignpost;
        return nextSignpost++;
    }
}
#endif  // __APPLE__


struct TimeMeasureImpl {
    String description;
    TimePoint start;
#ifdef __APPLE__
    uint32_t signpost;
#endif
};

TimeMeasure::TimeMeasure(String description) {
    impl = new TimeMeasureImpl;
    impl->description = move_(description);
    impl->start = SteadyClock::now();
#ifdef __APPLE__
    impl->signpost = getSignpost(impl->description);
    kdebug_signpost_start(impl->signpost, 0, 0, 0, 0);
#endif
}

TimeMeasure::~TimeMeasure() {
#ifdef __APPLE__
    kdebug_signpost_end(impl->signpost, 0, 0, 0, 0);
#endif

    TimePoint end = SteadyClock::now();

    Duration elapsed = end - impl->start;
    Log::info("Measure",
              String() << impl->description << " took "
                       << ns_to_s_d(elapsed)
                       << " seconds");

    delete impl;
}
