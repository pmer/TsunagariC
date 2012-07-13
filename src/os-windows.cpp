/*********************************
** Tsunagari Tile Engine        **
** os-windows.cpp               **
** Copyright 2011-2012 OmegaSDG **
*********************************/

#include "os-windows.h"
#include "window.h"

#include <Gosu/Gosu.hpp>

void wFixConsole()
{
	if (AttachConsole(ATTACH_PARENT_PROCESS)) {
		freopen("CONOUT$","wb",stdout);
		freopen("CONOUT$","wb",stderr);
	}
}

void wMessageBox(std::string title, std::string text)
{
	MessageBox(GameWindow::instance().handle(), 
		Gosu::widen(text).c_str(), 
		Gosu::widen(title).c_str(), 
		MB_OK
	);
}
