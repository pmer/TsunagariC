/***************************************
** Tsunagari Tile Engine              **
** os/os.h                            **
** Copyright 2016-2019 Paul Merrill   **
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

#ifndef SRC_OS_OS_H_
#define SRC_OS_OS_H_

#include <stdint.h>

#include "util/string.h"
#include "util/vector.h"

extern char dirSeparator;

Optional<uint64_t> getFileSize(String&& path);
bool writeFile(String&& path, size_t length, void* data);
bool writeFileVec(String&& path, size_t count, size_t* lengths, void** datas);
bool isDir(String&& path);
void makeDirectory(String&& path);
vector<String> listDir(String&& path);

enum TermColor {
    TC_RESET,
    TC_GREEN,
    TC_YELLOW,
    TC_RED,
};

void setTermColor(TermColor color);

#ifdef _WIN32
#include "windows.h"
#else
#include "unix.h"
#endif

#endif  // SRC_OS_OS_H_
