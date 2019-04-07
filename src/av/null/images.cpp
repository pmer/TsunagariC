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

class NullImage : public Image {
 public:
    NullImage() noexcept : Image(0, 0) {}

    void draw(double, double, double) noexcept final {}
    void drawSubrect(double,
                     double,
                     double,
                     double,
                     double,
                     double,
                     double) noexcept final {}
};


class NullTiledImage : public TiledImage {
 public:
    size_t size() const noexcept final { return 1000; }

    Rc<Image> operator[](size_t) const noexcept final {
        return Rc<Image>(new NullImage);
    }
};


class NullImages : public Images {
 public:
    Rc<Image> load(StringView) noexcept final { return Rc<Image>(); }

    Rc<TiledImage> loadTiles(StringView, unsigned, unsigned) noexcept final {
        return Rc<TiledImage>(new NullTiledImage);
    }

    void garbageCollect() noexcept final {}
};


static NullImages globalImages;

Images&
Images::instance() noexcept {
    return globalImages;
}
