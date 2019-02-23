/********************************
** Tsunagari Tile Engine       **
** dispatch-queue-impl.cpp     **
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

#include "util/dispatch-queue-impl.h"
#include "util/move.h"

bool operator<(const TaskContext& lhs, const TaskContext& rhs) {
    return lhs.qualityOfService < rhs.qualityOfService;
}

DispatchQueueImpl::DispatchQueueImpl() {
    unsigned n = std::thread::hardware_concurrency();
    //n = 1;  // debug, and to help with MusicWorker until better mechanism for series-of-tasks implemented
    for (unsigned i = 0; i < n; i++) {
        std::thread thread(&DispatchQueueImpl::runTasks, this);
        threads.emplace_back(move_(thread));
    }
}

DispatchQueueImpl::~DispatchQueueImpl() {
    tasks.end();
    for (auto& thread : threads) {
        thread.join();
    }
}

void DispatchQueueImpl::async(Task task, DispatchQueue::QualityOfService qos) {
     tasks.push({task, qos});
}

void DispatchQueueImpl::runTasks() {
    while (true) {
        Optional<TaskContext> context = tasks.pop();
        if (context) {
            context->task();
        } else {
            return;
        }
    }
}
