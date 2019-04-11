/*************************************
** Tsunagari Tile Engine            **
** ui.h                             **
** Copyright 2017-2019 Paul Merrill **
*************************************/

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

#ifndef SRC_PACK_UI_H_
#define SRC_PACK_UI_H_

#include "util/int.h"
#include "util/string-view.h"

extern bool verbose;

// RAII object whose destructor waits for our UI to exit.
class UI {
 public:
    UI() noexcept;
    ~UI() noexcept;
};

void uiShowSkippedMissingFile(StringView path) noexcept;
void uiShowAddedFile(StringView path, size_t size) noexcept;
void uiShowWritingArchive(StringView arhivePath) noexcept;
void uiShowListingEntry(StringView blobPath, uint64_t blobSize) noexcept;
void uiShowExtractingFile(StringView blobPath, uint64_t blogSize) noexcept;

#endif  // SRC_PACK_UI_H_
