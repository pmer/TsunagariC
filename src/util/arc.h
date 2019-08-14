/*************************************
** Tsunagari Tile Engine            **
** arc.h                            **
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

#ifndef SRC_UTIL_ARC_H_
#define SRC_UTIL_ARC_H_

#include <atomic>

#include "util/rc.h"

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

// Wrapper to increase clarity of error messages.
struct Atomic {
    std::atomic<size_t> x;
    Atomic(size_t x) noexcept : x(x) {}
    Atomic& operator++() noexcept {
        ++x;
        return *this;
    }
    Atomic& operator--() noexcept {
        --x;
        return *this;
    }
    bool operator==(size_t x) noexcept { return this->x == x; }
    size_t get() noexcept { return x; }
};

// FIXME: The symbol Arc already exists in Windows. Rename?
// template<typename T>
// using Arc = SharedPtr<T, Atomic>;

template<typename T> using CompactArc = CompactSharedPtr<T, Atomic>;

#endif  // SRC_UTIL_ARC_H_
