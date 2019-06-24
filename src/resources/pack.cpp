/*************************************
** Tsunagari Tile Engine            **
** pack.cpp                         **
** Copyright 2016-2019 Paul Merrill **
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

#include "core/log.h"
#include "core/measure.h"
#include "core/resources.h"
#include "data/data-world.h"
#include "os/mutex.h"
#include "pack/pack-reader.h"
#include "util/int.h"
#include "util/unique.h"

static Mutex mutex;
static Unique<PackReader> pack;

static bool
openPackFile() noexcept {
    if (pack) {
        return true;
    }

    StringView path = DataWorld::instance().datafile;

    // TimeMeasure m("Opened " + path);

    pack = PackReader::fromFile(path);
    if (!pack) {
        Log::fatal("PackResources",
                   String() << path << ": could not open archive");
        return false;
    }

    return true;
}

static String
getFullPath(StringView path) noexcept {
    return String() << DataWorld::instance().datafile << "/" << path;
}

Optional<StringView>
resourceLoad(StringView path) noexcept {
    LockGuard lock(mutex);

    if (!openPackFile()) {
        return Optional<StringView>();
    }

    PackReader::BlobIndex index = pack->findIndex(path);

    if (index == PackReader::BLOB_NOT_FOUND) {
        Log::err("PackResources",
                 String() << getFullPath(path) << ": file missing");
        return Optional<StringView>();
    }

    uint32_t blobSize = pack->getBlobSize(index);

    // Will it fit in memory?
    if (blobSize > UINT32_MAX) {
        Log::err("PackResources",
                 String() << getFullPath(path) << ": file too large");
        return Optional<StringView>();
    }

    void* data = pack->getBlobData(index);

    return Optional<StringView>(StringView(static_cast<char*>(data), blobSize));
}
