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
#include "option-dialogs.h"
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
	Fl_Menu_Item
		*_continuous_mi = NULL,
		*_channel_1_mute_mi = NULL,
		*_channel_2_mute_mi = NULL,
		*_channel_3_mute_mi = NULL,
		*_channel_4_mute_mi = NULL,
		*_zoom_mi = NULL,
		*_key_labels_mi = NULL,
		*_full_screen_mi = NULL;
	Toolbar_Button
		*_new_tb = NULL,
		*_open_tb = NULL,
		*_save_tb = NULL,
		*_save_as_tb = NULL,
		*_play_pause_tb = NULL,
		*_stop_tb = NULL;
	Toolbar_Toggle_Button
		*_loop_tb = NULL,
		*_continuous_tb = NULL;
	Toolbar_Button
		*_undo_tb = NULL,
		*_redo_tb = NULL;
	Toolbar_Radio_Button
		*_channel_1_tb = NULL,
		*_channel_2_tb = NULL,
		*_channel_3_tb = NULL,
		*_channel_4_tb = NULL;
	Toolbar_Toggle_Button
		*_zoom_tb = NULL;
	Toolbar_Button
		*_decrease_spacing_tb = NULL,
		*_increase_spacing_tb = NULL;
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
		*_step_backward_mi = NULL,
		*_step_forward_mi = NULL,
		*_skip_backward_mi = NULL,
		*_skip_forward_mi = NULL,
		*_undo_mi = NULL,
		*_redo_mi = NULL,
		*_select_all_mi = NULL,
		*_select_none_mi = NULL,
		*_pitch_up_mi = NULL,
		*_pitch_down_mi = NULL,
		*_octave_up_mi = NULL,
		*_octave_down_mi = NULL,
		*_move_left_mi = NULL,
		*_move_right_mi = NULL,
		*_shorten_mi = NULL,
		*_lengthen_mi = NULL,
		*_delete_mi = NULL,
		*_snip_mi = NULL,
		*_split_note_mi = NULL,
		*_glue_note_mi = NULL,
		*_channel_1_mi = NULL,
		*_channel_2_mi = NULL,
		*_channel_3_mi = NULL,
		*_channel_4_mi = NULL,
		*_next_channel_mi = NULL,
		*_previous_channel_mi = NULL,
		*_decrease_spacing_mi = NULL,
		*_increase_spacing_mi = NULL;
	// Dialogs
	Directory_Chooser *_new_dir_chooser;
	Fl_Native_File_Chooser *_asm_open_chooser, *_asm_save_chooser;
	Modal_Dialog *_error_dialog, *_warning_dialog, *_success_dialog, *_unsaved_dialog, *_about_dialog;
	Song_Options_Dialog *_song_options_dialog;
	Help_Window *_help_window;
	// Data
	std::string _status_message = "Ready";
	std::string _directory, _asm_file;
	std::string _recent[NUM_RECENT];
	Song _song;
	std::vector<Wave> _waves;
	IT_Module *_it_module = nullptr;
	int32_t _tick = -1;
	bool _showed_it_warning = false;
	// Work properties
	int _selected_channel = 0;
	bool _sync_requested = false;
	Pitch _playing_pitch = Pitch::REST;
	int32_t _playing_octave = 0;
	// Threads
	std::thread _audio_thread;
	std::mutex _audio_mutex;
	std::promise<void> _audio_kill_signal;
	std::thread _interactive_thread;
	std::mutex _interactive_mutex;
	std::promise<void> _interactive_kill_signal;
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
	inline bool continuous_scroll(void) const { return _continuous_mi && !!_continuous_mi->value(); }
	inline bool channel_1_muted(void) const { return _channel_1_mute_mi && !!_channel_1_mute_mi->value(); }
	inline bool channel_2_muted(void) const { return _channel_2_mute_mi && !!_channel_2_mute_mi->value(); }
	inline bool channel_3_muted(void) const { return _channel_3_mute_mi && !!_channel_3_mute_mi->value(); }
	inline bool channel_4_muted(void) const { return _channel_4_mute_mi && !!_channel_4_mute_mi->value(); }
	inline bool zoom(void) const { return _zoom_mi && !!_zoom_mi->value(); }
	inline bool key_labels(void) const { return _key_labels_mi && !!_key_labels_mi->value(); }
	inline bool full_screen(void) const { return _full_screen_mi && !!_full_screen_mi->value(); }
	inline bool loop(void) const { return _loop_mi && !!_loop_mi->value(); }
	void continuous_scroll(bool c) { c ? _continuous_mi->set() : _continuous_mi->clear(); _continuous_tb->value(_continuous_mi->value()); _menu_bar->update(); }
	inline int selected_channel(void) const { return _selected_channel; }
	bool unsaved(void) const;
	const char *modified_filename(void);
	int handle(int event) override;
	void set_song_position(int32_t tick);
	void open_song(const char *filename);

	bool play_note(Pitch pitch, int32_t octave);
	bool stop_note();
	bool playing_note() { return _playing_pitch != Pitch::REST; }
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
	void start_interactive_thread();
	void stop_interactive_thread();
	void update_icons(void);
	void update_zoom(void);
	// File menu
	static void new_cb(Fl_Widget *w, Main_Window *mw);
	static void open_cb(Fl_Widget *w, Main_Window *mw);
	static void open_recent_cb(Fl_Menu_ *m, Main_Window *mw);
	static void clear_recent_cb(Fl_Widget *w, Main_Window *mw);
	static void close_cb(Fl_Widget *w, Main_Window *mw);
	static void save_cb(Fl_Widget *w, Main_Window *mw);
	static void save_as_cb(Fl_Widget *w, Main_Window *mw);
	static void exit_cb(Fl_Widget *w, Main_Window *mw);
	// Play menu
	static void play_pause_cb(Fl_Widget *w, Main_Window *mw);
	static void stop_cb(Fl_Widget *w, Main_Window *mw);
	static void loop_cb(Fl_Menu_ *m, Main_Window *mw);
	static void continuous_cb(Fl_Menu_ *m, Main_Window *mw);
	static void channel_1_mute_cb(Fl_Widget *w, Main_Window *mw);
	static void channel_2_mute_cb(Fl_Widget *w, Main_Window *mw);
	static void channel_3_mute_cb(Fl_Widget *w, Main_Window *mw);
	static void channel_4_mute_cb(Fl_Widget *w, Main_Window *mw);
	static void step_backward_cb(Fl_Widget *w, Main_Window *mw);
	static void step_forward_cb(Fl_Widget *w, Main_Window *mw);
	static void skip_backward_cb(Fl_Widget *w, Main_Window *mw);
	static void skip_forward_cb(Fl_Widget *w, Main_Window *mw);
	// Edit menu
	void put_note(Pitch pitch);
	static void undo_cb(Fl_Widget *w, Main_Window *mw);
	static void redo_cb(Fl_Widget *w, Main_Window *mw);
	static void select_all_cb(Fl_Widget *w, Main_Window *mw);
	static void select_none_cb(Fl_Widget *w, Main_Window *mw);
	static void pitch_up_cb(Fl_Widget *w, Main_Window *mw);
	static void pitch_down_cb(Fl_Widget *w, Main_Window *mw);
	static void octave_up_cb(Fl_Widget *w, Main_Window *mw);
	static void octave_down_cb(Fl_Widget *w, Main_Window *mw);
	static void move_left_cb(Fl_Widget *w, Main_Window *mw);
	static void move_right_cb(Fl_Widget *w, Main_Window *mw);
	static void shorten_cb(Fl_Widget *w, Main_Window *mw);
	static void lengthen_cb(Fl_Widget *w, Main_Window *mw);
	static void delete_cb(Fl_Widget *w, Main_Window *mw);
	static void snip_cb(Fl_Widget *w, Main_Window *mw);
	static void split_note_cb(Fl_Widget *w, Main_Window *mw);
	static void glue_note_cb(Fl_Widget *w, Main_Window *mw);
	static void channel_1_cb(Fl_Widget *w, Main_Window *mw);
	static void channel_2_cb(Fl_Widget *w, Main_Window *mw);
	static void channel_3_cb(Fl_Widget *w, Main_Window *mw);
	static void channel_4_cb(Fl_Widget *w, Main_Window *mw);
	static void next_channel_cb(Fl_Widget *w, Main_Window *mw);
	static void previous_channel_cb(Fl_Widget *w, Main_Window *mw);
	void sync_channel_buttons();
	// View menu
	static void classic_theme_cb(Fl_Widget *w, Main_Window *mw);
	static void aero_theme_cb(Fl_Widget *w, Main_Window *mw);
	static void metro_theme_cb(Fl_Widget *w, Main_Window *mw);
	static void aqua_theme_cb(Fl_Widget *w, Main_Window *mw);
	static void greybird_theme_cb(Fl_Widget *w, Main_Window *mw);
	static void ocean_theme_cb(Fl_Widget *w, Main_Window *mw);
	static void blue_theme_cb(Fl_Widget *w, Main_Window *mw);
	static void olive_theme_cb(Fl_Widget *w, Main_Window *mw);
	static void rose_gold_theme_cb(Fl_Widget *w, Main_Window *mw);
	static void dark_theme_cb(Fl_Widget *w, Main_Window *mw);
	static void brushed_metal_theme_cb(Fl_Widget *w, Main_Window *mw);
	static void high_contrast_theme_cb(Fl_Widget *w, Main_Window *mw);
	static void zoom_cb(Fl_Menu_ *m, Main_Window *mw);
	static void decrease_spacing_cb(Fl_Widget *w, Main_Window *mw);
	static void increase_spacing_cb(Fl_Widget *w, Main_Window *mw);
	static void key_labels_cb(Fl_Widget *w, Main_Window *mw);
	static void full_screen_cb(Fl_Widget *w, Main_Window *mw);
	// Toolbar buttons
	static void loop_tb_cb(Toolbar_Toggle_Button *tb, Main_Window *mw);
	static void continuous_tb_cb(Toolbar_Toggle_Button *tb, Main_Window *mw);
	static void channel_1_tb_cb(Toolbar_Radio_Button *tb, Main_Window *mw);
	static void channel_2_tb_cb(Toolbar_Radio_Button *tb, Main_Window *mw);
	static void channel_3_tb_cb(Toolbar_Radio_Button *tb, Main_Window *mw);
	static void channel_4_tb_cb(Toolbar_Radio_Button *tb, Main_Window *mw);
	static void zoom_tb_cb(Toolbar_Toggle_Button *tb, Main_Window *mw);
	// Help menu
	static void help_cb(Fl_Widget *w, Main_Window *mw);
	static void about_cb(Fl_Widget *w, Main_Window *mw);
	// Audio playback
	static void playback_thread(Main_Window *mw, std::future<void> kill_signal);
	static void sync_cb(Main_Window *mw);
	static void interactive_thread(Main_Window *mw, std::future<void> kill_signal);
};

#endif
