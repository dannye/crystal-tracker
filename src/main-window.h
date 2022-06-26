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
		*_play_stop_tb = NULL;
	// GUI outputs
	Piano_Roll *_piano_roll = NULL;
	Label *_status_label;
	// Conditional menu items
	Fl_Menu_Item
		*_close_mi = NULL,
		*_save_mi = NULL,
		*_save_as_mi = NULL,
		*_play_stop_mi = NULL;
	// Dialogs
	Directory_Chooser *_new_dir_chooser;
	Fl_Native_File_Chooser *_asm_open_chooser;
	Modal_Dialog *_error_dialog, *_warning_dialog, *_success_dialog, *_about_dialog;
	Help_Window *_help_window;
	// Data
	std::string _status_message = "Ready";
	std::string _directory, _asm_file;
	std::string _recent[NUM_RECENT];
	Song _song;
	std::vector<Wave> _waves;
	IT_Module *_it_module = nullptr;
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
	int handle(int event) override;
	void open_song(const char *filename);
private:
	void update_active_controls(void);
	void store_recent_song(void);
	void update_recent_songs(void);
	void open_song(const char *directory, const char *filename);
	void open_recent(int n);
	void toggle_playback();
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
	static void play_stop_cb(Fl_Widget *w, Main_Window *mw);
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
	// Help menu
	static void help_cb(Fl_Widget *w, Main_Window *mw);
	static void about_cb(Fl_Widget *w, Main_Window *mw);
	// Audio playback
	static void playback_thread(Main_Window *mw, std::future<void> kill_signal);
};

#endif
