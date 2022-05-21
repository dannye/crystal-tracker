#include <FL/Fl.H>
#include <FL/fl_draw.H>

#include "version.h"
#include "main-window.h"

#ifdef _WIN32
#include "resource.h"
#else
#include "app-icon.xpm"
#endif

Main_Window::Main_Window(int x, int y, int w, int h, const char*) : Fl_Window(x, y, w, h, PROGRAM_NAME) {
	// Configure window icon
#ifdef _WIN32
	icon((const void *)LoadIcon(fl_display, MAKEINTRESOURCE(IDI_ICON1)));
#else
	fl_open_display();
	XpmCreatePixmapFromData(fl_display, DefaultRootWindow(fl_display), (char **)&APP_ICON_XPM, &_icon_pixmap, &_icon_mask, NULL);
	icon((const void *)_icon_pixmap);
#endif
}

Main_Window::~Main_Window() {
	//
}

void Main_Window::show() {
	Fl_Window::show();
}
