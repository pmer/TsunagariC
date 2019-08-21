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

#include "os/c.h"
#include "util/constexpr.h"
#include "util/int.h"
#include "util/markable.h"
#include "util/noexcept.h"

typedef Markable<size_t,SIZE_MAX> StringPosition;

class StringView {
 public:
    inline CONSTEXPR11 StringView() noexcept : data(nullptr), size(0) {};
    inline StringView(const char* data) noexcept
            : data(data), size(strlen(data)) {}
    template<size_t N>
    inline CONSTEXPR11 StringView(const char (&data)[N]) noexcept
            : data(data), size(N){};
    inline CONSTEXPR11 StringView(const char* data, size_t size) noexcept
            : data(data), size(size){};
    inline CONSTEXPR11 StringView(const StringView& s) noexcept
            : data(s.data), size(s.size){};

    StringView& operator=(const StringView& s) = default;

    inline CONSTEXPR11 const char* begin() const noexcept { return data; }
    inline CONSTEXPR11 const char* end() const noexcept { return data + size; }

    StringPosition find(char needle) const noexcept;
    StringPosition find(StringView needle) const noexcept;
    StringPosition find(StringView needle, size_t start) const noexcept;
    StringPosition rfind(char needle) const noexcept;

	CONSTEXPR14 StringView substr(const size_t from) const noexcept {
        assert_(from <= this->size);
        return StringView(data + from, size - from);
    }
    CONSTEXPR14 StringView substr(const size_t from, const size_t span) const
            noexcept {
        assert_(from <= size);
        assert_(from + span <= size);
        return StringView(data + from, span);
    }

 public:
    const char* data;
    size_t size;
};

inline CONSTEXPR11 bool
operator==(const StringView& a, const StringView& b) noexcept {
    return (a.size == b.size) && memcmp(a.data, b.data, a.size) == 0;
}

inline CONSTEXPR11 bool
operator!=(const StringView& a, const StringView& b) noexcept {
    return !(a == b);
}

inline CONSTEXPR14 bool
operator>(const StringView& a, const StringView& b) noexcept {
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

inline CONSTEXPR14 bool
operator<(const StringView& a, const StringView& b) noexcept {
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

size_t hash_(StringView s) noexcept;
size_t hash_(const char* s) noexcept;

#endif  // SRC_UTIL_STRING_VIEW_H_
