#ifndef RULER_H
#define RULER_H

#pragma warning(push, 0)
#include <FL/Fl_Box.H>
#pragma warning(pop)

class Ruler : public Fl_Box {
public:
	Ruler(int x, int y, int w, int h, const char *l = NULL);
	int handle(int event) override;
	void draw(void);
};

#endif
