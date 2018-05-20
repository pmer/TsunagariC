/*************************************
** Tsunagari Tile Engine            **
** jsons.cpp                        **
** Copyright 2016-2018 Paul Merrill **
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

#define RAPIDJSON_HAS_STDSTRING 1
#define RAPIDJSON_SSE42 1

#include "core/jsons.h"

#include <string>

#include "rapidjson/document.h"
#include "rapidjson/reader.h"

#include "cache/cache-template.cpp"
#include "cache/readercache.h"
#include "core/log.h"
#include "core/measure.h"
#include "core/resources.h"
#include "util/move.h"
#include "util/string2.h"

#define CHECK(x)  if (!(x)) { return false; }

typedef rapidjson::Document RJDocument;
typedef RJDocument::ValueType::ConstObject RJObject;
typedef RJDocument::ValueType::ConstArray RJArray;

class JSONObjectImpl : public JSONObject {
 public:
    ~JSONObjectImpl() override = default;

    vector<std::string> names() const final;

    bool hasBool(const std::string& name) const final;
    bool hasInt(const std::string& name) const final;
    bool hasUnsigned(const std::string& name) const final;
    bool hasDouble(const std::string& name) const final;
    bool hasString(const std::string& name) const final;
    bool hasObject(const std::string& name) const final;
    bool hasArray(const std::string& name) const final;

    bool hasStringDouble(const std::string& name) const final;

    bool boolAt(const std::string& name) const final;
    int intAt(const std::string& name) const final;
    int intAt(const std::string& name, int lowerBound, int upperBound) const final;
    unsigned unsignedAt(const std::string& name) const final;
    double doubleAt(const std::string& name) const final;
    std::string stringAt(const std::string& name) const final;
    JSONObjectPtr objectAt(const std::string& name) const final;
    JSONArrayPtr arrayAt(const std::string& name) const final;

    double stringDoubleAt(const std::string& name) const final;

 protected:
    JSONObjectImpl() = default;

    virtual const RJObject& get() const = 0;
};

class JSONObjectReal : public JSONObjectImpl {
 public:
    explicit JSONObjectReal(RJObject object);

 protected:
    const RJObject& get() const final;

 private:
    const RJObject object;
};

class JSONArrayImpl : public JSONArray {
 public:
    explicit JSONArrayImpl(RJArray array);

    size_t size() const final;

    bool isBool(size_t index) const final;
    bool isInt(size_t index) const final;
    bool isUnsigned(size_t index) const final;
    bool isDouble(size_t index) const final;
    bool isString(size_t index) const final;
    bool isObject(size_t index) const final;
    bool isArray(size_t index) const final;

    bool boolAt(size_t index) const final;
    int intAt(size_t index) const final;
    unsigned unsignedAt(size_t index) const final;
    double doubleAt(size_t index) const final;
    std::string stringAt(size_t index) const final;
    JSONObjectPtr objectAt(size_t index) const final;
    JSONArrayPtr arrayAt(size_t index) const final;

 private:
    const RJArray::PlainType& at(size_t index) const;

 private:
    RJArray array;
};


class JSONDocImpl : public JSONObjectImpl {
 public:
    explicit JSONDocImpl(std::string json);

    bool isValid() const;

 protected:
    const RJObject& get() const final;

 private:
    std::string json;
    RJDocument document;
    void* object;
    void* objectAddr;
};

static JSONObjectRef genJSON(const std::string& path);

class JSONsImpl : public JSONs {
 public:
    JSONObjectRef load(const std::string& path) final;

    void garbageCollect() final;

 private:
    ReaderCache<JSONObjectRef, genJSON> documents;
};


static JSONsImpl globalJSONs;

JSONs& JSONs::instance() {
    return globalJSONs;
}

vector<std::string> JSONObjectImpl::names() const {
    vector<std::string> names;
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
    const std::string str = val.GetString();
    double d;
    return parseDouble(str, &d);
}

template<typename T>
static double stringDoubleFrom(const T& val) {
    const std::string str = val.GetString();
    double d;
    parseDouble(str, &d);
    return d;
}

bool JSONObjectImpl::hasBool(const std::string& name) const { return get().HasMember(name) && get()[name].IsBool(); }
bool JSONObjectImpl::hasInt(const std::string& name) const { return get().HasMember(name) && get()[name].IsInt(); }
bool JSONObjectImpl::hasUnsigned(const std::string& name) const { return get().HasMember(name) && get()[name].IsUint(); }
bool JSONObjectImpl::hasDouble(const std::string& name) const { return get().HasMember(name) && get()[name].IsDouble(); }
bool JSONObjectImpl::hasString(const std::string& name) const { return get().HasMember(name) && get()[name].IsString(); }
bool JSONObjectImpl::hasObject(const std::string& name) const { return get().HasMember(name) && get()[name].IsObject(); }
bool JSONObjectImpl::hasArray(const std::string& name) const { return get().HasMember(name) && get()[name].IsArray(); }
bool JSONObjectImpl::hasStringDouble(const std::string& name) const { return get().HasMember(name) && isStringDouble(get()[name]); }

bool JSONObjectImpl::boolAt(const std::string& name) const { return get()[name].GetBool(); }
int JSONObjectImpl::intAt(const std::string& name) const { return get()[name].GetInt(); }
int JSONObjectImpl::intAt(const std::string& name, int lowerBound, int upperBound) const {
    int i = get()[name].GetInt();
    if (i < lowerBound) {
        Log::fatal("JSONObject::intAt", "Value out of range");
    }
    if (i > upperBound) {
        Log::fatal("JSONObject::intAt", "Value out of range");
    }
    return i;
}
unsigned JSONObjectImpl::unsignedAt(const std::string& name) const { return get()[name].GetUint(); }
double JSONObjectImpl::doubleAt(const std::string& name) const { return get()[name].GetDouble(); }
std::string JSONObjectImpl::stringAt(const std::string& name) const { return get()[name].GetString(); }
JSONObjectPtr JSONObjectImpl::objectAt(const std::string& name) const { return JSONObjectPtr(new JSONObjectReal(get()[name].GetObject())); }
JSONArrayPtr JSONObjectImpl::arrayAt(const std::string& name) const { return JSONArrayPtr(new JSONArrayImpl(get()[name].GetArray())); }
double JSONObjectImpl::stringDoubleAt(const std::string& name) const { return stringDoubleFrom(get()[name]); }

JSONObjectReal::JSONObjectReal(const RJObject object) : object(move_(object)) {}

const RJObject& JSONObjectReal::get() const {
    return object;
}


JSONArrayImpl::JSONArrayImpl(RJArray array) : array(move_(array)) {}

size_t JSONArrayImpl::size() const {
    return array.Size();
}

bool JSONArrayImpl::isBool(size_t index) const { return at(index).IsBool(); }
bool JSONArrayImpl::isInt(size_t index) const { return at(index).IsInt(); }
bool JSONArrayImpl::isUnsigned(size_t index) const { return at(index).IsUint(); }
bool JSONArrayImpl::isDouble(size_t index) const { return at(index).IsDouble(); }
bool JSONArrayImpl::isString(size_t index) const { return at(index).IsString(); }
bool JSONArrayImpl::isObject(size_t index) const { return at(index).IsObject(); }
bool JSONArrayImpl::isArray(size_t index) const { return at(index).IsArray(); }

bool JSONArrayImpl::boolAt(size_t index) const { return at(index).GetBool(); }
int JSONArrayImpl::intAt(size_t index) const { return at(index).GetInt(); }
unsigned JSONArrayImpl::unsignedAt(size_t index) const { return at(index).GetUint(); }
double JSONArrayImpl::doubleAt(size_t index) const { return at(index).GetDouble(); }
std::string JSONArrayImpl::stringAt(size_t index) const { return at(index).GetString(); }
JSONObjectPtr JSONArrayImpl::objectAt(size_t index) const { return JSONObjectPtr(new JSONObjectReal(at(index).GetObject())); }
JSONArrayPtr JSONArrayImpl::arrayAt(size_t index) const { return JSONArrayPtr(new JSONArrayImpl(at(index).GetArray())); }

const RJArray::PlainType& JSONArrayImpl::at(size_t index) const {
    return array[static_cast<unsigned>(index)];
};


JSONDocImpl::JSONDocImpl(std::string json)
        : json(move_(json)) {
    document.ParseInsitu<
            rapidjson::kParseInsituFlag |
            rapidjson::kParseCommentsFlag |
            rapidjson::kParseTrailingCommasFlag>(
                const_cast<char*>(this->json.c_str()));
    object = &document;
    objectAddr = &object;
}

bool JSONDocImpl::isValid() const {
    return !document.HasParseError() && document.IsObject();
}

const RJObject& JSONDocImpl::get() const {
    return (RJObject&)object;
}


static JSONObjectRef genJSON(const std::string& path) {
    Unique<Resource> r = Resources::instance().load(path);
    if (!r) {
        return JSONObjectRef();
    }
    std::string json(static_cast<const char*>(r->data()), r->size());

    TimeMeasure m("Constructed " + path + " as json");

    JSONDocImpl* document = new JSONDocImpl(move_(json));

    if (document->isValid()) {
        return JSONObjectRef(document);
    }
    else {
        delete document;
        return JSONObjectRef();
    }
}

JSONObjectRef JSONsImpl::load(const std::string& path) {
    return documents.lifetimeRequest(path);
}

JSONObjectPtr JSONs::parse(std::string data) {
    return JSONObjectPtr(new JSONDocImpl(move_(data)));
}

void JSONsImpl::garbageCollect() {
    documents.garbageCollect();
}
