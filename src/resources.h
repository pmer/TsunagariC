/***************************************
** Tsunagari Tile Engine              **
** resources.h                        **
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

#ifndef RESOURCES_H
#define RESOURCES_H

#include <memory>
#include <string>

class Resource
{
public:
    virtual ~Resource() = default;

    virtual const void* data() = 0;
    virtual size_t size() = 0;

    const std::string asString();

protected:
    Resource() = default;

private:
    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;
};


/**
 * Provides data and resource extraction for a World.
 * Each World comes bundled with associated data.
 */
class Resources
{
public:
    //! Acquire the global Resources object.
    static Resources& instance();

    virtual ~Resources() = default;

    //! Load a resource from the file at the given path.
    //! Returns NULL if the resource does not exist.
    virtual std::unique_ptr<Resource> load(const std::string& path) = 0;

protected:
    Resources() = default;

private:
    Resources(const Resources&) = delete;
    Resources(Resources&&) = delete;
    Resources& operator=(const Resources&) = delete;
    Resources& operator=(Resources&&) = delete;
};

#endif
