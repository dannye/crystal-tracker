#ifndef PIANO_ROLL_H
#define PIANO_ROLL_H

#include <array>
#include <vector>

#pragma warning(push, 0)
#include <FL/Fl_Box.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Scroll.H>
#pragma warning(pop)

#include "command.h"
#include "song.h"

const Fl_Color NOTE_RED   = fl_rgb_color(217,   0,   0);
const Fl_Color NOTE_BLUE  = fl_rgb_color(  0, 117, 253);
const Fl_Color NOTE_GREEN = fl_rgb_color(  3, 196,   3);
const Fl_Color NOTE_BROWN = fl_rgb_color(140,  60,  25);

const Fl_Color NOTE_RED_LIGHT   = fl_lighter(NOTE_RED);
const Fl_Color NOTE_BLUE_LIGHT  = fl_lighter(NOTE_BLUE);
const Fl_Color NOTE_GREEN_LIGHT = fl_lighter(NOTE_GREEN);
const Fl_Color NOTE_BROWN_LIGHT = fl_lighter(NOTE_BROWN);

const Fl_Color NOTE_GHOST = fl_rgb_color(96);

const Fl_Color ALT_LIGHT_ROW = fl_rgb_color(35, 35, 35);
const Fl_Color ALT_DARK_ROW  = fl_rgb_color(52, 52, 52);

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

constexpr int TICK_WIDTH = 4;
constexpr int TICKS_PER_STEP = 12;
constexpr int TIME_STEP_WIDTH = TICK_WIDTH * TICKS_PER_STEP;

struct Note_Key {
	int x, y, w, h, delta;
	const char *label;
	bool white;
};

constexpr Note_Key NOTE_KEYS[NUM_NOTES_PER_OCTAVE] {
	{ 0, WHITE_KEY_HEIGHT * 0, WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT,  0, "B", true },
	{ 0, WHITE_KEY_HEIGHT * 1, WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT, +1, "A", true },
	{ 0, WHITE_KEY_HEIGHT * 2, WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT, +1, "G", true },
	{ 0, WHITE_KEY_HEIGHT * 3, WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT, +1, "F", true },
	{ 0, WHITE_KEY_HEIGHT * 4, WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT, -1, "E", true },
	{ 0, WHITE_KEY_HEIGHT * 5, WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT, -1, "D", true },
	{ 0, WHITE_KEY_HEIGHT * 6, WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT, -1, "C", true },
	{ 0, BLACK_KEY_OFFSET + NOTE_ROW_HEIGHT *  1, BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, 0, "B♭/A♯", false },
	{ 0, BLACK_KEY_OFFSET + NOTE_ROW_HEIGHT *  3, BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, 0, "A♭/G♯", false },
	{ 0, BLACK_KEY_OFFSET + NOTE_ROW_HEIGHT *  5, BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, 0, "G♭/F♯", false },
	{ 0, BLACK_KEY_OFFSET + NOTE_ROW_HEIGHT *  8, BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, 0, "E♭/D♯", false },
	{ 0, BLACK_KEY_OFFSET + NOTE_ROW_HEIGHT * 10, BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, 0, "D♭/C♯", false },
};
constexpr size_t PITCH_TO_KEY_INDEX[NUM_NOTES_PER_OCTAVE] {
	6,  // C
	11, // C#
	5,  // D
	10, // D#
	4,  // E
	3,  // F
	9,  // F#
	2,  // G
	8,  // G#
	1,  // A
	7,  // A#
	0,  // B
};

class Note_Box : public Fl_Box {
private:
	const Note_View &_note_view;
	bool _selected = false;
	bool _ghost = false;
public:
	Note_Box(const Note_View &n, int X, int Y, int W, int H, const char *l=0)
		: Fl_Box(X, Y, W, H, l), _note_view(n) {}

	inline const Note_View &note_view(void) const { return _note_view; }
	inline bool selected(void) const { return _selected; }
	inline void selected(bool s) { _selected = s; }
	inline bool ghost(void) const { return _ghost; }
	inline void ghost(bool g) { _ghost = g; }
protected:
	void draw() override;
};

