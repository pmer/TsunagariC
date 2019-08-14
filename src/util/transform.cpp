/*************************************
** Tsunagari Tile Engine            **
** transform.cpp                    **
** Copyright 2017-2019 Paul Merrill **
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

#include "transform.h"

#include "util/noexcept.h"

Transform
Transform::identity() noexcept {
    return Transform{{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}};
}

Transform
Transform::scale(float factor) noexcept {
    return Transform{
            {factor, 0, 0, 0, 0, factor, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}};
}

Transform
Transform::translate(float x, float y) noexcept {
    return Transform{{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, x, y, 0, 1}};
}

float& Transform::operator[](int i) noexcept {
    return matrix[i];
}

const float& Transform::operator[](int i) const noexcept {
    return matrix[i];
}

Transform Transform::operator*(const Transform& right) noexcept {
    const Transform& left = *this;

    Transform result;
    for (int i = 0; i < 16; i++) {
        result[i] = left[i / 4 * 4 + 0] * right[i % 4 + 0 * 4] +
                    left[i / 4 * 4 + 1] * right[i % 4 + 1 * 4] +
                    left[i / 4 * 4 + 2] * right[i % 4 + 2 * 4] +
                    left[i / 4 * 4 + 3] * right[i % 4 + 3 * 4];
    }
    return result;
}
