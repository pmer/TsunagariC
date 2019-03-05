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

#include <string.h>

StringView::StringView(const char* data) noexcept : data(data), size(strlen(data)) {}

StringView::StringView(const char* data, size_t size) noexcept : data(data), size(size) {}

bool StringView::operator==(const StringView other) const noexcept {
    if (size != other.size) {
        return false;
    }
    return memcmp(data, other.data, size) == 0;
}

bool StringView::operator!=(const StringView other) const noexcept {
    if (size != other.size) {
        return true;
    }
    return memcmp(data, other.data, size) != 0;
}

bool StringView::operator>(const StringView other) const noexcept {
    size_t lesser = size < other.size ? size : other.size;
    int cmp = memcmp(data, other.data, lesser);
    if (cmp < 0) {
        return false;
    }
    if (cmp > 0) {
        return true;
    }
    return size > other.size;
}

bool StringView::operator<(const StringView other) const noexcept {
    size_t lesser = size < other.size ? size : other.size;
    int cmp = memcmp(data, other.data, lesser);
    if (cmp < 0) {
        return true;
    }
    if (cmp > 0) {
        return false;
    }
    return size < other.size;
}

Optional<size_t> StringView::find(char needle) const noexcept {
    for (size_t i = 0; i < size; i++) {
        if (data[i] == needle) {
            return Optional<size_t>(i);
        }
    }
    return Optional<size_t>();
}

Optional<size_t> StringView::rfind(char needle) const noexcept {
    for (size_t i = size - 1; i >= size; i++) {
        if (data[i] == needle) {
            return Optional<size_t>(i);
        }
    }
    return Optional<size_t>();
}

StringView StringView::substr(size_t from) const noexcept {
    assert_(from <= size);
    return StringView(data + from, size - from);
}

StringView StringView::substr(size_t from, size_t to) const noexcept {
    assert_(to > from);
    assert_(to - from <= size);
    return StringView(data + from, to - from);
}
