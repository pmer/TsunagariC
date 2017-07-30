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

#include <limits.h>

#include "core/images.h"

class NullImage : public Image {
 public:
    NullImage() : Image(0, 0) {}

    void draw(double, double, double) {}
    void drawSubrect(double, double, double, double, double, double, double) {}
};


class NullTiledImage: public TiledImage {
 public:
    size_t size() const { return 1000; }

    Rc<Image> operator[](size_t) const {
        return Rc<Image>();
    }
};


class NullImages : public Images {
 public:
    Rc<Image> load(const std::string&) {
        return Rc<Image>();
    }

    Rc<TiledImage> loadTiles(const std::string&,
                                          unsigned, unsigned) {
        return Rc<TiledImage>();
    }

    void garbageCollect() {}
};


static NullImages globalImages;

Images& Images::instance() {
    return globalImages;
}
