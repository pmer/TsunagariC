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

#if defined(__APPLE__) && defined(MAKE_MACOS_SIGNPOSTS)
#include "util/hashtable.h"

// sys/kdebug_signpost.h
extern "C" {
int kdebug_signpost_start(uint32_t,
                          uintptr_t,
                          uintptr_t,
                          uintptr_t,
                          uintptr_t) noexcept;
int kdebug_signpost_end(uint32_t,
                        uintptr_t,
                        uintptr_t,
                        uintptr_t,
                        uintptr_t) noexcept;
}

static uint32_t nextSignpost = 0;
static Hashmap<String, uint32_t> signposts;

static uint32_t
getSignpost(String description) noexcept {
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
#endif  // defined(__APPLE__) && defined(MAKE_MACOS_SIGNPOSTS)

TimeMeasure::TimeMeasure(String description) noexcept {
    this->description = move_(description);
    start = SteadyClock::now();
#if defined(__APPLE__) && defined(MAKE_MACOS_SIGNPOSTS)
    signpost = getSignpost(description);
    kdebug_signpost_start(signpost, 0, 0, 0, 0);
#endif
}

TimeMeasure::~TimeMeasure() noexcept {
#if defined(__APPLE__) && defined(MAKE_MACOS_SIGNPOSTS)
    kdebug_signpost_end(signpost, 0, 0, 0, 0);
#endif

    TimePoint end = SteadyClock::now();

    Duration elapsed = end - start;
    Log::info("Measure",
              String() << description << " took " << ns_to_s_d(elapsed)
                       << " seconds");
}
