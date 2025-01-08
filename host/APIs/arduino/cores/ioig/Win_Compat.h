#pragma once

#if defined(_WIN32) || defined(_WIN64)

#include <memory.h>


//TODO: Fix it on windows 
//The variable MSYS_INCLUDE_DIR is defined on CMakeLists.txt and exposed on code 
//but concatenation with string.h doest not work
//We need to include the full path of string.h file
//Including <string.h> in windows falls in the local file String.h as Windows in case insensitive
#include <C:\msys64\ucrt64\include\string.h>


// These #define statements are used to temporarily redefine conflicting symbols
// in the Windows header file to avoid redefinition errors during the build process.
#define INADDR_NONE WINDOWS_INADDR_NONE  
#define boolean WINDOWS_BOOLEAN    
#define INPUT WINDOWS_INPUT
#include <windows.h>
#undef INPUT
#undef boolean
#undef INADDR_NONE

int WinMain(HINSTANCE, HINSTANCE, LPSTR, int);

#pragma comment(linker, "/alternatename:_pWeakValue=WinMain")

#define atexit _GLOBAL_ATEXIT

#endif

