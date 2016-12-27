/***************************************
** Tsunagari Tile Engine              **
** resources-physfs.cpp               **
** Copyright 2011-2015 Michael Reiley **
** Copyright 2011-2016 Paul Merrill   **
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

#include "resources/resources-physfs.h"

#include <physfs.h>

#include <limits>
#include <mutex>

#include "core/formatter.h"
#include "core/log.h"
#include "core/measure.h"

#include "data/data-world.h"

PhysfsResource::PhysfsResource(std::unique_ptr<const char[]> data, size_t size)
    : _data(std::move(data)), _size(size) {}

const void* PhysfsResource::data() const {
    return _data.get();
}

size_t PhysfsResource::size() const {
    return _size;
}


static PhysfsResources globalResources;
static std::mutex globalPhysfsMutex;

Resources& Resources::instance() {
    return globalResources;
}

PhysfsResources::PhysfsResources()
    : initialized(false) {}

static void initialize() {
    if (!PHYSFS_init(nullptr)) {
        Log::fatal("Resources", "PHYSFS_init");
    }

    const std::string& path = DataWorld::instance().datafile;

    if (!PHYSFS_mount(path.c_str(), nullptr, 0)) {
        Log::fatal(
            "Resources",
            Formatter("%: could not open archive: %")
                % path % PHYSFS_getLastError()
        );
    }
}

std::unique_ptr<Resource> PhysfsResources::load(const std::string& path) {
    std::lock_guard<std::mutex> lock(globalPhysfsMutex);

    TimeMeasure m("Mapped " + path);

    if (!initialized) {
        initialized = true;
        initialize();
    }

    const std::string fullPath = DataWorld::instance().datafile + "/" + path;

    if (!PHYSFS_exists(path.c_str())) {
        Log::err(
            "Resources",
            Formatter("%: file missing") % fullPath
        );
        return std::unique_ptr<Resource>();
    }

    PHYSFS_File* zf = PHYSFS_openRead(path.c_str());
    if (!zf) {
        Log::err(
            "Resources",
            Formatter("%: error opening file: %")
                % fullPath % PHYSFS_getLastError()
        );
        return std::unique_ptr<Resource>();
    }

    PHYSFS_sint64 foundSize = PHYSFS_fileLength(zf);

    if (foundSize == -1) {
        Log::err(
            "Resources",
            Formatter("%: could not determine file size: %")
                % fullPath % PHYSFS_getLastError()
        );
        PHYSFS_close(zf);
        return std::unique_ptr<Resource>();;
    }
    else if ((size_t)foundSize > std::numeric_limits<size_t>::max()) {
        // Won't fit in memory.
        Log::err(
            "Resources",
            Formatter("%: file too large") % fullPath
        );
        PHYSFS_close(zf);
        return std::unique_ptr<Resource>();;
    }
    else if (foundSize > std::numeric_limits<uint32_t>::max()) {
        // FIXME: Technically, we just need to issue multiple calls to
        // PHYSFS_read. Fix when needed.
        Log::err(
            "Resources",
            Formatter("%: file too large") % fullPath
        );
        PHYSFS_close(zf);
        return std::unique_ptr<Resource>();;
    }
    else if (foundSize < -1) {
        Log::err(
            "Resources",
            Formatter("%: invalid file size: %")
                % fullPath % PHYSFS_getLastError()
        );
        PHYSFS_close(zf);
        return std::unique_ptr<Resource>();;
    }

    size_t size = (size_t)foundSize;

    auto data = std::make_unique<char[]>(size);
    if (size == 0) {
        // Don't perform file read if file is zero bytes.
        return std::make_unique<PhysfsResource>(std::move(data), size);
    }

    PHYSFS_sint64 err = PHYSFS_read(zf, (char*)data.get(),
        (PHYSFS_uint32)size, 1);
    if (err != 1) {
        Log::err(
            "Resources",
            Formatter("%: error reading file: %")
                % fullPath % PHYSFS_getLastError()
        );
        PHYSFS_close(zf);
        return std::unique_ptr<Resource>();;
    }

    PHYSFS_close(zf);

    return std::make_unique<PhysfsResource>(std::move(data), size);
}
