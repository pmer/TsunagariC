/*************************************
** Tsunagari Tile Engine            **
** scheduler.cpp                    **
** Copyright 2016-2019 Paul Merrill **
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

#include "util/jobs.h"

#include "os/condition-variable.h"
#include "os/mutex.h"
#include "os/thread.h"
#include "util/assert.h"
#include "util/function.h"
#include "util/int.h"
#include "util/move.h"
#include "util/vector.h"

static size_t workerLimit = 0;
static Vector<Thread> workers;
static int jobsRunning = 0;

// Empty jobs are the signal to quit.
static Vector<Function<void()>> jobs;

// Whether the destructor has been called,
static bool tearingDown = false;

// Access to workers vector, jobs deque, jobsRunning, and tearingDown.
static Mutex jobsMutex;

// Events for when a job is added.
static ConditionVariable jobAvailable;

// Events for when a job is finished.
static ConditionVariable jobsDone;

static void
work() noexcept {
    Function<void()> job;

    do {
        {
            LockGuard lock(jobsMutex);

            while (jobs.empty()) {
                jobAvailable.wait(lock);
            }

            job = move_(jobs.front());
            jobs.erase(jobs.begin());

            jobsRunning += 1;
        }

        if (job) {
            job();
        }

        {
            LockGuard lock(jobsMutex);

            jobsRunning -= 1;

            if (jobsRunning == 0 && jobs.empty()) {
                jobsDone.notifyOne();
            }
        }
    } while (job);
}

void
JobsEnqueue(Job job) noexcept {
    LockGuard lock(jobsMutex);

    assert_(!tearingDown);

    jobs.push_back(job);

    if (workerLimit == 0) {
        workerLimit = Thread::hardware_concurrency();
    }

    if (workers.size() < workerLimit) {
        workers.push_back(Thread(Function<void()>(work)));
    }

    jobAvailable.notifyOne();
}

void
JobsFlush() noexcept {
	// TODO: Don't quit the threads.

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

	// Reset.
    workers.clear();
	tearingDown = false;
}
