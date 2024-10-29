#ifndef COCOA_H
#define COCOA_H

#pragma warning(push, 0)
#include <FL/Fl_Window.H>
#pragma warning(pop)

enum cocoa_appearance {
	COCOA_APPEARANCE_AQUA,
	COCOA_APPEARANCE_DARK_AQUA
};

void cocoa_set_appearance(const Fl_Window *w, enum cocoa_appearance appearance_id);
bool cocoa_is_dark_mode();

#endif
