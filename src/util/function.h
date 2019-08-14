/********************************
** Tsunagari Tile Engine       **
** function.h                  **
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

#ifndef SRC_UTIL_FUNCTION_H_
#define SRC_UTIL_FUNCTION_H_

#include "util/align.h"
#include "util/assert.h"
#include "util/meta.h"
#include "util/move.h"
#include "util/noexcept.h"

#pragma warning(push)
#pragma warning(disable: 26495)  // Always initialize a member variable.

template<class F>
class Function;  // Undefined.

namespace function {
    template<class R>
	class base;

    template<class R, class... ArgTypes>
	class base<R(ArgTypes...)> {
        base(const base&) noexcept;
        base& operator=(const base&) noexcept;

     public:
        inline base() noexcept {}
        inline virtual ~base() noexcept {}
        virtual base* clone() const noexcept = 0;
        virtual void clone(base*) const noexcept = 0;
        virtual void destroy() noexcept = 0;
        virtual void destroyDeallocate() noexcept = 0;
        virtual R operator()(ArgTypes&&...) noexcept = 0;
    };

    template<class F, class R>
	class func;

    template<class F, class R, class... ArgTypes>
    class func<F, R(ArgTypes...)> : public base<R(ArgTypes...)> {
        F f;

     public:
        inline explicit func(F&& f) noexcept : f(move_(f)) {}
        inline explicit func(const F& f) noexcept : f(f) {}

        virtual base<R(ArgTypes...)>* clone() const noexcept;
        virtual void clone(base<R(ArgTypes...)>*) const noexcept;
        virtual void destroy() noexcept;
        virtual void destroyDeallocate() noexcept;
        virtual R operator()(ArgTypes&&... args) noexcept;
    };

    template<class F, class R, class... ArgTypes>
    base<R(ArgTypes...)>* func<F, R(ArgTypes...)>::clone() const noexcept {
        return new func(f);
    }

    template<class F, class R, class... ArgTypes>
    void func<F, R(ArgTypes...)>::clone(base<R(ArgTypes...)>* p) const noexcept {
        new (p) func(f);
    }

    template<class F, class R, class... ArgTypes>
    void func<F, R(ArgTypes...)>::destroy() noexcept {
        f.~F();
    }

    template<class F, class R, class... ArgTypes>
    void func<F, R(ArgTypes...)>::destroyDeallocate() noexcept {
        delete this;
    }

    template<class Class, class Ret, class This, class... Args>
    decltype(auto) invoke(Ret Class::*f, This&& t, Args&&... args) noexcept {
        return (forward_<This>(t).*f)(forward_<Args>(args)...);
    }

    template<class F, class... Args>
    decltype(auto) invoke(F&& f, Args&&... args) {
        return forward_<F>(f)(forward_<Args>(args)...);
    }

    template<class F, class R, class... ArgTypes>
    R func<F, R(ArgTypes...)>::operator()(ArgTypes&&... args) noexcept {
        return invoke(f, forward_<ArgTypes>(args)...);
    }
}  // namespace function

template<class R, class... ArgTypes>
class Function<R(ArgTypes...)> {
 private:
    typedef function::base<R(ArgTypes...)> base;

    static base* asBase(void* p) noexcept { return reinterpret_cast<base*>(p); }

    Align<void * [3]> buf;
    base* f;

 public:
    inline Function() noexcept : f(nullptr) {}
    Function(const Function&) noexcept;
    Function(Function&&) noexcept;
    template<class F>
    Function(F) noexcept;
    ~Function() noexcept;

    template<class F>
    void set(F& something,
             EnableIf<sizeof(function::func<F, R(ArgTypes...)>) <= sizeof(buf)> = True());
    template<class F>
    void set(F& something,
             EnableIf<!(sizeof(function::func<F, R(ArgTypes...)>) <= sizeof(buf))> = True());

    Function& operator=(const Function&) noexcept;
    Function& operator=(Function&&) noexcept;
    template<class F>
    Function& operator=(F&&) noexcept;

    void swap(Function&) noexcept;

    inline explicit operator bool() const noexcept { return f != nullptr; }

    R operator()(ArgTypes...) const noexcept;
};

template<class R, class... ArgTypes>
Function<R(ArgTypes...)>::Function(const Function& other) noexcept {
    if (other.f == nullptr) {
        f = nullptr;
    }
    else if ((void*)other.f == &other.buf) {
        f = asBase(&buf);
        other.f->clone(f);
    }
    else {
        f = other.f->clone();
    }
}

template<class R, class... ArgTypes>
Function<R(ArgTypes...)>::Function(Function&& other) noexcept {
    if (other.f == nullptr) {
        f = nullptr;
    }
    else if ((void*)other.f == &other.buf) {
        f = asBase(&buf);
        other.f->clone(f);
    }
    else {
        f = other.f;
        other.f = nullptr;
    }
}

template<class R, class... ArgTypes>
template<class F>
Function<R(ArgTypes...)>::Function(F something) noexcept : f(nullptr) {
    set(something);
}

template<class R, class... ArgTypes>
template<class F>
void Function<R(ArgTypes...)>::set(F& something, EnableIf<sizeof(function::func<F, R(ArgTypes...)>) <= sizeof(buf)>) {
    f = new ((void*)&buf) function::func<F, R(ArgTypes...)>(move_(something));
}

template<class R, class... ArgTypes>
template<class F>
void Function<R(ArgTypes...)>::set(F& something, EnableIf<!(sizeof(function::func<F, R(ArgTypes...)>) <= sizeof(buf))>) {
    f = new function::func<F, R(ArgTypes...)>(move_(something));
}

template<class R, class... ArgTypes>
Function<R(ArgTypes...)>&
Function<R(ArgTypes...)>::operator=(const Function& other) noexcept {
    Function(other).swap(*this);
    return *this;
}

template<class R, class... ArgTypes>
Function<R(ArgTypes...)>&
Function<R(ArgTypes...)>::operator=(Function&& other) noexcept {
    this->~Function();
    if (other.f == nullptr) {
        f = nullptr;
    }
    else if ((void*)other.f == &other.buf) {
        f = asBase(&buf);
        other.f->clone(f);
    }
    else {
        f = other.f;
        other.f = nullptr;
    }
    return *this;
}

template<class R, class... ArgTypes>
template<class F>
Function<R(ArgTypes...)>&
Function<R(ArgTypes...)>::operator=(F&& other) noexcept {
    Function(forward_<F>(other)).swap(*this);
    return *this;
}

template<class R, class... ArgTypes>
Function<R(ArgTypes...)>::~Function() noexcept {
    if ((void*)f == &buf) {
        f->destroy();
    }
    else if (f) {
        f->destroyDeallocate();
    }
}

template<class R, class... ArgTypes>
void
Function<R(ArgTypes...)>::swap(Function& other) noexcept {
    if (&other == this) {
        return;
    }

    if ((void*)f == &buf && (void*)other.f == &other.buf) {
        alignas(alignof(void * [3])) char tempbuf[3 * sizeof(void*)];
        base* t = asBase(&tempbuf);
        f->clone(t);
        f->destroy();
        f = nullptr;
        other.f->clone(asBase(&buf));
        other.f->destroy();
        other.f = nullptr;
        f = asBase(&buf);
        t->clone(asBase(&other.buf));
        t->destroy();
        other.f = asBase(&other.buf);
    }
    else if ((void*)f == &buf) {
        f->clone(asBase(&other.buf));
        f->destroy();
        f = other.f;
        other.f = asBase(&other.buf);
    }
    else if ((void*)other.f == &other.buf) {
        other.f->clone(asBase(&buf));
        other.f->destroy();
        other.f = f;
        f = asBase(&buf);
    }
    else {
        swap_(f, other.f);
    }
}

template<class R, class... ArgTypes>
R
Function<R(ArgTypes...)>::operator()(ArgTypes... args) const noexcept {
    assert_(f != nullptr);
    return (*f)(forward_<ArgTypes>(args)...);
}

#pragma warning(pop)

#endif  // SRC_UTIL_FUNCTION_H_
