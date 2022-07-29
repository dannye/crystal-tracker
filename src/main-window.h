#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <future>

#pragma warning(push, 0)
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <FL/Fl_Native_File_Chooser.H>
#pragma warning(pop)

#include "widgets.h"
#include "modal-dialog.h"
#include "song.h"
#include "piano-roll.h"
#include "it-module.h"
#include "parse-waves.h"
#include "help-window.h"
#include "directory-chooser.h"

#define NUM_RECENT 10

#define NEW_SONG_NAME "New Song"

class Main_Window : public Fl_Double_Window {
private:
	// GUI containers
	Fl_Sys_Menu_Bar *_menu_bar;
	Toolbar *_toolbar;
	Toolbar *_status_bar;
	// GUI inputs
	Fl_Menu_Item *_recent_mis[NUM_RECENT];
	Fl_Menu_Item
		*_classic_theme_mi = NULL,
		*_aero_theme_mi = NULL,
		*_metro_theme_mi = NULL,
		*_aqua_theme_mi = NULL,
		*_greybird_theme_mi = NULL,
		*_ocean_theme_mi = NULL,
		*_blue_theme_mi = NULL,
		*_olive_theme_mi = NULL,
		*_rose_gold_theme_mi = NULL,
		*_dark_theme_mi = NULL,
		*_brushed_metal_theme_mi = NULL,
		*_high_contrast_theme_mi = NULL;
	Toolbar_Button
		*_new_tb = NULL,
		*_open_tb = NULL,
		*_save_tb = NULL,
		*_save_as_tb = NULL,
		*_play_pause_tb = NULL,
		*_stop_tb = NULL;
	Toolbar_Toggle_Button
		*_loop_tb = NULL;
	Toolbar_Button
		*_undo_tb = NULL,
		*_redo_tb = NULL;
	Toolbar_Radio_Button
		*_channel_one_tb = NULL,
		*_channel_two_tb = NULL,
		*_channel_three_tb = NULL,
		*_channel_four_tb = NULL;
	// GUI outputs
	Piano_Roll *_piano_roll = NULL;
	Label *_status_label;
	// Conditional menu items
	Fl_Menu_Item
		*_close_mi = NULL,
		*_save_mi = NULL,
		*_save_as_mi = NULL,
		*_play_pause_mi = NULL,
		*_stop_mi = NULL,
		*_loop_mi = NULL,
		*_undo_mi = NULL,
		*_redo_mi = NULL,
		*_delete_mi = NULL,
		*_channel_one_mi = NULL,
		*_channel_two_mi = NULL,
		*_channel_three_mi = NULL,
		*_channel_four_mi = NULL,
		*_next_channel_mi = NULL,
		*_previous_channel_mi = NULL;
	// Dialogs
	Directory_Chooser *_new_dir_chooser;
	Fl_Native_File_Chooser *_asm_open_chooser, *_asm_save_chooser;
	Modal_Dialog *_error_dialog, *_warning_dialog, *_success_dialog, *_unsaved_dialog, *_about_dialog;
	Help_Window *_help_window;
	// Data
	std::string _status_message = "Ready";
	std::string _directory, _asm_file;
	std::string _recent[NUM_RECENT];
	Song _song;
	std::vector<Wave> _waves;
	IT_Module *_it_module = nullptr;
	// Work properties
	int _selected_channel = 0;
	// Threads
	std::thread _audio_thread;
	std::mutex _audio_mutex;
	std::promise<void> _kill_signal;
	// Window size cache
	int _wx, _wy, _ww, _wh;
#ifdef __X11__
	// Window icons
	Pixmap _icon_pixmap, _icon_mask;
#endif
public:
	Main_Window(int x, int y, int w, int h, const char *l = NULL);
	~Main_Window();
	void show(void) override;
	void resize(int X, int Y, int W, int H) override;
	bool maximized(void) const;
	void maximize(void);
	inline bool loop(void) const { return _loop_mi && !!_loop_mi->value(); }
	inline int selected_channel(void) const { return _selected_channel; }
	bool unsaved(void) const;
	const char *modified_filename(void);
	int handle(int event) override;
	void open_song(const char *filename);
private:
	inline void selected_channel(int i) { _selected_channel = i; }
	void update_active_controls(void);
	void update_channel_detail(void);
	void store_recent_song(void);
	void update_recent_songs(void);
	void open_song(const char *directory, const char *filename);
	void open_recent(int n);
	bool save_song(bool force);
	void toggle_playback();
	void stop_playback();
	void start_audio_thread();
	void stop_audio_thread();
	void update_icons(void);
	// File menu
	static void new_cb(Fl_Widget *w, Main_Window *mw);
	static void open_cb(Fl_Widget *w, Main_Window *mw);
	static void open_recent_cb(Fl_Menu_ *m, Main_Window *mw);
	static void clear_recent_cb(Fl_Menu_ *m, Main_Window *mw);
	static void close_cb(Fl_Widget *w, Main_Window *mw);
	static void save_cb(Fl_Widget *w, Main_Window *mw);
	static void save_as_cb(Fl_Widget *w, Main_Window *mw);
	static void exit_cb(Fl_Widget *w, Main_Window *mw);
	// Play menu
	static void play_pause_cb(Fl_Widget *w, Main_Window *mw);
	static void stop_cb(Fl_Widget *w, Main_Window *mw);
	static void loop_cb(Fl_Menu_ *m, Main_Window *mw);
	// Edit menu
	static void undo_cb(Fl_Widget *w, Main_Window *mw);
	static void redo_cb(Fl_Widget *w, Main_Window *mw);
	static void delete_cb(Fl_Widget *w, Main_Window *mw);
	static void channel_one_cb(Fl_Menu_ *m, Main_Window *mw);
	static void channel_two_cb(Fl_Menu_ *m, Main_Window *mw);
	static void channel_three_cb(Fl_Menu_ *m, Main_Window *mw);
	static void channel_four_cb(Fl_Menu_ *m, Main_Window *mw);
	static void next_channel_cb(Fl_Menu_ *m, Main_Window *mw);
	static void previous_channel_cb(Fl_Menu_ *m, Main_Window *mw);
	void sync_channel_buttons();
	// View menu
	static void classic_theme_cb(Fl_Menu_ *m, Main_Window *mw);
	static void aero_theme_cb(Fl_Menu_ *m, Main_Window *mw);
	static void metro_theme_cb(Fl_Menu_ *m, Main_Window *mw);
	static void aqua_theme_cb(Fl_Menu_ *m, Main_Window *mw);
	static void greybird_theme_cb(Fl_Menu_ *m, Main_Window *mw);
	static void ocean_theme_cb(Fl_Menu_ *m, Main_Window *mw);
	static void blue_theme_cb(Fl_Menu_ *m, Main_Window *mw);
	static void olive_theme_cb(Fl_Menu_ *m, Main_Window *mw);
	static void rose_gold_theme_cb(Fl_Menu_ *m, Main_Window *mw);
	static void dark_theme_cb(Fl_Menu_ *m, Main_Window *mw);
	static void brushed_metal_theme_cb(Fl_Menu_ *m, Main_Window *mw);
	static void high_contrast_theme_cb(Fl_Menu_ *m, Main_Window *mw);
	// Toolbar buttons
	static void loop_tb_cb(Toolbar_Toggle_Button *tb, Main_Window *mw);
	static void channel_one_tb_cb(Toolbar_Radio_Button *tb, Main_Window *mw);
	static void channel_two_tb_cb(Toolbar_Radio_Button *tb, Main_Window *mw);
	static void channel_three_tb_cb(Toolbar_Radio_Button *tb, Main_Window *mw);
	static void channel_four_tb_cb(Toolbar_Radio_Button *tb, Main_Window *mw);
	// Help menu
	static void help_cb(Fl_Widget *w, Main_Window *mw);
	static void about_cb(Fl_Widget *w, Main_Window *mw);
	// Audio playback
	static void playback_thread(Main_Window *mw, std::future<void> kill_signal);
	static void sync_cb(Main_Window *mw);
};

#endif
