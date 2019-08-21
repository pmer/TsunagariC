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

#include "os/c.h"
#include "os/mutex.h"
#include "os/os.h"
#include "pack/pack-reader.h"
#include "pack/pack-writer.h"
#include "pack/ui.h"
#include "pack/walker.h"
#include "util/int.h"
#include "util/move.h"
#include "util/noexcept.h"
#include "util/optional.h"
#include "util/string-view.h"
#include "util/string.h"
#include "util/unique.h"

static String exe;

static void
usage() noexcept {
    fprintf(stderr,
            "usage: %s create [-v] <output-archive> [input-file]...\n",
            exe.null().get());
    fprintf(stderr, "       %s list <input-archive>\n", exe.null().get());
    fprintf(stderr,
            "       %s extract [-v] <input-archive>\n",
            exe.null().get());
}

struct CreateArchiveContext {
    Unique<PackWriter> pack;
    Mutex packMutex;
};

static void
addFile(CreateArchiveContext& ctx, StringView path) noexcept {
    Optional<String> data = readFile(path);

    if (!data) {
        uiShowSkippedMissingFile(path);
        return;
    }

    String data_ = move_(*data);

    uiShowAddedFile(path, data_.size());

    // Write the file path to the pack file with '/' instead of '\\' on Windows.
    String standardizedPath;

    if (dirSeparator != '/') {
        standardizedPath = path;

        for (size_t i = 0; i < path.size; i++) {
            if (standardizedPath[i] == dirSeparator) {
                standardizedPath[i] = '/';
            }
        }

        path = standardizedPath;
    }

    LockGuard guard(ctx.packMutex);
    ctx.pack->addBlob(move_(path), static_cast<uint32_t>(data_.size()), data_.data());

    data_.reset_lose_memory();  // Don't delete data pointer.
}

static bool
createArchive(StringView archivePath, Vector<StringView> paths) noexcept {
    UI ui;

    CreateArchiveContext ctx;
    ctx.pack = PackWriter::make();

    walk(move_(paths), [&](StringView path) { addFile(ctx, path); });

    uiShowWritingArchive(archivePath);

    return ctx.pack->writeToFile(archivePath);
}

static bool
listArchive(StringView archivePath) noexcept {
    UI ui;

    Unique<PackReader> pack = PackReader::fromFile(archivePath);

    if (pack) {
        String output;

        for (PackReader::BlobIndex i = 0; i < pack->size(); i++) {
            StringView blobPath = pack->getBlobPath(i);
            uint64_t blobSize = pack->getBlobSize(i);

            // Print file paths with '\\' Windows.
            String standardizedPath;

            if (dirSeparator != '/') {
                standardizedPath = blobPath;

                for (size_t i = 0; i < blobPath.size; i++) {
                    if (standardizedPath[i] == '/') {
                        standardizedPath[i] = dirSeparator;
                    }
                }

                blobPath = standardizedPath;
            }

            output.append(blobPath.data, blobPath.size);
            output << ": " << blobSize << " bytes\n";
        }

        printf("%s", output.null().get());
        return true;
    }
    else {
        fprintf(stderr,
                "%s",
                (String() << exe << ": " << archivePath << ": not found\n")
                        .null()
                        .get());
        return false;
    }
}

static Optional<StringView>
getParentPath(StringView path) noexcept {
    StringPosition sep = path.rfind(dirSeparator);
    if (!sep) {
        return none;
    }
    else {
        return Optional<StringView>(path.substr(0, *sep));
    }
}

static void
createDirs(StringView path) noexcept {
    Optional<StringView> parentPath = getParentPath(path);
    if (parentPath) {
        // Make sure parentPath's parent exists.
        createDirs(*parentPath);

        makeDirectory(*parentPath);
    }
}

static void
putFile(StringView path, uint32_t size, void* data) noexcept {
    createDirs(path);

    // TODO: Propagate error up.
    writeFile(path, size, data);
}

static bool
extractArchive(StringView archivePath) noexcept {
    UI ui;

    Unique<PackReader> pack = PackReader::fromFile(archivePath);

    if (pack) {
        Vector<PackReader::BlobIndex> blobIndicies;
        for (PackReader::BlobIndex i = 0; i < pack->size(); i++) {
            blobIndicies.push_back(i);
        }

        Vector<void*> blobDatas = pack->getBlobDatas(blobIndicies);

        for (PackReader::BlobIndex i = 0; i < pack->size(); i++) {
            StringView blobPath = pack->getBlobPath(i);
            uint32_t blobSize = pack->getBlobSize(i);
            void* blobData = blobDatas[i];

            // Change file paths to use '\\' on Windows.
            String standardizedPath;

            if (dirSeparator != '/') {
                standardizedPath = blobPath;

                for (size_t i = 0; i < blobPath.size; i++) {
                    if (standardizedPath[i] == '/') {
                        standardizedPath[i] = dirSeparator;
                    }
                }

                blobPath = standardizedPath;
            }

            uiShowExtractingFile(blobPath, blobSize);

            putFile(blobPath, blobSize, blobData);
        }

        return true;
    }
    else {
        fprintf(stderr,
                "%s: %s: not found\n",
                exe.null().get(),
                String(archivePath).null().get());
        return false;
    }
}

int
main(int argc, char* argv[]) noexcept {
    exe = argv[0];
    StringPosition dir = exe.view().rfind(dirSeparator);
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
    Vector<StringView> args;

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
    }
    else if (command == "list") {
        verbose = true;

        if (args.size() != 1) {
            usage();
            return 1;
        }

        exitCode = listArchive(args[0]) ? 0 : 1;
    }
    else if (command == "extract") {
        if (args.size() > 0 && args[0] == "-v") {
            verbose = true;
            args.erase(args.begin());
        }

        if (args.size() != 1) {
            usage();
            return 1;
        }

        exitCode = extractArchive(args[0]) ? 0 : 1;
    }
    else {
        usage();
        return 1;
    }

    return exitCode;
}
