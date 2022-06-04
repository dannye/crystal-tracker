#include <FL/fl_draw.H>

#include "piano-roll.h"

#include "themes.h"

static inline bool is_white_key(size_t i) {
	return !(i == 1 || i == 3 || i == 5 || i == 8 || i == 10);
}

Piano_Keys::Piano_Keys(int x, int y, int w, int h, const char *l) : Fl_Group(x, y, w, h, l) {
	for (size_t _y = 0; _y < NUM_OCTAVES; ++_y) {
		int y_pos = OCTAVE_HEIGHT * _y;
		for (size_t _x = 0; _x < NUM_NOTES_PER_OCTAVE; ++_x) {
			size_t i = _y * NUM_NOTES_PER_OCTAVE + _x;
			_notes[i] = new Fl_Box(NOTE_KEYS[_x].x + x, y_pos + NOTE_KEYS[_x].y + y, NOTE_KEYS[_x].w, NOTE_KEYS[_x].h, NOTE_KEYS[_x].label);
			_notes[i]->box(FL_BORDER_BOX);
			if (NOTE_KEYS[_x].white) {
				_notes[i]->color(FL_BACKGROUND2_COLOR);
				_notes[i]->labelcolor(FL_FOREGROUND_COLOR);
			}
			else {
				_notes[i]->color(FL_FOREGROUND_COLOR);
				_notes[i]->labelcolor(FL_BACKGROUND2_COLOR);
			}
		}
	}
	this->end();
}

Piano_Keys::~Piano_Keys() noexcept {
	for (size_t i = 0; i < NUM_NOTES_PER_OCTAVE * NUM_OCTAVES; ++i) {
		if (_notes[i]) {
			delete _notes[i];
			_notes[i] = nullptr;
		}
	}
}

Piano_Timeline::Piano_Timeline(int x, int y, int w, int h, const char *l) : Fl_Group(x, y, w, h, l) {
	_keys = new Piano_Keys(x, y, WHITE_KEY_WIDTH, OCTAVE_HEIGHT * NUM_OCTAVES);
	this->end();
}

Piano_Timeline::~Piano_Timeline() noexcept {
	clear();
	if (_keys) {
		delete _keys;
		_keys = nullptr;
	}
}

void Piano_Timeline::clear() {
	for (Fl_Box *note : _channel_1_notes) {
		delete note;
	}
	_channel_1_notes.clear();
	for (Fl_Box *note : _channel_2_notes) {
		delete note;
	}
	_channel_2_notes.clear();
	for (Fl_Box *note : _channel_3_notes) {
		delete note;
	}
	_channel_3_notes.clear();
	for (Fl_Box *note : _channel_4_notes) {
		delete note;
	}
	_channel_4_notes.clear();
}

void Piano_Timeline::set_channel_timeline(std::list<Fl_Box *> &notes, const std::list<Note_View> &timeline, Fl_Color color) {
	this->begin();
	int x_pos = x() + WHITE_KEY_WIDTH;
	for (const Note_View &note : timeline) {
		if (note.pitch != Pitch::REST) {
			int y_pos = y() + (NUM_OCTAVES - note.octave) * OCTAVE_HEIGHT + (NUM_PITCHES - (size_t)note.pitch) * NOTE_ROW_HEIGHT;
			Fl_Box *box = new Fl_Box(x_pos, y_pos, TIME_STEP_WIDTH * note.length, NOTE_ROW_HEIGHT);
			box->box(FL_BORDER_BOX);
			box->color(color);
			notes.push_back(box);
		}
		x_pos += TIME_STEP_WIDTH * note.length;
	}
	this->end();

	if (notes.size() > 0) {
		const int width = notes.back()->x() + notes.back()->w();
		if (width > w()) {
			w(width);
		}
	}

	// fix the keys as the last child
	Fl_Widget **a = (Fl_Widget **)array();
	if (a[children() - 1] != (Fl_Widget *)_keys) {
		int i, j;
		for (i = j = 0; j < children(); j++) {
			if (a[j] != (Fl_Widget *)_keys) {
				a[i++] = a[j];
			}
		}
		a[i] = (Fl_Widget *)_keys;
	}
}

void Piano_Timeline::draw() {
	bool dark = OS::is_dark_theme(OS::current_theme());
	Fl_Color light_row = dark ? ALT_LIGHT_ROW : FL_LIGHT1;
	Fl_Color dark_row = dark ? ALT_DARK_ROW : FL_DARK2;
	if (damage() & ~FL_DAMAGE_CHILD) {
		int y_pos = y();
		for (size_t _y = 0; _y < NUM_OCTAVES; ++_y) {
			for (size_t _x = 0; _x < NUM_NOTES_PER_OCTAVE; ++_x) {
				if (is_white_key(_x)) {
					fl_rectf(x(), y_pos, w(), NOTE_ROW_HEIGHT, light_row);
				}
				else {
					fl_rectf(x(), y_pos, w(), NOTE_ROW_HEIGHT, dark_row);
				}
				fl_color(FL_BLACK);
				fl_xyline(x(), y_pos - 1, x() + w());
				fl_xyline(x(), y_pos, x() + w());
				y_pos += NOTE_ROW_HEIGHT;
			}
		}
		int x_pos = x();
		const size_t num_dividers = w() / TIME_STEP_WIDTH + 1;
		for (size_t i = 0; i < num_dividers; ++i) {
			fl_yxline(x_pos - 1, y(), y() + h());
			fl_yxline(x_pos, y(), y() + h());
			x_pos += TIME_STEP_WIDTH;
		}
	}
	draw_children();
}

Piano_Roll::Piano_Roll(int x, int y, int w, int h, const char *l) : Fl_Scroll(x, y, w, h, l) {
	type(BOTH_ALWAYS);
	_piano_timeline = new Piano_Timeline(x, y, w * 2, OCTAVE_HEIGHT * NUM_OCTAVES);
	this->end();

	hscrollbar.callback((Fl_Callback *)hscrollbar_cb);
}

Piano_Roll::~Piano_Roll() noexcept {
	if (_piano_timeline) {
		delete _piano_timeline;
		_piano_timeline = nullptr;
	}
}

void Piano_Roll::set_size(int W, int H) {
	if (W != w() || H != h()) {
		size(W, H);
		// if window is too wide, increase the width of the timeline
		if (hscrollbar.value() > _piano_timeline->w() - (W - scrollbar.w())) {
			_piano_timeline->w(hscrollbar.value() + (W - scrollbar.w()));
		}
		// if window is too tall, clamp the vertical scroll position
		if (scrollbar.value() > _piano_timeline->h() - (H - hscrollbar.h())) {
			scroll_to(xposition(), _piano_timeline->h() - (H - hscrollbar.h()));
		}
	}
}

void Piano_Roll::clear() {
	_piano_timeline->clear();
	_piano_timeline->w(w() * 2);
	scroll_to(0, yposition());
	_piano_timeline->_keys->position(0, _piano_timeline->_keys->y());
}

void Piano_Roll::hscrollbar_cb(Fl_Scrollbar *sb, void *) {
	Piano_Roll *scroll = (Piano_Roll *)(sb->parent());
	scroll->scroll_to(sb->value(), scroll->yposition());
	scroll->_piano_timeline->_keys->position(0, scroll->_piano_timeline->_keys->y());
	scroll->redraw();
}
