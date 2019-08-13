/***************************************
** Tsunagari Tile Engine              **
** log.cpp                            **
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

#include "core/log.h"

#include "core/client-conf.h"
#include "core/window.h"
#include "os/c.h"
#include "os/mutex.h"
#include "os/os.h"
#include "util/algorithm.h"

#ifdef _WIN32
#include "os/windows.h"
#endif

#ifdef __APPLE__
#include "os/mac-gui.h"
#endif

static Log::Verbosity verb = Log::Verbosity::NORMAL;

static time_t startTime;

static Mutex stdoutMutex;

static StringView
chomp(StringView str) noexcept {
    size_t size = str.size;
    while (size > 0 &&
           (str.data[size - 1] == ' ' || str.data[size - 1] == '\t' ||
            str.data[size - 1] == '\n' || str.data[size - 1] == '\r')) {
        size -= 1;
    }
    return str.substr(0, size);
}

static String
makeTimestamp() noexcept {
    time_t now = GameWindow::time();

    double secs = (now - startTime) / (long double)1000.0;

    String s;
    s << secs;

    StringView v = s.view();

    Optional<size_t> idx = v.find('.');
    if (idx) {
        size_t dot = *idx;
        v = v.substr(0, min_(v.size, dot + 4));
    }

    String s2;
    s2 << "[" << v << "]";
    return s2;
}

bool
Log::init() noexcept {
    startTime = GameWindow::time();
    return true;
}

void
Log::setVerbosity(Log::Verbosity v) noexcept {
    verb = v;
}

void
Log::info(StringView domain, StringView msg) noexcept {
    if (verb > Log::Verbosity::NORMAL) {
        LockGuard lock(stdoutMutex);

        setTermColor(TC_GREEN);
        printf("%s ", makeTimestamp().null().get());

        setTermColor(TC_YELLOW);
        String s;
        s << "Info [" << domain << "]";
        printf("%s", s.null().get());

        setTermColor(TC_RESET);
        s.clear();
        s << " - " << chomp(msg) << "\n";
        printf("%s", s.null().get());
    }
}

void
Log::err(StringView domain, StringView msg) noexcept {
    if (verb > Log::Verbosity::QUIET) {
        {
            LockGuard lock(stdoutMutex);

            setTermColor(TC_GREEN);
            fprintf(stderr, "%s ", makeTimestamp().null().get());

            setTermColor(TC_RED);
            String s;
            s << "Error [" << domain << "]";
            fprintf(stderr, "%s", s.null().get());

            setTermColor(TC_RESET);
            s.clear();
            s << " - " << chomp(msg) << "\n";
            fprintf(stderr, "%s", s.null().get());
        }

        String s;
        s << "Error [" << domain << "] - " << chomp(msg);

#ifdef _WIN32
        wMessageBox("Tsunagari - Error", s);
#endif
#ifdef __APPLE__
        macMessageBox(StringView("Tsunagari - Error"), s);
#endif
    }
}

#ifdef _WIN32
extern "C" {
__declspec(dllimport) int __stdcall IsDebuggerPresent();
}
void __cdecl __debugbreak();
#endif

void
Log::fatal(StringView domain, StringView msg) noexcept {
    {
        LockGuard lock(stdoutMutex);

        setTermColor(TC_GREEN);
        fprintf(stderr, "%s ", makeTimestamp().null().get());

        setTermColor(TC_RED);
        String s;
        s << "Fatal [" << domain << "]";
        fprintf(stderr, "%s", s.null().get());

        setTermColor(TC_RESET);
        s.clear();
        s << " - " << chomp(msg) << "\n";
        fprintf(stderr, "%s", s.null().get());
    }

    String s;
    s << "Fatal [" << domain << "] - " << chomp(msg);

#ifdef _WIN32
    wMessageBox("Tsunagari - Fatal", s);

	if (IsDebuggerPresent()) {
        __debugbreak();
    }
#endif
#ifdef __APPLE__
    macMessageBox(StringView("Tsunagari - Fatal"), s);
#endif

    exit(1);
}

void
Log::reportVerbosityOnStartup() noexcept {
    LockGuard lock(stdoutMutex);

    StringView verbString;
    switch (Conf::verbosity) {
    case Log::Verbosity::QUIET:
        verbString = "QUIET";
        break;
    case Log::Verbosity::NORMAL:
        verbString = "NORMAL";
        break;
    case Log::Verbosity::VERBOSE:
        verbString = "VERBOSE";
        break;
    }

    setTermColor(TC_GREEN);
    printf("%s ", makeTimestamp().null().get());

    setTermColor(TC_RESET);
    String s;
    s << "Reporting engine messages in " << verbString << " mode.\n";
    printf("%s", s.null().get());
}
