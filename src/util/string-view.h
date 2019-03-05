/********************************
** Tsunagari Tile Engine       **
** string-view.h               **
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

#ifndef SRC_UTIL_STRING_VIEW_H_
#define SRC_UTIL_STRING_VIEW_H_

#include <stddef.h>

#include "util/optional.h"

class StringView {
 public:
    StringView() = default;
    StringView(const char* data) noexcept;
    StringView(const char* data, size_t size) noexcept;
    StringView(const StringView& s) = default;
    ~StringView() = default;

    StringView& operator=(const StringView& s) = default;

    bool operator==(const StringView s) const noexcept;
    bool operator!=(const StringView s) const noexcept;
    bool operator<(const StringView s) const noexcept;
    bool operator>(const StringView s) const noexcept;

    Optional<size_t> find(char needle) const noexcept;
    Optional<size_t> rfind(char needle) const noexcept;
    StringView substr(size_t from) const noexcept;
    StringView substr(size_t from, size_t to) const noexcept;

 public:
    const char* data;
    size_t size;
};

#endif  // SRC_UTIL_STRING_VIEW_H_
