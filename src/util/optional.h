/********************************
** Tsunagari Tile Engine       **
** optional.h                  **
** Copyright 2016 Paul Merrill **
********************************/

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

#ifndef SRC_UTIL_OPTIONAL_H_
#define SRC_UTIL_OPTIONAL_H_

#include <assert.h>

#include <memory>

template<typename T>
class Optional {
    T x;
    bool exists;

 public:
    explicit Optional() : x(), exists(false) {}
    explicit Optional(T&& x) : x(std::move(x)), exists(true) {}
    explicit Optional(const T& x) : x(x), exists(true) {}

    Optional &operator=(const T &x) {
        this->x = x;
        exists = true;
        return *this;
    }

    operator bool() const { return exists; }
    const T* operator->() const { assert(exists); return &x; }
    const T& operator*() const { assert(exists); return x; }
};

#endif  // SRC_UTIL_OPTIONAL_H_
