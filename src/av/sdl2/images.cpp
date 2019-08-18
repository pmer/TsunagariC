/*************************************
** Tsunagari Tile Engine            **
** images.cpp                       **
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

#include "av/sdl2/sdl2.h"
#include "av/sdl2/window.h"
#include "core/images.h"
#include "core/measure.h"
#include "core/resources.h"
#include "core/world.h"
#include "util/hashtable.h"
#include "util/int.h"
#include "util/noexcept.h"
#include "util/pool.h"
#include "util/string-view.h"
#include "util/string.h"

struct SDL2TiledImage {
    int numUsers = 0;
    time_t lastUse = 0;

    SDL_Texture* texture = nullptr;
    int width = 0;
    int height = 0;
    int tileWidth = 0;
    int tileHeight = 0;
    int numTiles = 0;
};

static bool operator==(const SDL2TiledImage& a,
                       const SDL2TiledImage& b) noexcept {
    return a.texture == b.texture;
}

struct SDL2Image {
    enum { STANDALONE, FROM_TILED_IMAGE } origin = STANDALONE;

    int numUsers = 0;
    time_t lastUse = 0;

    SDL_Texture* texture = nullptr;
    int width = 0;
    int height = 0;
    int xoff = 0;  // Offset into texture if this is a part of a TiledImage.
    int yoff = 0;  // Offset into texture if this is a part of a TiledImage.
};

static bool operator==(const SDL2Image& a, const SDL2Image& b) noexcept {
    return a.texture == b.texture &&
           a.xoff == b.xoff &&
           a.yoff == b.yoff;
}

static Hashmap<String, ImageID> imageIDs;
static Hashmap<String, TiledImageID> tiledImageIDs;
static Pool<SDL2Image> imagePool;
static Pool<SDL2TiledImage> tiledImagePool;

static SDL2Image
makeImage(StringView path) {
    Optional<StringView> r = resourceLoad(path);
    if (!r) {
        // Error logged.
        return SDL2Image();
    }

    assert_(r->size < UINT32_MAX);

    SDL_RWops* ops =
            SDL_RWFromMem(static_cast<void*>(const_cast<char*>(r->data)),
                          static_cast<int>(r->size));

    SDL_Texture* texture;

    {
        TimeMeasure m(String() << "Constructed " << path << " as image");
        // FIXME: Replace with IMG_LoadTexture_RW
        SDL_Surface* surface = IMG_Load_RW(ops, 1);
        if (!surface) {
            SDL_FreeSurface(surface);
            return SDL2Image();
        }
        SDL_Renderer* renderer = SDL2GameWindow::renderer;
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (!texture) {
            // TODO: SDL_FreeSurace(surface); ??
            return SDL2Image();
        }
    }

    int width;
    int height;
    SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);

    assert_(width <= 4096);
    assert_(height <= 4096);

    int iid = imagePool.allocate();
    SDL2Image& image = imagePool[iid];
    image.origin = SDL2Image::STANDALONE;
    image.numUsers = 1;
    image.lastUse = 0;
    image.texture = texture;
    image.width = width;
    image.height = height;
    image.xoff = 0;
    image.yoff = 0;

    return image;
}

static SDL2TiledImage
makeTiledImage(StringView path, int tileWidth, int tileHeight) {
    assert_(tileWidth <= 4096);
    assert_(tileHeight <= 4096);

    Optional<StringView> r = resourceLoad(path);
    if (!r) {
        // Error logged.
        return SDL2TiledImage();
    }

    assert_(r->size < UINT32_MAX);

    SDL_RWops* ops =
            SDL_RWFromMem(static_cast<void*>(const_cast<char*>(r->data)),
                          static_cast<int>(r->size));

    SDL_Texture* texture;

    {
        TimeMeasure m(String() << "Constructed " << path << " as image");
        SDL_Surface* surface = IMG_Load_RW(ops, 1);
        if (!surface) {
            SDL_FreeSurface(surface);
            return SDL2TiledImage();
        }
        SDL_Renderer* renderer = SDL2GameWindow::renderer;
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (!texture) {
            // TODO: SDL_FreeSurace(surface); ??
            return SDL2TiledImage();
        }
    }

    int width;
    int height;
    SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);

    assert_(width <= 4096);
    assert_(height <= 4096);

    int tiid = tiledImagePool.allocate();
    SDL2TiledImage& ti = tiledImagePool[tiid];
    ti.numUsers = 1;
    ti.lastUse = 0;
    ti.texture = texture;
    ti.width = width;
    ti.height = height;
    ti.tileWidth = tileWidth;
    ti.tileHeight = tileHeight;
    ti.numTiles = (width / tileWidth) * (height / tileHeight);

    return ti;
}

ImageID Images::load(StringView path) noexcept {
    Optional<ImageID*> cachedId = imageIDs.tryAt(path);
    if (cachedId) {
        int iid = ***cachedId;
        SDL2Image& image = imagePool[iid];
        image.numUsers += 1;
        return ImageID(iid);
    }

    SDL2Image image = makeImage(path);
    if (image == SDL2Image()) {
        imageIDs[path] = mark;
        return mark;
    }

    int iid = imagePool.allocate();
    imagePool[iid] = image;

    imageIDs[path] = iid;

    return ImageID(iid);
}

TiledImageID Images::loadTiles(StringView path,
                               int tileWidth,
                               int tileHeight) noexcept {
    Optional<TiledImageID*> cachedId = tiledImageIDs.tryAt(path);
    if (cachedId) {
        int tiid = ***cachedId;
        SDL2TiledImage& tiledImage = tiledImagePool[tiid];
        tiledImage.numUsers += 1;
        return TiledImageID(tiid);
    }

    SDL2TiledImage tiledImage = makeTiledImage(path, tileWidth, tileHeight);
    if (tiledImage == SDL2TiledImage()) {
        tiledImageIDs[path] = mark;
        return mark;
    }

    int tiid = tiledImagePool.allocate();
    tiledImagePool[tiid] = tiledImage;

    tiledImageIDs[path] = tiid;

    return TiledImageID(tiid);
}

void Images::prune(time_t latestPermissibleUse) noexcept {
    // TODO
}

int TiledImage::size(TiledImageID tiid) noexcept {
    assert_(tiid);

    SDL2TiledImage& tiledImage = tiledImagePool[*tiid];
    return tiledImage.numTiles;
}

ImageID TiledImage::getTile(TiledImageID tiid, int i) noexcept {
    assert_(tiid);

    SDL2TiledImage& ti = tiledImagePool[*tiid];

    int xoff = ti.tileWidth * static_cast<int>(i) % ti.width;
    int yoff = ti.tileWidth * static_cast<int>(i) / ti.width * ti.tileHeight;

    int iid = imagePool.allocate();
    SDL2Image& image = imagePool[iid];
    image.origin = SDL2Image::FROM_TILED_IMAGE;
    image.numUsers = 0;
    image.lastUse = 0;
    image.texture = ti.texture;
    image.width = ti.tileWidth;
    image.height = ti.tileHeight;
    image.xoff = xoff;
    image.yoff = yoff;

    return ImageID(iid);
}

void TiledImage::release(TiledImageID tiid) noexcept {
    if (!tiid) {
        return;
    }

    SDL2TiledImage& tiledImage = tiledImagePool[*tiid];

    tiledImage.numUsers -= 1;
    assert_(tiledImage.numUsers >= 0);

    if (tiledImage.numUsers == 0) {
        tiledImage.lastUse = World::time();
    }
}

void Image::draw(ImageID iid, float x, float y, float z) noexcept {
    assert_(iid);

    SDL2Image& i = imagePool[*iid];

    SDL_Renderer* renderer = SDL2GameWindow::renderer;
    rvec2 translation = SDL2GameWindow::translation;
    rvec2 scaling = SDL2GameWindow::scaling;

    SDL_Rect src{i.xoff, i.yoff, i.width, i.height};
    SDL_Rect dst{static_cast<int>((x + translation.x) * scaling.x),
                 static_cast<int>((y + translation.y) * scaling.y),
                 static_cast<int>(i.width * scaling.x),
                 static_cast<int>(i.height * scaling.y)};
    SDL_RenderCopy(renderer, i.texture, &src, &dst);
}

int Image::width(ImageID iid) noexcept {
    assert_(iid);

    SDL2Image& image = imagePool[*iid];
    return image.width;
}

int Image::height(ImageID iid) noexcept {
    assert_(iid);

    SDL2Image& image = imagePool[*iid];
    return image.height;
}

void Image::release(ImageID iid) noexcept {
    if (!iid) {
        return;
    }

    SDL2Image& image = imagePool[*iid];

    if (image.origin == SDL2Image::FROM_TILED_IMAGE) {
        imagePool.release(iid);
    }
    else {
        image.numUsers -= 1;
        assert_(image.numUsers >= 0);

        if (image.numUsers == 0) {
            image.lastUse = World::time();
        }
    }
}
