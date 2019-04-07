/***************************************
** Tsunagari Tile Engine              **
** animation.h                        **
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

#ifndef SRC_CORE_ANIMATION_H_
#define SRC_CORE_ANIMATION_H_

#include "core/images.h"
#include "util/int.h"
#include "util/rc.h"
#include "util/vector.h"

/**
 * An Animation is a sequence of bitmap images (called frames) used to creates
 * the illusion of motion. Frames are cycled over with an even amount of time
 * given to each, and the whole animation starts over after the last frame is
 * displayed.
 *
 * Mechanically, it is a list of images and a period of time over which to
 * play.
 */
class Animation {
 public:
    /**
     * Constructs an empty, but safe, Animation. All methods on this
     * object will be null.
     */
    Animation() noexcept;

    /**
     * Constructs a single-frame Animation. It will function like a static
     * image.
     *
     * @param frame static image
     */
    explicit Animation(Rc<Image> frame) noexcept;

    /**
     * Constructs a Animation from a list of frames.
     *
     * If given more than one frame, frameTime must be a positive,
     * non-zero value.
     *
     * @param frames list of frames to cycle through
     * @param frameTime length of time in milliseconds that each frame
     *        will display for
     */
    Animation(Vector<Rc<Image>> frames, time_t frameTime) noexcept;

    /**
     * Starts the animation over.
     *
     * @now current time in milliseconds
     */
    void startOver(time_t now) noexcept;

    /**
     * Has this Animation switched frames since frame() was last called?
     *
     * @now current time in milliseconds
     */
    bool needsRedraw(time_t now) const noexcept;

    /**
     * Returns the image that should be displayed at this time.
     *
     * @now current time in milliseconds
     */
    Image* frame(time_t now) noexcept;

    /**
     * Returns the last image that should have been displayed.
     */
    Image* frame() const noexcept;

 private:
    /** List of images in animation. */
    Vector<Rc<Image>> frames;

    /** Length of each frame in animation in milliseconds. */
    time_t frameTime;

    /** Length of one complete cycle through animation in milliseconds. */
    time_t cycleTime;

    /** Index of frame currently displaying on screen. */
    size_t frameShowing;

    /** Time offset to find current animation frame. */
    time_t offset;
};

#endif  // SRC_CORE_ANIMATION_H_
