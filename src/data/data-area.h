/***************************************
** Tsunagari Tile Engine              **
** data-area.h                        **
** Copyright 2014      Michael Reiley **
** Copyright 2014-2019 Paul Merrill   **
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

#ifndef SRC_DATA_DATA_AREA_H_
#define SRC_DATA_DATA_AREA_H_

#include "data/inprogress.h"
#include "util/function.h"
#include "util/hashtable.h"
#include "util/int.h"
#include "util/string-view.h"
#include "util/string.h"
#include "util/unique.h"
#include "util/vector.h"

class Area;
class Entity;
class Tile;

class DataArea {
 public:
    typedef void (DataArea::*TileScript)(Entity& triggeredBy, Tile& tile);
    typedef Function<void(float)> ProgressFn;
    typedef Function<void()> ThenFn;

    virtual ~DataArea() = default;

    Area* area = nullptr;  // borrowed reference

    virtual void onLoad() noexcept;
    virtual void onFocus() noexcept;
    virtual void onTick(time_t dt) noexcept;
    virtual void onTurn() noexcept;

    // For scripts

    //! Play a sound with a 3% speed variation applied to it.
    void playSoundEffect(StringView sound) noexcept;

    void playSoundAndThen(StringView sound, ThenFn then) noexcept;
    void timerProgress(time_t duration, ProgressFn progress) noexcept;
    void timerThen(time_t duration, ThenFn then) noexcept;
    void timerProgressAndThen(time_t duration,
                              ProgressFn progress,
                              ThenFn then) noexcept;

    // For engine
    void tick(time_t dt) noexcept;
    void turn() noexcept;
    TileScript script(StringView scriptName) noexcept;

 protected:
    DataArea() = default;

    Hashmap<StringView, TileScript> scripts;

 private:
    DataArea(const DataArea&) = delete;
    DataArea(DataArea&&) = delete;
    DataArea& operator=(const DataArea&) = delete;
    DataArea& operator=(DataArea&&) = delete;

    Vector<Unique<InProgress>> inProgresses;
};

#endif  // SRC_DATA_DATA_AREA_H_
