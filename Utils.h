#pragma once

#include <windows.h>  
#include <stdlib.h>  
#include <string.h>  
#include <tchar.h>  

#if !defined(_WIN32) //we check this because we use Win32 API stuff so this only works with Windows. it has nothing to do with Robot, because Robot works with Windows, Mac, and Linux.
#error This program doesn't support an OS other than Windows
#endif

namespace Utils {

	static inline void msgBox(const TCHAR* msg, const TCHAR* title) { MessageBox(NULL, msg, title, MB_OK); }
	//returns true if user pressed the given button ID (IDOK, IDCANCEL, etc.)
	static inline bool msgBox_multiOption(const TCHAR* msg, const TCHAR* title, UINT type = MB_OKCANCEL, int buttonIDTypeForWhichToReturnTrueIfPressed = IDOK) { return (bool)(MessageBox(NULL, msg, title, type) == buttonIDTypeForWhichToReturnTrueIfPressed); }

}