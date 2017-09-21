/********************************
** Tsunagari Tile Engine       **
** walker.cpp                  **
** Copyright 2017 Paul Merrill **
********************************/

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

#include "pack/walker.h"

#include <algorithm>

#include "os/os.h"
#include "pack/pool.h"
#include "util/unique.h"

struct WalkContext {
    Unique<Pool> pool;
    std::function<void(std::string)> op;
};

static void walkPath(WalkContext& ctx, std::string path) {
    if (isDir(path)) {
        vector<std::string> names = listDir(path);
        std::sort(names.begin(), names.end());
        for (auto& name : names) {
            std::string child = path + dirSeparator + name;
            ctx.pool->schedule([&ctx, child] {
                walkPath(ctx, child);
            });
        }
    } else {
        ctx.op(path);
    }
}

void walk(vector<std::string> paths, std::function<void(std::string)> op) {
    WalkContext ctx{
        Pool::makePool("walk"),
        move_(op)
    };

    for (auto& path : paths) {
        ctx.pool->schedule([&] {
            walkPath(ctx, move_(path));
        });
    }
}
