#ifndef PIANO_ROLL_H
#define PIANO_ROLL_H

#include <array>

#include <FL/Fl_Box.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Scroll.H>

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

constexpr int TIME_STEP_WIDTH = 50;

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
	{ 0, BLACK_KEY_OFFSET + NOTE_ROW_HEIGHT *  1, BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, "A#", false },
	{ 0, BLACK_KEY_OFFSET + NOTE_ROW_HEIGHT *  3, BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, "G#", false },
	{ 0, BLACK_KEY_OFFSET + NOTE_ROW_HEIGHT *  5, BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, "F#", false },
	{ 0, BLACK_KEY_OFFSET + NOTE_ROW_HEIGHT *  8, BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, "D#", false },
	{ 0, BLACK_KEY_OFFSET + NOTE_ROW_HEIGHT * 10, BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT, "C#", false },
};

class Piano_Timeline : Fl_Group {
private:
	std::array<Fl_Box *, NUM_OCTAVES * NUM_NOTES_PER_OCTAVE> rows;
	std::array<Fl_Box *, 64> dividers; // xxx
public:
	Piano_Timeline(int x, int y, int w, int h, const char *l = nullptr);
	~Piano_Timeline() noexcept;

	Piano_Timeline(const Piano_Timeline&) = delete;
	Piano_Timeline& operator=(const Piano_Timeline&) = delete;
};

class Piano_Octave : Fl_Group {
private:
	std::array<Fl_Box *, NUM_NOTES_PER_OCTAVE> notes;
public:
	Piano_Octave(int x, int y, int w, int h, const char *l = nullptr);
	~Piano_Octave() noexcept;

	Piano_Octave(const Piano_Octave&) = delete;
	Piano_Octave& operator=(const Piano_Octave&) = delete;
};

class Piano_Roll : Fl_Scroll {
private:
	std::array<Piano_Octave *, NUM_OCTAVES> octaves;
	Piano_Timeline *_piano_timeline;
public:
	Piano_Roll(int x, int y, int w, int h, const char *l = nullptr);
	~Piano_Roll() noexcept;

	Piano_Roll(const Piano_Roll&) = delete;
	Piano_Roll& operator=(const Piano_Roll&) = delete;
};

#endif
