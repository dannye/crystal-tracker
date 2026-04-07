#ifndef WAVE_WINDOW_H
#define WAVE_WINDOW_H

#include <vector>

#pragma warning(push, 0)
#include <FL/Fl_Double_Window.H>
#pragma warning(pop)

#include "parse-waves.h"
#include "widgets.h"

class Wave_Double_Window : public Fl_Double_Window {
public:
	Wave_Double_Window(int x, int y, int w, int h, const char *l = NULL);
	int handle(int event);
};

class Wave_Window {
private:
	int _dx, _dy;
	bool _canceled;
	Wave_Double_Window *_window;
	OS_Browser *_wave_browser;
	Default_Button *_ok_button;
	friend class Wave_Double_Window;
	std::vector<Wave> _waves;
	int32_t _num_waves;
public:
	Wave_Window(int x, int y);
	~Wave_Window();
private:
	void initialize(void);
	void refresh(void);
public:
	inline bool canceled(void) const { return _canceled; }
	inline void canceled(bool c) { _canceled = c; }
	void waves(const std::vector<Wave> &w, int32_t n);
	void show(const Fl_Widget *p);
private:
	static void close_cb(Fl_Widget *w, Wave_Window *ww);
	static void cancel_cb(Fl_Widget *w, Wave_Window *ww);
};

#endif
