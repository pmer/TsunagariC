/***************************************
** Tsunagari Tile Engine              **
** vec.h                              **
** Copyright 2011-2013 Michael Reiley **
** Copyright 2011-2019 Paul Merrill   **
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

#ifndef SRC_CORE_VEC_H_
#define SRC_CORE_VEC_H_

#include "os/c.h"
#include "util/fnv.h"
#include "util/noexcept.h"

/**
 * Virtual integer coordinate.
 *
 * x and y are the same as a physical integer coordinate.
 * z is a virtual layer depth within an Area.
 */
struct vicoord {
    int x, y;
    float z;
};

struct icube {
    int x1, y1, z1;
    int x2, y2, z2;
};

template<class T>
class vec2 {
 public:
    T x, y;

    operator bool() noexcept { return x || y; }

    float distanceTo(vec2<T> other) noexcept {
        T dx = x - other.x;
        T dy = y - other.y;
        return sqrt(dx * dx + dy * dy);
    }
};

template<class T>
struct vec3 {
 public:
    T x, y, z;

    operator bool() noexcept { return x || y || z; }

    float distanceTo(vec3<T> other) noexcept {
        T dx = x - other.x;
        T dy = y - other.y;
        return static_cast<float>(sqrt(dx * dx + dy * dy));
    }
};

template<class T>
vec2<T>
operator+(const vec2<T>& a, const vec2<T>& b) noexcept {
    return {
        a.x + b.x,
        a.y + b.y
    };
}

template<class T>
vec3<T>
operator+(const vec3<T>& a, const vec3<T>& b) noexcept {
    return {
        a.x + b.x,
        a.y + b.y,
        a.z + b.z
    };
}

template<class T>
vec2<T>
operator-(const vec2<T>& a, const vec2<T>& b) noexcept {
    return {
        a.x - b.x,
        a.y - b.y
    };
}

template<class T>
vec3<T>
operator-(const vec3<T>& a, const vec3<T>& b) noexcept {
    return {
        a.x - b.x,
        a.y - b.y,
        a.z - b.z
    };
}

template<class T, class CO>
vec2<T> operator*(CO co, const vec2<T>& a) noexcept {
    return {
        a.x * (T)co,
        a.y * (T)co
    };
}

template<class T, class CO>
vec3<T> operator*(CO co, const vec3<T>& a) noexcept {
    return {
        a.x * (T)co,
        a.y * (T)co,
        a.z * (T)co
    };
}

template<class T, class CO>
vec2<T>
operator/(const vec2<T>& a, CO co) noexcept {
    return {
        a.x / (T)co,
        a.y / (T)co
    };
}

template<class T, class CO>
vec3<T>
operator/(const vec3<T>& a, CO co) noexcept {
    return {
        a.x / (T)co,
        a.y / (T)co,
        a.z / (T)co
    };
}

template<class T>
bool
operator==(const vec2<T>& a, const vec2<T>& b) noexcept {
    return a.x == b.x && a.y == b.y;
}

template<class T>
bool
operator!=(const vec2<T>& a, const vec2<T>& b) noexcept {
    return a.x != b.x || a.y != b.y;
}

template<class T>
bool
operator==(const vec3<T>& a, const vec3<T>& b) noexcept {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

template<class T>
bool
operator!=(const vec3<T>& a, const vec3<T>& b) noexcept {
    return a.x != b.x || a.y != b.y || a.z != b.z;
}

template<class T>
size_t
hash_(vec2<T> a) noexcept {
    return fnvHash(reinterpret_cast<const char*>(&a), sizeof(a));
}

template<class T>
size_t
hash_(vec3<T> a) noexcept {
    return fnvHash(reinterpret_cast<const char*>(&a), sizeof(a));
}

//! Integer vector.
typedef vec2<int> ivec2;
typedef vec3<int> ivec3;

//! Real vector.
typedef vec2<float> rvec2;
typedef vec3<float> rvec3;

//! Coordinates.
typedef ivec3 icoord;
typedef rvec3 rcoord;

#endif  // SRC_CORE_VEC_H_
