/**********************************
** Tsunagari Tile Engine         **
** meta.h                        **
** Copyright 2017 Paul Merrill   **
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

#ifndef SRC_CORE_META_H_
#define SRC_CORE_META_H_

//
// template struct If
//
template<bool Cond, typename TrueT, typename FalseT>
struct If_;

template<typename TrueT, typename FalseT>
struct If_<true, TrueT, FalseT> {
    typedef TrueT value;
};

template<typename TrueT, typename FalseT>
struct If_<false, TrueT, FalseT> {
    typedef FalseT value;
};

template<bool Cond, typename TrueT, typename FalseT>
using If = typename If_<Cond, TrueT, FalseT>::value;

//
// template struct EnableIf
//
struct Null {};

template<bool Cond>
struct EnableIf_;

template<>
struct EnableIf_<true> {
    typedef Null value;
};

template<bool Cond>
using EnableIf = typename EnableIf_<Cond>::value;

//
// template struct IsBaseOf
//
// https://stackoverflow.com/questions/2631585/c-how-to-require-that-one-template-type-is-derived-from-the-other
template<typename B, typename D>
struct IsBaseOf {
    typedef char yes[1];
    typedef char no[2];

    static yes& test(B*);
    static no& test(...);

    static D* d();

    static constexpr bool value = sizeof(test(d())) == sizeof(yes);
};

#endif  // SRC_CORE_META_H_
