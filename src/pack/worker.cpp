/********************************
** Tsunagari Tile Engine       **
** worker.cpp                  **
** Copyright 2017 Paul Merrill **
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

#include "pack/worker.h"

#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>

static std::thread* t = nullptr;
static std::deque<std::function<void()>> jobs;

static std::condition_variable cv;
static std::mutex mutex;
static bool wantQuit = false;

static void runJobs() {
    bool anotherJob = true;

    while (anotherJob) {
        std::function<void()> job;

        {
            std::unique_lock<std::mutex> lock(mutex);
            if (jobs.empty()) {
                cv.wait(lock);
            }

            job = std::move(jobs.front());
            jobs.pop_front();

            if (jobs.empty() && wantQuit) {
                anotherJob = false;
            }
        }

        job();
    }
}

void scheduleJob(std::function<void()> job) {
    if (t == nullptr) {
        t = new std::thread(runJobs);
    }

    {
        std::unique_lock<std::mutex> lock(mutex);
        jobs.push_back(job);
    }
    cv.notify_one();
}

void finishJobs() {
    if (t) {
        {
            std::unique_lock<std::mutex> lock(mutex);
            jobs.push_back([]{});
            wantQuit = true;
        }
        cv.notify_one();
        t->join();
    }
}
