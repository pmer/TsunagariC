/***************************************
** Tsunagari Tile Engine              **
** gosu-images.cpp                    **
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

#include "av/gosu/gosu-images.h"

#include <Gosu/Bitmap.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Image.hpp>
#include <core/formatter.h>

#include "av/gosu/gosu-cbuffer.h"
#include "av/gosu/gosu-window.h"
#include "core/formatter.h"
#include "core/images.h"
#include "core/measure.h"
#include "core/resources.h"
#include "core/window.h"

#ifdef BACKEND_GOSU
static GosuImages globalImages;

Images& Images::instance() {
    return globalImages;
}
#endif

static Gosu::Graphics& graphics() {
    static GameWindow& window = GameWindow::instance();
    static GosuGameWindow& gosu = (GosuGameWindow&)window;
    return gosu.graphics();
}


GosuImage::GosuImage(Gosu::Image&& image)
        : Image(image.width(), image.height()),
          image(move_(image)) {}

void GosuImage::draw(double dstX, double dstY, double z) {
    image.draw(dstX, dstY, z);
}

void GosuImage::drawSubrect(double dstX, double dstY, double z,
                            double srcX, double srcY,
                            double srcW, double srcH) {
    static Gosu::Graphics& g = graphics();
    g.clip_to(dstX + srcX, dstY + srcY, srcW, srcH, [=] {
        draw(dstX, dstY, z);
    });
}


GosuTiledImage::GosuTiledImage(std::vector<Arc<Image>>&& images)
        : images(move_(images)) {}

size_t GosuTiledImage::size() const {
    return images.size();
}

Arc<Image> GosuTiledImage::operator[](size_t n) const {
    return images[n];
}


static Arc<Image> genImage(const std::string& path) {
    std::unique_ptr<Resource> r = Resources::instance().load(path);
    if (!r) {
        // Error logged.
        return Arc<Image>();
    }
    GosuCBuffer buffer(r->data(), r->size());
    Gosu::Bitmap bitmap;

    {
        TimeMeasure m("Bitmapped " + path);
        Gosu::load_image_file(bitmap, buffer.front_reader());
    }

    Log::info("Images", Formatter("Bitmap " + path + " is %×%") % bitmap.width() % bitmap.height());

    TimeMeasure m("Constructed " + path + " as image");
    return Arc<Image>(new GosuImage(Gosu::Image(bitmap, Gosu::IF_TILEABLE)));
}

static Arc<TiledImage> genTiledImage(const std::string& path,
                                     unsigned tileW, unsigned tileH) {
    std::unique_ptr<Resource> r = Resources::instance().load(path);
    if (!r) {
        // Error logged.
        return Arc<TiledImage>();
    }
    GosuCBuffer buffer(r->data(), r->size());
    Gosu::Bitmap bitmap;

    {
        TimeMeasure m("Bitmapped " + path);
        Gosu::load_image_file(bitmap, buffer.front_reader());
    }

    Log::info("Images", Formatter("Bitmap " + path + " is %×%") % bitmap.width() % bitmap.height());

    TimeMeasure m("Constructed " + path + " as tiles");
    std::vector<Arc<Image>> images;
    for (unsigned y = 0; y < bitmap.height(); y += tileH) {
        for (unsigned x = 0; x < bitmap.width(); x += tileW) {
            images.emplace_back(Arc<Image>(new GosuImage(
                Gosu::Image(bitmap, x, y, tileW, tileH, Gosu::IF_TILEABLE)
            )));
        }
    }

    Log::info("Images", Formatter("TiledImage " + path + " has % tiles") % images.size());
    return Arc<TiledImage>(new GosuTiledImage(move_(images)));
}


GosuImages::GosuImages() : images(genImage) {}

Arc<Image> GosuImages::load(const std::string& path) {
    return images.lifetimeRequest(path);
}

Arc<TiledImage> GosuImages::loadTiles(const std::string& path,
                                      unsigned tileW, unsigned tileH) {
    auto tiledImage = tiledImages.lifetimeRequest(path);
    if (!tiledImage) {
        tiledImage = genTiledImage(path, tileW, tileH);
        tiledImages.lifetimePut(path, tiledImage);
    }
    return tiledImage;
}

void GosuImages::garbageCollect() {
    images.garbageCollect();
    tiledImages.garbageCollect();
}
