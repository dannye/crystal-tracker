#ifndef DRUMKIT_WINDOW_H
#define DRUMKIT_WINDOW_H

#pragma warning(push, 0)
#include <FL/Fl_Double_Window.H>
#pragma warning(pop)

#include "modal-dialog.h"
#include "parse-drumkits.h"
#include "widgets.h"

class Drumkit_Window {
private:
	int _dx, _dy;
	bool _canceled = false;
	Fl_Double_Window *_window = nullptr;
	OS_Tabs *_tabs = nullptr;
	OS_Tab *_drumkit_tab = nullptr;
	OS_Button *_add_drumkit_button = nullptr;
	OS_Button *_remove_drumkit_button = nullptr;
	OS_Button *_drumkit_up_button = nullptr;
	OS_Button *_drumkit_down_button = nullptr;
	OS_Browser *_drumkit_browser = nullptr;
	OS_Tab *_drum_tab = nullptr;
	OS_Button *_add_drum_button = nullptr;
	OS_Button *_remove_drum_button = nullptr;
	OS_Button *_drum_up_button = nullptr;
	OS_Button *_drum_down_button = nullptr;
	OS_Browser *_drum_browser = nullptr;
	Default_Button *_save_button = nullptr;
	OS_Button *_revert_button = nullptr;
	OS_Button *_close_button = nullptr;
	Modal_Dialog *_error_dialog = nullptr;
	Modal_Dialog *_success_dialog = nullptr;
	Modal_Dialog *_confirm_dialog = nullptr;

	Drumkits _saved_drumkits;
	Drumkits _drumkits;
	int _selected_drumkit = 0;
	int _selected_drum = 0;
public:
	Drumkit_Window(int x, int y);
	~Drumkit_Window();
private:
	void initialize();
	void refresh();
public:
	inline bool canceled() const { return _canceled; }
	inline void canceled(bool c) { _canceled = c; }
	inline const Drumkits &saved_drumkits() const { return _saved_drumkits; }
	Drumkit *drumkit();
	Drum *drum();
	void drumkits(const Drumkits &d);
	void show(const Fl_Widget *p);
private:
	static void close_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void cancel_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void save_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void revert_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void add_drumkit_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void remove_drumkit_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void move_drumkit_up_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void move_drumkit_down_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void select_drumkit_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void add_drum_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void remove_drum_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void move_drum_up_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void move_drum_down_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void select_drum_cb(Fl_Widget *w, Drumkit_Window *dw);
};

#endif
