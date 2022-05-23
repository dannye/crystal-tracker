#ifndef CONFIG_H
#define CONFIG_H

#include <cstdlib>

class Config {
private:
public:
	static bool project_path_from_asm_path(const char *asm_path, char *project_path);
};

#endif
