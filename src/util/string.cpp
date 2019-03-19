/*********************************
 ** Tsunagari Tile Engine       **
 ** string.cpp                  **
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

#include "util/string.h"

#include "os/cstdio.h"
#include "os/cstring.h"

NullTerminatedString::NullTerminatedString(String& s) : s(s) {
    s.push_back('\0');
}

NullTerminatedString::~NullTerminatedString() {
    s.pop_back();
}

const char*
NullTerminatedString::get() {
    return s.data();
}

NullTerminatedString::operator const char*() {
    return s.data();
}

String::String(const char* value) {
    *this << value;
}

String::String(StringView value) {
    *this << value;
}

String::String(String&& other) noexcept {
    swap(other);
}

String&
String::operator=(String&& other) noexcept {
    swap(other);
    return *this;
}

bool
String::operator<(const String& other) const noexcept {
    return view() < other.view();
}

bool
String::operator>(const String& other) const noexcept {
    return view() > other.view();
}

String&
String::operator<<(char value) {
    push_back(value);
    return *this;
}

String&
String::operator<<(const char* value) {
    append(value, strlen(value));
    return *this;
}

String&
String::operator<<(StringView value) {
    append(value.data, value.size);
    return *this;
}

String&
String::operator<<(bool value) {
    return *this << (value ? "true" : "false");
}

String&
String::operator<<(int value) {
    char buf[64];
    sprintf(buf, "%d", value);
    return *this << buf;
}

String&
String::operator<<(unsigned int value) {
    char buf[64];
    sprintf(buf, "%u", value);
    return *this << buf;
}

String&
String::operator<<(long value) {
    char buf[64];
    sprintf(buf, "%ld", value);
    return *this << buf;
}

String&
String::operator<<(unsigned long value) {
    char buf[64];
    sprintf(buf, "%lu", value);
    return *this << buf;
}

String&
String::operator<<(long long value) {
    char buf[64];
    sprintf(buf, "%lld", value);
    return *this << buf;
}

String&
String::operator<<(unsigned long long value) {
    char buf[64];
    sprintf(buf, "%llu", value);
    return *this << buf;
}

String&
String::operator<<(float value) {
    char buf[64];
    sprintf(buf, "%f", value);
    return *this << buf;
}

String&
String::operator<<(double value) {
    char buf[64];
    sprintf(buf, "%f", value);
    return *this << buf;
}

String::operator StringView() const {
    return StringView(data(), size());
}

StringView
String::view() const {
    return StringView(data(), size());
}

NullTerminatedString
String::null() {
    return NullTerminatedString(*this);
}
