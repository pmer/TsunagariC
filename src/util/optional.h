/*************************************
** Tsunagari Tile Engine            **
** optional.h                       **
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

#ifndef SRC_UTIL_OPTIONAL_H_
#define SRC_UTIL_OPTIONAL_H_

#include "util/assert.h"
#include "util/move.h"
#include "util/new.h"

template<typename T>
class Optional {
    alignas(alignof(T)) char x[sizeof(T)];
    bool exists;

 public:
    explicit Optional() noexcept : exists(false) {}
    explicit Optional(T&& x) noexcept : exists(true) {
        cast() = move_(x);
    }
    explicit Optional(const T& x) noexcept : exists(true) {
        cast() = x;
    }

    Optional(Optional<T>&& other) noexcept : exists(other.exists) {
        if (exists) {
            cast() = move_(other.cast());
            other.cast().~T();
        }
        other.exists = false;
    }
    Optional(const Optional<T>& other) noexcept : exists(other.exists) {
        if (exists) {
            cast() = other.cast();
        }
    }

    ~Optional() noexcept {
        if (exists) {
            cast().~T();
        }
    }

    inline Optional& operator=(const T &x) {
        if (exists) {
            cast() = x;
        }
        else {
            new (&this->x) T(x);
        }
        exists = true;
        return *this;
    }

    inline Optional& operator=(Optional<T>&& other) {
        if (exists) {
            if (other.exists) {
                cast() = move_(other.cast());
            }
            else {
                cast().~T();
            }
        }
        else {
            if (other.exists) {
                new (x) T(other.cast());
            }
        }
        exists = other.exists;
        other.exists = false;
        return *this;
    }
    inline Optional& operator=(const Optional<T>& other) {
        if (exists) {
            if (other.exists) {
                cast() = move_(other.cast());
            }
            else {
                cast().~T();
            }
        }
        else {
            if (other.exists) {
                new (x) T(other.cast());
            }
        }
        exists = other.exists;
        return *this;
    }

    inline operator bool() const { return exists; }

    inline const T* operator->() const { assert_(exists); return &cast(); }
    inline const T& operator*() const { assert_(exists); return cast(); }

    inline T* operator->() { assert_(exists); return &cast(); }
    inline T& operator*() { assert_(exists); return cast(); }

    inline bool operator==(const Optional<T>& other) const {
        if (exists != other.exists) {
            return false;
        }
        if (!exists) {
            return true;
        }
        return cast() == other.cast();
    }

 private:
    inline T& cast() { return *reinterpret_cast<T*>(&x); }
    inline const T& cast() const { return *reinterpret_cast<const T*>(&x); }
};

#endif  // SRC_UTIL_OPTIONAL_H_
