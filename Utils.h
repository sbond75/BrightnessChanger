#pragma once

//settings//
//#define WIN32_SUPPORT_ONLY
// //

#ifdef _WIN32
#include <windows.h>  //(it seems winmain can be required - OH its because of my subsystem setting being subsystem:windows or whatever.. no matter though!
#endif
#ifdef WIN32_SUPPORT_ONLY
#include <stdlib.h>  
#include <string.h>  
#include <tchar.h>  
typedef TCHAR Char;
#define text(t) _T(t)

#if !defined(_WIN32) //we check this because we use Win32 API stuff so this only works with Windows. it has nothing to do with Robot, because Robot works with Windows, Mac, and Linux.
#error This program doesn't support an OS other than Windows
#endif
#else
#include <boxer/boxer.h>
typedef char Char;
#define text(t) (t)
#endif

namespace Utils {

#ifdef _WIN32
	static const Char* SYSTEM_KEY_NAME = "Windows Key"; //this is the name of the key that is the "super" key, meta key, or windows logo key - all the same things..! (referred to my microsoft as "Windows system key" ( https://www.google.com/search?q=system+key&oq=system+key&aqs=chrome..69i57j69i65j0l4.3015j0j7&sourceid=chrome&es_sm=122&ie=UTF-8#safe=off&q=system+key+on+keyboard )
#elif __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_IPHONE_SIMULATOR
         // iOS Simulator
    #elif TARGET_OS_IPHONE
        // iOS device
    #elif TARGET_OS_MAC
        // Other kinds of Mac OS
	static const Char* SYSTEM_KEY_NAME = "Command Key"; //TODO [major]: will this not conflict with mac buttons... probably will conflict! command+B is for activating bold when typing in mac, but we use it here! and command+esc... is that ok?
    #else
    #   error "Unknown Apple platform"
    #endif
#elif defined(__linux__) || defined(__unix__) || defined(_POSIX_VERSION)
	static const Char* SYSTEM_KEY_NAME = "Super/Meta Key";
#else
#   error "Unknown compiler"
#endif

#ifdef WIN32_SUPPORT_ONLY
	static inline void msgBox(const Char* msg, const Char* title) { MessageBox(NULL, msg, title, MB_OK); }
	//returns true if user pressed the given button ID (IDOK, IDCANCEL, etc.)
	static inline bool msgBox_multiOption(const Char* msg, const Char* title, UINT type = MB_OKCANCEL, int buttonIDTypeForWhichToReturnTrueIfPressed = IDOK) { return (bool)(MessageBox(NULL, msg, title, type) == buttonIDTypeForWhichToReturnTrueIfPressed); }
#else
	static inline void msgBox(const Char* msg, const Char* title) { boxer::show(msg, title); } //(note that Boxer still needs to be compiled for every needed platform USING CMAKE, whenever you are ready to deploy on other platforms of course...)
	/* //TODO, if need more messgaebox functionality: just use the other buttons yo ucan put on there:
		"A style / set of buttons may also be specified, and the user's selection can be determined from the function's return value:
		boxer::Selection sel = boxer::show("Make a choice:", "Decision", boxer::Style::Warning, boxer::Buttons::YesNo);
		Calls to 'show' are blocking - execution of your program will not continue until the user dismisses the message box." 
	*/ // (https://github.com/aaronmjacobs/Boxer )
#endif

}






//useful reference ( https://stackoverflow.com/questions/5919996/how-to-detect-reliably-mac-os-x-ios-linux-windows-in-c-preprocessor ) :
/*"
There are predefined macros that are used by most compilers, you can find the list [here: http://sourceforge.net/p/predef/wiki/OperatingSystems/ ]. GCC compiler predefined macros can be found [here: http://gcc.gnu.org/onlinedocs/cpp/Predefined-Macros.html ]. Here is an example for gcc:
"*/
/*
#ifdef _WIN32
   //define something for Windows (32-bit and 64-bit, this part is common)
   #ifdef _WIN64
      //define something for Windows (64-bit only)
   #else
      //define something for Windows (32-bit only)
   #endif
#elif __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_IPHONE_SIMULATOR
         // iOS Simulator
    #elif TARGET_OS_IPHONE
        // iOS device
    #elif TARGET_OS_MAC
        // Other kinds of Mac OS
    #else
    #   error "Unknown Apple platform"
    #endif
#elif __linux__
    // linux
#elif __unix__ // all unices not caught above
    // Unix
#elif defined(_POSIX_VERSION)
    // POSIX
#else
#   error "Unknown compiler"
#endif
*/
/*"
This defined macroses depends on compiler that you are going to use.
The _WIN64 #ifdef can be nested into the _WIN32 #ifdef because _WIN32 is defined when targeting Windows, not only the x86 version. This prevents code duplication if some includes are common to both.
"*/