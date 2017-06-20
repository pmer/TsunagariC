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
#include <stdlib.h>

#include <atomic>
#include <type_traits>
#include <utility>

#include "util/meta.h"

//
// Move
//   move()  same as std::move
//
template<typename T>
struct Refless {
    typedef T value;
};
template<typename T>
struct Refless<T&&> {
    typedef T value;
};
template<typename T>
struct Refless<T&> {
    typedef T value;
};

template<typename T>
inline constexpr typename Refless<T>::value&& move_(T&& x) noexcept {
    return static_cast<typename Refless<T>::value&&>(x);
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

template<typename T>
struct DefaultUniqueDestructor {
    void operator()(T* x) { delete x; }
};

// Unique pointers delete their object when destroyed.
template<typename T, typename Deleter = DefaultUniqueDestructor<T>>
class Unique {
    T* x;
    Deleter d;

 public:
    Unique() : x(nullptr) {}

    Unique(Unique&& other) noexcept : x(other.x) {
        other.x = nullptr;
    }

    Unique(T* x) : x(x) {}

    template<typename ...Args>
    explicit Unique(Args&& ...args)
            : x(new T(std::forward<Args>(args)...)) {}

    ~Unique() {
        d(x);
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

    operator bool() const noexcept { return x != nullptr; }

    T* get() const noexcept { return x; }
    T* operator->() const noexcept { assert(x); return x; }
    T& operator*() const noexcept { assert(x); return *x; }

 private:
    // Unique pointers cannot be copied.
    Unique(const Unique&) {}
    Unique& operator=(const Unique&) { return *this; }

    // Meaningless...
    bool operator==(Unique&&) { return false; }
    bool operator==(const Unique&) const { return false; }
};

// FIXME: How can we reduce memory usage? Also, can we reduce allocations so
//        none are needed for default delete?

template<typename T>
struct Deleter {
    virtual ~Deleter() {}
    virtual void operator()(T* t) = 0;
};

template<typename B>
struct DefaultDeleter : public Deleter<B> {
    ~DefaultDeleter() {}
    void operator()(B* x) { delete x; }
};

template<typename T>
struct AttemptedAbstractDeleter : public Deleter<T> {
    ~AttemptedAbstractDeleter() {}
    void operator()(T*) { abort(); }
};

template<typename T, typename Count>
struct SharedData {
    Count count;
    Unique<Deleter<T>> deleter;
    bool deleted = false;

    SharedData(Deleter<T>* deleter)
            : count(1), deleter(deleter) {
    }

    void incref() {
        ++count;
    }

    void decref() {
        --count;
    }

    void destroy(T* x) {
        (*deleter)(x);
    }
};

// Shared pointer that allows pointers to abstract types and custom deleters.
template<typename T, typename Count>
class SharedPtr {
 public:
    T* x;
    SharedData<T, Count>* data;

 public:
    // Default constructor
    SharedPtr() : x(nullptr), data(nullptr) {}

    // Move constructors
    SharedPtr(SharedPtr&& other) noexcept
            : x(other.x), data(other.data) {
        other.x = nullptr;
        other.data = nullptr;
    }

    template<typename D>
    SharedPtr(SharedPtr<D, Count>&& other, EnableIfSubclass<T, D> = Null())
            : x(other.x), data(other.data) {
        other.x = nullptr;
        other.data = nullptr;
    }

    SharedPtr(SharedPtr& other)
            : x(other.x), data(other.data) {
        incref();
    }

    template<typename D>
    SharedPtr(SharedPtr<D, Count>& other, EnableIfSubclass<T, D> = Null())
            : x(other.x), data(reinterpret_cast<SharedData<T, Count>*>(other.data)) {
        incref();
    }

    // Copy constructors
    SharedPtr(const SharedPtr& other)
            : x(other.x), data(other.data) {
        incref();
    }

    template<typename D>
    SharedPtr(const SharedPtr<D, Count>& other, EnableIfSubclass<T, D> = Null())
            : x(other.x), data(other.data) {
        incref();
    }

    // Raw-pointer constructor
    template<typename D>
    SharedPtr(D* d, EnableIfSubclass<T*, D*> = Null()) : x(d) {
        data = d ? new SharedData<T, Count>(new DefaultDeleter<T>())
                 : nullptr;
    }

    // FIXME: Add T* t constructor that does not store pointer to deleter.

    /* WAIT UNTIL MORE STABLE
    // Emplace constructor
    // FIXME: Optimize SharedData with a custom impl that does not have a deleter. Just like the stdlib does.
    template<typename ...Args>
    explicit SharedPtr(Args&& ...args)
            : x(new T(std::forward<Args>(args)...)),
              data(new SharedData<T, Count>{1, new DefaultDeleter<T, T>()}) {}
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
        data = other.data;
        other.x = nullptr;
        other.data = nullptr;
        return *this;
    }

    SharedPtr& operator=(const SharedPtr& other) noexcept {
        x = other.x;
        data = other.data;
        incref();
        return *this;
    }

    operator bool() const noexcept { return x != nullptr; }

    T* get() const noexcept { return x; }
    T* operator->() const noexcept { assert(x); return x; }
    T& operator*() const noexcept { assert(x); return *x; }

    bool operator==(const SharedPtr& other) const noexcept {
        return x == other.x;
    }

    bool unique() const noexcept {
        return data && data->count == 1;
    }

    size_t refCount() const noexcept {
        return data ? data->count : 0;
    }

 private:
    // Meaningless...
    bool operator==(SharedPtr&&) { return false; }

    inline void incref() {
        if (data) {
            data->incref();
        }
    }

    inline void decref() {
        if (data) {
            data->decref();
            if (data->count == 0) {
                data->destroy(x);
                delete data;
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

    template<typename ...Args>
    explicit CompactSharedData(Args&& ...args)
            : x(new T(std::forward<Args>(args)...)),
              count{1} {}
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

    template<typename ...Args>
    explicit CompactSharedPtr(Args&& ...args)
            : data(std::forward<Args>(args)...) {}

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
    T* operator->() const noexcept { assert(data); return data->x; }
    T& operator*() const noexcept { assert(data); return *data->x; }

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

// Wrappers to increase clarity of error messages.
struct NonAtomic {
    size_t x;
    NonAtomic(size_t x) : x(x) {}
    NonAtomic& operator++() { ++x; return *this; }
    NonAtomic& operator--() { --x; return *this; }
    bool operator==(size_t x) { return this->x == x; }
    size_t get() { return x; }
};

struct Atomic {
    std::atomic<size_t> x;
    Atomic(size_t x) : x(x) {}
    Atomic& operator++() { ++x; return *this; }
    Atomic& operator--() { --x; return *this; }
    bool operator==(size_t x) { return this->x == x; }
    size_t get() { return x; }
};

// Shared pointers delete their pointer only when the last pointer to the same
// object is destroyed.
template<typename T>
using Rc = SharedPtr<T, NonAtomic>;

template<typename T>
using Arc = SharedPtr<T, Atomic>;

template<typename T>
using CompactRc = CompactSharedPtr<T, NonAtomic>;

template<typename T>
using CompactArc = CompactSharedPtr<T, Atomic>;

#endif  // SRC_CORE_MEMORY_H_
