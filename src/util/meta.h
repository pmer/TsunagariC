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

#ifndef SRC_UTIL_META_H_
#define SRC_UTIL_META_H_


//
// template struct If
//
template<bool Cond, typename WhenTrue, typename WhenFalse>
struct If_;

template<typename WhenTrue, typename WhenFalse>
struct If_<true, WhenTrue, WhenFalse> {
    typedef WhenTrue value;
};

template<typename WhenTrue, typename WhenFalse>
struct If_<false, WhenTrue, WhenFalse> {
    typedef WhenFalse value;
};

template<bool Cond, typename WhenTrue, typename WhenFalse>
using If = typename If_<Cond, WhenTrue, WhenFalse>::value;


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
// template DeclVal()
//
template<typename T>
T&& DeclVal();


//
// template struct IsConvertible
//
template<typename T>
void ConvertibleTest(T);

template<typename From, typename To, typename = void>
struct IsConvertible {
    static constexpr bool value = false;
};

template<typename From, typename To>
struct IsConvertible<From, To, decltype(ConvertibleTest<To>(DeclVal<From>()))> {
    static constexpr bool value = true;
};


//
// template struct EnableIfSubclass
//
template<typename B, typename D>
using EnableIfSubclass = typename EnableIf_<IsConvertible<D, B>::value>::value;


//
// template struct IsAbstract
//
template<typename T>
struct IsAbstract {
    typedef char yes[1];
    typedef char no[2];

    static no& test(T (*)[1]);
    static yes& test(...);

    static constexpr bool value = sizeof(test(nullptr)) == sizeof(yes);
};

#endif  // SRC_UTIL_META_H_
