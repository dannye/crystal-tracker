#ifndef PIANO_ROLL_H
#define PIANO_ROLL_H

#include <array>
#include <list>

#pragma warning(push, 0)
#include <FL/Fl_Box.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Scroll.H>
#pragma warning(pop)

#include "command.h"

const Fl_Color NOTE_RED   = fl_rgb_color(217,   0,   0);
const Fl_Color NOTE_BLUE  = fl_rgb_color(  0, 117, 253);
const Fl_Color NOTE_GREEN = fl_rgb_color(  3, 196,   3);
const Fl_Color NOTE_GRAY  = fl_rgb_color(153, 153, 153);

const Fl_Color ALT_LIGHT_ROW  = fl_rgb_color(35, 35, 35);
const Fl_Color ALT_DARK_ROW   = fl_rgb_color(52, 52, 52);

constexpr size_t NUM_WHITE_NOTES = 7;
constexpr size_t NUM_BLACK_NOTES = 5;

constexpr size_t NUM_NOTES_PER_OCTAVE = NUM_WHITE_NOTES + NUM_BLACK_NOTES;
constexpr size_t NUM_OCTAVES = 8;

constexpr int WHITE_KEY_WIDTH  = 150;
constexpr int WHITE_KEY_HEIGHT = 36;

constexpr int BLACK_KEY_WIDTH  = 100;
constexpr int BLACK_KEY_HEIGHT = 30;

constexpr int OCTAVE_HEIGHT = WHITE_KEY_HEIGHT * NUM_WHITE_NOTES;
constexpr int NOTE_ROW_HEIGHT = OCTAVE_HEIGHT / NUM_NOTES_PER_OCTAVE;

constexpr int BLACK_KEY_OFFSET = NOTE_ROW_HEIGHT / 2 - BLACK_KEY_HEIGHT / 2;

constexpr int TIME_STEP_WIDTH = 48;
constexpr int DEFAULT_SPEED = 12;

struct Note_Key {
	int x, y, w, h;
	const char *label;
	bool white;
};

constexpr Note_Key NOTE_KEYS[NUM_NOTES_PER_OCTAVE] {
	{ 0, WHITE_KEY_HEIGHT * 0 + 0, WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT + 0, "                         B", true },
	{ 0, WHITE_KEY_HEIGHT * 1 + 0, WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT + 1, "                         A", true },
	{ 0, WHITE_KEY_HEIGHT * 2 + 1, WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT + 1, "                         G", true },
	{ 0, WHITE_KEY_HEIGHT * 3 + 2, WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT + 1, "                         F", true },
	{ 0, WHITE_KEY_HEIGHT * 4 + 3, WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT - 1, "                         E", true },
	{ 0, WHITE_KEY_HEIGHT * 5 + 2, WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT - 1, "                         D", true },
	{ 0, WHITE_KEY_HEIGHT * 6 + 1, WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT - 1, "                         C", true },
	{ 0, BLACK_KEY_OFFSET + NOTE_ROW_HEIGHT *  1, BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, "B♭/A♯", false },
	{ 0, BLACK_KEY_OFFSET + NOTE_ROW_HEIGHT *  3, BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, "A♭/G♯", false },
	{ 0, BLACK_KEY_OFFSET + NOTE_ROW_HEIGHT *  5, BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, "G♭/F♯", false },
	{ 0, BLACK_KEY_OFFSET + NOTE_ROW_HEIGHT *  8, BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, "E♭/D♯", false },
	{ 0, BLACK_KEY_OFFSET + NOTE_ROW_HEIGHT * 10, BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, "D♭/C♯", false },
};

class Piano_Keys : Fl_Group {
	friend class Piano_Roll;
private:
	std::array<Fl_Box *, NUM_NOTES_PER_OCTAVE * NUM_OCTAVES> _notes;
public:
	Piano_Keys(int x, int y, int w, int h, const char *l = nullptr);
	~Piano_Keys() noexcept;

	Piano_Keys(const Piano_Keys&) = delete;
	Piano_Keys& operator=(const Piano_Keys&) = delete;
};

