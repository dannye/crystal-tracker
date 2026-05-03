#ifndef DRUMKIT_WINDOW_H
#define DRUMKIT_WINDOW_H

#include <array>

#pragma warning(push, 0)
#include <FL/Fl_Double_Window.H>
#pragma warning(pop)

#include "modal-dialog.h"
#include "option-dialogs.h"
#include "parse-drumkits.h"
#include "widgets.h"

struct Drum_Dropdown {
	int x, y;
	const char *label;
};

constexpr Drum_Dropdown DRUM_DROPDOWNS[NUM_DRUMS_PER_DRUMKIT] {
	{ 463,  45, "Rest:" },
	{ 210, 286, "C:" },
	{ 410, 268, "D♭/C♯:" },
	{ 210, 250, "D:" },
	{ 410, 232, "E♭/D♯:" },
	{ 210, 214, "E:" },
	{ 210, 178, "F:" },
	{ 410, 160, "G♭/F♯:" },
	{ 210, 142, "G:" },
	{ 410, 124, "A♭/G♯:" },
	{ 210, 106, "A:" },
	{ 410,  88, "B♭/A♯:" },
	{ 210,  70, "B:" },
};

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
	std::array<Dropdown *, NUM_DRUMS_PER_DRUMKIT> _drumkit_drums;
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
	Drumkit_Name_Dialog *_drumkit_name_dialog = nullptr;

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
	bool modified();
	bool write_drumkits(const char *f);
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
	static void edit_drumkit_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void add_drum_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void remove_drum_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void move_drum_up_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void move_drum_down_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void select_drum_cb(Fl_Widget *w, Drumkit_Window *dw);
};

#endif