class Loop_Box : public Fl_Box {
public:
	using Fl_Box::Fl_Box;
protected:
	void draw() override;
};

class Call_Box : public Fl_Box {
public:
	using Fl_Box::Fl_Box;
protected:
	void draw() override;
};

class White_Key_Box : public Fl_Box {
public:
	using Fl_Box::Fl_Box;
protected:
	void draw() override;
};

class Piano_Keys : public Fl_Group {
private:
	std::array<Fl_Box *, NUM_NOTES_PER_OCTAVE * NUM_OCTAVES> _notes;
public:
	Piano_Keys(int x, int y, int w, int h, const char *l = nullptr);
	~Piano_Keys() noexcept;

	Piano_Keys(const Piano_Keys&) = delete;
	Piano_Keys& operator=(const Piano_Keys&) = delete;

	void highlight_key(Pitch pitch, int32_t octave, Fl_Color color);
	void reset_key_colors();
};

class Piano_Timeline : public Fl_Group {
	friend class Piano_Roll;
private:
	Piano_Keys *_keys;
	std::vector<Note_Box *> _channel_1_notes;
	std::vector<Note_Box *> _channel_2_notes;
	std::vector<Note_Box *> _channel_3_notes;
	std::vector<Note_Box *> _channel_4_notes;
	std::vector<Loop_Box *> _channel_1_loops;
	std::vector<Loop_Box *> _channel_2_loops;
	std::vector<Loop_Box *> _channel_3_loops;
	std::vector<Loop_Box *> _channel_4_loops;
	std::vector<Call_Box *> _channel_1_calls;
	std::vector<Call_Box *> _channel_2_calls;
	std::vector<Call_Box *> _channel_3_calls;
	std::vector<Call_Box *> _channel_4_calls;
public:
	Piano_Timeline(int x, int y, int w, int h, const char *l = nullptr);
	~Piano_Timeline() noexcept;

	void clear();
	void clear_channel_1();
	void clear_channel_2();
	void clear_channel_3();
	void clear_channel_4();

	Piano_Timeline(const Piano_Timeline&) = delete;
	Piano_Timeline& operator=(const Piano_Timeline&) = delete;

	int handle(int event) override;
	bool handle_note_selection(int event);
	bool select_all();
	bool select_none();

	void highlight_channel_1_tick(int32_t tick) { highlight_tick(_channel_1_notes, tick, NOTE_RED_LIGHT); }
	void highlight_channel_2_tick(int32_t tick) { highlight_tick(_channel_2_notes, tick, NOTE_BLUE_LIGHT); }
	void highlight_channel_3_tick(int32_t tick) { highlight_tick(_channel_3_notes, tick, NOTE_GREEN_LIGHT); }
	void highlight_channel_4_tick(int32_t tick) { highlight_tick(_channel_4_notes, tick, NOTE_BROWN_LIGHT); }

	void set_channel_1(const std::vector<Note_View> &notes) { set_channel(_channel_1_notes, notes, NOTE_RED); }
	void set_channel_2(const std::vector<Note_View> &notes) { set_channel(_channel_2_notes, notes, NOTE_BLUE); }
	void set_channel_3(const std::vector<Note_View> &notes) { set_channel(_channel_3_notes, notes, NOTE_GREEN); }
	void set_channel_4(const std::vector<Note_View> &notes) { set_channel(_channel_4_notes, notes, NOTE_BROWN); }

	void set_channel_1_detailed(bool detailed) { set_channel_detailed(_channel_1_notes, _channel_1_loops, _channel_1_calls, detailed); }
	void set_channel_2_detailed(bool detailed) { set_channel_detailed(_channel_2_notes, _channel_2_loops, _channel_2_calls, detailed); }
	void set_channel_3_detailed(bool detailed) { set_channel_detailed(_channel_3_notes, _channel_3_loops, _channel_3_calls, detailed); }
	void set_channel_4_detailed(bool detailed) { set_channel_detailed(_channel_4_notes, _channel_4_loops, _channel_4_calls, detailed); }

