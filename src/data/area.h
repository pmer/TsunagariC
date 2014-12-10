/**********************************
** Tsunagari Tile Engine         **
** area.h                        **
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

#ifndef DATAAREA_H
#define DATAAREA_H

#include <map>
#include <memory>
#include <string>

class Area;
class Entity;
class Tile;

class DataArea
{
public:
	typedef void (DataArea::* TileScript)(Entity& triggeredBy, Tile& tile);

	virtual ~DataArea();

	Area* area;

	virtual void onLoad();
	virtual void onTick();

	TileScript script(const std::string& scriptName);

protected:
	DataArea();

	std::map<std::string,TileScript> scripts;

private:
	DataArea(const DataArea&) = delete;
	DataArea(DataArea&&) = delete;
	DataArea& operator=(const DataArea&) = delete;
	DataArea& operator=(DataArea&&) = delete;
};

typedef std::shared_ptr<DataArea> DataAreaRef;

#endif
