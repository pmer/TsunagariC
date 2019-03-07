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
    inline constexpr StringView() noexcept : data(nullptr), size(0) {};
    inline StringView(const char* data) noexcept
            : data(data) {
        size_t len = 0;
        for (; *data != 0; ++data)
            ++len;
        size = len;
    }
    inline constexpr StringView(const char* data, size_t size) noexcept
            : data(data), size(size) {};
    inline constexpr StringView(const StringView& s) noexcept
            : data(s.data), size(s.size) {};

    StringView& operator=(const StringView& s) = default;
    
    inline constexpr const char* begin() const noexcept { return data; }
    inline constexpr const char* end() const noexcept { return data + size; }

    Optional<size_t> find(char needle) const noexcept;
    Optional<size_t> find(StringView needle) const noexcept;
    Optional<size_t> find(StringView needle, size_t start) const noexcept;
    Optional<size_t> rfind(char needle) const noexcept;

    constexpr StringView substr(const size_t from) const noexcept {
        assert_(from <= this->size);
        return StringView(data + from, size - from);
    }
    constexpr StringView substr(const size_t from, const size_t span) const noexcept {
        assert_(from <= size);
        assert_(from + span <= size);
        return StringView(data + from, span);
    }

 public:
    const char* data;
    size_t size;
};

inline constexpr bool operator==(const StringView& a,
                                 const StringView& b) noexcept {
    if (a.size != b.size) {
        return false;
    }

    size_t s = a.size;
    const char* ad = a.data;
    const char* bd = b.data;

    while (s--) {
        if (*ad++ != *bd++) {
            return false;
        }
    }
    return true;
}

inline constexpr bool operator!=(const StringView& a,
                                 const StringView& b) noexcept {
    return !(a == b);
}

inline constexpr bool operator>(const StringView& a,
                                const StringView& b) noexcept {
    size_t s = a.size < b.size ? a.size : b.size;
    const char* ad = a.data;
    const char* bd = b.data;
    
    while (s--) {
        if (*ad != *bd) {
            return *ad > *bd;
        }
        ad++;
        bd++;
    }
    if (a.size != b.size) {
        return a.size > b.size;
    }
    return false;
}

inline constexpr bool operator<(const StringView& a,
                                const StringView& b) noexcept {
    size_t s = a.size < b.size ? a.size : b.size;
    const char* ad = a.data;
    const char* bd = b.data;
    
    while (s--) {
        if (*ad != *bd) {
            return *ad < *bd;
        }
        ad++;
        bd++;
    }
    if (a.size != b.size) {
        return a.size < b.size;
    }
    return false;
}

size_t hash_(StringView s);

#endif  // SRC_UTIL_STRING_VIEW_H_
