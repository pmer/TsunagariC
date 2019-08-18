/***************************************
** Tsunagari Tile Engine              **
** rc-reader-cache.h                  **
** Copyright 2011-2013 Michael Reiley **
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

#ifndef SRC_CACHE_RC_READER_CACHE_H_
#define SRC_CACHE_RC_READER_CACHE_H_

#include "cache/rc-cache.h"
#include "util/string-view.h"

template<typename T>
using GenFn = T (*)(StringView name);

template<typename T, GenFn<T> fn>
class RcReaderCache {
 public:
    T momentaryRequest(StringView name) noexcept {
        T t = cache.momentaryRequest(name);
        if (t) {
            return t;
        }

        t = fn(name);
        cache.momentaryPut(name, t);
        return t;
    }

    T lifetimeRequest(StringView name) noexcept {
        T t = cache.lifetimeRequest(name);
        if (t) {
            return t;
        }

        t = fn(name);
        cache.lifetimePut(name, t);
        return t;
    }

    void garbageCollect() noexcept { cache.garbageCollect(); }

 private:
    RcCache<T> cache;
};

#endif  // SRC_CACHE_RC_READER_CACHE_H_