	void reset_note_colors();
private:
	void highlight_tick(std::vector<Note_Box *> &notes, int32_t tick, Fl_Color color);
	void set_channel(std::vector<Note_Box *> &channel, const std::vector<Note_View> &notes, Fl_Color color);
	void set_channel_detailed(std::vector<Note_Box *> &notes, std::vector<Loop_Box *> &loops, std::vector<Call_Box *> &calls, bool detailed);

	std::vector<Note_Box *> *active_channel();
protected:
	void draw() override;
};

class Piano_Roll : public Fl_Scroll {
private:
	Piano_Timeline *_piano_timeline;
	int32_t _tick = -1;
	bool _following = false;
	bool _realtime = true;

	std::vector<Note_View> _channel_1_notes;
	std::vector<Note_View> _channel_2_notes;
	std::vector<Note_View> _channel_3_notes;
	std::vector<Note_View> _channel_4_notes;
	int32_t _channel_1_loop_tick = -1;
	int32_t _channel_2_loop_tick = -1;
	int32_t _channel_3_loop_tick = -1;
	int32_t _channel_4_loop_tick = -1;
	int32_t _channel_1_end_tick = -1;
	int32_t _channel_2_end_tick = -1;
	int32_t _channel_3_end_tick = -1;
	int32_t _channel_4_end_tick = -1;
public:
	Piano_Roll(int x, int y, int w, int h, const char *l = nullptr);
	~Piano_Roll() noexcept;

	Piano_Roll(const Piano_Roll&) = delete;
	Piano_Roll& operator=(const Piano_Roll&) = delete;

	inline int32_t tick(void) const { return _tick; }

	const std::vector<Note_View> &channel_1_notes() const { return _channel_1_notes; }
	const std::vector<Note_View> &channel_2_notes() const { return _channel_2_notes; }
	const std::vector<Note_View> &channel_3_notes() const { return _channel_3_notes; }
	const std::vector<Note_View> &channel_4_notes() const { return _channel_4_notes; }

	int handle(int event) override;

	void set_size(int W, int H);

	bool set_timeline(const Song &song);
	void set_channel_timeline(const int selected_channel, const Song &song);
	void set_active_channel_selection(const std::set<int32_t> &selection);

	bool build_note_view(std::vector<Loop_Box *> &loops, std::vector<Call_Box *> &calls, std::vector<Note_View> &notes, const std::vector<Command> &commands, int32_t end_tick, Fl_Color color);

	int32_t get_song_length() const;
	int32_t get_loop_tick() const;

	void set_channel_1_detailed(bool detailed) { _piano_timeline->set_channel_1_detailed(detailed); }
	void set_channel_2_detailed(bool detailed) { _piano_timeline->set_channel_2_detailed(detailed); }
	void set_channel_3_detailed(bool detailed) { _piano_timeline->set_channel_3_detailed(detailed); }
	void set_channel_4_detailed(bool detailed) { _piano_timeline->set_channel_4_detailed(detailed); }

	void clear();

	void start_following();
	void unpause_following();
	void stop_following();
	void pause_following();
	void highlight_tick(int32_t t);

	bool pitch_up(Song &song);
	bool pitch_down(Song &song);
	bool octave_up(Song &song);
	bool octave_down(Song &song);
	bool move_left(Song &song);
	bool move_right(Song &song);
	bool shorten(Song &song);
	bool lengthen(Song &song);
	bool delete_selection(Song &song);
	bool snip_selection(Song &song);
	bool select_all() { return _piano_timeline->select_all(); }
	bool select_none() { return _piano_timeline->select_none(); }
private:
	std::vector<Note_View> *active_channel();

	static void hscrollbar_cb(Fl_Scrollbar *sb, void *);
};

#endif
