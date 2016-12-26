/***********************************
** Tsunagari Tile Engine          **
** main.cpp                       **
** Copyright 2016 Paul Merrill    **
***********************************/

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

#include <stddef.h>
#include <stdio.h>

#include "os/os.h"
#include "pack/pack-file.h"

static void usage(const char* argv0) {
    printf("%s: <output-archive> [input-file]...\n", argv0);
}


void slurp(const std::string& path, uint64_t* size, void** data) {
    *size = getFileSize(path);
    *data = new char*[*size];
    FILE* f = fopen(path.c_str(), "r");
    fread(*data, *size, 1, f);
    fclose(f);
}

static void addPath(PackWriter* pack, std::string path);

static void addFile(PackWriter* pack, std::string path) {
    printf("Adding %s\n", path.c_str());
    uint64_t size;
    void* data;
    slurp(path, &size, &data);
    pack->addBlob(std::move(path), size, data);
}

static void addDir(PackWriter* pack, std::string path) {
    std::vector<std::string> names = listDir(path);
    std::sort(names.begin(), names.end());
    for (auto& name : names) {
        addPath(pack, path + dirSeparator + name);
    }
}

static void addPath(PackWriter* pack, std::string path) {
    if (isDir(path)) {
        addDir(pack, path);
    } else {
        addFile(pack, path);
    }
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        usage(argv[0]);
        return 0;
    }
    if (argc == 2) {
        usage(argv[0]);
        return 1;
    }

    std::unique_ptr<PackWriter> pack = PackWriter::make();

    for (int i = 2; i < argc; i++) {
        addPath(pack.get(), argv[i]);
    }

    std::string archive = argv[1];

    printf("Writing to %s\n", archive.c_str());
    if (pack->writeToFile(argv[1])) {
        return 0;
    } else {
        return 1;
    }
}
