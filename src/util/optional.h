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
    explicit Optional(T&& x) noexcept : x(std::move(x)), exists(true) {}
    explicit Optional(const T& x) : x(x), exists(true) {}

    Optional(Optional<T>&& other) : exists(other.exists) {
        if (exists) {
            x = other.x;
        }
        other.exists = false;
    }
    Optional(const Optional<T>& other) : exists(other.exists) {
        if (exists) {
            x = other.x;
        }
    }

    Optional& operator=(const T &x) {
        this->x = x;
        exists = true;
        return *this;
    }

    Optional& operator=(Optional<T>&& other) {
        if (other.exists) {
            x = std::move(other.x);
        }
        exists = other.exists;
        other.exists = false;
        return *this;
    }
    Optional& operator=(const Optional<T>& other) {
        if (other.exists) {
            x = std::move(other.x);
        }
        exists = other.exists;
        return *this;
    }

    operator bool() const { return exists; }

    const T* operator->() const { assert(exists); return &x; }
    const T& operator*() const { assert(exists); return x; }

    T* operator->() { assert(exists); return &x; }
    T& operator*() { assert(exists); return x; }

    bool operator==(const Optional<T>& other) {
        if (exists != other.exists) {
            return false;
        }
        if (!exists) {
            return true;
        }
        return exists ? x == other.x : true;
    }
};

#endif  // SRC_UTIL_OPTIONAL_H_
