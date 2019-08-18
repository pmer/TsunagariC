/********************************
** Tsunagari Tile Engine       **
** pool.h                      **
** Copyright 2019 Paul Merrill **
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

#ifndef SRC_UTIL_POOL_H_
#define SRC_UTIL_POOL_H_

#include "os/c.h"
#include "util/constexpr.h"
#include "util/noexcept.h"

template<typename T>
class Pool {
 private:
    typedef int Link;

    static CONSTEXPR11 const Link END = -1;

 public:
    Pool() = default;
    ~Pool() {
        free(reinterpret_cast<char*>(storage));
    }

    // Returns an unconstructed piece of memory.
    int allocate() noexcept {
        if (nextFree == END) {
            grow();
        }
        int id = nextFree;
        nextFree = asLink(nextFree);
        return id;
    }

    // Releases memory without destructing the object within.
    void release(int i) noexcept {
        asLink(i) = nextFree;
        nextFree = i;
    }

    T& operator[](int i) noexcept {
        assert_(0 <= i && i < allocated);
        return storage[i];
    }

 private:
    Link& asLink(int i) noexcept {
        return *reinterpret_cast<Link*>(reinterpret_cast<char*>(&storage[i]));
    }

    void grow() {
        int newAllocated = allocated == 0 ? 4 : allocated * 2;

        T* newStorage = reinterpret_cast<T*>(malloc(sizeof(T) * newAllocated));
        memcpy(newStorage, storage, sizeof(T) * allocated);
        free(reinterpret_cast<char*>(storage));

        nextFree = allocated;
        storage = newStorage;

        for (int i = allocated; i < newAllocated - 1; i++) {
            asLink(i) = i + 1;
        }
        asLink(newAllocated - 1) = END;

        allocated = newAllocated;
    }

    T* storage = nullptr;
    int allocated = 0;
    Link nextFree = END;
};


#endif  // SRC_UTIL_POOL_H_
