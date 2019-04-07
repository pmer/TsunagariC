/***************************************
** Tsunagari Tile Engine              **
** string2.h                          **
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

#ifndef SRC_UTIL_STRING2_H_
#define SRC_UTIL_STRING2_H_

#include "util/optional.h"
#include "util/string-view.h"
#include "util/string.h"
#include "util/vector.h"

//! Returns true if the string contains only digits, whitespace, and minus.
bool isInteger(StringView s);

//! Returns true if the string contains only digits, whitespace, minus, and
//! period.
bool isDecimal(StringView s);

bool isRanges(StringView s);


//! Whether two strings are case-insensative equals.
bool iequals(StringView a, StringView b);

//! Return a bool from a "true"/"false" string.
Optional<bool> parseBool(StringView s);

Optional<int> parseInt(String& s);
Optional<int> parseInt(StringView s);
Optional<unsigned> parseUInt(String& s);
Optional<unsigned> parseUInt(StringView s);
Optional<double> parseDouble(String& s);
Optional<double> parseDouble(StringView s);

int parseInt100(const char* s);

//! Split a string by a delimiter.
Vector<String> splitStr(StringView str, StringView delimiter);

/**
 * Parse ranges of integers separated by commas.
 * Can take things such as "5-7,2,12-18".
 */
Optional<Vector<int>> parseRanges(StringView format);

Optional<String> slurp(String& path);
Optional<String> slurp(StringView path);

#endif  // SRC_UTIL_STRING2_H_
