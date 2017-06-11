/**********************************
** Tsunagari Tile Engine         **
** memory.h                      **
** Copyright 2017 Paul Merrill   **
**********************************/

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

#ifndef SRC_CORE_MEMORY_H_
#define SRC_CORE_MEMORY_H_

#include <assert.h>
#include <stddef.h>

#include <atomic>

#include "util/meta.h"

//
// Move
//   move()  same as std::move
//
template<typename T>
struct Refless {
    typedef T type;
};
template<typename T>
struct Refless<T&&> {
    typedef T type;
};
template<typename T>
struct Refless<T&> {
    typedef T type;
};

template<typename T>
inline constexpr typename Refless<T>::type&& move_(T&& x) noexcept {
    return static_cast<typename Refless<T>::type&&>(x);
}


//
// Pointers
//   class Unique      Unique pointer
//   class Rc          "Reference counter" shared pointer
//   class Arc         "Atomic reference counter" thread-safe shared pointer
//   class CompactRc   Fast shared pointer (does not handle abstract classes or
//                                          custom deleters)
//   class CompactArc  Fast thread-safe shared pointer
//

// Unique pointers delete their object when destroyed.
template<typename T>
class Unique {
    T* x = nullptr;

 public:
    Unique() {}

    Unique(T* x) : x(x) {}

    template<typename ...Args>
    explicit Unique(Args&& ...args)
            : x(new T(std::forward<Args>(args)...)) {}

    Unique(Unique&& other) : x(other.x) {
        other.x = nullptr;
    }

    ~Unique() {
        delete x;
    }

    Unique& operator=(T* x) {
        delete this->x;
        this->x = x;
        return *this;
    }

    Unique& operator=(Unique&& other) {
        x = other.x;
        other.x = nullptr;
        return *this;
    }

    operator bool() const { return x; }

    const T* get() const { return x; }
    const T* operator->() const { assert(x); return x; }
    const T& operator*() const { assert(x); return *x; }

    T* get() { return x; }
    T* operator->() { assert(x); return x; }
    T& operator*() { assert(x); return *x; }

 private:
    // Unique pointers cannot be copied.
    Unique(const Unique&) {}
    Unique& operator=(const Unique&) { return *this; }

    // Meaningless...
    bool operator==(Unique&&) { return false; }
    bool operator==(const Unique&) const { return false; }
};

template<typename T>
struct DefaultDelete {
    void operator()(T* x) { delete x; }
};

// Shared pointer that allows pointers to abstract types and custom deleters.
template<typename T, typename Count, typename Delete>
class SharedPtr {
    T* x = nullptr;
    Count* count = nullptr;
    Delete deleter = Delete();

 public:
    SharedPtr() {}

    SharedPtr(T* x) : x(x) {
        count = x ? new Count(1) : nullptr;
    }

    template<typename Subclass>
    SharedPtr(Subclass* x, EnableIf<IsBaseOf<T, Subclass>::value> = Null())
            : x(x) {
        count = x ? new Count(1) : nullptr;
    }

    template<typename ...Args>
    explicit SharedPtr(Args&& ...args)
            : x(new T(std::forward<Args>(args)...)),
              count(new Count(1)) {}

    SharedPtr(SharedPtr&& other) : x(other.x), count(other.count) {
        other.x = nullptr;
        other.count = nullptr;
    }

    SharedPtr(const SharedPtr& other) : x(other.x), count(other.count) {
        incref();
    }

    ~SharedPtr() {
        decref();
    }

    SharedPtr& operator=(T* x) {
        decref();
        *this = SharedPtr(x);
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) {
        x = other.x;
        count = other.count;
        other.x = nullptr;
        other.count = nullptr;
        return *this;
    }

    SharedPtr& operator=(const SharedPtr& other) {
        x = other.x;
        count = other.count;
        incref();
        return *this;
    }

    operator bool() const { return x; }

    const T* get() const { return x; }
    const T* operator->() const { assert(x); return x; }
    const T& operator*() const { assert(x); return *x; }

    T* get() { return x; }
    T* operator->() { assert(x); return x; }
    T& operator*() { assert(x); return *x; }

    bool operator==(const SharedPtr& other) const {
        return x == other.x;
    }

    bool unique() const {
        return count && *count == 1;
    }

 private:
    // Meaningless...
    bool operator==(SharedPtr&&) { return false; }

    inline void incref() {
        if (count) {
            *count += 1;
        }
    }

    inline void decref() {
        if (count && --*count == 0) {
            deleter(x);
            delete count;
            //x = nullptr;
            //count = nullptr;
        }
    }
};

// SharedData is a compact pair of some object T and a reference count of type
// Count. Count is either an atomic or non-atomic size_t for Arc and Rc,
// respectively.
template<typename T, typename Count>
struct SharedData {
    T x;
    Count count;

    template<typename ...Args>
    explicit SharedData(Args&& ...args)
            : x(new T(std::forward<Args>(args)...)),
              count(1) {}
};

// Shared pointer for complete, AKA non-abstract, data types. We combine the
// pointed-to object with the count in a single struct, a SharedData, that we
// allocate or destroy at once. Since we know the size of the pointed-to object
// (by definition of completeness) we can save an allocation by packing it in
// with the count.
//
// Compact shared pointers are missing a T* constructor since they need to
// allocate their own T.
template<typename T, typename Count>
class CompactSharedPtr {
    SharedData<T, Count>* data = nullptr;

 public:
    CompactSharedPtr() {}

    template<typename ...Args>
    explicit CompactSharedPtr(Args&& ...args)
            : data(std::forward<Args>(args)...) {}

    CompactSharedPtr(CompactSharedPtr&& other) : data(other.data) {
        other.data = nullptr;
    }

    CompactSharedPtr(const CompactSharedPtr& other) : data(other.data) {
        incref();
    }

    ~CompactSharedPtr() {
        decref();
    }

    CompactSharedPtr& operator=(T* x) {
        decref();
        *this = CompactSharedPtr(x);
        return *this;
    }

    CompactSharedPtr& operator=(CompactSharedPtr&& other) {
        data = other.data;
        other.data = nullptr;
        return *this;
    }

    CompactSharedPtr& operator=(const CompactSharedPtr& other) {
        data = other.data;
        incref();
        return *this;
    }

    operator bool() const { return data; }

    const T* get() const { return data->x; }
    const T* operator->() const { assert(data); return data->x; }
    const T& operator*() const { assert(data); return *data->x; }

    T* get() { return data->x; }
    T* operator->() { assert(data); return data->x; }
    T& operator*() { assert(data); return *data->x; }

    bool operator==(const CompactSharedPtr& other) const {
        return data == other.data;
    }

    bool unique() const {
        return data && data->count == 1;
    }

 private:
    // Meaningless...
    bool operator==(CompactSharedPtr&&) { return false; }

    inline void incref() {
        if (data) {
            data->count++;
        }
    }

    inline void decref() {
        if (data && --data->count == 0) {
            delete data;
            //data = nullptr;
        }
    }
};

// Shared pointers delete their pointer only when the last pointer to the same
// object is destroyed.
template<typename T, typename Delete = DefaultDelete<T>>
using Rc = SharedPtr<T, size_t, Delete>;

template<typename T, typename Delete = DefaultDelete<T>>
using Arc = SharedPtr<T, std::atomic<size_t>, Delete>;

template<typename T>
using CompactRc = CompactSharedPtr<T, size_t>;

template<typename T>
using CompactArc = CompactSharedPtr<T, std::atomic<size_t>>;

#endif  // SRC_CORE_MEMORY_H_
