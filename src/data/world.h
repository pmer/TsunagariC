/**********************************
** Tsunagari Tile Engine         **
** world.h                       **
** Copyright 2014 PariahSoft LLC **
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

#ifndef DATAWORLD_H
#define DATAWORLD_H

#include <map>

#include "area.h"

class DataWorld
{
public:
	static DataWorld& instance();

	virtual ~DataWorld();

	//! After the engine has booted, initialize the world.
	virtual bool init() = 0;

	DataArea* area(const std::string& areaName);

protected:
	DataWorld();

	std::map<std::string,DataAreaRef> areas;

private:
	DataWorld(const DataWorld&) = delete;
	DataWorld(DataWorld&&) = delete;
	DataWorld& operator=(const DataWorld&) = delete;
	DataWorld& operator=(DataWorld&&) = delete;
};

#endif
