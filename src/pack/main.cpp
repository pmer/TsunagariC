/*************************************
** Tsunagari Tile Engine            **
** main.cpp                         **
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

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <unordered_set>

#include "os/os.h"
#include "pack/pack-file.h"
#include "util/optional.h"

const char* exe = nullptr;

static void usage() {
    fprintf(stderr, "usage: %s create <output-archive> [input-file]...\n", exe);
    fprintf(stderr, "       %s list <archive>\n", exe);
    fprintf(stderr, "       %s extract <archive>\n", exe);
}


static void slurp(const std::string& path, uint64_t* size, void** data) {
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

template<typename Iterator>
static bool createArchive(const std::string& archivePath,
                          Iterator inputsBegin,
                          Iterator inputsEnd) {
    std::unique_ptr<PackWriter> pack = PackWriter::make();

    for (; inputsBegin != inputsEnd; ++inputsBegin) {
        const std::string& inputPath = *inputsBegin;
        addPath(pack.get(), inputPath);
    }

    printf("Writing to %s\n", archivePath.c_str());
    return pack->writeToFile(archivePath);
}

static bool listArchive(const std::string& archivePath) {
    std::unique_ptr<PackReader> pack = PackReader::fromFile(archivePath);

    if (pack) {
        for (PackReader::BlobIndex i = 0; i < pack->size(); i++) {
            std::string blobPath = pack->getBlobPath(i);
            uint64_t blobSize = pack->getBlobSize(i);
            printf("%s %llu\n", blobPath.c_str(), blobSize);
        }
        return true;
    } else {
        fprintf(stderr, "%s: %s: not found\n", exe, archivePath.c_str());
        return false;
    }
}

struct ExtractContext {
    std::unordered_set<std::string> createdDirectories;
};

static Optional<std::string> getParentPath(const std::string& path) {
    auto sep = path.find_last_of('/');
    if (sep == path.npos) {
        return Optional<std::string>();
    } else {
        return Optional<std::string>(path.substr(0, sep));
    }
}

static void createDirs(ExtractContext* ctx, const std::string& path) {
    Optional<std::string> parentPath = getParentPath(path);
    if (parentPath) {
        if (ctx->createdDirectories.count(*parentPath) == 0) {
            // Make sure parentPath's parent exists.
            createDirs(ctx, *parentPath);

            ctx->createdDirectories.insert(*parentPath);
            makeDirectory(*parentPath);
        }
    }
}

static void putFile(ExtractContext* ctx, const std::string& path, uint64_t size,
                    void* data) {
    createDirs(ctx, path);
    FILE* f = fopen(path.c_str(), "w");
    fwrite(data, size, 1, f);
    fclose(f);
}

static bool extractArchive(const std::string& archivePath) {
    std::unique_ptr<PackReader> pack = PackReader::fromFile(archivePath);

    if (pack) {
        ExtractContext ctx;
        for (PackReader::BlobIndex i = 0; i < pack->size(); i++) {
            std::string blobPath = pack->getBlobPath(i);
            uint64_t blobSize = pack->getBlobSize(i);
            void* blobData = pack->getBlobData(i);
            printf("%s %llu\n", blobPath.c_str(), blobSize);
            putFile(&ctx, blobPath, blobSize, blobData);
        }
        return true;
    } else {
        fprintf(stderr, "%s: %s: not found\n", exe, archivePath.c_str());
        return false;
    }
}

int main(int argc, char* argv[]) {
    exe = strrchr(argv[0], dirSeparator);
    if (exe) {
        exe += 1;
    } else {
        exe = argv[0];
    }

    if (argc == 1) {
        usage();
        return 0;
    }
    if (argc == 2) {
        usage();
        return 1;
    }

    std::string command = argv[1];
    std::vector<std::string> args;

    for (int i = 2; i < argc; i++) {
        args.push_back(argv[i]);
    }

    if (command == "create") {
        return createArchive(args[0], args.begin() + 1, args.end()) ? 0 : 1;
    } else if (command == "list") {
        if (argc != 3) {
            usage();
            return 1;
        }

        return listArchive(args[0]) ? 0 : 1;
    } else if (command == "extract") {
        if (argc != 3) {
            usage();
            return 1;
        }

        return extractArchive(args[0]) ? 0 : 1;
    } else {
        usage();
        return 1;
    }
}
