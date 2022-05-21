#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <FL/Fl_Window.H>

#ifndef _WIN32
#include <X11/xpm.h>
#endif

class Main_Window : public Fl_Window {
private:
#ifndef _WIN32
	// Window icons
	Pixmap _icon_pixmap, _icon_mask;
#endif

public:
	Main_Window(int x, int y, int w, int h, const char* l = NULL);
	~Main_Window();
	void show(void);
};

#endif
