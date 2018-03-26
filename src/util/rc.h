/***************************************
** Tsunagari Tile Engine              **
** rc.h                               **
** Copyright 2017-2018 Paul Merrill   **
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

#ifndef SRC_UTIL_RC_H_
#define SRC_UTIL_RC_H_

#include <stddef.h>
#include <stdlib.h>

#include "util/assert.h"
#include "util/meta.h"
#include "util/unique.h"

//
// Pointers
//   unique.h
//     class Unique      Unique pointer
//   rc.h
//     class Rc          "Reference counter" shared pointer
//     class CompactRc   Fast shared pointer (does not handle abstract classes
//                                            or custom deleters)
//   arc.h
//     class Arc         "Atomic reference counter" thread-safe shared pointer
//     class CompactArc  Fast thread-safe shared pointer
//

// FIXME: How can we reduce memory usage? Also, can we reduce allocations so
//        none are needed for default delete?

// Shared pointer that allows pointers to abstract types and custom deleters.
template<typename T, typename Count>
class SharedPtr {
 public:
    T* x;
    Count* count;

 public:
    // Default constructor
    SharedPtr() : x(nullptr), count(nullptr) {}

    // Move constructors
    SharedPtr(SharedPtr&& other) noexcept
            : x(other.x), count(other.count) {
        other.x = nullptr;
        other.count = nullptr;
    }

    template<typename D>
    SharedPtr(SharedPtr<D, Count>&& other, EnableIfSubclass<T, D> = Null())
            : x(other.x), count(other.count) {
        other.x = nullptr;
        other.count = nullptr;
    }

    SharedPtr(SharedPtr& other)
            : x(other.x), count(other.count) {
        incref();
    }

    template<typename D>
    SharedPtr(SharedPtr<D, Count>& other, EnableIfSubclass<T, D> = Null())
            : x(other.x), count(other.count) {
        incref();
    }

    // Copy constructors
    SharedPtr(const SharedPtr& other)
            : x(other.x), count(other.count) {
        incref();
    }

    template<typename D>
    SharedPtr(const SharedPtr<D, Count>& other, EnableIfSubclass<T, D> = Null())
            : x(other.x), count(other.count) {
        incref();
    }

    // Raw-pointer constructor
    template<typename D>
    SharedPtr(D* d, EnableIfSubclass<T*, D*> = Null()) : x(d) {
        count = d ? new Count(1)
                  : nullptr;
    }

    // FIXME: Add T* t constructor that does not store pointer to deleter.

    /* WAIT UNTIL MORE STABLE
    // Emplace constructor
    // FIXME: Optimize SharedData with a custom impl that does not have a deleter. Just like the stdlib does.
    template<typename ...Args>
    explicit SharedPtr(Args&& ...args)
            : x(new T(std::forward<Args>(args)...)),
              count(1) {}
    */

    ~SharedPtr() {
        decref();
    }

    SharedPtr& operator=(T* x) noexcept {
        decref();
        *this = SharedPtr(x);
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) noexcept {
        x = other.x;
        count = other.count;
        other.x = nullptr;
        other.count = nullptr;
        return *this;
    }

    SharedPtr& operator=(const SharedPtr& other) noexcept {
        x = other.x;
        count = other.count;
        incref();
        return *this;
    }

    operator bool() const noexcept { return x != nullptr; }

    T* get() const noexcept { return x; }
    T* operator->() const noexcept { assert_(x); return x; }
    T& operator*() const noexcept { assert_(x); return *x; }

    bool operator==(const SharedPtr& other) const noexcept {
        return x == other.x;
    }

    bool unique() const noexcept {
        return count && *count == 1;
    }

    size_t refCount() const noexcept {
        return count ? *count : 0;
    }

 private:
    // Meaningless...
    bool operator==(SharedPtr&&) { return false; }

    inline void incref() {
        if (count) {
            ++*count;
        }
    }

    inline void decref() {
        if (count) {
            --*count;
            if (*count == 0) {
                delete x;
                delete count;
            }
        }
    }
};

// SharedData is a compact pair of some object T and a reference count of type
// Count. Count is either an atomic or non-atomic size_t for Arc and Rc,
// respectively.
template<typename T, typename Count>
struct CompactSharedData {
    T x;
    Count count;

    /*
    template<typename ...Args>
    explicit CompactSharedData(Args&& ...args)
            : x(new T(std::forward<Args>(args)...)),
              count{1} {}
    */
};

// Shared pointer for complete, AKA non-abstract, data types. We combine the
// pointed-to object with the count in a single struct, a CompactSharedData,
// that we allocate or destroy at once. Since we know the size of the pointed-to
// object (by definition of completeness) we can save an allocation by packing
// it in with the count.
//
// Compact shared pointers are missing a T* constructor since they need to
// allocate their own T.
template<typename T, typename Count>
class CompactSharedPtr {
    CompactSharedData<T, Count>* data;

 public:
    CompactSharedPtr() : data(nullptr) {}

    CompactSharedPtr(CompactSharedPtr&& other) noexcept : data(other.data) {
        other.data = nullptr;
    }

    CompactSharedPtr(CompactSharedPtr& other) : data(other.data) {
        incref();
    }

    CompactSharedPtr(const CompactSharedPtr& other) : data(other.data) {
        incref();
    }

    /*
    template<typename ...Args>
    explicit CompactSharedPtr(Args&& ...args)
            : data(std::forward<Args>(args)...) {}
    */

    ~CompactSharedPtr() {
        decref();
    }

    CompactSharedPtr& operator=(T* x) noexcept {
        decref();
        *this = CompactSharedPtr(x);
        return *this;
    }

    CompactSharedPtr& operator=(CompactSharedPtr&& other) noexcept {
        data = other.data;
        other.data = nullptr;
        return *this;
    }

    CompactSharedPtr& operator=(const CompactSharedPtr& other) noexcept {
        data = other.data;
        incref();
        return *this;
    }

    operator bool() const { return data != nullptr; }

    T* get() const noexcept { return data->x; }
    T* operator->() const noexcept { assert_(data); return data->x; }
    T& operator*() const noexcept { assert_(data); return *data->x; }

    bool operator==(const CompactSharedPtr& other) const noexcept {
        return data == other.data;
    }

    bool unique() const noexcept {
        return data && data->count == 1;
    }

    size_t refCount() const noexcept {
        return data ? data->count : 0;
    }

 private:
    // Meaningless...
    bool operator==(CompactSharedPtr&&) { return false; }

    inline void incref() noexcept {
        if (data) {
            ++data->count;
        }
    }

    inline void decref() noexcept {
        if (data && --data->count == 0) {
            delete data;
            //data = nullptr;
        }
    }
};

// Wrapper to increase clarity of error messages.
struct NonAtomic {
    size_t x;
    NonAtomic(size_t x) : x(x) {}
    NonAtomic& operator++() { ++x; return *this; }
    NonAtomic& operator--() { --x; return *this; }
    bool operator==(size_t x) { return this->x == x; }
    size_t get() { return x; }
};

// Shared pointers delete their pointer only when the last pointer to the same
// object is destroyed.
template<typename T>
using Rc = SharedPtr<T, NonAtomic>;

template<typename T>
using CompactRc = CompactSharedPtr<T, NonAtomic>;

// Allow use in std::unordered_map and std::unordered_set.
namespace std {
    template<typename T, typename AtomicClass>
    struct hash<SharedPtr<T, AtomicClass>> {
        size_t operator()(const SharedPtr<T, AtomicClass>& p) {
            return hash<T*>()(p.get());
        }
    };
}

#endif  // SRC_UTIL_RC_H_
