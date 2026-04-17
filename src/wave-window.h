#ifndef WAVE_WINDOW_H
#define WAVE_WINDOW_H

#include <future>
#include <vector>

#pragma warning(push, 0)
#include <FL/Fl_Double_Window.H>
#pragma warning(pop)

#include "command.h"
#include "it-module.h"
#include "modal-dialog.h"
#include "parse-waves.h"
#include "widgets.h"

class Wave_Graph : public Fl_Box {
public:
	Wave_Graph(int x, int y, int w, int h, const char *l = NULL);
	void draw();
	int handle(int event);
};

class Wave_Display : public Fl_Box {
public:
	Wave_Display(int x, int y, int w, int h, const char *l = NULL);
	void draw();
};

class Wave_Window {
private:
	int _dx, _dy;
	bool _canceled = false;
	Fl_Double_Window *_window = nullptr;
	OS_Button *_add_button = nullptr;
	OS_Button *_remove_button = nullptr;
	OS_Button *_up_button = nullptr;
	OS_Button *_down_button = nullptr;
	OS_Browser *_wave_browser = nullptr;
	Wave_Graph *_wave_graph = nullptr;
	Wave_Display *_wave_display = nullptr;
	OS_Light_Button *_play_button = nullptr;
	Dropdown *_pitch_input = nullptr;
	Dropdown *_octave_input = nullptr;
	OS_Button *_copy_button = nullptr;
	OS_Button *_paste_button = nullptr;
	OS_Button *_phase_left_button = nullptr;
	OS_Button *_phase_right_button = nullptr;
	OS_Button *_shift_up_button = nullptr;
	OS_Button *_shift_down_button = nullptr;
	OS_Button *_flip_button = nullptr;
	OS_Button *_invert_button = nullptr;
	Default_Button *_save_button = nullptr;
	OS_Button *_revert_button = nullptr;
	OS_Button *_close_button = nullptr;
	Modal_Dialog *_error_dialog = nullptr;
	Modal_Dialog *_success_dialog = nullptr;
	Modal_Dialog *_confirm_dialog = nullptr;

	Waves _saved_waves;
	Waves _waves;
	int _selected_wave = 0;
	Wave _clipboard;

	Pitch _playing_pitch = Pitch::REST;
	int32_t _playing_octave = 0;
	int _playing_instrument = 0;
	IT_Module *_mod = nullptr;
	int32_t _mod_channel = -1;
	std::thread _audio_thread;
	std::mutex _audio_mutex;
	std::promise<void> _audio_kill_signal;
public:
	Wave_Window(int x, int y);
	~Wave_Window();
private:
	void initialize();
	void refresh();
	void start_audio_thread();
	void stop_audio_thread();
	bool modified();
	bool write_waves(const char *f);
public:
	inline bool canceled() const { return _canceled; }
	inline void canceled(bool c) { _canceled = c; }
	inline const Waves &saved_waves() const { return _saved_waves; }
	Wave *wave();
	void waves(const Waves &w);
	void show(const Fl_Widget *p);
	void redraw_wave();
	void regenerate_mod();
private:
	static void close_cb(Fl_Widget *w, Wave_Window *ww);
	static void cancel_cb(Fl_Widget *w, Wave_Window *ww);
	static void save_cb(Fl_Widget *w, Wave_Window *ww);
	static void revert_cb(Fl_Widget *w, Wave_Window *ww);
	static void add_wave_cb(Fl_Widget *w, Wave_Window *ww);
	static void remove_wave_cb(Fl_Widget *w, Wave_Window *ww);
	static void move_wave_up_cb(Fl_Widget *w, Wave_Window *ww);
	static void move_wave_down_cb(Fl_Widget *w, Wave_Window *ww);
	static void select_wave_cb(Fl_Widget *w, Wave_Window *ww);
	static void play_cb(Fl_Widget *w, Wave_Window *ww);
	static void pitch_cb(Fl_Widget *w, Wave_Window *ww);
	static void octave_cb(Fl_Widget *w, Wave_Window *ww);
	static void copy_cb(Fl_Widget *w, Wave_Window *ww);
	static void paste_cb(Fl_Widget *w, Wave_Window *ww);
	static void phase_left_cb(Fl_Widget *w, Wave_Window *ww);
	static void phase_right_cb(Fl_Widget *w, Wave_Window *ww);
	static void shift_up_cb(Fl_Widget *w, Wave_Window *ww);
	static void shift_down_cb(Fl_Widget *w, Wave_Window *ww);
	static void flip_cb(Fl_Widget *w, Wave_Window *ww);
	static void invert_cb(Fl_Widget *w, Wave_Window *ww);

	static void playback_thread(Wave_Window *ww, std::future<void> kill_signal);
};

#endif
