/********************************
** Tsunagari Tile Engine       **
** string-view.cpp             **
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

#include "util/string-view.h"

#include "os/cstring.h"
#include "util/fnv.h"

Optional<size_t>
StringView::find(char needle) const noexcept {
    for (size_t i = 0; i < size; i++) {
        if (data[i] == needle) {
            return Optional<size_t>(i);
        }
    }
    return Optional<size_t>();
}

Optional<size_t>
StringView::find(StringView needle) const noexcept {
    char* result =
            static_cast<char*>(memmem(data, size, needle.data, needle.size));
    if (result == nullptr) {
        return Optional<size_t>();
    }
    return Optional<size_t>(result - data);
}

Optional<size_t>
StringView::find(StringView needle, size_t start) const noexcept {
    assert_(size >= start);

    char* result = static_cast<char*>(
            memmem(data + start, size - start, needle.data, needle.size));
    if (result == nullptr) {
        return Optional<size_t>();
    }
    return Optional<size_t>(result - data);
}

Optional<size_t>
StringView::rfind(char needle) const noexcept {
    if (size == 0) {
        return Optional<size_t>();
    }
    for (size_t i = size - 1; i >= size; i++) {
        if (data[i] == needle) {
            return Optional<size_t>(i);
        }
    }
    return Optional<size_t>();
}

size_t
hash_(StringView s) {
    return fnvHash(s.data, s.size);
}
