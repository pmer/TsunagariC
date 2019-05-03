/*********************************
 ** Tsunagari Tile Engine       **
 ** string.h                    **
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

#ifndef SRC_UTIL_STRING_H_
#define SRC_UTIL_STRING_H_

#include "util/string-view.h"
#include "util/vector.h"

class String;

class NullTerminatedString {
 public:
    explicit NullTerminatedString(String& s) noexcept;
    ~NullTerminatedString() noexcept;

    const char* get() noexcept;
    operator const char*() noexcept;

 public:
    String& s;
};

class String : public Vector<char> {
 public:
    String() = default;
    String(const char* value) noexcept;
    String(StringView value) noexcept;
    String(const String& other) = default;
    String(String&& other) noexcept;
    ~String() = default;

    String& operator=(const String& other) = default;
    String& operator=(String&& other) noexcept;
    bool operator<(const String& other) const noexcept;
    bool operator>(const String& other) const noexcept;

    String& operator<<(char value) noexcept;
    String& operator<<(const char* value) noexcept;
    String& operator<<(StringView value) noexcept;

    String& operator<<(bool value) noexcept;
    String& operator<<(int value) noexcept;
    String& operator<<(unsigned int value) noexcept;
    String& operator<<(long value) noexcept;
    String& operator<<(unsigned long value) noexcept;
    String& operator<<(long long value) noexcept;
    String& operator<<(unsigned long long value) noexcept;
    String& operator<<(float value) noexcept;
    String& operator<<(double value) noexcept;

    operator StringView() const noexcept;
    StringView view() const noexcept;

    NullTerminatedString null() noexcept;
};

size_t hash_(const String& s) noexcept;

#endif  // SRC_UTIL_STRING_H_
