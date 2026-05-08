#ifndef DRUMKIT_WINDOW_H
#define DRUMKIT_WINDOW_H

#include <array>
#include <future>
#include <vector>

#pragma warning(push, 0)
#include <FL/Fl_Double_Window.H>
#pragma warning(pop)

#include "command.h"
#include "it-module.h"
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
	{ 190, 286, "C:" },
	{ 410, 268, "D♭/C♯:" },
	{ 190, 250, "D:" },
	{ 410, 232, "E♭/D♯:" },
	{ 190, 214, "E:" },
	{ 190, 178, "F:" },
	{ 410, 160, "G♭/F♯:" },
	{ 190, 142, "G:" },
	{ 410, 124, "A♭/G♯:" },
	{ 190, 106, "A:" },
	{ 410,  88, "B♭/A♯:" },
	{ 190,  70, "B:" },
};

class Drum_Note_Table : public OS_Table {
private:
	enum Columns {
		LENGTH,
		VOLUME,
		FADE,
		SHIFT,
		WIDTH,
		DIVIDER,
		NUM_COLUMNS
	};
	static inline const char *COLUMN_LABELS[NUM_COLUMNS] = { "Length", "Volume", "Fade", "Shift", "Width", "Divider" };
	static inline int COLUMN_MIN[NUM_COLUMNS] = { 0, 0, -7, 0, 0, 0 };
	static inline int COLUMN_MAX[NUM_COLUMNS] = { 255, 15, 7, 15, 1, 7 };
public:
	Drum_Note_Table(int x, int y, int w, int h, const char *l = nullptr);
	void clear() override;
	void set(Drum *drum);
	void add_row();
	void remove_row();
protected:
	void draw_cell(TableContext context, int R = 0, int C = 0, int X = 0, int Y = 0, int W = 0, int H = 0) override;
	void find_coord(int X, int Y, int &R, int &C);
	Fl_Widget *find_child(int X, int Y);

	static void edit_note_cb(OS_Int_Input *i, Drum_Note_Table *dt);
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
	std::array<Dropdown *, NUM_DRUMS_PER_DRUMKIT> _drumkit_drum_dropdowns;
	std::array<OS_Button *, NUM_DRUMS_PER_DRUMKIT - 1> _drumkit_drum_buttons;
	OS_Tab *_drum_tab = nullptr;
	OS_Button *_add_drum_button = nullptr;
	OS_Button *_remove_drum_button = nullptr;
	OS_Button *_copy_drum_button = nullptr;
	OS_Button *_drum_up_button = nullptr;
	OS_Button *_drum_down_button = nullptr;
	OS_Browser *_drum_browser = nullptr;
	OS_Button *_add_note_button = nullptr;
	OS_Button *_remove_note_button = nullptr;
	OS_Light_Button *_play_button = nullptr;
	Drum_Note_Table *_drum_note_table = nullptr;
	Default_Button *_save_button = nullptr;
	OS_Button *_revert_button = nullptr;
	OS_Button *_close_button = nullptr;
	Modal_Dialog *_error_dialog = nullptr;
	Modal_Dialog *_success_dialog = nullptr;
	Modal_Dialog *_confirm_dialog = nullptr;
	New_Name_Dialog *_new_name_dialog = nullptr;

	Drumkits _saved_drumkits;
	Drumkits _drumkits;
	int _selected_drumkit = 0;
	int _selected_drum = 0;

	std::vector<std::vector<uint8_t>> _drum_samples;
	Pitch _playing_drum = Pitch::REST;
	int _playing_drumkit = 0;
	IT_Module *_mod = nullptr;
	int32_t _mod_channel = -1;
	std::thread _audio_thread;
	std::mutex _audio_mutex;
	std::promise<void> _audio_kill_signal;
public:
	Drumkit_Window(int x, int y);
	~Drumkit_Window();
private:
	void initialize();
	void refresh();
	void start_audio_thread();
	void stop_audio_thread();
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
	void regenerate_mod();
private:
	static void close_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void cancel_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void save_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void revert_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void tabs_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void add_drumkit_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void remove_drumkit_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void move_drumkit_up_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void move_drumkit_down_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void select_drumkit_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void edit_drumkit_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void play_drumkit_drum_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void add_drum_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void remove_drum_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void move_drum_up_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void move_drum_down_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void select_drum_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void add_note_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void remove_note_cb(Fl_Widget *w, Drumkit_Window *dw);
	static void play_drum_cb(Fl_Widget *w, Drumkit_Window *dw);

	static void playback_thread(Drumkit_Window *dw, std::future<void> kill_signal);
};

#endif
