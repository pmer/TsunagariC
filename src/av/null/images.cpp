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

#include "core/images.h"

ImageID Images::load(StringView path) noexcept { return ImageID(0); }
TiledImageID Images::loadTiles(StringView path,
                               int tileWidth,
                               int tileHeight) noexcept {
    return TiledImageID(0);
}
void Images::prune(time_t latestPermissibleUse) noexcept {}

int TiledImage::size(TiledImageID tiid) noexcept { return 1000; }
ImageID TiledImage::getTile(TiledImageID tiid, int i) noexcept {
    return ImageID(0);
}
void TiledImage::release(TiledImageID tiid) noexcept {}

void Image::draw(ImageID iid, float x, float y, float z) noexcept {}
int Image::width(ImageID iid) noexcept { return 1; }
int Image::height(ImageID iid) noexcept { return 1; }
void Image::release(ImageID iid) noexcept {}
