/***************************************
** Tsunagari Tile Engine              **
** cache-impl.h                       **
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

#ifndef SRC_CACHE_CACHE_IMPL_CPP_
#define SRC_CACHE_CACHE_IMPL_CPP_

#include "cache/cache.h"
#include "core/log.h"
#include "util/assert.h"
#include "util/move.h"

template<typename T>
Optional<T*>
Cache<T>::acquire(StringView key) noexcept {
    Optional<Entry*> e = entries.tryAt(key);
    if (e) {
        Log::info("Cache", String() << key << ": requested (cached)");
        (*e)->numUsing += 1;
        return Optional<X*>(&(*e)->data);
    }
    else {
        Log::info("Cache", String() << key << ": requested (not cached)");
        return none;
    }
}

template<typename T>
void
Cache<T>::release(StringView key, time_t now) noexcept {
    assert_(entries.contains(name);

    Entry& e = entries[key];
    e.numUsing -= 1;

    assert_(e.numUsing >= 0);

    if (e.numUsing == 0) {
        e.lastUsed = now;
    }
}

template<typename T>
void
Cache<T>::set(StringView key, T data) noexcept {
    assert_(!entries.contains(key));

    entries[key] = {move_(data), 1, 0};
}

template<typename T>
void
Cache<T>::garbageCollect(time_t lastUsedBefore) noexcept {
    for (auto it = entries.begin(); it != entries.end(); ) {
        CacheEntry& entry = it.value();
        if (entry.numUsed == 0 && entry.lastUsed < lastUsedBefore) {
            StringView key = it.key();
            Log::info("Cache", String() << key << ": purged");
            it = entries.erase(it);
        }
        else {
            ++it;
        }
    }
}

#endif  // SRC_CACHE_CACHE_IMPL_CPP_
