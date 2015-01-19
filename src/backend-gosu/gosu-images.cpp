/***************************************
** Tsunagari Tile Engine              **
** gosu-images.cpp                    **
** Copyright 2011-2015 PariahSoft LLC **
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

#include <Gosu/Bitmap.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Image.hpp>
#include <Gosu/IO.hpp>

#include "gosu-images.h"
#include "gosu-window.h"
#include "../reader.h"
#include "../window.h"

static Gosu::Graphics& graphics()
{
	static GameWindow& window = GameWindow::instance();
	static GosuGameWindow& gosu = (GosuGameWindow&)window;
	return gosu.graphics();
}


GosuImage::GosuImage(Gosu::Image&& image)
	: image(std::move(image))
{
}

void GosuImage::draw(double dstX, double dstY, double z)
{
	image.draw(dstX, dstY, z);
}

void GosuImage::drawSubrect(double dstX, double dstY, double z,
		 double srcX, double srcY,
		 double srcW, double srcH)
{
	static Gosu::Graphics& g = graphics();
	g.beginClipping(dstX + srcX, dstY + srcY, srcW, srcH);
	draw(dstX, dstY, z);
	g.endClipping();
}

unsigned GosuImage::width() const
{
	return image.width();
}

unsigned GosuImage::height() const
{
	return image.height();
}


GosuTiledImage::GosuTiledImage(std::vector<std::shared_ptr<Image>>&& images)
	: images(std::move(images))
{
}

size_t GosuTiledImage::size() const
{
	return images.size();
}

const std::shared_ptr<Image>& GosuTiledImage::operator[](size_t n) const
{
	return images[n];
}


static GosuImages globalImages;

Images& Images::instance()
{
	return globalImages;
}

static std::shared_ptr<Image> genImage(const std::string& path)
{
	static Gosu::Graphics& g = graphics();
	std::unique_ptr<Gosu::Buffer> buffer(Reader::readBuffer(path));
	if (!buffer) {
		// Error logged.
		return std::shared_ptr<Image>();
	}
	Gosu::Bitmap bitmap;
	Gosu::loadImageFile(bitmap, buffer->frontReader());
	return std::make_shared<GosuImage>(std::move(Gosu::Image(g, bitmap, false)));
}

static std::shared_ptr<TiledImage> genTiledImage(const std::string& path,
	unsigned tileW, unsigned tileH)
{
	std::shared_ptr<Gosu::Buffer> buffer(Reader::readBuffer(path));
	if (!buffer) {
		// Error logged.
		return std::shared_ptr<TiledImage>();
	}
	Gosu::Bitmap bitmap;
	Gosu::loadImageFile(bitmap, buffer->frontReader());
	std::vector<std::shared_ptr<Image>> images;
	for (unsigned y = 0; y < bitmap.height(); y += tileH) {
		for (unsigned x = 0; x < bitmap.width(); x += tileW) {
			images.emplace_back(std::make_shared<GosuImage>(
				std::move(Gosu::Image(
					graphics(), bitmap, x, y, tileW, tileH, false
				))
			));
		}
	}
	return std::make_shared<GosuTiledImage>(std::move(images));
}


GosuImages::GosuImages()
	: images(genImage)
{
}

std::shared_ptr<Image> GosuImages::load(const std::string& path)
{
	return images.lifetimeRequest(path);
}

std::shared_ptr<TiledImage> GosuImages::loadTiles(const std::string& path,
	unsigned tileW, unsigned tileH)
{
	auto tiledImage = tiledImages.lifetimeRequest(path);
	if (!tiledImage) {
		tiledImage = genTiledImage(path, tileW, tileH);
		tiledImages.lifetimePut(path, tiledImage);
	}
	return tiledImage;
}

void GosuImages::garbageCollect()
{
	images.garbageCollect();
	tiledImages.garbageCollect();
}
