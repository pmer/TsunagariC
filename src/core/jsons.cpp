/*************************************
** Tsunagari Tile Engine            **
** jsons.cpp                        **
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

#define RAPIDJSON_NOMEMBERITERATORCLASS
#define RAPIDJSON_NO_INT64DEFINE

#ifdef _MSC_VER
#define RAPIDJSON_SSE42
#endif

// clang-format off
#include "util/assert.h"
#include "util/int.h"
// clang-format on

#define RAPIDJSON_ASSERT assert_
#define RAPIDJSON_HAS_CXX11_NOEXCEPT 1

#include "rapidjson/document.h"
#include "rapidjson/reader.h"

// Skip later headers that conflict with ones that rapidjson includes.
// clang-format off
#define SRC_OS_C_H_
#define SRC_UTIL_NEW_H_
#include <math.h>
#include <time.h>
// clang-format on

#include "core/jsons.h"

#include "cache/rc-cache-impl.h"
#include "cache/rc-reader-cache.h"
#include "core/log.h"
#include "core/measure.h"
#include "core/resources.h"
#include "util/move.h"
#include "util/string2.h"

#define CHECK(x)      \
    if (!(x)) {       \
        return false; \
    }

typedef rapidjson::Document RJDocument;
typedef RJDocument::ValueType RJValue;
typedef RJDocument::ValueType::Object RJObject;
typedef RJDocument::ValueType::Array RJArray;

class JSONObjectImpl : public JSONObject {
 public:
    virtual ~JSONObjectImpl() override = default;

    Vector<StringView> names() noexcept final;

    bool hasBool(StringView name) noexcept final;
    bool hasInt(StringView name) noexcept final;
    bool hasUnsigned(StringView name) noexcept final;
    bool hasFloat(StringView name) noexcept final;
    bool hasString(StringView name) noexcept final;
    bool hasObject(StringView name) noexcept final;
    bool hasArray(StringView name) noexcept final;

    bool hasStringFloat(StringView name) noexcept final;

    bool boolAt(StringView name) noexcept final;
    int intAt(StringView name) noexcept final;
    int intAt(StringView name, int lowerBound, int upperBound) noexcept final;
    unsigned unsignedAt(StringView name) noexcept final;
    float floatAt(StringView name) noexcept final;
    StringView stringAt(StringView name) noexcept final;
    Unique<JSONObject> objectAt(StringView name) noexcept final;
    Unique<JSONArray> arrayAt(StringView name) noexcept final;

    float stringFloatAt(StringView name) noexcept final;

 protected:
    RJValue str(StringView name) noexcept;
    virtual RJObject get() noexcept = 0;
};

class JSONObjectReal : public JSONObjectImpl {
 public:
    explicit JSONObjectReal(RJObject object) noexcept;

 protected:
    RJObject get() noexcept final;

 private:
    RJObject object;
};

class JSONArrayImpl : public JSONArray {
 public:
    explicit JSONArrayImpl(RJArray array) noexcept;

    size_t size() noexcept final;

    bool isBool(size_t index) noexcept final;
    bool isInt(size_t index) noexcept final;
    bool isUnsigned(size_t index) noexcept final;
    bool isFloat(size_t index) noexcept final;
    bool isString(size_t index) noexcept final;
    bool isObject(size_t index) noexcept final;
    bool isArray(size_t index) noexcept final;

    bool boolAt(size_t index) noexcept final;
    int intAt(size_t index) noexcept final;
    unsigned unsignedAt(size_t index) noexcept final;
    float floatAt(size_t index) noexcept final;
    StringView stringAt(size_t index) noexcept final;
    Unique<JSONObject> objectAt(size_t index) noexcept final;
    Unique<JSONArray> arrayAt(size_t index) noexcept final;

 private:
    RJArray::PlainType& at(size_t index) noexcept;

 private:
    RJArray array;
};


class JSONDocImpl : public JSONObjectImpl {
 public:
    explicit JSONDocImpl(String json) noexcept;

    bool isValid() noexcept;

 protected:
    RJObject get() noexcept final;

 private:
    String json;
    RJDocument document;
};

Rc<JSONObject> genJSON(StringView path) noexcept;

Vector<StringView>
JSONObjectImpl::names() noexcept {
    Vector<StringView> names;

    names.reserve(get().MemberCount());

    for (auto& property : get()) {
        auto& name = property.name;
        if (name.IsString()) {
            names.push_back(name.GetString());
        }
    }

    return names;
}

template<typename T>
static bool
isStringFloat(const T& val) noexcept {
    CHECK(val.IsString());
    StringView str = val.GetString();
    return parseFloat(str);
}

template<typename T>
static float
stringFloat(const T& val) noexcept {
    StringView str = val.GetString();
    Optional<float> d = parseFloat(str);
    return *d;
}


bool
JSONObjectImpl::hasBool(StringView name) noexcept {
    return get().HasMember(str(name)) && get()[str(name)].IsBool();
}
bool
JSONObjectImpl::hasInt(StringView name) noexcept {
    return get().HasMember(str(name)) && get()[str(name)].IsInt();
}
bool
JSONObjectImpl::hasUnsigned(StringView name) noexcept {
    return get().HasMember(str(name)) && get()[str(name)].IsUint();
}
bool
JSONObjectImpl::hasFloat(StringView name) noexcept {
    return get().HasMember(str(name)) && get()[str(name)].IsDouble();
}
bool
JSONObjectImpl::hasString(StringView name) noexcept {
    return get().HasMember(str(name)) && get()[str(name)].IsString();
}
bool
JSONObjectImpl::hasObject(StringView name) noexcept {
    return get().HasMember(str(name)) && get()[str(name)].IsObject();
}
bool
JSONObjectImpl::hasArray(StringView name) noexcept {
    return get().HasMember(str(name)) && get()[str(name)].IsArray();
}
bool
JSONObjectImpl::hasStringFloat(StringView name) noexcept {
    return get().HasMember(str(name)) && isStringFloat(get()[str(name)]);
}

bool
JSONObjectImpl::boolAt(StringView name) noexcept {
    return get()[str(name)].GetBool();
}
int
JSONObjectImpl::intAt(StringView name) noexcept {
    return get()[str(name)].GetInt();
}
int
JSONObjectImpl::intAt(StringView name,
                      int lowerBound,
                      int upperBound) noexcept {
    int i = get()[str(name)].GetInt();
    if (i < lowerBound) {
        Log::fatal("JSONObject::intAt", "Value out of range");
    }
    if (i > upperBound) {
        Log::fatal("JSONObject::intAt", "Value out of range");
    }
    return i;
}
unsigned
JSONObjectImpl::unsignedAt(StringView name) noexcept {
    return get()[str(name)].GetUint();
}
float
JSONObjectImpl::floatAt(StringView name) noexcept {
    return static_cast<float>(get()[str(name)].GetDouble());
}
StringView
JSONObjectImpl::stringAt(StringView name) noexcept {
    return get()[str(name)].GetString();
}
Unique<JSONObject>
JSONObjectImpl::objectAt(StringView name) noexcept {
    return Unique<JSONObject>(new JSONObjectReal(get()[str(name)].GetObject()));
}
Unique<JSONArray>
JSONObjectImpl::arrayAt(StringView name) noexcept {
    return Unique<JSONArray>(new JSONArrayImpl(get()[str(name)].GetArray()));
}
float
JSONObjectImpl::stringFloatAt(StringView name) noexcept {
    return stringFloat(get()[str(name)]);
}

RJValue
JSONObjectImpl::str(StringView name) noexcept {
    return RJValue(rapidjson::StringRef(name.data, name.size));
}


JSONObjectReal::JSONObjectReal(RJObject object) noexcept
        : object(move_(object)) {}

RJObject
JSONObjectReal::get() noexcept {
    return object;
}


JSONArrayImpl::JSONArrayImpl(RJArray array) noexcept : array(move_(array)) {}

size_t
JSONArrayImpl::size() noexcept {
    return array.Size();
}

bool
JSONArrayImpl::isBool(size_t index) noexcept {
    return at(index).IsBool();
}
bool
JSONArrayImpl::isInt(size_t index) noexcept {
    return at(index).IsInt();
}
bool
JSONArrayImpl::isUnsigned(size_t index) noexcept {
    return at(index).IsUint();
}
bool
JSONArrayImpl::isFloat(size_t index) noexcept {
    return at(index).IsDouble();
}
bool
JSONArrayImpl::isString(size_t index) noexcept {
    return at(index).IsString();
}
bool
JSONArrayImpl::isObject(size_t index) noexcept {
    return at(index).IsObject();
}
bool
JSONArrayImpl::isArray(size_t index) noexcept {
    return at(index).IsArray();
}

bool
JSONArrayImpl::boolAt(size_t index) noexcept {
    return at(index).GetBool();
}
int
JSONArrayImpl::intAt(size_t index) noexcept {
    return at(index).GetInt();
}
unsigned
JSONArrayImpl::unsignedAt(size_t index) noexcept {
    return at(index).GetUint();
}
float
JSONArrayImpl::floatAt(size_t index) noexcept {
    return static_cast<float>(at(index).GetDouble());
}
StringView
JSONArrayImpl::stringAt(size_t index) noexcept {
    return at(index).GetString();
}
Unique<JSONObject>
JSONArrayImpl::objectAt(size_t index) noexcept {
    return Unique<JSONObject>(new JSONObjectReal(at(index).GetObject()));
}
Unique<JSONArray>
JSONArrayImpl::arrayAt(size_t index) noexcept {
    return Unique<JSONArray>(new JSONArrayImpl(at(index).GetArray()));
}

RJArray::PlainType&
JSONArrayImpl::at(size_t index) noexcept {
    return array[static_cast<unsigned>(index)];
};


JSONDocImpl::JSONDocImpl(String json) noexcept : json(move_(json)) {
    document.ParseInsitu<rapidjson::kParseInsituFlag |
                         rapidjson::kParseCommentsFlag |
                         rapidjson::kParseTrailingCommasFlag>(
            const_cast<char*>(this->json.null().get()));
}

bool
JSONDocImpl::isValid() noexcept {
    return !document.HasParseError() && document.IsObject();
}

RJObject
JSONDocImpl::get() noexcept {
    return document.GetObject();
}


Rc<JSONObject>
genJSON(StringView path) noexcept {
    Optional<StringView> r = Resources::load(path);
    if (!r) {
        return Rc<JSONObject>();
    }
    StringView json = *r;

    TimeMeasure m(String() << "Constructed " << path << " as json");

    JSONDocImpl document(json);
    if (!document.isValid()) {
        return Rc<JSONObject>();
    }

    return Rc<JSONObject>(new JSONDocImpl(move_(document)));
}

static RcReaderCache<Rc<JSONObject>, genJSON> documents;

Rc<JSONObject>
JSONs::load(StringView path) noexcept {
    return documents.lifetimeRequest(path);
}

Unique<JSONObject>
JSONs::parse(String data) noexcept {
    return Unique<JSONObject>(new JSONDocImpl(move_(data)));
}

void
JSONs::garbageCollect() noexcept {
    documents.garbageCollect();
}
