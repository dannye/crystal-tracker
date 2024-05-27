#ifndef RULER_H
#define RULER_H

#pragma warning(push, 0)
#include <FL/Fl_Box.H>
#pragma warning(pop)

#include "option-dialogs.h"

class Ruler : public Fl_Box {
private:
	Ruler_Config_Dialog::Ruler_Options _options;
public:
	Ruler(int x, int y, int w, int h, const char *l = NULL);
	int handle(int event) override;
	void draw(void) override;

	Ruler_Config_Dialog::Ruler_Options get_options() const { return _options; }
	void set_options(const Ruler_Config_Dialog::Ruler_Options &o) { _options = o; }
};

#endif
