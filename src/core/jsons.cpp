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


// FIXME: Pre-define operator new.
#include <new>

#include "core/jsons.h"

#define RAPIDJSON_HAS_CXX11_RVALUE_REFS 1

#include "rapidjson/document.h"
#include "rapidjson/reader.h"

#include "cache/cache-template.h"
#include "cache/readercache.h"
#include "core/log.h"
#include "core/measure.h"
#include "core/resources.h"
#include "util/move.h"
#include "util/string2.h"

#define CHECK(x)  if (!(x)) { return false; }

typedef rapidjson::Document RJDocument;
typedef RJDocument::ValueType RJValue;
typedef RJDocument::ValueType::Object RJObject;
typedef RJDocument::ValueType::Array RJArray;

class JSONObjectImpl : public JSONObject {
 public:
    virtual ~JSONObjectImpl() override = default;

    vector<StringView> names() final;

    bool hasBool(StringView name) final;
    bool hasInt(StringView name) final;
    bool hasUnsigned(StringView name) final;
    bool hasDouble(StringView name) final;
    bool hasString(StringView name) final;
    bool hasObject(StringView name) final;
    bool hasArray(StringView name) final;

    bool hasStringDouble(StringView name) final;

    bool boolAt(StringView name) final;
    int intAt(StringView name) final;
    int intAt(StringView name, int lowerBound, int upperBound) final;
    unsigned unsignedAt(StringView name) final;
    double doubleAt(StringView name) final;
    StringView stringAt(StringView name) final;
    Unique<JSONObject> objectAt(StringView name) final;
    Unique<JSONArray> arrayAt(StringView name) final;

    double stringDoubleAt(StringView name) final;

 protected:
    RJValue str(StringView name);
    virtual RJObject& get() = 0;
};

class JSONObjectReal : public JSONObjectImpl {
 public:
    explicit JSONObjectReal(RJObject object);

 protected:
    RJObject& get() final;

 private:
    RJObject object;
};

class JSONArrayImpl : public JSONArray {
 public:
    explicit JSONArrayImpl(RJArray array);

    size_t size() final;

    bool isBool(size_t index) final;
    bool isInt(size_t index) final;
    bool isUnsigned(size_t index) final;
    bool isDouble(size_t index) final;
    bool isString(size_t index) final;
    bool isObject(size_t index) final;
    bool isArray(size_t index) final;

    bool boolAt(size_t index) final;
    int intAt(size_t index) final;
    unsigned unsignedAt(size_t index) final;
    double doubleAt(size_t index) final;
    StringView stringAt(size_t index) final;
    Unique<JSONObject> objectAt(size_t index) final;
    Unique<JSONArray> arrayAt(size_t index) final;

 private:
    RJArray::PlainType& at(size_t index);

 private:
    RJArray array;
};


class JSONDocImpl : public JSONObjectImpl {
 public:
    explicit JSONDocImpl(String json);

    bool isValid();

 protected:
    RJObject& get() final;

 private:
    String json;
    RJDocument document;
    void* object;
    void* objectAddr;
};

static Rc<JSONObject> genJSON(StringView path);

class JSONsImpl : public JSONs {
 public:
    Rc<JSONObject> load(StringView path) final;

    void garbageCollect() final;

 private:
    ReaderCache<Rc<JSONObject>, genJSON> documents;
};


static JSONsImpl globalJSONs;

JSONs& JSONs::instance() {
    return globalJSONs;
}

vector<StringView> JSONObjectImpl::names() {
    vector<StringView> names(get().MemberCount());
    for (auto& property : get()) {
        auto& name = property.name;
        if (name.IsString()) {
            names.push_back(name.GetString());
        }
    }
    return names;
}

template<typename T>
static bool isStringDouble(const T& val) {
    CHECK(val.IsString());
    StringView str = val.GetString();
    return parseDouble(str);
}

template<typename T>
static double stringDoubleFrom(const T& val) {
    StringView str = val.GetString();
    Optional<double> d = parseDouble(str);
    return *d;
}


bool JSONObjectImpl::hasBool(StringView name) { return get().HasMember(str(name)) && get()[str(name)].IsBool(); }
bool JSONObjectImpl::hasInt(StringView name) { return get().HasMember(str(name)) && get()[str(name)].IsInt(); }
bool JSONObjectImpl::hasUnsigned(StringView name) { return get().HasMember(str(name)) && get()[str(name)].IsUint(); }
bool JSONObjectImpl::hasDouble(StringView name) { return get().HasMember(str(name)) && get()[str(name)].IsDouble(); }
bool JSONObjectImpl::hasString(StringView name) { return get().HasMember(str(name)) && get()[str(name)].IsString(); }
bool JSONObjectImpl::hasObject(StringView name) { return get().HasMember(str(name)) && get()[str(name)].IsObject(); }
bool JSONObjectImpl::hasArray(StringView name) { return get().HasMember(str(name)) && get()[str(name)].IsArray(); }
bool JSONObjectImpl::hasStringDouble(StringView name) { return get().HasMember(str(name)) && isStringDouble(get()[str(name)]); }

