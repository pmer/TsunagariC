/***************************************
** Tsunagari Tile Engine              **
** cache-template.h                   **
** Copyright 2011-2014 Michael Reiley **
** Copyright 2011-2019 Paul Merrill   **
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

#ifndef SRC_CACHE_CACHE_TEMPLATE_CPP_
#define SRC_CACHE_CACHE_TEMPLATE_CPP_

#include "cache/cache.h"
#include "core/client-conf.h"
#include "core/log.h"
#include "core/world.h"
#include "util/vector.h"

#define IN_USE_NOW -1

template<class T>
T
Cache<T>::momentaryRequest(StringView name) noexcept {
    auto it = map.find(name);
    if (it != map.end()) {
        //      Log::info("Cache", String() << name << ": requested (cached)");
        CacheEntry& entry = it.value();
        // Set lastUsed to now because it won't be used
        // by the time garbageCollect() gets to it.
        entry.lastUsed = World::instance().time();
        return entry.resource;
    }
    Log::info("Cache", String() << name << ": requested");
    return T();
}

template<class T>
T
Cache<T>::lifetimeRequest(StringView name) noexcept {
    auto it = map.find(name);
    if (it != map.end()) {
        //      Log::info("Cache", String() << name << ": requested (cached)");
        CacheEntry& entry = it.value();
        entry.lastUsed = IN_USE_NOW;
        return entry.resource;
    }
    Log::info("Cache", String() << name << ": requested");
    return T();
}

template<class T>
void
Cache<T>::momentaryPut(StringView name, T data) noexcept {
    CacheEntry entry;
    entry.resource = data;
    time_t now = World::instance().time();
    entry.lastUsed = now;
    map[name] = entry;
}

template<class T>
void
Cache<T>::lifetimePut(StringView name, T data) noexcept {
    CacheEntry entry;
    entry.resource = data;
    entry.lastUsed = IN_USE_NOW;
    map[name] = entry;
}

template<class T>
void
Cache<T>::garbageCollect() noexcept {
    time_t now = World::instance().time();
    Vector<String> dead;
    for (auto it = map.begin(); it != map.end(); it++) {
        StringView name = it.key();
        CacheEntry& cache = it.value();
        bool unused = !cache.resource || cache.resource.unique();
        if (!unused) {
            continue;
        }
        if (cache.lastUsed == IN_USE_NOW) {
            cache.lastUsed = now;
            //          Log::info("Cache", String() << name << ": unused");
        }
        else if (now > cache.lastUsed + conf.cacheTTL * 1000) {
            dead.push_back(name);
            Log::info("Cache", String() << name << ": purged");
        }
    }
    for (auto it = dead.begin(); it != dead.end(); it++) {
        map.erase(*it);
    }
}

#endif  // SRC_CACHE_CACHE_TEMPLATE_CPP_
