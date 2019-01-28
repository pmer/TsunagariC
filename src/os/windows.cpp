/***************************************
** Tsunagari Tile Engine              **
** windows.cpp                        **
** Copyright 2007 Julian Raschke      **
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

#include "os/windows.h"

#define UNICODE
#include <Windows.h>

#include <string>

#include "core/window.h"
#include "core/world.h"
#include "os/os.h"
#include "util/vector.h"

void wFixConsole() {
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        freopen("CONOUT$", "wb", stdout);  // Attach STDOUT.
        freopen("CONOUT$", "wb", stderr);  // Attach STDERR.
    }
}

/* From gosu/src/Utility.cpp by Julian Raschke in 2007 */
std::wstring widen(const std::string& s) {
    size_t wideLen = std::mbstowcs(0, s.c_str(), 0);
    if (wideLen == static_cast<size_t>(-1)) {
        throw std::runtime_error(
            "Could not convert from string to wstring: " + s);
    }

    vector<wchar_t> buf(wideLen + 1);
    mbstowcs(&buf.front(), s.c_str(), buf.size());

    return std::wstring(buf.begin(), buf.end() - 1);
}

void wMessageBox(const std::string& title, const std::string& text) {
    World::instance().setPaused(true);
	// FIXME: Try to get the window's native handle instead of passing NULL.
    MessageBox(NULL, widen(text).c_str(), widen(title).c_str(), MB_OK);
    World::instance().setPaused(false);
}

void setTermColor(TermColor color) {
    // TODO
}
