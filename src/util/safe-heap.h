/********************************
** Tsunagari Tile Engine       **
** safe-heap.h                 **
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

#ifndef SRC_UTIL_SAFE_HEAP_H_
#define SRC_UTIL_SAFE_HEAP_H_

#include <condition_variable>
#include <mutex>
#include <queue>

#include "util/optional.h"

template <class T>
class SafeHeap {
 public:
    SafeHeap()
        : q()
        , m()
        , c()
        , alive(true) {}

     void push(T t) {
         std::lock_guard<std::mutex> lock(m);
         q.push(t);
         c.notify_one();
     }

     Optional<T> pop() {
         std::unique_lock<std::mutex> lock(m);
         while (alive && q.empty()) {
                 c.wait(lock);
         }
         if (!q.empty()) {
             T val = q.top();
             q.pop();
             return Optional<T>(val);
         } else {
             return Optional<T>();
         }
     }

     void end() {
        alive = false;
        c.notify_all();
     }

 private:
     std::priority_queue<T> q;
     std::mutex m;
     std::condition_variable c;
     bool alive;
};

#endif  // SRC_UTIL_SAFE_HEAP_H_
