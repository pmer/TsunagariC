/*************************************
** Tsunagari Tile Engine            **
** rc.h                             **
** Copyright 2017-2019 Paul Merrill **
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

#ifndef SRC_UTIL_RC_H_
#define SRC_UTIL_RC_H_

#include "util/assert.h"
#include "util/int.h"
#include "util/meta.h"

//
// Pointers
//   unique.h
//     class Unique      Unique pointer
//   rc.h
//     class Rc          Reference counted pointer
//     class CompactRc   Half the size and half the number of allocations
//

template<typename T>
class Rc {
 public:
    T* x;
    size_t* count;

 public:
    Rc() noexcept : x(nullptr), count(nullptr) {}
    Rc(Rc&& other) noexcept : x(other.x), count(other.count) {
        other.x = nullptr;
        other.count = nullptr;
    }
    Rc(const Rc& other) noexcept : x(other.x), count(other.count) { incref(); }
    Rc(T* x) noexcept : x(x) { count = x ? new size_t(1) : nullptr; }
    ~Rc() noexcept { decref(); }

    Rc& operator=(T* x) noexcept {
        decref();
        *this = Rc(x);
        return *this;
    }

    Rc& operator=(Rc&& other) noexcept {
        decref();
        x = other.x;
        count = other.count;
        other.x = nullptr;
        other.count = nullptr;
        return *this;
    }

    Rc& operator=(const Rc& other) noexcept {
        decref();
        x = other.x;
        count = other.count;
        incref();
        return *this;
    }

    operator bool() const noexcept { return x != nullptr; }

    T* get() const noexcept { return x; }
    T* operator->() const noexcept {
        assert_(x);
        return x;
    }
    T& operator*() const noexcept {
        assert_(x);
        return *x;
    }

    bool operator==(const Rc& other) const noexcept {
        return x == other.x;
    }

    bool unique() const noexcept { return count && *count == 1; }

    size_t refCount() const noexcept { return count ? *count : 0; }

 private:
    inline void incref() noexcept {
        if (count) {
            ++*count;
        }
    }

    inline void decref() noexcept {
        if (count && --*count == 0) {
            delete x;
            delete count;
        }
    }
};

template<typename T>
struct CompactSharedData {
    T x;
    size_t count;
};

// Shared pointer for complete, AKA non-abstract, data types. We combine the
// pointed-to object with the count in a single struct, a CompactSharedData,
// that we allocate or destroy at once. Since we know the size of the pointed-to
// object (by definition of completeness) we can save an allocation by packing
// it in with the count.
//
// Compact shared pointers are missing a T* constructor since they need to
// allocate their own T.
template<typename T>
class CompactRc {
    CompactSharedData<T>* data = nullptr;

 public:
    CompactRc() noexcept {}
    CompactRc(CompactRc&& other) noexcept : data(other.data) {
        other.data = nullptr;
    }
    CompactRc(const CompactRc& other) noexcept : data(other.data) { incref(); }
    ~CompactRc() noexcept { decref(); }

	/*
    CompactRc& operator=(T* x) noexcept {
        decref();
        *this = CompactRc(x);
        return *this;
    }
    */

    CompactRc& operator=(CompactRc&& other) noexcept {
        decref();
        data = other.data;
        other.data = nullptr;
        return *this;
    }

    CompactRc& operator=(const CompactRc& other) noexcept {
        decref();
        data = other.data;
        incref();
        return *this;
    }

    operator bool() const noexcept { return data != nullptr; }

    T* get() const noexcept { return data->x; }
    T* operator->() const noexcept {
        assert_(data);
        return data->x;
    }
    T& operator*() const noexcept {
        assert_(data);
        return *data->x;
    }

    bool operator==(const CompactRc& other) const noexcept {
        return data == other.data;
    }

    bool unique() const noexcept { return data && data->count == 1; }

    size_t refCount() const noexcept { return data ? data->count : 0; }

 private:
    inline void incref() noexcept {
        if (data) {
            ++data->count;
        }
    }

    inline void decref() noexcept {
        if (data && --data->count == 0) {
            delete data;
        }
    }
};

#endif  // SRC_UTIL_RC_H_
