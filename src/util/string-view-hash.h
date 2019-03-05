/********************************
** Tsunagari Tile Engine       **
** string-view-hash.h          **
** Copyright 2019 Paul Merrill **
*********************************/

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

#ifndef SRC_UTIL_STRING_VIEW_HASH_H_
#define SRC_UTIL_STRING_VIEW_HASH_H_

#include "util/fnv.h"
#include "util/string-view.h"

namespace std {
    template<>
    struct hash<StringView> {
        typedef StringView argument_type;
        typedef size_t result_type;
        size_t operator()(StringView s) const {
            return fnvHash(s.data, s.size);
        }
    };
}

#endif  // SRC_UTIL_STRING_VIEW_STD_H_
