/********************************
** Tsunagari Tile Engine       **
** images.cpp                  **
** Copyright 2016 Paul Merrill **
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

#include "av/sdl2/images.h"

static SDL2Images globalImages;

Images& Images::instance() {
    return globalImages;
}


void SDL2Image::draw(double dstX, double dstY, double z) {}
void SDL2Image::drawSubrect(double dstX, double dstY, double z,
                 double srcX, double srcY,
                 double srcW, double srcH) {}

unsigned SDL2Image::width() const { return 16; }
unsigned SDL2Image::height() const { return 16; }

size_t SDL2TiledImage::size() const { return 500; }

const std::shared_ptr<Image>& SDL2TiledImage::operator[](size_t n) const {
    std::shared_ptr<Image>* image = new std::shared_ptr<Image>(new SDL2Image);
    return *image;
}


std::shared_ptr<Image> SDL2Images::load(const std::string& path) {
    std::shared_ptr<Image>* image = new std::shared_ptr<Image>(new SDL2Image);
    return *image;
}

std::shared_ptr<TiledImage> SDL2Images::loadTiles(const std::string& path,
                                      unsigned tileW, unsigned tileH) {
    std::shared_ptr<TiledImage>* image = new std::shared_ptr<TiledImage>(new SDL2TiledImage);
    return *image;
}

void SDL2Images::garbageCollect() {}
