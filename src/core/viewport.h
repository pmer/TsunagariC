/***************************************
** Tsunagari Tile Engine              **
** viewport.h                         **
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

#ifndef SRC_CORE_VIEWPORT_H_
#define SRC_CORE_VIEWPORT_H_

#include "core/vec.h"
#include "util/int.h"

class Area;
class Entity;

//! General control over where and how the map is rendered.
/*!

 */
class Viewport {
 public:
    static void setSize(rvec2 virtRes) noexcept;

    static void tick(time_t dt) noexcept;
    static void turn() noexcept;

    //! How far the map is scrolled in pixels, counting from the upper-left.
    static rvec2 getMapOffset() noexcept;

    //! Size of the letterbox matte bars in pixels.
    static rvec2 getLetterboxOffset() noexcept;

    //! Multiplier in X and Y dimensions to get from virtRes to physRes.
    static rvec2 getScale() noexcept;

    //! The resolution our game is actually being drawn at.
    static rvec2 getPhysRes() noexcept;

    //! The resolution our game thinks it is being drawn at. Chosen by a
    //! world's creator. This allows graphics to look the same on any
    //! setups of any resolution.
    static rvec2 getVirtRes() noexcept;

    // Immediatly center render offset. Stop any tracking.
    static void jumpToPt(ivec2 pt) noexcept;
    static void jumpToPt(rvec2 pt) noexcept;
    static void jumpToEntity(const Entity* e) noexcept;

    // Continuously follow.
    static void trackEntity(const Entity* e) noexcept;

    static void setArea(const Area* a) noexcept;
};

#endif  // SRC_CORE_VIEWPORT_H_
