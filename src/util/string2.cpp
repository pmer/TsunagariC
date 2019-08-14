/***************************************
** Tsunagari Tile Engine              **
** string2.cpp                        **
** Copyright 2011-2013 Michael Reiley **
** Copyright 2011-2019 Paul Merrill   **
***************************************/

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

#include "util/string2.h"

#include "os/c.h"
#include "os/os.h"
#include "util/int.h"
#include "util/math2.h"
#include "util/noexcept.h"
#include "util/string-view.h"
#include "util/string.h"

/**
 * Matches regex /\s*-?\d+/
 */
bool
isInteger(StringView s) noexcept {
    constexpr int space = 0;
    constexpr int sign = 1;
    constexpr int digit = 2;

    int state = space;

    for (size_t i = 0; i < s.size; i++) {
        char c = s.data[i];
        if (state == space) {
            if (c == ' ')
                continue;
            else
                state++;
        }
        if (state == sign) {
            state++;
            if (c == '-')
                continue;
        }
        if (state == digit) {
            if ('0' <= c && c <= '9')
                continue;
            else
                return false;
        }
    }
    return true;
}

/**
 * Matches regex /\s*-?\d+\.?\d* /   [sic: star-slash ends comment]
 */
bool
isDecimal(StringView s) noexcept {
    constexpr int space = 0;
    constexpr int sign = 1;
    constexpr int digit = 2;
    constexpr int dot = 3;
    constexpr int digit2 = 4;

    int state = space;

    for (size_t i = 0; i < s.size; i++) {
        char c = s.data[i];
        switch (state) {
        case space:
            if (c == ' ')
                continue;
            else
                state++;
        case sign:
            state++;
            if (c == '-')
                continue;
        case digit:
            if ('0' <= c && c <= '9')
                continue;
            else
                state++;
        case dot:
            state++;
            if (c == '.')
                continue;
            else
                return false;
        case digit2:
            if ('0' <= c && c <= '9')
                continue;
            else
                return false;
        }
    }
    return true;
}

/**
 * Matches "5-7,2,12-14" no whitespace.
 */
bool
isRanges(StringView s) noexcept {
    constexpr int sign = 0;
    constexpr int digit = 1;
    constexpr int dash = 3;
    constexpr int comma = 4;

    bool dashed = false;

    int state = sign;

    for (size_t i = 0; i < s.size; i++) {
        char c = s.data[i];
        switch (state) {
        case sign:
            state++;
            if (c == '-' || c == '+')
                break;
        case digit:
            if ('0' <= c && c <= '9')
                break;
            state++;
        case dash:
            state++;
            if (c == '-') {
                if (dashed)
                    return false;
                dashed = true;
                state = sign;
                break;
            }
        case comma:
            state++;
            if (c == ',') {
                dashed = false;
                state = sign;
                break;
            }
            return false;
        }
    }
    return true;
}

static inline char
tolower(char c) noexcept {
    if ('A' <= c && c <= 'Z') {
        return c + 'a' - 'A';
    }
    else {
        return c;
    }
}

bool
iequals(StringView a, StringView b) noexcept {
    if (a.size != b.size) {
        return false;
    }
    for (size_t i = 0; i < a.size; i++) {
        if (tolower(a.data[i]) != tolower(b.data[i])) {
            return false;
        }
    }
    return true;
}

Optional<bool>
parseBool(StringView s) noexcept {
    static const StringView true_ = "true";
    static const StringView yes = "yes";
    static const StringView on = "on";

    if (iequals(s, true_) || iequals(s, yes) || iequals(s, on) || s == "1") {
        return Optional<bool>(true);
    }

    static const StringView false_ = "false";
    static const StringView no = "no";
    static const StringView off = "off";

    if (iequals(s, false_) || iequals(s, no) || iequals(s, off) || s == "0") {
        return Optional<bool>(false);
    }

    return none;
}

Optional<unsigned>
parseUInt(String& s) noexcept {
    errno = 0;

    char* end;
    unsigned long ul = strtoul(s.null(), &end, 10);

    if (end != s.data() + s.size()) {
        return none;
    }

    if (errno != 0) {
        // Overflow.
        return none;
    }
    if (ul > UINT32_MAX) {
        // Overflow.
        return none;
    }

    return Optional<unsigned>(static_cast<unsigned>(ul));
}

Optional<unsigned>
parseUInt(StringView s) noexcept {
    String s_(s);
    return parseUInt(s_);
}

Optional<int>
parseInt(String& s) noexcept {
    errno = 0;

    char* end;
    long l = strtol(s.null(), &end, 10);

    if (end != s.data() + s.size()) {
        return none;
    }

    if (errno != 0) {
        // Overflow.
        return none;
    }
    if (l > UINT32_MAX) {
        // Overflow.
        return none;
    }

    return Optional<int>(static_cast<int>(l));
}

Optional<int>
parseInt(StringView s) noexcept {
    String s_(s);
    return parseInt(s_);
}

Optional<double>
parseDouble(String& s) noexcept {
    errno = 0;

    char* end;
    double d = strtod(s.null(), &end);

    if (end != s.data() + s.size()) {
        return none;
    }

    if (errno != 0) {
        // Overflow.
        return none;
    }

    return Optional<double>(d);
}

Optional<double>
parseDouble(StringView s) noexcept {
    String s_(s);
    return parseDouble(s_);
}

int
parseInt100(const char* s) noexcept {
    int i = atoi(s);
    return bound(i, 0, 100);
}

Vector<StringView>
splitStr(StringView input, StringView delimiter) noexcept {
    Vector<StringView> strlist;
    size_t i = 0;

    for (Optional<size_t> pos = input.find(delimiter); pos;
         pos = input.find(delimiter, i)) {
        strlist.push_back(input.substr(i, *pos - i));
        i = *pos + delimiter.size;
    }

    if (input.size != i) {
        strlist.push_back(input.substr(i));
    }
    return strlist;
}

Optional<Vector<int>>
parseRanges(StringView format) noexcept {
    Vector<int> ints;
    for (StringView range : splitStr(format, ",")) {
        Optional<size_t> dash = range.find("-");

        if (!dash) {
            Optional<int> i = parseInt(range);
            if (!i) {
                return none;
            }

            ints.push_back(*i);
        }
        else {
            size_t dash_ = *dash;

            StringView rngbeg = range.substr(0, dash_);
            StringView rngend = range.substr(dash_ + 1);

            Optional<int> beg = parseInt(rngbeg);
            Optional<int> end = parseInt(rngend);
            if (!beg || !end) {
                return none;
            }

            int beg_ = *beg;
            int end_ = *end;
            if (beg_ > end_) {
                return none;
            }

            for (int i = beg_; i <= end_; i++) {
                ints.push_back(i);
            }
        }
    }
    return Optional<Vector<int>>(move_(ints));
}
