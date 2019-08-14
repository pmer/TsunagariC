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

#include "util/align.h"
#include "util/assert.h"
#include "util/constexpr.h"
#include "util/move.h"
#include "util/new.h"
#include "util/noexcept.h"

struct None {};
static None none;

template<typename T>
class Optional {
    bool exists;
    Align<T> storage;

 public:
    explicit CONSTEXPR11 Optional() noexcept : storage(), exists(false) {}
    explicit CONSTEXPR14 Optional(T&& x_) noexcept : exists(true) {
        new (&storage) T(move_(x_));
    }
    explicit CONSTEXPR14 Optional(const T& x_) noexcept : exists(true) {
        new (&storage) T(x_);
    }
    CONSTEXPR11 Optional(None) noexcept : storage(), exists(false) {}

    CONSTEXPR11 Optional(Optional<T>&& other) noexcept
            : storage(), exists(other.exists) {
        if (other.exists) {
            new (&storage) T(move_(other.x()));
            other.x().~T();
            other.exists = false;
        }
    }
    CONSTEXPR14 Optional(const Optional<T>& other) noexcept
            : exists(other.exists) {
        if (other.exists) {
            new (&storage) T(other.x());
        }
    }

    ~Optional() noexcept {
        if (exists) {
            x().~T();
        }
    }

    inline Optional& operator=(T&& x_) noexcept {
        if (exists) {
            x() = move_(x_);
        }
        else {
            new (&storage) T(move_(x_));
            exists = true;
        }
        return *this;
    }
    inline Optional& operator=(const T& x_) noexcept {
        if (exists) {
            x() = x_;
        }
        else {
            new (&storage) T(x_);
            exists = true;
        }
        return *this;
    }

    inline Optional& operator=(Optional<T>&& other) noexcept {
        if (exists) {
            if (other.exists) {
                x() = move_(other.x());
            }
            else {
                x().~T();
            }
        }
        else {
            if (other.exists) {
                new (&storage) T(move_(other.x()));
            }
        }
        exists = other.exists;
        other.exists = false;
        return *this;
    }
    inline Optional& operator=(const Optional<T>& other) noexcept {
        if (exists) {
            if (other.exists) {
                x() = other.x();
            }
            else {
                x().~T();
            }
        }
        else {
            if (other.exists) {
                new (&storage) T(other.x());
            }
        }
        exists = other.exists;
        return *this;
    }

    inline CONSTEXPR11 operator bool() const noexcept { return exists; }

    inline CONSTEXPR14 const T* operator->() const noexcept {
        assert_(exists);
        return &x();
    }
    inline CONSTEXPR14 const T& operator*() const noexcept {
        assert_(exists);
        return x();
    }

    inline CONSTEXPR14 T* operator->() noexcept {
        assert_(exists);
        return &x();
    }
    inline CONSTEXPR14 T& operator*() noexcept {
        assert_(exists);
        return x();
    }

    friend CONSTEXPR11 bool operator==(const Optional<T>& a,
                                       const Optional<T>& b) noexcept;

 private:
    T& x() noexcept { return *reinterpret_cast<T*>(&storage.storage); }
    const T& x() const noexcept {
        return *reinterpret_cast<const T*>(&storage.storage);
    }
};

template<typename T>
inline CONSTEXPR11 bool
operator==(const Optional<T>& a, const Optional<T>& b) noexcept {
    return a.exists == b.exists && (!a.exists || a.x() == b.x());
}

#endif  // SRC_UTIL_OPTIONAL_H_
