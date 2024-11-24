#ifndef VERSION_H
#define VERSION_H

#define PROGRAM_NAME "Crystal Tracker"

#define PROGRAM_AUTHOR "dannye"

#define CURRENT_YEAR "2024"

#define PROGRAM_VERSION 0,8,7
#ifdef _DEBUG
#define PROGRAM_VERSION_STRING "0.8.7 [DEBUG]"
#else
#define PROGRAM_VERSION_STRING "0.8.7"
#endif

#define PROGRAM_EXE_NAME "crystaltracker"

#ifdef _WIN32
#define PROGRAM_EXE PROGRAM_EXE_NAME ".exe"
#elif defined(__APPLE__)
#define PROGRAM_EXE PROGRAM_NAME ".app"
#else
#define PROGRAM_EXE PROGRAM_EXE_NAME
#endif

#endif
