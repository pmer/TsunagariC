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

//#include "core/world.h"
#include "os/c.h"
#include "os/os.h"
#include "util/string-view.h"
#include "util/string.h"
#include "util/vector.h"

extern "C" {
WINBASEAPI BOOL WINAPI AttachConsole(DWORD processId);
_ACRTIMP FILE* __cdecl freopen(const char* filename,
                               const char* mode,
                               FILE* stream) noexcept;
WINUSERAPI int WINAPI MessageBoxA(HWND hwnd,
                                  LPCSTR text,
                                  LPCSTR caption,
                                  UINT type) noexcept;

#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)
#define MessageBox MessageBoxA

constexpr DWORD ATTACH_PARENT_PROCESS = -1;
constexpr UINT MB_OK = 0x00000000L;
}

char dirSeparator = '\\';

Optional<uint64_t>
getFileSize(StringView path) noexcept {
    return Optional<uint64_t>();
}

bool
writeFile(StringView path, uint32_t length, void* data) noexcept {
    return false;
}

bool
writeFileVec(StringView path,
             uint32_t count,
             uint32_t* lengths,
             void** datas) noexcept {
    return false;
}

bool
isDir(StringView path) noexcept {
    return false;
}

void
makeDirectory(StringView path) noexcept {}

Vector<String>
listDir(StringView path) noexcept {
    return Vector<String>();
}

Optional<String>
readFile(StringView path) noexcept {
    return Optional<String>();
}

void
setTermColor(TermColor color) noexcept {
    // TODO
}

void
wFixConsole() noexcept {
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        freopen("CONOUT$", "wb", stdout);  // Attach STDOUT.
        freopen("CONOUT$", "wb", stderr);  // Attach STDERR.
    }
}

/* From gosu/src/Utility.cpp by Julian Raschke in 2007 */
/*
std::wstring widen(StringView s) noexcept {
  size_t wideLen = mbstowcs(0, String(s).null(), 0);
  if (wideLen == static_cast<size_t>(-1)) {
    throw std::runtime_error("Could not convert from string to wstring: " + s);
  }

  vector<wchar_t> buf(wideLen + 1);
  mbstowcs(&buf.front(), String(s).null(), buf.size());

  return std::wstring(buf.begin(), buf.end() - 1);
}
*/

void
wMessageBox(StringView title, StringView text) noexcept {
    // World::instance().setPaused(true);
    // FIXME: Try to get the window's native handle instead of passing nullptr.
    MessageBox(nullptr, String(text).null(), String(title).null(), MB_OK);
    // World::instance().setPaused(false);
}
