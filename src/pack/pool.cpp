/*************************************
** Tsunagari Tile Engine            **
** pool.cpp                         **
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

#include "pack/pool.h"

#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>

#include "util/move.h"
#include "util/string.h"
#include "util/vector.h"

class PoolImpl : public Pool {
 public:
    ~PoolImpl() final;

    void schedule(std::function<void()> job) final;

    String name;

    size_t workerLimit;
    vector<std::thread> workers;
    int jobsRunning;

    // Empty jobs are the signal to quit.
    std::deque<std::function<void()>> jobs;

 public:
    // Protects access to all above variables and to jobsChanged.
    std::mutex mainMutex;

    // Protects access to finished.
    std::mutex finishedMutex;

    // Signaled when a job is added.
    std::condition_variable jobsChanged;

    // Signaled when all jobs are finished.
    std::condition_variable finished;
};

Pool* Pool::makePool(String name, size_t workerLimit) {
    auto pool = new PoolImpl;
    pool->name = move_(name);
    pool->workerLimit = workerLimit > 0 ? workerLimit
                                        : std::thread::hardware_concurrency();
    pool->jobsRunning = 0;
    return pool;
}

static void runJobs(PoolImpl* pool) {
    std::function<void()> job;

    do {
        {
            std::unique_lock<std::mutex> lock(pool->mainMutex);

            pool->jobsChanged.wait(lock, [&]{ return !pool->jobs.empty(); });

            job = move_(pool->jobs.front());
            pool->jobs.pop_front();

            pool->jobsRunning += 1;
        }

        if (job) {
            job();
        }

        {
            std::unique_lock<std::mutex> lock(pool->mainMutex);

            pool->jobsRunning -= 1;

            if (pool->jobsRunning == 0 && pool->jobs.empty()) {
                // Notify the PoolImpl destructor.
                pool->finished.notify_one();
            }
        }

    } while (job);
}

static void startWorker(PoolImpl* pool) {
    if (pool->workers.size() < pool->workerLimit) {
        pool->workers.push_back(std::thread(runJobs, pool));
    }
}

void PoolImpl::schedule(std::function<void()> job) {
    {
        std::lock_guard<std::mutex> lock(mainMutex);
        startWorker(this);
        jobs.push_back(job);
    }
    jobsChanged.notify_one();
}

PoolImpl::~PoolImpl() {
    {
        std::unique_lock<std::mutex> lock(finishedMutex);

        finished.wait(lock, [&]{ return jobsRunning == 0 && jobs.empty(); });

        std::unique_lock<std::mutex> lock2(mainMutex);

        for (size_t i = 0; i < workers.size(); i++) {
            // Send over empty jobs.
            jobs.emplace_back();
        }
    }

    jobsChanged.notify_all();

    for (auto& worker : workers) {
        worker.join();
    }
}
