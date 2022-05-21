#ifndef VERSION_H
#define VERSION_H

#define PROGRAM_VERSION 0,0,0
#ifdef _DEBUG
#define PROGRAM_VERSION_STRING "0.0.0 [DEBUG]"
#else
#define PROGRAM_VERSION_STRING "0.0.0"
#endif

#define PROGRAM_EXE_NAME "crystaltracker"

#ifdef _WIN32
#define PROGRAM_EXE PROGRAM_EXE_NAME ".exe"
#else
#define PROGRAM_EXE PROGRAM_EXE_NAME
#endif

#define PROGRAM_NAME "Crystal Tracker"

#define PROGRAM_AUTHOR "dannye"

#define CURRENT_YEAR "2022"

#endif
