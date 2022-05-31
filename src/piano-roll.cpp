#include "piano-roll.h"

static inline bool is_white_key(size_t i) {
	return !(i == 1 || i == 3 || i == 5 || i == 8 || i == 10);
}

Piano_Timeline::Piano_Timeline(int x, int y, int w, int h, const char *l) : Fl_Group(x, y, w, h, l) {
	int y_pos = y;
	for (size_t _y = 0; _y < NUM_OCTAVES; ++_y) {
		for (size_t _x = 0; _x < NUM_NOTES_PER_OCTAVE; ++_x) {
			const size_t i = _y * NUM_NOTES_PER_OCTAVE + _x;
			rows[i] = new Fl_Box(x, y_pos, w, NOTE_ROW_HEIGHT, l);
			rows[i]->box(FL_BORDER_BOX);
			if (is_white_key(_x)) {
				rows[i]->color(FL_LIGHT1);
			}
			else {
				rows[i]->color(FL_DARK1);
			}
			y_pos += NOTE_ROW_HEIGHT;
		}
	}
	for (size_t i = 0; i < 64; ++i) {
		dividers[i] = new Fl_Box(TIME_STEP_WIDTH * (i + 1) + x, y, 1, h);
		dividers[i]->box(FL_BORDER_BOX);
	}
	this->end();
}

Piano_Timeline::~Piano_Timeline() noexcept {
	for (size_t _y = 0; _y < NUM_OCTAVES; ++_y) {
		for (size_t _x = 0; _x < NUM_NOTES_PER_OCTAVE; ++_x) {
			const size_t i = _y * NUM_NOTES_PER_OCTAVE + _x;
			if (rows[i]) {
				delete rows[i];
				rows[i] = nullptr;
			}
		}
	}
	for (size_t i = 0; i < 64; ++i) {
		if (dividers[i]) {
			delete dividers[i];
			dividers[i] = nullptr;
		}
	}
}

Piano_Octave::Piano_Octave(int x, int y, int w, int h, const char *l) : Fl_Group(x, y, w, h, l) {
	for (size_t i = 0; i < NUM_NOTES_PER_OCTAVE; ++i) {
		notes[i] = new Fl_Box(NOTE_KEYS[i].x + x, NOTE_KEYS[i].y + y, NOTE_KEYS[i].w, NOTE_KEYS[i].h, NOTE_KEYS[i].label);
		notes[i]->box(FL_BORDER_BOX);
		if (NOTE_KEYS[i].white) {
			notes[i]->color(FL_BACKGROUND2_COLOR);
			notes[i]->labelcolor(FL_FOREGROUND_COLOR);
		}
		else {
			notes[i]->color(FL_FOREGROUND_COLOR);
			notes[i]->labelcolor(FL_BACKGROUND2_COLOR);
		}
	}
	this->end();
}

Piano_Octave::~Piano_Octave() noexcept {
	for (size_t i = 0; i < NUM_NOTES_PER_OCTAVE; ++i) {
		if (notes[i]) {
			delete notes[i];
			notes[i] = nullptr;
		}
	}
}

Piano_Roll::Piano_Roll(int x, int y, int w, int h, const char *l) : Fl_Scroll(x, y, w, h, l) {
	_piano_timeline = new Piano_Timeline(WHITE_KEY_WIDTH + x, y, (w - WHITE_KEY_WIDTH) * 2, OCTAVE_HEIGHT * NUM_OCTAVES);
	for (size_t i = 0; i < NUM_OCTAVES; ++i) {
		octaves[i] = new Piano_Octave(x, OCTAVE_HEIGHT * i + y, WHITE_KEY_WIDTH, OCTAVE_HEIGHT);
	}
	this->end();
}

Piano_Roll::~Piano_Roll() noexcept {
	if (_piano_timeline) {
		delete _piano_timeline;
		_piano_timeline = nullptr;
	}
	for (size_t i = 0; i < NUM_OCTAVES; ++i) {
		if (octaves[i]) {
			delete octaves[i];
			octaves[i] = nullptr;
		}
	}
}
