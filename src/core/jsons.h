/*************************************
** Tsunagari Tile Engine            **
** jsons.h                          **
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

#ifndef SRC_CORE_JSONS_H_
#define SRC_CORE_JSONS_H_

#include "util/rc.h"
#include "util/string-view.h"
#include "util/string.h"
#include "util/unique.h"
#include "util/vector.h"

class JSONArray;
class JSONObject;

class JSONObject {
 public:
    virtual ~JSONObject() = default;

    virtual Vector<StringView> names() noexcept = 0;

    virtual bool hasBool(StringView name) noexcept = 0;
    virtual bool hasInt(StringView name) noexcept = 0;
    virtual bool hasUnsigned(StringView name) noexcept = 0;
    virtual bool hasFloat(StringView name) noexcept = 0;
    virtual bool hasString(StringView name) noexcept = 0;
    virtual bool hasObject(StringView name) noexcept = 0;
    virtual bool hasArray(StringView name) noexcept = 0;

    virtual bool hasStringFloat(StringView name) noexcept = 0;

    virtual bool boolAt(StringView name) noexcept = 0;
    virtual int intAt(StringView name) noexcept = 0;
    virtual int intAt(StringView name,
                      int lowerBound,
                      int upperBound) noexcept = 0;
    virtual unsigned unsignedAt(StringView name) noexcept = 0;
    virtual float floatAt(StringView name) noexcept = 0;
    virtual StringView stringAt(StringView name) noexcept = 0;
    virtual Unique<JSONObject> objectAt(StringView name) noexcept = 0;
    virtual Unique<JSONArray> arrayAt(StringView name) noexcept = 0;

    virtual float stringFloatAt(StringView name) noexcept = 0;
};

class JSONArray {
 public:
    virtual ~JSONArray() = default;

    virtual size_t size() noexcept = 0;

    virtual bool isBool(size_t index) noexcept = 0;
    virtual bool isInt(size_t index) noexcept = 0;
    virtual bool isUnsigned(size_t index) noexcept = 0;
    virtual bool isFloat(size_t index) noexcept = 0;
    virtual bool isString(size_t index) noexcept = 0;
    virtual bool isObject(size_t index) noexcept = 0;
    virtual bool isArray(size_t index) noexcept = 0;

    virtual bool boolAt(size_t index) noexcept = 0;
    virtual int intAt(size_t index) noexcept = 0;
    virtual unsigned unsignedAt(size_t index) noexcept = 0;
    virtual float floatAt(size_t index) noexcept = 0;
    virtual StringView stringAt(size_t index) noexcept = 0;
    virtual Unique<JSONObject> objectAt(size_t index) noexcept = 0;
    virtual Unique<JSONArray> arrayAt(size_t index) noexcept = 0;
};

class JSONs {
 public:
    //! Load a JSON document.
    static Rc<JSONObject> load(StringView path) noexcept;

    //! Parse a document from the outside world.
    static Unique<JSONObject> parse(String data) noexcept;

    //! Free JSON documents not recently used.
    static void garbageCollect() noexcept;
};

#endif  // SRC_CORE_JSONS_H_
