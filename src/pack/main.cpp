/*************************************
** Tsunagari Tile Engine            **
** main.cpp                         **
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

#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <mutex>
#include <unordered_set>

#include "os/os.h"
#include "pack/pack-reader.h"
#include "pack/pack-writer.h"
#include "pack/pool.h"
#include "pack/walker.h"
#include "pack/ui.h"
#include "util/move.h"
#include "util/optional.h"
#include "util/unique.h"

const char* exe = nullptr;

static void usage() {
    fprintf(stderr, "usage: %s create <output-archive> [input-file]...\n", exe);
    fprintf(stderr, "       %s list <input-archive>\n", exe);
    fprintf(stderr, "       %s extract <input-archive>\n", exe);
}


struct CreateArchiveContext {
    Unique<PackWriter> pack;
    std::mutex packMutex;
};

static void slurp(const std::string& path, uint64_t* size, void** data) {
    *size = getFileSize(path);
    *data = new char*[*size];
    FILE* f = fopen(path.c_str(), "r");
    fread(*data, *size, 1, f);
    fclose(f);
}

static void addFile(CreateArchiveContext& ctx, std::string path) {
    uiShowAddingFile(path);

    uint64_t size;
    void* data;
    slurp(path, &size, &data);

    std::lock_guard<std::mutex> guard(ctx.packMutex);
    ctx.pack->addBlob(move_(path), size, data);
}

template<typename Iterator>
static bool createArchive(const std::string& archivePath,
                          Iterator inputsBegin,
                          Iterator inputsEnd) {
    CreateArchiveContext ctx;
    ctx.pack = PackWriter::make();

    vector<std::string> paths;
    for (; inputsBegin != inputsEnd; ++inputsBegin) {
        paths.emplace_back(*inputsBegin);
    }

    walk(move_(paths), [&](std::string path) {
        addFile(ctx, move_(path));
    });

    uiShowWritingArchive(archivePath);

    return ctx.pack->writeToFile(archivePath);
}

static bool listArchive(const std::string& archivePath) {
    Unique<PackReader> pack = PackReader::fromFile(archivePath);

    if (pack) {
        std::string output;

        for (PackReader::BlobIndex i = 0; i < pack->size(); i++) {
            std::string blobPath = pack->getBlobPath(i);
            uint64_t blobSize = pack->getBlobSize(i);

            output.append(blobPath);
            output.append(": ");
            output.append(std::to_string(blobSize));
            output.append(" bytes\n");
        }
        printf("%s", output.c_str());
        return true;
    } else {
        fprintf(stderr, "%s: %s: not found\n", exe, archivePath.c_str());
        return false;
    }
}

static Optional<std::string> getParentPath(const std::string& path) {
    auto sep = path.find_last_of('/');
    if (sep == path.npos) {
        return Optional<std::string>();
    } else {
        return Optional<std::string>(path.substr(0, sep));
    }
}

static void createDirs(const std::string& path) {
    Optional<std::string> parentPath = getParentPath(path);
    if (parentPath) {
        // Make sure parentPath's parent exists.
        createDirs(*parentPath);

        makeDirectory(*parentPath);
    }
}

static void putFile(const std::string& path, uint64_t size, void* data) {
    createDirs(path);

	// FIXME: Use native IO.
	FILE* f = fopen(path.c_str(), "w");
	if (!f) {
		// TODO: Propagate error up.
	}
	fwrite(data, size, 1, f);
	fclose(f);
}

static bool extractArchive(const std::string& archivePath) {
    Unique<PackReader> pack = PackReader::fromFile(archivePath);

    if (pack) {
        vector<PackReader::BlobIndex> blobIndicies;
        for (PackReader::BlobIndex i = 0; i < pack->size(); i++) {
            blobIndicies.push_back(i);
        }

        vector<void*> blobDatas = pack->getBlobDatas(blobIndicies);

        Unique<Pool> pool(Pool::makePool("extract", 1));

        for (PackReader::BlobIndex i = 0; i < pack->size(); i++) {
            std::string blobPath = pack->getBlobPath(i);
            uint64_t blobSize = pack->getBlobSize(i);
            void* blobData = blobDatas[i];

            pool->schedule([=] {
                uiShowExtractingFile(blobPath, blobSize);

                putFile(blobPath, blobSize, blobData);
            });
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
    vector<std::string> args;

    for (int i = 2; i < argc; i++) {
        args.push_back(argv[i]);
    }

    int exitCode;

    if (command == "create") {
        return createArchive(args[0], args.begin() + 1, args.end()) ? 0 : 1;
    } else if (command == "list") {
        if (argc != 3) {
            usage();
            return 1;
        }

        exitCode = listArchive(args[0]) ? 0 : 1;
    } else if (command == "extract") {
        if (argc != 3) {
            usage();
            return 1;
        }

        exitCode = extractArchive(args[0]) ? 0 : 1;
    } else {
        usage();
        return 1;
    }

    return exitCode;
}
