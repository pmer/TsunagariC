/*************************************
** Tsunagari Tile Engine            **
** ui-log.cpp                       **
** Copyright 2016-2017 Paul Merrill **
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

#include "pack/ui.h"

#include "pack/pool.h"
#include "util/memory.h"

static Unique<Pool> pool(Pool::makePool("ui", 1));

void uiShowAddingFile(const std::string& path) {
    pool->schedule([=] {
        printf("Adding %s\n", path.c_str());
    });
}

void uiShowWritingArchive(const std::string& archivePath) {
    pool->schedule([=] {
        printf("Writing to %s\n", archivePath.c_str());
    });
}

void uiShowListingEntry(const std::string& blobPath, uint64_t blobSize) {
    pool->schedule([=] {
        printf("%s: %llu bytes\n", blobPath.c_str(), blobSize);
    });
}

void uiShowExtractingFile(const std::string& blobPath, uint64_t blobSize) {
    pool->schedule([=] {
        printf("Extracting %s: %llu bytes\n", blobPath.c_str(), blobSize);
    });
}