bool JSONObjectImpl::boolAt(StringView name) { return get()[str(name)].GetBool(); }
int JSONObjectImpl::intAt(StringView name) { return get()[str(name)].GetInt(); }
int JSONObjectImpl::intAt(StringView name, int lowerBound, int upperBound) {
    int i = get()[str(name)].GetInt();
    if (i < lowerBound) {
        Log::fatal("JSONObject::intAt", "Value out of range");
    }
    if (i > upperBound) {
        Log::fatal("JSONObject::intAt", "Value out of range");
    }
    return i;
}
unsigned JSONObjectImpl::unsignedAt(StringView name) { return get()[str(name)].GetUint(); }
double JSONObjectImpl::doubleAt(StringView name) { return get()[str(name)].GetDouble(); }
StringView JSONObjectImpl::stringAt(StringView name) { return get()[str(name)].GetString(); }
Unique<JSONObject> JSONObjectImpl::objectAt(StringView name) { return Unique<JSONObject>(new JSONObjectReal(get()[str(name)].GetObject())); }
Unique<JSONArray> JSONObjectImpl::arrayAt(StringView name) { return Unique<JSONArray>(new JSONArrayImpl(get()[str(name)].GetArray())); }
double JSONObjectImpl::stringDoubleAt(StringView name) { return stringDoubleFrom(get()[str(name)]); }

RJValue JSONObjectImpl::str(StringView name) {
    return RJValue(rapidjson::StringRef(name.data, name.size));
}


JSONObjectReal::JSONObjectReal(RJObject object) : object(move_(object)) {}

RJObject& JSONObjectReal::get() {
    return object;
}


JSONArrayImpl::JSONArrayImpl(RJArray array) : array(move_(array)) {}

size_t JSONArrayImpl::size() {
    return array.Size();
}

bool JSONArrayImpl::isBool(size_t index) { return at(index).IsBool(); }
bool JSONArrayImpl::isInt(size_t index) { return at(index).IsInt(); }
bool JSONArrayImpl::isUnsigned(size_t index) { return at(index).IsUint(); }
bool JSONArrayImpl::isDouble(size_t index) { return at(index).IsDouble(); }
bool JSONArrayImpl::isString(size_t index) { return at(index).IsString(); }
bool JSONArrayImpl::isObject(size_t index) { return at(index).IsObject(); }
bool JSONArrayImpl::isArray(size_t index) { return at(index).IsArray(); }

bool JSONArrayImpl::boolAt(size_t index) { return at(index).GetBool(); }
int JSONArrayImpl::intAt(size_t index) { return at(index).GetInt(); }
unsigned JSONArrayImpl::unsignedAt(size_t index) { return at(index).GetUint(); }
double JSONArrayImpl::doubleAt(size_t index) { return at(index).GetDouble(); }
StringView JSONArrayImpl::stringAt(size_t index) { return at(index).GetString(); }
Unique<JSONObject> JSONArrayImpl::objectAt(size_t index) {
    return Unique<JSONObject>(new JSONObjectReal(at(index).GetObject()));
}
Unique<JSONArray> JSONArrayImpl::arrayAt(size_t index) {
    return Unique<JSONArray>(new JSONArrayImpl(at(index).GetArray()));
}

RJArray::PlainType& JSONArrayImpl::at(size_t index) {
    return array[static_cast<unsigned>(index)];
};


JSONDocImpl::JSONDocImpl(String json)
        : json(move_(json)) {
    document.ParseInsitu<
            rapidjson::kParseInsituFlag |
            rapidjson::kParseCommentsFlag |
            rapidjson::kParseTrailingCommasFlag>(
                const_cast<char*>(this->json.null().get()));
    object = &document;
    objectAddr = &object;
}

bool JSONDocImpl::isValid() {
    return !document.HasParseError() && document.IsObject();
}

RJObject& JSONDocImpl::get() {
    return (RJObject&)object;
}


static Rc<JSONObject> genJSON(StringView path) {
    Optional<StringView> r = resourceLoad(path);
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

Rc<JSONObject> JSONsImpl::load(StringView path) {
    return documents.lifetimeRequest(path);
}

Unique<JSONObject> JSONs::parse(String data) {
    return Unique<JSONObject>(new JSONDocImpl(move_(data)));
}

void JSONsImpl::garbageCollect() {
    documents.garbageCollect();
}
