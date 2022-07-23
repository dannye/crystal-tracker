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

class Note_Box : public Fl_Box {
private:
	bool _selected = false;
	bool _ghost = false;
public:
	using Fl_Box::Fl_Box;

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
};

class Call_Box : public Fl_Box {
public:
	using Fl_Box::Fl_Box;
};

class Piano_Keys : public Fl_Group {
private:
	std::array<Fl_Box *, NUM_NOTES_PER_OCTAVE * NUM_OCTAVES> _notes;
public:
	Piano_Keys(int x, int y, int w, int h, const char *l = nullptr);
	~Piano_Keys() noexcept;

	Piano_Keys(const Piano_Keys&) = delete;
	Piano_Keys& operator=(const Piano_Keys&) = delete;
};

class Piano_Timeline : public Fl_Group {
	friend class Piano_Roll;
private:
	Piano_Keys *_keys;
	std::vector<Note_Box *> _channel_1_timeline;
	std::vector<Note_Box *> _channel_2_timeline;
	std::vector<Note_Box *> _channel_3_timeline;
	std::vector<Note_Box *> _channel_4_timeline;
	std::vector<Loop_Box *> _channel_1_loops;
	std::vector<Loop_Box *> _channel_2_loops;
	std::vector<Loop_Box *> _channel_3_loops;
	std::vector<Loop_Box *> _channel_4_loops;
	std::vector<Call_Box *> _channel_1_calls;
	std::vector<Call_Box *> _channel_2_calls;
	std::vector<Call_Box *> _channel_3_calls;
	std::vector<Call_Box *> _channel_4_calls;

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
	Piano_Timeline(int x, int y, int w, int h, const char *l = nullptr);
	~Piano_Timeline() noexcept;

	void clear();

	Piano_Timeline(const Piano_Timeline&) = delete;
	Piano_Timeline& operator=(const Piano_Timeline&) = delete;

	int handle(int event) override;
	int handle_note_selection(int event);

	bool set_timeline(const Song &song);

	Note_Box *get_channel_1_note_at_tick(int32_t tick) { return get_note_at_tick(_channel_1_timeline, tick); }
	Note_Box *get_channel_2_note_at_tick(int32_t tick) { return get_note_at_tick(_channel_2_timeline, tick); }
	Note_Box *get_channel_3_note_at_tick(int32_t tick) { return get_note_at_tick(_channel_3_timeline, tick); }
	Note_Box *get_channel_4_note_at_tick(int32_t tick) { return get_note_at_tick(_channel_4_timeline, tick); }

	int32_t get_loop_tick() const;

	void set_channel_1_detailed(bool detailed) { set_channel_detailed(_channel_1_timeline, _channel_1_loops, _channel_1_calls, detailed); }
	void set_channel_2_detailed(bool detailed) { set_channel_detailed(_channel_2_timeline, _channel_2_loops, _channel_2_calls, detailed); }
	void set_channel_3_detailed(bool detailed) { set_channel_detailed(_channel_3_timeline, _channel_3_loops, _channel_3_calls, detailed); }
	void set_channel_4_detailed(bool detailed) { set_channel_detailed(_channel_4_timeline, _channel_4_loops, _channel_4_calls, detailed); }

	void reset_note_colors();
private:
	void set_channel_timeline(std::vector<Note_Box *> &timeline, const std::vector<Note_View> &notes, Fl_Color color);
	bool build_note_view(std::vector<Loop_Box *> &loops, std::vector<Call_Box *> &calls, std::vector<Note_View> &notes, const std::list<Command> &commands, int32_t end_tick, Fl_Color color);

	Note_Box *get_note_at_tick(std::vector<Note_Box *> &timeline, int32_t tick);

	void set_channel_detailed(std::vector<Note_Box *> &timeline, std::vector<Loop_Box *> &loops, std::vector<Call_Box *> &calls, bool detailed);

	std::vector<Note_Box *> *active_timeline();
protected:
	void draw() override;
};

class Piano_Roll : public Fl_Scroll {
	friend class Piano_Timeline;
private:
	Piano_Timeline *_piano_timeline;
	int32_t _tick = -1;
	bool _following = false;
	bool _realtime = true;
public:
	Piano_Roll(int x, int y, int w, int h, const char *l = nullptr);
	~Piano_Roll() noexcept;

	Piano_Roll(const Piano_Roll&) = delete;
	Piano_Roll& operator=(const Piano_Roll&) = delete;

	int handle(int event) override;

	void set_size(int W, int H);

	bool set_timeline(const Song &song) { return _piano_timeline->set_timeline(song); }

	const std::vector<Note_View> &channel_1_notes() const { return _piano_timeline->_channel_1_notes; }
	const std::vector<Note_View> &channel_2_notes() const { return _piano_timeline->_channel_2_notes; }
	const std::vector<Note_View> &channel_3_notes() const { return _piano_timeline->_channel_3_notes; }
	const std::vector<Note_View> &channel_4_notes() const { return _piano_timeline->_channel_4_notes; }

	int32_t get_loop_tick() const { return _piano_timeline->get_loop_tick(); }

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
private:
	static void hscrollbar_cb(Fl_Scrollbar *sb, void *);
};

#endif
