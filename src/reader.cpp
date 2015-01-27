/***************************************
** Tsunagari Tile Engine              **
** reader.cpp                         **
** Copyright 2011-2015 PariahSoft LLC **
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

#include <errno.h>
#include <stdlib.h>

#include <Gosu/IO.hpp>
#include <map>
#include <physfs.h>

#include "cache.h"
#include "cache-template.cpp"
#include "client-conf.h"
#include "formatter.h"
#include "images.h"
#include "log.h"
#include "music.h"
#include "reader.h"
#include "sounds.h"
#include "window.h"
#include "xmls.h"

#include "data/data-world.h"

#define ASSERT(x)  if (!(x)) { return false; }

// Caches that store processed, game-ready objects. Garbage collected.
static Cache<std::shared_ptr<std::string>> texts;


static std::string path(const std::string& entryName)
{
	// XXX: archive might not be world
	return DataWorld::instance().datafile + "/" + entryName;
}

template <class T>
static bool readFromDisk(const std::string& name, T& buf)
{
	PHYSFS_sint64 size;
	PHYSFS_File* zf;

	if (!PHYSFS_exists(name.c_str())) {
		Log::err("Reader", Formatter("%: file missing")
				% path(name));
		return false;
	}

	zf = PHYSFS_openRead(name.c_str());
	if (!zf) {
		Log::err("Reader", Formatter("%: error opening file: %")
				% path(name) % PHYSFS_getLastError());
		return false;
	}

	size = PHYSFS_fileLength(zf);
	if (size == -1) {
		Log::err("Reader", Formatter("%: could not determine file size: %")
				% path(name) % PHYSFS_getLastError());
		PHYSFS_close(zf);
		return false;
	}
	else if (size > std::numeric_limits<uint32_t>::max()) {
		// FIXME: Technically, we just need to issue multiple calls to
		// PHYSFS_read. Fix when needed.
		Log::err("Reader", Formatter("%: file too long (>4GB)")
				% path(name));
		PHYSFS_close(zf);
		return false;
	}
	else if (size < -1) {
		Log::err("Reader", Formatter("%: invalid file size: %")
				% path(name) % PHYSFS_getLastError());
		PHYSFS_close(zf);
		return false;
	}

	buf.resize((size_t)size);
	if (size == 0) {
		PHYSFS_close(zf);
		return true;
	}

	if (PHYSFS_read(zf, (char*)(buf.data()),
			(PHYSFS_uint32)size, 1) != 1) {
		Log::err("Reader", Formatter("%: error reading file: %")
				% path(name) % PHYSFS_getLastError());
		PHYSFS_close(zf);
		return false;
	}

	PHYSFS_close(zf);
	return true;
}



bool Reader::init()
{
	const std::string& path = DataWorld::instance().datafile;
	int err = PHYSFS_mount(path.c_str(), NULL, 0);
	if (err == 0) {
		Log::fatal("Reader", Formatter("%: could not open archive: %")
				% path % PHYSFS_getLastError());
		return false;
	}
	return true;
}

bool Reader::resourceExists(const std::string& name)
{
	return PHYSFS_exists(name.c_str());
}

bool Reader::directoryExists(const std::string& name)
{
	return resourceExists(name) && PHYSFS_isDirectory(name.c_str());
}

bool Reader::fileExists(const std::string& name)
{
	return resourceExists(name) && !PHYSFS_isDirectory(name.c_str());
}

Gosu::Buffer* Reader::readBuffer(const std::string& name)
{
	Gosu::Buffer* buf = new Gosu::Buffer();

	if (readFromDisk(name, *buf)) {
		return buf;
	}
	else {
		delete buf;
		return NULL;
	}
}

std::string Reader::readString(const std::string& name)
{
	std::string str;
	return readFromDisk(name, str) ? str : "";
}

std::string Reader::getText(const std::string& name)
{
	std::shared_ptr<std::string> existing = texts.momentaryRequest(name);
	if (existing)
		return *existing.get();

	std::shared_ptr<std::string> result(new std::string(readString(name)));

	texts.momentaryPut(name, result);
	return *result.get();
}

void Reader::garbageCollect()
{
	Images::instance().garbageCollect();
	Music::instance().garbageCollect();
	Sounds::instance().garbageCollect();
	XMLs::instance().garbageCollect();
	texts.garbageCollect();
}

