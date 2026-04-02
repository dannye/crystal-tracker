#ifndef DRUMKIT_WINDOW_H
#define DRUMKIT_WINDOW_H

#pragma warning(push, 0)
#include <FL/Fl_Double_Window.H>
#pragma warning(pop)

#include "widgets.h"

class Drumkit_Double_Window : public Fl_Double_Window {
public:
	Drumkit_Double_Window(int x, int y, int w, int h, const char *l = NULL);
	int handle(int event);
};

class Drumkit_Window {
private:
	int _dx, _dy;
	bool _canceled;
	Drumkit_Double_Window *_window;
	Default_Button *_ok_button;
	friend class Drumkit_Double_Window;
public:
	Drumkit_Window(int x, int y);
	~Drumkit_Window();
private:
	void initialize(void);
	void refresh(void);
public:
	inline bool canceled(void) const { return _canceled; }
	inline void canceled(bool c) { _canceled = c; }
	void show(const Fl_Widget *p);
private:
	static void close_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void cancel_cb(Fl_Widget *w, Drumkit_Window *dw);
};

#endif
