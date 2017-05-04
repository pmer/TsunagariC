/********************************
** Tsunagari Tile Engine       **
** pool.cpp                    **
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

#include "pack/pool.h"
#include "util/optional.h"

#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>

class PoolImpl : public Pool {
 public:
    ~PoolImpl();

    void schedule(std::function<void()> job);

    size_t workerLimit;
    std::vector<std::thread> workers;

    // Empty jobs are the signal to quit.
    std::deque<std::function<void()>> jobs;

    std::condition_variable available;
    std::mutex mutex;
};

Pool& Pool::makePool(size_t workerLimit) {
    PoolImpl* pool = new PoolImpl;
    pool->workerLimit = workerLimit;
    return *pool;
}

static void runJobs(PoolImpl* pool) {
    std::function<void()> job;

    do {
        {
            std::unique_lock<std::mutex> lock(pool->mutex);
            if (pool->jobs.empty()) {
                pool->available.wait(lock);
            }

            job = std::move(pool->jobs.front());
            pool->jobs.pop_front();
        }

        if (job) {
            job();
        }
    } while (job);
}

static void startWorker(PoolImpl* pool) {
    if (pool->workers.size() < pool->workerLimit) {
        if (pool->workers.size() < std::thread::hardware_concurrency()) {
            pool->workers.push_back(std::thread(runJobs, pool));
        }
    }
}

void PoolImpl::schedule(std::function<void()> job) {
    startWorker(this);

    {
        std::unique_lock<std::mutex> lock(mutex);
        jobs.push_back(job);
    }
    available.notify_one();
}

PoolImpl::~PoolImpl() {
    {
        std::unique_lock<std::mutex> lock(mutex);
        for (size_t i = 0; i < workers.size(); i++) {
            // Send over empty jobs.
            jobs.push_back(std::function<void()>());
        }
    }

    available.notify_all();

    for (auto& worker : workers) {
        worker.join();
    }
}
