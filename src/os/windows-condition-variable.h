/**********************************
** Tsunagari Tile Engine         **
** condition-variable.h          **
** Copyright 2019 Paul Merrill   **
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

#ifndef SRC_OS_CONDITION_VARIBLE_H_
#define SRC_OS_CONDITION_VARIBLE_H_

#ifdef _WIN32
#include "os/windows-condition-variable.h"
#else
#include "os/unix-condition-variable.h"
#endif

class condition_variable
{
public:
    condition_variable();
    ~condition_variable();
    
    condition_variable(const condition_variable&) = delete;
    condition_variable& operator=(const condition_variable&) = delete;
    
    void notify_one() noexcept;
    void notify_all() noexcept;
    
    void wait(unique_lock<mutex>& lock);
    template <class Predicate>
    void wait(unique_lock<mutex>& lock, Predicate pred);
    
    template <class Clock, class Duration>
    cv_status
    wait_until(unique_lock<mutex>& lock,
               const chrono::time_point<Clock, Duration>& abs_time);
    
    template <class Clock, class Duration, class Predicate>
    bool
    wait_until(unique_lock<mutex>& lock,
               const chrono::time_point<Clock, Duration>& abs_time,
               Predicate pred);
    
    template <class Rep, class Period>
    cv_status
    wait_for(unique_lock<mutex>& lock,
             const chrono::duration<Rep, Period>& rel_time);
    
    template <class Rep, class Period, class Predicate>
    bool
    wait_for(unique_lock<mutex>& lock,
             const chrono::duration<Rep, Period>& rel_time,
             Predicate pred);
    
    typedef pthread_cond_t* native_handle_type;
    native_handle_type native_handle();
};

#endif  // SRC_OS_CONDITION_VARIBLE_H_
