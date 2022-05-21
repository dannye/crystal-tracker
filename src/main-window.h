#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <FL/Fl_Window.H>

class Main_Window : public Fl_Window {
private:

public:
	Main_Window(int x, int y, int w, int h, const char* l = NULL);
	~Main_Window();
	void show(void);
};

#endif