class Piano_Timeline : Fl_Group {
	friend class Piano_Roll;
private:
	Piano_Keys *_keys;
	std::list<Fl_Box *> _channel_1_notes;
	std::list<Fl_Box *> _channel_2_notes;
	std::list<Fl_Box *> _channel_3_notes;
	std::list<Fl_Box *> _channel_4_notes;
public:
	Piano_Timeline(int x, int y, int w, int h, const char *l = nullptr);
	~Piano_Timeline() noexcept;

	void clear();

	Piano_Timeline(const Piano_Timeline&) = delete;
	Piano_Timeline& operator=(const Piano_Timeline&) = delete;

	void set_channel_1_timeline(const std::list<Note_View> &timeline) { set_channel_timeline(_channel_1_notes, timeline, NOTE_RED); }
	void set_channel_2_timeline(const std::list<Note_View> &timeline) { set_channel_timeline(_channel_2_notes, timeline, NOTE_BLUE); }
	void set_channel_3_timeline(const std::list<Note_View> &timeline) { set_channel_timeline(_channel_3_notes, timeline, NOTE_GREEN); }
	void set_channel_4_timeline(const std::list<Note_View> &timeline) { set_channel_timeline(_channel_4_notes, timeline, NOTE_GRAY); }

	Fl_Box *get_channel_1_note_at_tick(int32_t tick) { return get_note_at_tick(_channel_1_notes, tick); };
	Fl_Box *get_channel_2_note_at_tick(int32_t tick) { return get_note_at_tick(_channel_2_notes, tick); };
	Fl_Box *get_channel_3_note_at_tick(int32_t tick) { return get_note_at_tick(_channel_3_notes, tick); };
	Fl_Box *get_channel_4_note_at_tick(int32_t tick) { return get_note_at_tick(_channel_4_notes, tick); };

	void toggle_channel_1_box_type() { toggle_channel_box_type(_channel_1_notes); }
	void toggle_channel_2_box_type() { toggle_channel_box_type(_channel_2_notes); }
	void toggle_channel_3_box_type() { toggle_channel_box_type(_channel_3_notes); }
	void toggle_channel_4_box_type() { toggle_channel_box_type(_channel_4_notes); }

	void reset_note_colors();
private:
	void set_channel_timeline(std::list<Fl_Box *> &notes, const std::list<Note_View> &timeline, Fl_Color color);

	Fl_Box *get_note_at_tick(std::list<Fl_Box *> &notes, int32_t tick);

	void toggle_channel_box_type(std::list<Fl_Box *> &notes);
protected:
	void draw() override;
};

class Piano_Roll : Fl_Scroll {
	friend class Piano_Timeline;
private:
	Piano_Timeline *_piano_timeline;
	int32_t _tick = 0;
	bool _following = false;
	bool _realtime = true;
public:
	Piano_Roll(int x, int y, int w, int h, const char *l = nullptr);
	~Piano_Roll() noexcept;

	Piano_Roll(const Piano_Roll&) = delete;
	Piano_Roll& operator=(const Piano_Roll&) = delete;

	int handle(int event) override;

	void set_size(int W, int H);

	void set_channel_1_timeline(const std::list<Note_View> &timeline) { _piano_timeline->set_channel_1_timeline(timeline); }
	void set_channel_2_timeline(const std::list<Note_View> &timeline) { _piano_timeline->set_channel_2_timeline(timeline); }
	void set_channel_3_timeline(const std::list<Note_View> &timeline) { _piano_timeline->set_channel_3_timeline(timeline); }
	void set_channel_4_timeline(const std::list<Note_View> &timeline) { _piano_timeline->set_channel_4_timeline(timeline); }

	void toggle_channel_1_box_type() { _piano_timeline->toggle_channel_1_box_type(); }
	void toggle_channel_2_box_type() { _piano_timeline->toggle_channel_2_box_type(); }
	void toggle_channel_3_box_type() { _piano_timeline->toggle_channel_3_box_type(); }
	void toggle_channel_4_box_type() { _piano_timeline->toggle_channel_4_box_type(); }

	void clear();

	void start_following();
	void stop_following();
	void highlight_tick(int32_t t);
private:
	static void hscrollbar_cb(Fl_Scrollbar *sb, void *);
};

#endif
