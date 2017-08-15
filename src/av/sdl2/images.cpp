/*************************************
** Tsunagari Tile Engine            **
** images.cpp                       **
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

#include "av/sdl2/images.h"

#include <SDL2/SDL_image.h>

#include <limits.h>

#include "core/measure.h"
#include "core/resources.h"

#include "av/sdl2/window.h"


Images& Images::instance() {
    static auto globalImages = new SDL2Images;
    return *globalImages;
}


static Rc<Image> genImage(const std::string& path) {
    Unique<Resource> r = Resources::instance().load(path);
    if (!r) {
        // Error logged.
        return Rc<Image>();
    }

    assert_(r->size() < INT_MAX);

    SDL_RWops* ops = SDL_RWFromMem(const_cast<void*>(r->data()),
                                   static_cast<int>(r->size()));

    TimeMeasure m("Constructed " + path + " as image");
    SDL_Surface* surface = IMG_Load_RW(ops, 1);
    SDL_Renderer* renderer = SDL2GetRenderer();
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    int width;
    int height;
    SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);

    assert_(width <= 4096);
    assert_(height <= 4096);

    return Rc<Image>(new SDL2Image(texture, width, height));
}

static Rc<TiledImage> genTiledImage(const std::string& path,
                                    unsigned tileW, unsigned tileH) {
    assert_(tileW <= 4096);
    assert_(tileH <= 4096);

    Unique<Resource> r = Resources::instance().load(path);
    if (!r) {
        // Error logged.
        return Rc<TiledImage>();
    }

    assert_(r->size() < INT_MAX);

    SDL_RWops* ops = SDL_RWFromMem(const_cast<void*>(r->data()),
                                   static_cast<int>(r->size()));

    TimeMeasure m("Constructed " + path + " as image");
    SDL_Surface* surface = IMG_Load_RW(ops, 1);
    SDL_Renderer* renderer = SDL2GetRenderer();
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    int width;
    int height;
    SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);

    assert_(width <= 4096);
    assert_(height <= 4096);

    return Rc<TiledImage>(new SDL2TiledImage(texture,
                                             width, height, tileW, tileH));
}


SDL2Texture::SDL2Texture(SDL_Texture* texture) : texture(texture) {}

SDL2Texture::~SDL2Texture() {
    SDL_DestroyTexture(texture);
}


SDL2Image::SDL2Image(SDL_Texture* texture, int width, int height)
        : Image(width, height), texture(texture) {}

void SDL2Image::draw(double dstX, double dstY, double z) {}

void SDL2Image::drawSubrect(double dstX, double dstY, double z,
                            double srcX, double srcY,
                            double srcW, double srcH) {}


SDL2TiledSubImage::SDL2TiledSubImage(Rc<SDL2Texture> texture,
                                     int width, int height,
                                     int xOff, int yOff)
        : Image(width, height), xOff(xOff), yOff(yOff), texture(move_(texture)) {}

void SDL2TiledSubImage::draw(double dstX, double dstY, double z) {}

void SDL2TiledSubImage::drawSubrect(double dstX, double dstY, double z,
                                    double srcX, double srcY,
                                    double srcW, double srcH) {}


SDL2TiledImage::SDL2TiledImage(SDL_Texture* texture,
                               int width, int height, int tileW, int tileH)
        : width(width),
          height(height), tileW(tileW), tileH(tileH),
          numTiles((width / tileW) * (height / tileH)),
          texture(new SDL2Texture(texture)) {}

size_t SDL2TiledImage::size() const { return static_cast<size_t>(numTiles); }

Rc<Image> SDL2TiledImage::operator[](size_t n) const {
    int xOff = (tileW * n) % width;
    int yOff = (tileW * n) / width;
    Rc<Image>* image = new Rc<Image>(new SDL2TiledSubImage(texture,
                                                           tileW, tileH,
                                                           xOff, yOff));
    return *image;
}


SDL2Images::SDL2Images() : images(genImage) {}

Rc<Image> SDL2Images::load(const std::string& path) {
    return images.lifetimeRequest(path);
}

Rc<TiledImage> SDL2Images::loadTiles(const std::string& path,
                                     unsigned tileW, unsigned tileH) {
    auto tiledImage = tiledImages.lifetimeRequest(path);
    if (!tiledImage) {
        tiledImage = genTiledImage(path, tileW, tileH);
        tiledImages.lifetimePut(path, tiledImage);
    }
    return tiledImage;
}

void SDL2Images::garbageCollect() {
    images.garbageCollect();
    tiledImages.garbageCollect();
}
