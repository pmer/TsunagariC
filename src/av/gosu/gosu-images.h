/***************************************
** Tsunagari Tile Engine              **
** gosu-images.h                      **
** Copyright 2011-2015 Michael Reiley **
** Copyright 2011-2017 Paul Merrill   **
***************************************/

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

#ifndef SRC_AV_GOSU_IMAGES_H_
#define SRC_AV_GOSU_IMAGES_H_

#include <vector>

#include <Gosu/Image.hpp>

#include "cache/cache-template.cpp"
#include "cache/readercache.h"
#include "core/images.h"
#include "util/move.h"

class GosuImage : public Image {
 public:
    explicit GosuImage(Gosu::Image&& image);
    ~GosuImage() = default;

    void draw(double dstX, double dstY, double z);
    void drawSubrect(double dstX, double dstY, double z,
                     double srcX, double srcY,
                     double srcW, double srcH);

 private:
    Gosu::Image image;
};


class GosuTiledImage: public TiledImage {
 public:
    explicit GosuTiledImage(std::vector<Arc<Image>>&& images);
    ~GosuTiledImage() = default;

    size_t size() const;

    Arc<Image> operator[](size_t n) const;

 private:
    std::vector<Arc<Image>> images;
};


class GosuImages : public Images {
 public:
    GosuImages();
    ~GosuImages() = default;

    Arc<Image> load(const std::string& path);

    Arc<TiledImage> loadTiles(const std::string& path,
                              unsigned tileW, unsigned tileH);

    void garbageCollect();

 private:
    GosuImages(const GosuImages&) = delete;
    GosuImages& operator=(const GosuImages&) = delete;

    ReaderCache<Arc<Image>> images;
    // We can't use a ReaderCache here because TiledImages are constructed
    // with three arguments, but a ReaderCache only supports the use of
    // one.
    Cache<Arc<TiledImage>> tiledImages;
};

#endif
