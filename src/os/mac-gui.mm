/****************************************
** Tsunagari Tile Engine               **
** os/mac-gui.mm                       **
** Copyright 2013      Michael Reiley  **
** Copyright 2013-2016 Paul Merrill    **
****************************************/

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

#ifdef __APPLE__

#include <stdlib.h>

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

#include "core/world.h"

void macSetWorkingDirectory() {
    UInt8 pathBytes[512];
    CFBundleRef mainBundle;
    CFURLRef url;
    NSString* appPath;

    /* FIXME: memory leaks? */
    mainBundle = CFBundleGetMainBundle();
    url = CFBundleCopyBundleURL(mainBundle);
    CFURLGetFileSystemRepresentation(url, true, pathBytes, sizeof(pathBytes));
    appPath = [[NSString alloc] initWithBytes:pathBytes
                                       length:strlen((char*)pathBytes)+1
                                     encoding:NSUTF8StringEncoding];
    [[NSFileManager defaultManager] changeCurrentDirectoryPath:appPath];

    chdir("Contents/Resources");

    [appPath release];
}

void macMessageBox(const char* title, const char* msg) {
    World::instance().setPaused(true);

    NSString *nsTitle = [[NSString alloc] initWithCString:title
                                                 encoding:NSUTF8StringEncoding];
    NSString *nsMsg = [[NSString alloc] initWithCString:msg
                                               encoding:NSUTF8StringEncoding];

    NSAlert *alert = [[NSAlert alloc] init];
    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText:nsTitle];
    [alert setInformativeText:nsMsg];
    [alert setAlertStyle:NSAlertStyleCritical];
    [alert runModal];

    [alert release];
    [nsTitle release];
    [nsMsg release];

    World::instance().setPaused(false);
}

#endif  // __APPLE__
