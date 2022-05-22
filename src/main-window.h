#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#pragma warning(push, 0)
#include <FL/Fl_Double_Window.H>
#pragma warning(pop)

class Main_Window : public Fl_Double_Window {
private:
	// Window size cache
	int _wx, _wy, _ww, _wh;
#ifndef _WIN32
	// Window icons
	Pixmap _icon_pixmap, _icon_mask;
#endif
public:
	Main_Window(int x, int y, int w, int h, const char* l = NULL);
	~Main_Window();
	void show(void);
	bool maximized(void) const;
	void maximize(void);
private:
	// File menu
	static void exit_cb(Fl_Widget *w, Main_Window *mw);
};

#endif
