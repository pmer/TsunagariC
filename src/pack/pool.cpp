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

#include "os/condition-variable.h"
#include "os/mutex.h"
#include "os/thread.h"
#include "util/function.h"
#include "util/move.h"
#include "util/string.h"
#include "util/vector.h"

class PoolImpl : public Pool {
 public:
    ~PoolImpl() final;

    void schedule(Function<void()> job) final;

    String name;

    size_t workerLimit;

    size_t numWorkers;
    vector<Thread> workers;

    // Empty jobs are the signal to quit.
    vector<Function<void()>> jobs;

    int jobsRunning;

    // Whether the destructor has been called,
    bool tearingDown;

    // Access to workers vector, jobs deque, jobsRunning, and tearingDown.
    Mutex jobsMutex;

    // Events for when a job is added.
    ConditionVariable jobAvailable;

    // Events for when a job is finished.
    ConditionVariable jobsDone;
};

Pool*
Pool::makePool(StringView name, size_t workerLimit) {
    auto pool = new PoolImpl;
    pool->name = name;
    pool->workerLimit =
            workerLimit > 0 ? workerLimit : Thread::hardware_concurrency();
    pool->numWorkers = 0;
    pool->jobsRunning = 0;
    pool->tearingDown = false;
    return pool;
}

static void
jobWorker(PoolImpl* pool) {
    Function<void()> job;

    do {
        {
            LockGuard lock(pool->jobsMutex);

            while (pool->jobs.empty()) {
                pool->jobAvailable.wait(lock);
            }

            job = move_(pool->jobs.front());
            pool->jobs.erase(pool->jobs.begin());

            pool->jobsRunning += 1;
        }

        if (job) {
            job();
        }

        {
            LockGuard lock(pool->jobsMutex);

            pool->jobsRunning -= 1;

            if (pool->jobsRunning == 0 && pool->jobs.empty()) {
                pool->jobsDone.notifyOne();
            }
        }
    } while (job);
}

void
PoolImpl::schedule(Function<void()> job) {
    {
        LockGuard lock(jobsMutex);

        assert_(!tearingDown);

        jobs.push_back(job);

        if (numWorkers < workerLimit) {
            if (numWorkers < workerLimit) {
                workers.push_back(Thread(
                        Function<void()>([this]() { jobWorker(this); })));
            }
        }

        jobAvailable.notifyOne();
    }
}

PoolImpl::~PoolImpl() {
    // Wait for all jobs to finish.
    {
        Mutex m;
        LockGuard lock(m);

        while (jobsRunning > 0 || jobs.size() > 0) {
            jobsDone.wait(lock);
        }
    }

    // Tell workers they can quit.
    {
        LockGuard lock(jobsMutex);

        for (size_t i = 0; i < workers.size(); i++) {
            // Send over empty jobs.
            jobs.emplace_back();
        }

        tearingDown = true;
    }

    jobAvailable.notifyAll();

    // Wait for workers to quit.
    for (auto& worker : workers) {
        worker.join();
    }
}
