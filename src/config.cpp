#include <cstdio>
#include <cstring>

#pragma warning(push, 0)
#include <FL/filename.H>
#pragma warning(pop)

#include "utils.h"
#include "config.h"

bool Config::project_path_from_asm_path(const char *asm_path, char *project_path) {
	char scratch_path[FL_PATH_MAX] = {};
	fl_filename_absolute(scratch_path, asm_path);
	char makefile[FL_PATH_MAX] = {};
	for (;;) {
		char *pivot = strrchr(scratch_path, *DIR_SEP);
		if (!pivot) { return false; }
		*pivot = '\0';
		// Make sure there's enough room for "/Makefile\0" (10 chars)
		if (pivot - scratch_path + 10 > FL_PATH_MAX) { return false; }
		strcpy(makefile, scratch_path);
		strcat(makefile, DIR_SEP "Makefile");
		if (!file_exists(makefile)) {
			strcpy(makefile, scratch_path);
			strcat(makefile, DIR_SEP "makefile");
		}
		if (file_exists(makefile)) { // the project directory contains a Makefile
			strcat(scratch_path, DIR_SEP);
			strcpy(project_path, scratch_path);
			return true;
		}
	}
}
