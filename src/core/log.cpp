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

// FIXME: Pre-declare operator new.
#include <new>

#include "core/log.h"

#include <iostream>
#include <mutex>

#include "core/client-conf.h"
#include "core/window.h"

#include "os/os.h"

#include "util/algorithm.h"

#ifdef _WIN32
    #include "os/windows.h"
#endif

#ifdef __APPLE__
    #include "os/mac-gui.h"
#endif

static verbosity_t verb = V_NORMAL;

static time_t startTime;

static std::mutex stdoutMutex;

static StringView chomp(StringView str) {
    size_t size = str.size;
    while (str.data[str.size - 1] == ' ' ||
           str.data[str.size - 1] == '\t' ||
           str.data[str.size - 1] == '\n' ||
           str.data[str.size - 1] == '\r') {
        size -= 1;
    }
    return str.substr(0, size);
}

static String makeTimestamp() {
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

bool Log::init() {
    startTime = GameWindow::time();
    return true;
}

void Log::setVerbosity(verbosity_t v) {
    verb = v;
}

void Log::info(StringView domain, StringView msg) {
    if (verb > V_NORMAL) {
        std::unique_lock<std::mutex> lock(stdoutMutex);

        setTermColor(TC_GREEN);
        std::cout << makeTimestamp().null();

        setTermColor(TC_YELLOW);
        String s;
        s << "Info [" << domain << "]";
        std::cout << s.null();

        setTermColor(TC_RESET);
        s.clear();
        s << " - " << chomp(msg) << "\n";
        std::cout << s.null();
    }
}

void Log::err(StringView domain, StringView msg) {
    if (verb > V_QUIET) {
        {
            std::unique_lock<std::mutex> lock(stdoutMutex);

            setTermColor(TC_GREEN);
            std::cerr << makeTimestamp().null();

            setTermColor(TC_RED);
            String s;
            s << "Error [" << domain << "]";
            std::cerr << s.null();

            setTermColor(TC_RESET);
            s.clear();
            s << " - " << chomp(msg) << "\n";
            std::cerr << s.null();
        }

        String s;
        s << "Error [" << domain << "] - " << chomp(msg);

        #ifdef _WIN32
            wMessageBox("Tsunagari - Error", s.null());
        #endif
        #ifdef __APPLE__
            macMessageBox("Tsunagari - Error", s.null());
        #endif
    }
}

void Log::fatal(StringView domain, StringView msg) {
    {
        std::unique_lock<std::mutex> lock(stdoutMutex);

        setTermColor(TC_GREEN);
        std::cerr << makeTimestamp().null();

        setTermColor(TC_RED);
        String s;
        s << "Fatal [" << domain << "]";
        std::cerr << s.null();

        setTermColor(TC_RESET);
        s.clear();
        s << " - " << chomp(msg) << "\n";
        std::cerr << s.null();
    }

    String s;
    s << "Fatal [" << domain << "] - " << chomp(msg);

    #ifdef _WIN32
        wMessageBox("Tsunagari - Fatal", s.null());
    #endif
    #ifdef __APPLE__
        macMessageBox("Tsunagari - Fatal", s.null());
    #endif

    exit(1);
}

void Log::reportVerbosityOnStartup() {
    std::unique_lock<std::mutex> lock(stdoutMutex);

    StringView verbString;
    switch (conf.verbosity) {
    case V_QUIET:
        verbString = "QUIET";
        break;
    case V_NORMAL:
        verbString = "NORMAL";
        break;
    case V_VERBOSE:
        verbString = "VERBOSE";
        break;
    }

    setTermColor(TC_GREEN);
    std::cout << makeTimestamp().null();

    setTermColor(TC_RESET);
    String s;
    s << "Reporting engine messages in " << verbString
      << " mode.\n";
    std::cout << s.null();
}
