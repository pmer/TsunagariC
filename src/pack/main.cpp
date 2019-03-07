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

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <mutex>

#include "os/os.h"

#include "pack/pack-reader.h"
#include "pack/pack-writer.h"
#include "pack/walker.h"
#include "pack/ui.h"

#include "util/move.h"
#include "util/optional.h"
#include "util/string-view.h"
#include "util/string.h"
#include "util/unique.h"

static String exe;

static void usage() {
    fprintf(stderr, "usage: %s create [-v] <output-archive> [input-file]...\n",
            exe.null().get());
    fprintf(stderr, "       %s list <input-archive>\n",
            exe.null().get());
    fprintf(stderr, "       %s extract [-v] <input-archive>\n",
            exe.null().get());
}

struct CreateArchiveContext {
    Unique<PackWriter> pack;
    std::mutex packMutex;
};

static void addFile(CreateArchiveContext& ctx, StringView path) {
    Optional<String> data = readFile(path);

    if (!data) {
        uiShowSkippedMissingFile(path);
        return;
    }
    
    String data_ = move_(*data);

    uiShowAddedFile(path, data_.size());

    std::lock_guard<std::mutex> guard(ctx.packMutex);
    ctx.pack->addBlob(move_(path), data_.size(), data_.data());

    data_.reset_lose_memory();  // Don't delete data pointer.
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

static void putFile(StringView path, uint64_t size, void* data) {
    createDirs(path);

    // TODO: Propagate error up.
    writeFile(path, size, data);
}

static bool extractArchive(StringView archivePath) {
    Unique<PackReader> pack = PackReader::fromFile(archivePath);

    if (pack) {
        vector<PackReader::BlobIndex> blobIndicies;
        for (PackReader::BlobIndex i = 0; i < pack->size(); i++) {
            blobIndicies.push_back(i);
        }

        vector<void*> blobDatas = pack->getBlobDatas(blobIndicies);

        for (PackReader::BlobIndex i = 0; i < pack->size(); i++) {
            StringView blobPath = pack->getBlobPath(i);
            uint64_t blobSize = pack->getBlobSize(i);
            void* blobData = blobDatas[i];

            uiShowExtractingFile(blobPath, blobSize);

            putFile(blobPath, blobSize, blobData);
        }

        return true;
    } else {
        fprintf(stderr, "%s: %s: not found\n",
                exe.null().get(),
                String(archivePath).null().get());
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
        if (args.size() > 0 && args[0] == "-v") {
            verbose = true;
            args.erase(args.begin());
        }
        
        if (args.size() < 2) {
            usage();
            return 1;
        }

        // The first argument is the archive, the rest are files to add to it.
        StringView archivePath = args[0];
        args.erase(args.begin());

        return createArchive(archivePath, move_(args)) ? 0 : 1;
    } else if (command == "list") {
        verbose = true;

        if (args.size() != 1) {
            usage();
            return 1;
        }

        exitCode = listArchive(args[0]) ? 0 : 1;
    } else if (command == "extract") {
        if (args.size() > 0 && args[0] == "-v") {
            verbose = true;
            args.erase(args.begin());
        }
        
        if (args.size() != 1) {
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
