/**********************************
** Tsunagari Tile Engine         **
** windows-math.h                **
** Copyright 2019 Paul Merrill   **
**********************************/

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

#ifndef SRC_OS_WINDOWS_MATH_H_
#define SRC_OS_WINDOWS_MATH_H_

#ifdef _WIN32
#define WINDOWSC __cdecl
#define DLLIMPORT __declspec(dllimport)
#define UNIXC
#else
#define WINDOWSC
#define DLLIMPORT
#define UNIXC extern "C"
#endif

UNIXC double WINDOWSC atan2(double, double);
UNIXC DLLIMPORT double WINDOWSC ceil(double);
#ifdef _WIN32
#ifndef _INC_MATH
inline float WINDOWSC ceilf(float x) {
  return static_cast<float>(ceil(x));
}
#endif
#else
UNIXC DLLIMPORT float WINDOWSC ceilf(float);
#endif
UNIXC double WINDOWSC cos(double);
UNIXC DLLIMPORT double WINDOWSC floor(double);
UNIXC double WINDOWSC sin(double);
UNIXC double WINDOWSC sqrt(double);

#endif  // SRC_OS_WINDOWS_MATH_H_
