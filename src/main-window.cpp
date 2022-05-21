
#include <FL/Fl.H>

#include "version.h"
#include "main-window.h"

Main_Window::Main_Window(int x, int y, int w, int h, const char*) : Fl_Window(x, y, w, h, PROGRAM_NAME) {
	//
}

Main_Window::~Main_Window() {
	//
}

void Main_Window::show() {
	Fl_Window::show();
}
