/*************************************
** Tsunagari Tile Engine            **
** images.h                         **
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

#ifndef SRC_AV_SDL2_IMAGE_H_
#define SRC_AV_SDL2_IMAGE_H_

#include "core/images.h"

#include "cache/cache-template.cpp"
#include "cache/readercache.h"

typedef struct SDL_Texture SDL_Texture;

class SDL2Image : public Image {
 public:
    explicit SDL2Image(SDL_Texture* texture);
    ~SDL2Image();

    void draw(double dstX, double dstY, double z);
    void drawSubrect(double dstX, double dstY, double z,
                     double srcX, double srcY,
                     double srcW, double srcH);

    SDL_Texture* texture;
};


class SDL2TiledImage: public TiledImage {
 public:
    size_t size() const;

    Rc<Image> operator[](size_t n) const;
};


class SDL2Images : public Images {
 public:
    SDL2Images();

    Rc<Image> load(const std::string& path);

    Rc<TiledImage> loadTiles(const std::string& path,
        unsigned tileW, unsigned tileH);

    void garbageCollect();

 private:
    SDL2Images(const SDL2Images&) = delete;
    SDL2Images& operator=(const SDL2Images&) = delete;

    ReaderCache<Rc<Image>> images;
};

#endif  // SRC_AV_SDL2_IMAGE_H_
