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

#include "os/os.h"
#include "pack/pack-reader.h"
#include "pack/pack-writer.h"
#include "pack/pool.h"
#include "pack/walker.h"
#include "pack/ui.h"
#include "util/move.h"
#include "util/optional.h"
#include "util/string-view-std.h"
#include "util/string-view.h"
#include "util/string.h"
#include "util/unique.h"

static String exe;

static void usage() {
    fprintf(stderr, "usage: %s create <output-archive> [input-file]...\n",
            exe.null().get());
    fprintf(stderr, "       %s list <input-archive>\n",
            exe.null().get());
    fprintf(stderr, "       %s extract <input-archive>\n",
            exe.null().get());
}

struct CreateArchiveContext {
    Unique<PackWriter> pack;
    std::mutex packMutex;
};

static void slurp(String&& path, uint64_t* size, void** data) {
    Optional<uint64_t> size_ = getFileSize(forward_<String>(path));
    if (!size_) {
        return;
    }

    *size = *size_;
    *data = new char*[*size];

    FILE* f = fopen(path.null(), "r");
    fread(*data, *size, 1, f);
    fclose(f);
}

static void addFile(CreateArchiveContext& ctx, StringView path) {
    uint64_t size = 0;
    void* data = nullptr;
    slurp(path, &size, &data);

    if (!data) {
        uiShowSkippedMissingFile(path);
        return;
    }

    uiShowAddedFile(path, size);

    std::lock_guard<std::mutex> guard(ctx.packMutex);
    ctx.pack->addBlob(move_(path), size, data);
}

static bool createArchive(StringView archivePath, vector<StringView> paths) {
    CreateArchiveContext ctx;
    ctx.pack = PackWriter::make();

    walk(move_(paths), [&](StringView path) {
        addFile(ctx, path);
    });

    uiShowWritingArchive(archivePath);

    return ctx.pack->writeToFile(archivePath);
}

static bool listArchive(StringView archivePath) {
    Unique<PackReader> pack = PackReader::fromFile(archivePath);

    if (pack) {
        String output;

        for (PackReader::BlobIndex i = 0; i < pack->size(); i++) {
            StringView blobPath = pack->getBlobPath(i);
            uint64_t blobSize = pack->getBlobSize(i);

            output.append(blobPath.data, blobPath.size);
            output << ": " << blobSize << " bytes\n";
        }

        printf("%s", output.null().get());
        return true;
    } else {
        String msg;
        msg << exe << ": " << archivePath << ": not found\n";
        fprintf(stderr, "%s", msg.null().get());
        return false;
    }
}

static Optional<StringView> getParentPath(StringView path) {
    Optional<size_t> sep = path.rfind('/');
    if (!sep) {
        return Optional<StringView>();
    } else {
        return Optional<StringView>(path.substr(0, sep));
    }
}

static void createDirs(StringView path) {
    Optional<StringView> parentPath = getParentPath(path);
    if (parentPath) {
        // Make sure parentPath's parent exists.
        createDirs(*parentPath);

        makeDirectory(*parentPath);
    }
}

static void putFile(String&& path, uint64_t size, void* data) {
    createDirs(path);

	// FIXME: Use native IO.
	FILE* f = fopen(path.null(), "w");
	if (!f) {
		// TODO: Propagate error up.
	}
	fwrite(data, size, 1, f);
	fclose(f);
}

static bool extractArchive(String&& archivePath) {
    Unique<PackReader> pack = PackReader::fromFile(archivePath);

    if (pack) {
        vector<PackReader::BlobIndex> blobIndicies;
        for (PackReader::BlobIndex i = 0; i < pack->size(); i++) {
            blobIndicies.push_back(i);
        }

        vector<void*> blobDatas = pack->getBlobDatas(blobIndicies);

        Unique<Pool> pool(Pool::makePool("extract", 1));

        for (PackReader::BlobIndex i = 0; i < pack->size(); i++) {
            StringView blobPath = pack->getBlobPath(i);
            uint64_t blobSize = pack->getBlobSize(i);
            void* blobData = blobDatas[i];

            pool->schedule([=] {
                uiShowExtractingFile(blobPath, blobSize);

                putFile(blobPath, blobSize, blobData);
            });
        }

        return true;
    } else {
        fprintf(stderr, "%s: %s: not found\n",
                exe.null().get(),
                archivePath.null().get());
        return false;
    }
}

int main(int argc, char* argv[]) {
    exe = argv[0];
    Optional<size_t> dir = exe.view().rfind(dirSeparator);
    if (dir) {
        exe = exe.view().substr(*dir + 1);
    }

    if (argc == 1) {
        usage();
        return 0;
    }
    if (argc == 2) {
        usage();
        return 1;
    }

    StringView command = argv[1];
    vector<StringView> args;

    for (int i = 2; i < argc; i++) {
        args.push_back(argv[i]);
    }

    int exitCode;

    if (command == "create") {
        // The first argument is the archive, the rest are files to add to it.
        StringView archivePath = args[0];
        args.erase(args.begin());
        return createArchive(archivePath, move_(args)) ? 0 : 1;
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
