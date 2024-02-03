#include <cassert>
#include <map>
#include <stack>

#pragma warning(push, 0)
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#pragma warning(pop)

#include "piano-roll.h"

#include "main-window.h"
#include "themes.h"

static inline bool is_white_key(size_t i) {
	return !(i == 1 || i == 3 || i == 5 || i == 8 || i == 10);
}

static const char *pitch_label(const Note_View &note) {
	return NOTE_KEYS[PITCH_TO_KEY_INDEX[((size_t)note.pitch - 1 + note.transpose_pitches) % NUM_NOTES_PER_OCTAVE]].label;
}

Note_Box::Note_Box(const Note_View &n, int32_t t, int X, int Y, int W, int H, const char *l) : Fl_Box(X, Y, W, H, l), _note_view(n), _tick(t) {
	labelfont(OS_FONT);
	align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
}

void Note_Box::draw() {
	draw_box();
	if (ghost()) {
		draw_box(FL_BORDER_FRAME, NOTE_GHOST);
		if (box() != FL_BORDER_FRAME && x() + w() > 1) {
			draw_box(FL_BORDER_FRAME, x() + 1, y() + 1, w() - 2, h() - 2, NOTE_GHOST);
			draw_box(FL_BORDER_FRAME, x() + 2, y() + 2, w() - 4, h() - 4, NOTE_GHOST);
		}
		else if (box() == FL_BORDER_FRAME && x() + w() > 1) {
			draw_box(FL_BORDER_FRAME, x() + 1, y() + 1, w() - 2, h() - 2, NOTE_GHOST);
		}
	}
	else if (_selected) {
		draw_box(FL_BORDER_FRAME, FL_FOREGROUND_COLOR);
		if (box() != FL_BORDER_FRAME && x() + w() > 1) {
			draw_box(FL_BORDER_FRAME, x() + 1, y() + 1, w() - 2, h() - 2, FL_WHITE);
			draw_box(FL_BORDER_FRAME, x() + 2, y() + 2, w() - 4, h() - 4, FL_WHITE);
		}
		else if (box() == FL_BORDER_FRAME && x() + w() > 1) {
			draw_box(FL_BORDER_FRAME, x() + 1, y() + 1, w() - 2, h() - 2, FL_FOREGROUND_COLOR);
		}
	}
	else if (box() == FL_BORDER_FRAME && x() + w() > 1) {
		draw_box(FL_BORDER_FRAME, x() + 1, y() + 1, w() - 2, h() - 2, color());
	}
	draw_label();
}

void Loop_Box::draw() {
	draw_box();
	draw_box(box(), x() - 1, y() - 1, w() + 2, h() + 2, color());
	draw_label();
}

void Call_Box::draw() {
	draw_box();
	draw_box(box(), x() - 1, y() - 1, w() + 2, h() + 2, color());
	draw_label();
}

int Key_Box::handle(int event) {
	Main_Window *mw = parent()->parent()->parent()->parent();
	Key_Box *key_below_mouse = nullptr;
	switch (event) {
	case FL_PUSH:
		if (
			Fl::event_button() == FL_LEFT_MOUSE &&
			mw->play_note(_pitch, _octave)
		) {
			parent()->update_key_colors();
			parent()->redraw();
			return 1;
		}
		break;
	case FL_RELEASE:
		if (mw->stop_note()) {
			parent()->update_key_colors();
			parent()->redraw();
			return 1;
		}
		break;
	case FL_DRAG:
		if (
			mw->playing_note() &&
			parent()->find_key_below_mouse(key_below_mouse) &&
			mw->play_note(key_below_mouse->_pitch, key_below_mouse->_octave)
		) {
			parent()->update_key_colors();
			parent()->redraw();
			return 1;
		}
		break;
	}
	return Fl_Box::handle(event);
}

void Key_Box::draw() {
	draw_box();
	if (OS::current_theme() == OS::Theme::HIGH_CONTRAST)
		draw_box(FL_BORDER_FRAME, fl_darker(FL_SELECTION_COLOR));
	draw_label();
}

void White_Key_Box::draw() {
	draw_box();
	if (OS::current_theme() == OS::Theme::HIGH_CONTRAST)
		draw_box(FL_BORDER_FRAME, fl_darker(FL_SELECTION_COLOR));
	draw_label(x() + BLACK_KEY_WIDTH, y(), w() - BLACK_KEY_WIDTH, h());
}

Piano_Keys::Piano_Keys(int X, int Y, int W, int H, const char *l) : Fl_Group(X, Y, W, H, l) {
	resizable(nullptr);
	for (size_t _y = 0; _y < NUM_OCTAVES; ++_y) {
		for (size_t _x = 0; _x < NUM_NOTES_PER_OCTAVE; ++_x) {
			size_t i = _y * NUM_NOTES_PER_OCTAVE + _x;
			if (NOTE_KEYS[_x].white) {
				_keys[i] = new White_Key_Box(NOTE_KEYS[_x].pitch, NUM_OCTAVES - _y, X, Y, 0, 0, NOTE_KEYS[_x].label);
				_keys[i]->box(FL_BORDER_BOX);
				_keys[i]->color(BACKGROUND3_COLOR);
				_keys[i]->labelcolor(FL_FOREGROUND_COLOR);
				if (NOTE_KEYS[_x].pitch == Pitch::C_NAT) {
					_keys[i]->label(C_KEY_LABELS[NUM_OCTAVES - _y - 1]);
				}
			}
			else {
				_keys[i] = new Key_Box(NOTE_KEYS[_x].pitch, NUM_OCTAVES - _y, X, Y, 0, 0, NOTE_KEYS[_x].label);
				_keys[i]->box(FL_BORDER_BOX);
				_keys[i]->color(FL_FOREGROUND_COLOR);
				_keys[i]->labelcolor(BACKGROUND3_COLOR);
			}
		}
	}
	end();
	calc_sizes();
}

void Piano_Keys::calc_sizes() {
	const Piano_Roll *piano_roll = parent()->parent();
	const bool zoomed = piano_roll->zoomed();
	const int white_key_height = piano_roll->white_key_height();
	const int black_key_height = piano_roll->black_key_height();
	const int octave_height = piano_roll->octave_height();
	const int note_row_height = piano_roll->note_row_height();
	const int black_key_offset = piano_roll->black_key_offset();

	int white_delta = 0, black_delta = 0;

	int y_top = _keys[0]->y();

	for (size_t _y = 0; _y < NUM_OCTAVES; ++_y) {
		int y_pos = octave_height * _y;
		for (size_t _x = 0; _x < NUM_NOTES_PER_OCTAVE; ++_x) {
			size_t i = _y * NUM_NOTES_PER_OCTAVE + _x;
			int delta = zoomed ? NOTE_KEYS[_x].delta1 : NOTE_KEYS[_x].delta2;
			if (NOTE_KEYS[_x].white) {
				_keys[i]->resize(
					_keys[i]->x(),
					y_top + y_pos + NOTE_KEYS[_x].y * white_key_height + white_delta,
					WHITE_KEY_WIDTH,
					white_key_height + delta
				);
				white_delta += delta;
			}
			else {
				_keys[i]->resize(
					_keys[i]->x(),
					y_top + y_pos + NOTE_KEYS[_x].y * note_row_height + black_key_offset + black_delta,
					BLACK_KEY_WIDTH,
					black_key_height + delta
				);
				black_delta += delta;
			}
		}
	}

	size(w(), NUM_OCTAVES * octave_height);
}

void Piano_Keys::key_labels(bool show) {
	for (size_t _y = 0; _y < NUM_OCTAVES; ++_y) {
		for (size_t _x = 0; _x < NUM_NOTES_PER_OCTAVE; ++_x) {
			size_t i = _y * NUM_NOTES_PER_OCTAVE + _x;
			if (NOTE_KEYS[_x].pitch != Pitch::C_NAT) {
				_keys[i]->label(show ? NOTE_KEYS[_x].label : nullptr);
			}
		}
	}
}

void Piano_Keys::set_key_color(Pitch pitch, int32_t octave, Fl_Color color) {
	size_t _y = NUM_OCTAVES - octave;
	size_t _x = PITCH_TO_KEY_INDEX[(size_t)pitch - 1];
	size_t i = _y * NUM_NOTES_PER_OCTAVE + _x;
	if (_keys[i]->color() != color) {
		_keys[i]->color(color);
		_keys[i]->redraw();
	}
}

void Piano_Keys::update_key_colors() {
	reset_key_colors();
	if (_channel_1_pitch != Pitch::REST) {
		set_key_color(_channel_1_pitch, _channel_1_octave, NOTE_RED_LIGHT);
	}
	if (_channel_2_pitch != Pitch::REST) {
		set_key_color(_channel_2_pitch, _channel_2_octave, NOTE_BLUE_LIGHT);
	}
	if (_channel_3_pitch != Pitch::REST) {
		set_key_color(_channel_3_pitch, _channel_3_octave, NOTE_GREEN_LIGHT);
	}
	if (_channel_4_pitch != Pitch::REST) {
		set_key_color(_channel_4_pitch, _channel_4_octave, NOTE_BROWN_LIGHT);
	}

	Pitch interactive_pitch = parent()->parent()->parent()->playing_pitch();
	int32_t interactive_octave = parent()->parent()->parent()->playing_octave();
	if (interactive_pitch != Pitch::REST) {
		set_key_color(interactive_pitch, interactive_octave, fl_color_average(FL_FOREGROUND_COLOR, BACKGROUND3_COLOR, 0.5f));
	}
}

void Piano_Keys::reset_key_colors() {
	for (size_t _y = 0; _y < NUM_OCTAVES; ++_y) {
		for (size_t _x = 0; _x < NUM_NOTES_PER_OCTAVE; ++_x) {
			size_t i = _y * NUM_NOTES_PER_OCTAVE + _x;
			Fl_Color color = NOTE_KEYS[_x].white ? BACKGROUND3_COLOR : FL_FOREGROUND_COLOR;
			if (_keys[i]->color() != color) {
				_keys[i]->color(color);
				_keys[i]->redraw();
			}
		}
	}
}

bool Piano_Keys::find_key_below_mouse(Key_Box *&key) {
	for (size_t _y = 0; _y < NUM_OCTAVES; ++_y) {
		for (size_t _x = NUM_NOTES_PER_OCTAVE - 1; _x < NUM_NOTES_PER_OCTAVE; --_x) {
			size_t i = _y * NUM_NOTES_PER_OCTAVE + _x;
			if (Fl::event_inside(_keys[i])) {
				key = _keys[i];
				return true;
			}
		}
	}
	return false;
}

void Piano_Keys::set_channel_pitch(int channel_number, Pitch p, int32_t o) {
	assert(channel_number >= 1 && channel_number <= 4);
	if (channel_number == 1) {
		set_channel_1_pitch(p, o);
	}
	else if (channel_number == 2) {
		set_channel_2_pitch(p, o);
	}
	else if (channel_number == 3) {
		set_channel_3_pitch(p, o);
	}
	else {
		set_channel_4_pitch(p, o);
	}
}

void Piano_Keys::reset_channel_pitches() {
	set_channel_1_pitch(Pitch::REST, 0);
	set_channel_2_pitch(Pitch::REST, 0);
	set_channel_3_pitch(Pitch::REST, 0);
	set_channel_4_pitch(Pitch::REST, 0);
	update_key_colors();
}

Piano_Timeline::Piano_Timeline(int X, int Y, int W, int H, const char *l) :
	Fl_Group(X, Y, W, H, l),
	_trashcan(0, 0, 0, 0),
	_end(),
	_keys(X, Y, WHITE_KEY_WIDTH, H)
{
	resizable(nullptr);
	end();
	_trashcan.hide();
}

Piano_Timeline::~Piano_Timeline() noexcept {
	remove(_keys);
	remove(_trashcan);
	Fl_Group::clear();
}

void Piano_Timeline::clear() {
	remove(_keys);
	remove(_trashcan);
	Fl_Group::clear();
	add(_trashcan);
	add(_keys);

	_channel_1_notes.clear();
	_channel_2_notes.clear();
	_channel_3_notes.clear();
	_channel_4_notes.clear();

	_channel_1_loops.clear();
	_channel_2_loops.clear();
	_channel_3_loops.clear();
	_channel_4_loops.clear();

	_channel_1_calls.clear();
	_channel_2_calls.clear();
	_channel_3_calls.clear();
	_channel_4_calls.clear();

	_channel_1_flags.clear();
	_channel_2_flags.clear();
	_channel_3_flags.clear();
	_channel_4_flags.clear();

	_selection_region.x = -1;
	_selection_region.y = -1;
	_selection_region.w = 0;
	_selection_region.h = 0;
	_erasing = false;
	_positioning = false;
}

void Piano_Timeline::clear_channel_1() {
	for (Note_Box *note : _channel_1_notes) {
		_trashcan.add(*note);
	}
	_channel_1_notes.clear();

	for (Loop_Box *loop : _channel_1_loops) {
		_trashcan.add(*loop);
	}
	_channel_1_loops.clear();

	for (Call_Box *call : _channel_1_calls) {
		_trashcan.add(*call);
	}
	_channel_1_calls.clear();

	for (Flag_Box *flag : _channel_1_flags) {
		_trashcan.add(*flag);
	}
	_channel_1_flags.clear();

	_trashcan.clear();
}

void Piano_Timeline::clear_channel_2() {
	for (Note_Box *note : _channel_2_notes) {
		_trashcan.add(*note);
	}
	_channel_2_notes.clear();

	for (Loop_Box *loop : _channel_2_loops) {
		_trashcan.add(*loop);
	}
	_channel_2_loops.clear();

	for (Call_Box *call : _channel_2_calls) {
		_trashcan.add(*call);
	}
	_channel_2_calls.clear();

	for (Flag_Box *flag : _channel_2_flags) {
		_trashcan.add(*flag);
	}
	_channel_2_flags.clear();

	_trashcan.clear();
}

void Piano_Timeline::clear_channel_3() {
	for (Note_Box *note : _channel_3_notes) {
		_trashcan.add(*note);
	}
	_channel_3_notes.clear();

	for (Loop_Box *loop : _channel_3_loops) {
		_trashcan.add(*loop);
	}
	_channel_3_loops.clear();

	for (Call_Box *call : _channel_3_calls) {
		_trashcan.add(*call);
	}
	_channel_3_calls.clear();

	for (Flag_Box *flag : _channel_3_flags) {
		_trashcan.add(*flag);
	}
	_channel_3_flags.clear();

	_trashcan.clear();
}

void Piano_Timeline::clear_channel_4() {
	for (Note_Box *note : _channel_4_notes) {
		_trashcan.add(*note);
	}
	_channel_4_notes.clear();

	for (Loop_Box *loop : _channel_4_loops) {
		_trashcan.add(*loop);
	}
	_channel_4_loops.clear();

	for (Call_Box *call : _channel_4_calls) {
		_trashcan.add(*call);
	}
	_channel_4_calls.clear();

	for (Flag_Box *flag : _channel_4_flags) {
		_trashcan.add(*flag);
	}
	_channel_4_flags.clear();

	_trashcan.clear();
}

inline int Piano_Timeline::selected_channel() const {
	return parent()->selected_channel();
}

void Piano_Timeline::calc_sizes() {
	const int octave_height = parent()->octave_height();
	const int note_row_height = parent()->note_row_height();
	const int tick_width = parent()->tick_width();
	const int note_labelsize = parent()->note_labelsize();

	const auto tick_to_x_pos = [&](int32_t tick) {
		return x() + WHITE_KEY_WIDTH + tick * tick_width;
	};
	const auto pitch_to_y_pos = [&](Pitch pitch, int32_t octave) {
		return y() + (NUM_OCTAVES - octave) * octave_height + (NUM_NOTES_PER_OCTAVE - (size_t)(pitch)) * note_row_height;
	};

	const auto resize_notes = [&](std::vector<Note_Box *> &notes) {
		for (Note_Box *note : notes) {
			note->resize(
				tick_to_x_pos(note->tick()),
				pitch_to_y_pos(note->note_view().pitch, note->note_view().octave),
				note->note_view().length * note->note_view().speed * tick_width,
				note_row_height
			);
			note->labelsize(note_labelsize);
		}
	};

	resize_notes(_channel_1_notes);
	resize_notes(_channel_2_notes);
	resize_notes(_channel_3_notes);
	resize_notes(_channel_4_notes);

	const auto resize_wrappers = [&](auto &wrappers) {
		for (Wrapper_Box *wrapper : wrappers) {
			int x_left =   tick_to_x_pos(wrapper->start_tick());
			int x_right =  tick_to_x_pos(wrapper->end_tick());
			int y_top =    pitch_to_y_pos(wrapper->max_pitch(), wrapper->max_octave());
			int y_bottom = pitch_to_y_pos(wrapper->min_pitch(), wrapper->min_octave());
			wrapper->resize(
				x_left,
				y_top,
				x_right - x_left,
				y_bottom + note_row_height - y_top
			);
		}
	};

	resize_wrappers(_channel_1_loops);
	resize_wrappers(_channel_2_loops);
	resize_wrappers(_channel_3_loops);
	resize_wrappers(_channel_4_loops);

	resize_wrappers(_channel_1_calls);
	resize_wrappers(_channel_2_calls);
	resize_wrappers(_channel_3_calls);
	resize_wrappers(_channel_4_calls);

	const auto resize_flags = [&](std::vector<Flag_Box *> &flags, const std::vector<Note_Box *> &notes) {
		for (Flag_Box *flag : flags) {
			const Note_Box *note = notes[flag->note_index()];
			flag->resize(
				note->x(),
				note->note_view().octave == 8 ?
					note->y() + note->h() + tick_width * 4 * flag->row_offset() :
					note->y() - tick_width * 4 - tick_width * 4 * flag->row_offset(),
				tick_width * 4,
				tick_width * 4
			);
		}
	};

	resize_flags(_channel_1_flags, _channel_1_notes);
	resize_flags(_channel_2_flags, _channel_2_notes);
	resize_flags(_channel_3_flags, _channel_3_notes);
	resize_flags(_channel_4_flags, _channel_4_notes);
}

void Piano_Timeline::note_labels(bool show) {
	int active_channel = selected_channel();

	const auto show_note_labels = [](std::vector<Note_Box *> &notes, bool show) {
		for (Note_Box *note : notes) {
			note->label(show ? pitch_label(note->note_view()) : nullptr);
		}
	};

	show_note_labels(_channel_1_notes, show && (active_channel == 1 || active_channel == 0));
	show_note_labels(_channel_2_notes, show && (active_channel == 2 || active_channel == 0));
	show_note_labels(_channel_3_notes, show && (active_channel == 3 || active_channel == 0));
	show_note_labels(_channel_4_notes, show && (active_channel == 4 || active_channel == 0));
}

int Piano_Timeline::handle(int event) {
	bool pencil_mode = parent()->parent()->pencil_mode();
	switch (event) {
	case FL_PUSH:
		_erasing = false;
		_positioning = false;
		if (
			!Fl::event_inside(&_keys) &&
			parent()->handle_mouse_click_song_position(event)
		) {
			handle_note_selection_cancel(event);
			_positioning = true;
			return 1;
		}
		if (
			!Fl::event_inside(&_keys) &&
			parent()->handle_mouse_click_continuous_scroll(event)
		) {
			return 1;
		}
		if (
			!pencil_mode &&
			Fl::event_button() == FL_LEFT_MOUSE &&
			!Fl::event_inside(&_keys) &&
			handle_note_selection_start(event)
		) {
			return 1;
		}
		else if (
			pencil_mode &&
			Fl::event_button() == FL_LEFT_MOUSE &&
			!Fl::event_inside(&_keys) &&
			!parent()->following() &&
			!parent()->paused() &&
			handle_note_pencil(event)
		) {
			return 1;
		}
		else if (
			pencil_mode &&
			Fl::event_button() == FL_RIGHT_MOUSE &&
			!Fl::event_inside(&_keys) &&
			!parent()->following() &&
			!parent()->paused() &&
			handle_note_eraser(event)
		) {
			_erasing = true;
			return 1;
		}
		break;
	case FL_RELEASE:
		_erasing = false;
		_positioning = false;
		if (
			!pencil_mode &&
			Fl::event_button() == FL_LEFT_MOUSE &&
			handle_note_selection_end(event)
		) {
			handle_note_selection_cancel(event);
			return 1;
		}
		else if (
			!pencil_mode &&
			Fl::event_button() == FL_LEFT_MOUSE &&
			!Fl::event_inside(&_keys) &&
			_selection_region.x != -1 && _selection_region.y != -1 &&
			handle_note_selection(event)
		) {
			handle_note_selection_cancel(event);
			return 1;
		}
		handle_note_selection_cancel(event);
		break;
	case FL_DRAG:
		if (
			_positioning &&
			parent()->handle_mouse_click_song_position(event)
		) {
			return 1;
		}
		else if (
			!pencil_mode &&
			handle_note_selection_update(event)
		) {
			return 1;
		}
		else if (
			pencil_mode &&
			_erasing &&
			!Fl::event_inside(&_keys) &&
			!parent()->following() &&
			!parent()->paused() &&
			handle_note_eraser(event)
		) {
			return 1;
		}
		break;
	case FL_SHORTCUT:
	case FL_KEYBOARD:
		if (
			Fl::event_key() == FL_Enter &&
			!parent()->following() &&
			!parent()->paused() &&
			handle_note_selection(event)
		) {
			return 1;
		}
		else if (
			!pencil_mode &&
			Fl::event_key() == FL_Escape &&
			handle_note_selection_cancel(event)
		) {
			return 1;
		}
		break;
	}
	return Fl_Group::handle(event);
}

bool Piano_Timeline::handle_note_pencil(int event) {
	int32_t tick = (Fl::event_x() - x() - WHITE_KEY_WIDTH) / parent()->tick_width();
	tick = std::max(parent()->quantize_tick(tick), 0);

	int32_t row = (Fl::event_y() - y()) / parent()->note_row_height();
	Pitch pitch = (Pitch)(NUM_NOTES_PER_OCTAVE - row % NUM_NOTES_PER_OCTAVE);
	int32_t octave = (NUM_OCTAVES - row / NUM_NOTES_PER_OCTAVE);

	assert(pitch >= Pitch::C_NAT && pitch <= Pitch::B_NAT);
	assert(octave >= 1 && octave <= 8);

	return parent()->parent()->put_note(pitch, octave, tick);
}

bool Piano_Timeline::handle_note_eraser(int event) {
	auto channel = active_channel_boxes();
	if (!channel) return false;

	for (Note_Box *note : *channel) {
		if (Fl::event_inside(note) && !note->ghost()) {
			select_none();
			note->selected(true);
			parent()->parent()->delete_selection();
			return true;
		}
	}

	return true; // keep focus for drag
}

bool Piano_Timeline::handle_note_selection(int event) {
	auto channel = active_channel_boxes();
	if (!channel) return false;

	if (!Fl::event_shift() && !Fl::event_command()) {
		// clear selection first
		for (Note_Box *note : *channel) {
			if (note->selected()) {
				note->selected(false);
				note->redraw();
				_keys.redraw();
			}
		}
	}

	int32_t tick = parent()->tick();

	bool clicked_note = false;
	for (Note_Box *note : *channel) {
		if (note->ghost()) break;

		const Note_View &view = note->note_view();
		int32_t t_left = note->tick();
		int32_t t_right = t_left + view.length * view.speed;
		if (
			(event == FL_RELEASE && Fl::event_inside(note)) ||
			(event != FL_RELEASE && t_left <= tick && tick < t_right)
		) {
			note->selected(!note->selected() || !Fl::event_command());
			note->redraw();
			_keys.redraw();
			clicked_note = true;
			break;
		}
	}

	const auto find_selection_start = [](std::vector<Note_Box *> *channel) {
		for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
			if ((*note_itr)->selected()) {
				return note_itr;
			}
		}
		return channel->end();
	};
	const auto find_selection_end = [](std::vector<Note_Box *> *channel) {
		for (auto note_itr = channel->rbegin(); note_itr != channel->rend(); ++note_itr) {
			if ((*note_itr)->selected()) {
				return note_itr.base();
			}
		}
		return channel->end();
	};

	if (Fl::event_shift() && clicked_note) {
		const auto selection_start = find_selection_start(channel);
		const auto selection_end = find_selection_end(channel);
		for (auto note_itr = selection_start; note_itr != selection_end; ++note_itr) {
			(*note_itr)->selected(true);
			(*note_itr)->redraw();
		}
	}

	parent()->refresh_note_properties();

	return clicked_note;
}

bool Piano_Timeline::handle_note_selection_start(int event) {
	_selection_region.x = Fl::event_x() - x();
	_selection_region.y = Fl::event_y() - y();
	_selection_region.w = 0;
	_selection_region.h = 0;
	redraw();
	return true;
}

bool Piano_Timeline::handle_note_selection_update(int event) {
	if (_selection_region.x == -1 && _selection_region.y == -1) return false;
	_selection_region.w = Fl::event_x() - x() - _selection_region.x;
	_selection_region.h = Fl::event_y() - y() - _selection_region.y;
	redraw();
	return true;
}

bool Piano_Timeline::handle_note_selection_end(int event) {
	if (_selection_region.x == -1 && _selection_region.y == -1) return false;
	bool made_selection =
		std::abs(_selection_region.w) > SELECTION_REGION_MIN ||
		std::abs(_selection_region.h) > SELECTION_REGION_MIN;
	if (!made_selection) return false;

	auto channel = active_channel_boxes();
	if (!channel) return false;

	if (!Fl::event_shift() && !Fl::event_command()) {
		// clear selection first
		for (Note_Box *note : *channel) {
			if (note->selected()) {
				note->selected(false);
				note->redraw();
				_keys.redraw();
			}
		}
	}

	int selection_x = _selection_region.x + x();
	int selection_y = _selection_region.y + y();
	int selection_w = _selection_region.w;
	int selection_h = _selection_region.h;
	if (selection_w < 0) {
		selection_x += selection_w;
		selection_w *= -1;
	}
	if (selection_h < 0) {
		selection_y += selection_h;
		selection_h *= -1;
	}

	bool selected_note = false;
	for (Note_Box *note : *channel) {
		if (note->ghost()) break;

		if (
			selection_x < note->x() + note->w() &&
			note->x() < selection_x + selection_w &&
			selection_y < note->y() + note->h() &&
			note->y() < selection_y + selection_h
		) {
			note->selected(true);
			note->redraw();
			_keys.redraw();
			selected_note = true;
		}
	}

	const auto find_selection_start = [](std::vector<Note_Box *> *channel) {
		for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
			if ((*note_itr)->selected()) {
				return note_itr;
			}
		}
		return channel->end();
	};
	const auto find_selection_end = [](std::vector<Note_Box *> *channel) {
		for (auto note_itr = channel->rbegin(); note_itr != channel->rend(); ++note_itr) {
			if ((*note_itr)->selected()) {
				return note_itr.base();
			}
		}
		return channel->end();
	};

	if (Fl::event_shift() && selected_note) {
		const auto selection_start = find_selection_start(channel);
		const auto selection_end = find_selection_end(channel);
		for (auto note_itr = selection_start; note_itr != selection_end; ++note_itr) {
			(*note_itr)->selected(true);
			(*note_itr)->redraw();
		}
	}

	parent()->refresh_note_properties();

	return selected_note;
}

bool Piano_Timeline::handle_note_selection_cancel(int event) {
	if (_selection_region.x == -1 && _selection_region.y == -1) return false;
	_selection_region.x = -1;
	_selection_region.y = -1;
	_selection_region.w = 0;
	_selection_region.h = 0;
	redraw();
	return true;
}

bool Piano_Timeline::select_all() {
	auto channel = active_channel_boxes();
	if (!channel) return false;

	bool note_selected = false;
	for (Note_Box *note : *channel) {
		if (!note->selected() && !note->ghost()) {
			note->selected(true);
			note->redraw();
			_keys.redraw();
			note_selected = true;
		}
	}

	parent()->refresh_note_properties();

	return note_selected;
}

bool Piano_Timeline::select_none() {
	auto channel = active_channel_boxes();
	if (!channel) return false;

	bool note_deselected = false;
	for (Note_Box *note : *channel) {
		if (note->selected()) {
			note->selected(false);
			note->redraw();
			_keys.redraw();
			note_deselected = true;
		}
	}

	parent()->refresh_note_properties();

	return note_deselected;
}

void Piano_Timeline::reset_note_colors() {
	for (Note_Box *note : _channel_1_notes) {
		note->color(NOTE_RED);
	}
	for (Note_Box *note : _channel_2_notes) {
		note->color(NOTE_BLUE);
	}
	for (Note_Box *note : _channel_3_notes) {
		note->color(NOTE_GREEN);
	}
	for (Note_Box *note : _channel_4_notes) {
		note->color(NOTE_BROWN);
	}
}

void Piano_Timeline::highlight_tick(std::vector<Note_Box *> &notes, int channel_number, int32_t tick, bool muted, Fl_Color color) {
	_keys.set_channel_pitch(channel_number, Pitch::REST, 0);
	for (Note_Box *note : notes) {
		const Note_View &view = note->note_view();
		int32_t t_left = note->tick();
		int32_t t_right = t_left + view.length * view.speed;
		if (t_left > tick) {
			return;
		}
		if (t_right > tick && !muted) {
			_keys.set_channel_pitch(channel_number, view.pitch, view.octave);
		}
		if (note->color() != color) {
			note->color(color);
			note->redraw();
		}
	}
}

void Piano_Timeline::select_note_at_tick(std::vector<Note_Box *> &notes, int32_t tick) {
	for (Note_Box *note : notes) {
		const Note_View &view = note->note_view();
		int32_t t_left = note->tick();
		int32_t t_right = t_left + view.length * view.speed;
		if (t_left > tick) {
			break;
		}
		if (t_right > tick) {
			note->selected(true);
			break;
		}
	}
	parent()->refresh_note_properties();
}

void Piano_Timeline::set_channel(std::vector<Note_Box *> &channel, std::vector<Flag_Box *> &flags, int channel_number, const std::vector<Note_View> &notes, Fl_Color color) {
	const int octave_height = parent()->octave_height();
	const int note_row_height = parent()->note_row_height();
	const int tick_width = parent()->tick_width();
	const int note_labelsize = parent()->note_labelsize();
	const bool note_labels = parent()->note_labels();

	const auto tick_to_x_pos = [&](int32_t tick) {
		return x() + WHITE_KEY_WIDTH + tick * tick_width;
	};
	const auto pitch_to_y_pos = [&](Pitch pitch, int32_t octave) {
		return y() + (NUM_OCTAVES - octave) * octave_height + (NUM_NOTES_PER_OCTAVE - (size_t)(pitch)) * note_row_height;
	};

	Note_View prev_note;

	begin();
	int32_t tick = 0;
	for (const Note_View &note : notes) {
		if (note.pitch != Pitch::REST) {
			Note_Box *box = new Note_Box(
				note,
				tick,
				tick_to_x_pos(tick),
				pitch_to_y_pos(note.pitch, note.octave),
				note.length * note.speed * tick_width,
				note_row_height,
				note_labels ? pitch_label(note) : nullptr
			);
			box->box(FL_BORDER_BOX);
			box->labelsize(note_labelsize);
			box->color(color);
			channel.push_back(box);

			int32_t row_offset = 0;
			const auto add_flag = [&](Fl_Color c) {
				Flag_Box *flag = new Flag_Box(
					channel.size() - 1,
					row_offset,
					box->x(),
					note.octave == 8 ?
						box->y() + box->h() + tick_width * 4 * row_offset :
						box->y() - tick_width * 4 - tick_width * 4 * row_offset,
					tick_width * 4,
					tick_width * 4
				);
				flag->box(FL_BORDER_BOX);
				flag->color(c);
				flag->hide();
				flags.push_back(flag);
				row_offset += 1;
			};

			if (
				!note.ghost &&
				(((channel_number == 1 || channel_number == 2) && note.duty != prev_note.duty) ||
				(channel_number == 3 && note.wave != prev_note.wave) ||
				(channel_number == 4 && note.drumkit != prev_note.drumkit))
			) {
				add_flag(FLAG_DUTY_WAVE_DRUMKIT);
			}
			if (
				!note.ghost &&
				(note.vibrato_delay != prev_note.vibrato_delay ||
				note.vibrato_extent != prev_note.vibrato_extent ||
				note.vibrato_rate != prev_note.vibrato_rate)
			) {
				add_flag(FLAG_VIBRATO);
			}
			if (
				!note.ghost &&
				(note.volume != prev_note.volume ||
				((channel_number == 1 || channel_number == 2) && note.fade != prev_note.fade))
			) {
				add_flag(FLAG_VOLUME);
			}
			if (
				!note.ghost &&
				note.speed != prev_note.speed
			) {
				add_flag(FLAG_SPEED);
			}
			prev_note = note;
		}
		tick += note.length * note.speed;
	}
	end();

	// fix the keys as the last child
	Fl_Widget **a = (Fl_Widget **)array();
	if (a[children() - 1] != &_keys) {
		int i, j;
		for (i = j = 0; j < children(); j++) {
			if (a[j] != &_keys) {
				a[i++] = a[j];
			}
		}
		a[i] = &_keys;
	}
}

void Piano_Timeline::set_channel_detail(
	std::vector<Note_Box *> &notes,
	std::vector<Loop_Box *> &loops,
	std::vector<Call_Box *> &calls,
	std::vector<Flag_Box *> &flags,
	int detail
) {
	Fl_Boxtype note_box = detail > 0 ? FL_BORDER_BOX : FL_BORDER_FRAME;
	Fl_Boxtype group_box = detail > 0 ? FL_BORDER_FRAME : FL_NO_BOX;
	const bool note_labels = parent()->note_labels();
	for (Note_Box *note : notes) {
		note->box(note_box);
		note->label(detail > 0 && note_labels ? pitch_label(note->note_view()) : nullptr);
	}
	for (Loop_Box *loop : loops) {
		loop->box(group_box);
	}
	for (Call_Box *call : calls) {
		call->box(group_box);
	}
	for (Flag_Box *flag : flags) {
		if (detail > 1) flag->show();
		else flag->hide();
	}
	redraw();
}

std::vector<Note_Box *> *Piano_Timeline::active_channel_boxes() {
	int active_channel = selected_channel();
	if (active_channel == 1) return &_channel_1_notes;
	if (active_channel == 2) return &_channel_2_notes;
	if (active_channel == 3) return &_channel_3_notes;
	if (active_channel == 4) return &_channel_4_notes;
	return nullptr;
}

std::vector<Loop_Box *> *Piano_Timeline::active_channel_loops() {
	int active_channel = selected_channel();
	if (active_channel == 1) return &_channel_1_loops;
	if (active_channel == 2) return &_channel_2_loops;
	if (active_channel == 3) return &_channel_3_loops;
	if (active_channel == 4) return &_channel_4_loops;
	return nullptr;
}

std::vector<Call_Box *> *Piano_Timeline::active_channel_calls() {
	int active_channel = selected_channel();
	if (active_channel == 1) return &_channel_1_calls;
	if (active_channel == 2) return &_channel_2_calls;
	if (active_channel == 3) return &_channel_3_calls;
	if (active_channel == 4) return &_channel_4_calls;
	return nullptr;
}

void Piano_Timeline::draw() {
	OS::Theme theme = OS::current_theme();
	bool dark = theme == OS::Theme::DARK;
	bool hc = theme == OS::Theme::HIGH_CONTRAST;
	bool gray = theme == OS::Theme::CLASSIC || theme == OS::Theme::GREYBIRD || theme == OS::Theme::BRUSHED_METAL;
	bool colorful = theme == OS::Theme::BLUE || theme == OS::Theme::OLIVE || theme == OS::Theme::ROSE_GOLD;

	Fl_Color light_row = (dark || hc) ? BACKGROUND3_COLOR : FL_LIGHT1;
	Fl_Color dark_row =
		(dark || hc) ? fl_color_average(light_row, FL_WHITE, 0.95f) :
		(gray || colorful) ? fl_color_average(light_row, FL_DARK2, 0.5f) :
		FL_DARK2;
	Fl_Color row_divider = dark ? FL_DARK2 : dark_row;
	Fl_Color col_divider = (gray || colorful) ? FL_DARK2 : FL_DARK3;
	Fl_Color cursor_color = (dark || hc) ? FL_YELLOW : fl_color_average(FL_MAGENTA, FL_BLACK, 0.9f);

	if (damage() & ~FL_DAMAGE_CHILD) {
		Piano_Roll *p = parent();
		const int note_row_height = p->note_row_height();
		const int tick_width = p->tick_width();
		const int ticks_per_step = p->ticks_per_step();

		int y_pos = y();
		for (size_t _y = 0; _y < NUM_OCTAVES; ++_y) {
			for (size_t _x = 0; _x < NUM_NOTES_PER_OCTAVE; ++_x) {
				if (is_white_key(_x)) {
					fl_rectf(x(), y_pos, w(), note_row_height, light_row);
				}
				else {
					fl_rectf(x(), y_pos, w(), note_row_height, dark_row);
				}
				if (_x == 0 || _x == 7) {
					fl_color(row_divider);
					fl_xyline(x(), y_pos - 1, x() + w());
					fl_xyline(x(), y_pos, x() + w());
				}
				y_pos += note_row_height;
			}
		}

		int x_pos = x() + WHITE_KEY_WIDTH;
		int time_step_width = tick_width * ticks_per_step;
		const size_t num_dividers = (w() - WHITE_KEY_WIDTH) / time_step_width + 1;
		for (size_t i = 0; i < num_dividers; ++i) {
			fl_color(col_divider);
			fl_yxline(x_pos - 1, y(), y() + h());
			x_pos += time_step_width;
		}

		int active_channel = selected_channel();

		int32_t loop_tick = p->get_loop_tick();
		int32_t channel_1_loop_tick = p->channel_1_loop_tick();
		int32_t channel_2_loop_tick = p->channel_2_loop_tick();
		int32_t channel_3_loop_tick = p->channel_3_loop_tick();
		int32_t channel_4_loop_tick = p->channel_4_loop_tick();

		if (loop_tick != -1 &&
			(channel_1_loop_tick == -1 || channel_1_loop_tick == loop_tick) &&
			(channel_2_loop_tick == -1 || channel_2_loop_tick == loop_tick) &&
			(channel_3_loop_tick == -1 || channel_3_loop_tick == loop_tick) &&
			(channel_4_loop_tick == -1 || channel_4_loop_tick == loop_tick)
		) {
			x_pos = x() + loop_tick * tick_width + WHITE_KEY_WIDTH;
			fl_color(FL_FOREGROUND_COLOR);
			fl_yxline(x_pos - 1, y(), y() + h());
			fl_yxline(x_pos, y(), y() + h());
		}
		else {
			if (channel_1_loop_tick != -1 && (active_channel == 0 || active_channel == 1)) {
				x_pos = x() + channel_1_loop_tick * tick_width + WHITE_KEY_WIDTH;
				fl_color(NOTE_RED);
				fl_yxline(x_pos - 1, y(), y() + h());
				fl_yxline(x_pos, y(), y() + h());
			}
			if (channel_2_loop_tick != -1 && (active_channel == 0 || active_channel == 2)) {
				x_pos = x() + channel_2_loop_tick * tick_width + WHITE_KEY_WIDTH;
				fl_color(NOTE_BLUE);
				fl_yxline(x_pos - 1, y(), y() + h());
				fl_yxline(x_pos, y(), y() + h());
			}
			if (channel_3_loop_tick != -1 && (active_channel == 0 || active_channel == 3)) {
				x_pos = x() + channel_3_loop_tick * tick_width + WHITE_KEY_WIDTH;
				fl_color(NOTE_GREEN);
				fl_yxline(x_pos - 1, y(), y() + h());
				fl_yxline(x_pos, y(), y() + h());
			}
			if (channel_4_loop_tick != -1 && (active_channel == 0 || active_channel == 4)) {
				x_pos = x() + channel_4_loop_tick * tick_width + WHITE_KEY_WIDTH;
				fl_color(NOTE_BROWN);
				fl_yxline(x_pos - 1, y(), y() + h());
				fl_yxline(x_pos, y(), y() + h());
			}
		}

		int32_t end_tick = p->song_length();
		int32_t channel_1_end_tick = p->channel_1_end_tick();
		int32_t channel_2_end_tick = p->channel_2_end_tick();
		int32_t channel_3_end_tick = p->channel_3_end_tick();
		int32_t channel_4_end_tick = p->channel_4_end_tick();

		if (end_tick != -1 &&
			(channel_1_end_tick == -1 || channel_1_end_tick == end_tick) &&
			(channel_2_end_tick == -1 || channel_2_end_tick == end_tick) &&
			(channel_3_end_tick == -1 || channel_3_end_tick == end_tick) &&
			(channel_4_end_tick == -1 || channel_4_end_tick == end_tick)
		) {
			x_pos = x() + end_tick * tick_width + WHITE_KEY_WIDTH;
			fl_color(FL_FOREGROUND_COLOR);
			fl_yxline(x_pos - 1, y(), y() + h());
			fl_yxline(x_pos, y(), y() + h());
		}
		else {
			if (channel_1_end_tick != -1 && (active_channel == 0 || active_channel == 1)) {
				x_pos = x() + channel_1_end_tick * tick_width + WHITE_KEY_WIDTH;
				fl_color(NOTE_RED);
				fl_yxline(x_pos - 1, y(), y() + h());
				fl_yxline(x_pos, y(), y() + h());
			}
			if (channel_2_end_tick != -1 && (active_channel == 0 || active_channel == 2)) {
				x_pos = x() + channel_2_end_tick * tick_width + WHITE_KEY_WIDTH;
				fl_color(NOTE_BLUE);
				fl_yxline(x_pos - 1, y(), y() + h());
				fl_yxline(x_pos, y(), y() + h());
			}
			if (channel_3_end_tick != -1 && (active_channel == 0 || active_channel == 3)) {
				x_pos = x() + channel_3_end_tick * tick_width + WHITE_KEY_WIDTH;
				fl_color(NOTE_GREEN);
				fl_yxline(x_pos - 1, y(), y() + h());
				fl_yxline(x_pos, y(), y() + h());
			}
			if (channel_4_end_tick != -1 && (active_channel == 0 || active_channel == 4)) {
				x_pos = x() + channel_4_end_tick * tick_width + WHITE_KEY_WIDTH;
				fl_color(NOTE_BROWN);
				fl_yxline(x_pos - 1, y(), y() + h());
				fl_yxline(x_pos, y(), y() + h());
			}
		}

		_cursor_tick = p->tick();
		if (_cursor_tick != -1 && (parent()->following() || parent()->paused())) {
			_cursor_tick = _cursor_tick / ticks_per_step * ticks_per_step;
		}
		x_pos = x() + _cursor_tick * tick_width + WHITE_KEY_WIDTH;
		fl_color(cursor_color);
		fl_yxline(x_pos - 1, y(), y() + h());
		fl_yxline(x_pos, y(), y() + h());
	}
	draw_children();

	if (
		_selection_region.x != -1 && _selection_region.y != -1 &&
		(std::abs(_selection_region.w) > SELECTION_REGION_MIN || std::abs(_selection_region.h) > SELECTION_REGION_MIN)
	) {
		int selection_x = _selection_region.x + x();
		int selection_y = _selection_region.y + y();
		int selection_w = _selection_region.w;
		int selection_h = _selection_region.h;
		if (selection_w < 0) {
			selection_x += selection_w;
			selection_w *= -1;
		}
		if (selection_h < 0) {
			selection_y += selection_h;
			selection_h *= -1;
		}
		fl_color(FL_WHITE);
		fl_line_style(FL_SOLID);
		fl_rect(selection_x, selection_y, selection_w, selection_h);
		fl_color(FL_BLACK);
		fl_line_style((dark || hc) ? FL_DOT : FL_DASH);
		fl_rect(selection_x, selection_y, selection_w, selection_h);
		fl_line_style(FL_SOLID);
	}
}

Piano_Roll::Piano_Roll(int X, int Y, int W, int H, const char *l) :
	OS_Scroll(X, Y, W, H, l),
	_piano_timeline(X, Y, W - scrollbar.w(), NUM_OCTAVES * octave_height())
{
	type(BOTH_ALWAYS);
	end();

	scrollbar.callback((Fl_Callback *)scrollbar_cb);
	hscrollbar.callback((Fl_Callback *)hscrollbar_cb);
}

Piano_Roll::~Piano_Roll() noexcept {
	remove(_piano_timeline);
}

inline int Piano_Roll::selected_channel() const {
	return parent()->selected_channel();
}

int Piano_Roll::white_key_height() const {
	return _zoomed ? WHITE_KEY_HEIGHT_ZOOMED : WHITE_KEY_HEIGHT_UNZOOMED;
}

int Piano_Roll::black_key_height() const {
	return _zoomed ? BLACK_KEY_HEIGHT_ZOOMED : BLACK_KEY_HEIGHT_UNZOOMED;
}

int Piano_Roll::octave_height() const {
	return white_key_height() * NUM_WHITE_NOTES;
}

int Piano_Roll::note_row_height() const {
	return octave_height() / NUM_NOTES_PER_OCTAVE;
}

int Piano_Roll::black_key_offset() const {
	return note_row_height() / 2 - black_key_height() / 2;
}

int Piano_Roll::tick_width() const {
	return _zoomed ? TICK_WIDTH_ZOOMED : TICK_WIDTH_UNZOOMED;
}

int Piano_Roll::note_labelsize() const {
	return _zoomed ? NOTE_LABELSIZE_ZOOMED : NOTE_LABELSIZE_UNZOOMED;
}

bool Piano_Roll::note_labels() const {
	return parent()->note_labels();
}

int Piano_Roll::handle(int event) {
	switch (event) {
	case FL_ENTER:
		fl_cursor(parent()->pencil_mode() ? FL_CURSOR_CROSS : FL_CURSOR_DEFAULT);
		return 1;
	case FL_LEAVE:
		fl_cursor(FL_CURSOR_DEFAULT);
		return 1;
	case FL_MOUSEWHEEL:
		if (Fl::event_shift()) {
			std::swap(Fl::e_dx, Fl::e_dy);
		}
		break;
	case FL_SHORTCUT:
	case FL_KEYBOARD:
		// don't hog arrow keys with ctrl
		if (Fl::event_command() && (
			Fl::event_key() == FL_Up ||
			Fl::event_key() == FL_Down ||
			Fl::event_key() == FL_Left ||
			Fl::event_key() == FL_Right
		)) {
			return 0;
		}

		// hack to reverse scrollbar priority
		if (Fl::event_key() == FL_Page_Up) {
			if (xposition() > 0) {
				int x_pos = xposition() - (w() - WHITE_KEY_WIDTH * 2);
				scroll_to(std::max(x_pos, 0), yposition());
				sticky_keys();
				if (_following) {
					focus_cursor();
				}
				redraw();
			}
			return 1;
		}
		else if (Fl::event_key() == FL_Page_Down) {
			if (xposition() < scroll_x_max()) {
				int x_pos = xposition() + (w() - WHITE_KEY_WIDTH * 2);
				scroll_to(std::min(x_pos, scroll_x_max()), yposition());
				sticky_keys();
				if (_following) {
					focus_cursor();
				}
				redraw();
			}
			return 1;
		}
		else if (Fl::event_key() == FL_Home) {
			if (xposition() > 0) {
				scroll_to(0, yposition());
				sticky_keys();
				if (_following) {
					focus_cursor();
				}
				redraw();
			}
			return 1;
		}
		else if (Fl::event_key() == FL_End) {
			if (xposition() < scroll_x_max()) {
				scroll_to(scroll_x_max(), yposition());
				sticky_keys();
				if (_following) {
					focus_cursor();
				}
				redraw();
			}
			return 1;
		}

		if (Fl::event_key() == FL_Escape && _tick != -1) {
			_tick = -1;
			parent()->set_song_position(0);
			redraw();
		}
		break;
	}
	return OS_Scroll::handle(event);
}

void Piano_Roll::set_tick_from_x_pos(int X) {
	int32_t t = (X - _piano_timeline.x() - WHITE_KEY_WIDTH) / tick_width();
	t = std::max(quantize_tick(t), 0);

	if (_tick != t && t < _song_length) {
		_tick = t;
		parent()->set_song_position(_tick);
		redraw();
	}
}

bool Piano_Roll::handle_mouse_click_song_position(int event) {
	if (
		(event == FL_PUSH && (
			Fl::event_button() == FL_MIDDLE_MOUSE ||
			(Fl::event_command() && Fl::event_button() == FL_RIGHT_MOUSE)
		)) ||
		(event != FL_PUSH && !_following)
	) {
		set_tick_from_x_pos(Fl::event_x());

		return true;
	}
	return false;
}

bool Piano_Roll::handle_mouse_click_continuous_scroll(int event) {
	if (_following) {
		parent()->continuous_scroll(!_continuous);
		parent()->redraw();
		return true;
	}
	return false;
}

void Piano_Roll::refresh_note_properties() {
	auto channel = _piano_timeline.active_channel_boxes();
	if (channel && !_following && !_paused) {
		std::vector<const Note_View *> selected_notes;
		for (Note_Box *note : *channel) {
			if (note->selected()) {
				selected_notes.push_back(&note->note_view());
			}
		}
		if (selected_notes.size() > 0) {
			parent()->open_note_properties();
			parent()->set_note_properties(selected_notes);
			return;
		}
	}
	parent()->close_note_properties();
}

void Piano_Roll::step_backward() {
	if (_song_length == -1) return;
	if (_tick == -1) _tick = 0;

	int32_t delta = ticks_per_step();

	const auto view = active_channel_view();
	if (view) {
		const Note_View *note = find_note_view_at_tick(*view, _tick - 1);
		if (note) {
			delta = note->speed;
		}
	}

	_tick -= delta;
	if (_tick < 0) _tick = 0;
	parent()->set_song_position(_tick);
}

void Piano_Roll::step_forward() {
	if (_song_length == -1) return;
	if (_tick == -1) _tick = 0;

	int32_t delta = ticks_per_step();

	const auto view = active_channel_view();
	if (view) {
		const Note_View *note = find_note_view_at_tick(*view, _tick);
		if (note) {
			delta = note->speed;
		}
	}

	_tick += delta;
	if (_tick > _song_length) _tick = _song_length;
	parent()->set_song_position(_tick);
}

void Piano_Roll::skip_backward() {
	if (_song_length == -1) return;
	if (_tick == -1) _tick = 0;

	int32_t delta = ticks_per_step() * 4;

	const auto view = active_channel_view();
	if (view) {
		int32_t tick_offset = 0;
		const Note_View *note = find_note_view_at_tick(*view, _tick - 1, &tick_offset);
		if (note) {
			delta = tick_offset + 1;
		}
	}

	_tick -= delta;
	if (_tick < 0) _tick = 0;
	parent()->set_song_position(_tick);
}

void Piano_Roll::skip_forward() {
	if (_song_length == -1) return;
	if (_tick == -1) _tick = 0;

	int32_t delta = ticks_per_step() * 4;

	const auto view = active_channel_view();
	if (view) {
		int32_t tick_offset = 0;
		const Note_View *note = find_note_view_at_tick(*view, _tick, &tick_offset);
		if (note) {
			delta = note->length * note->speed - tick_offset;
		}
	}

	_tick += delta;
	if (_tick > _song_length) _tick = _song_length;
	parent()->set_song_position(_tick);
}

void Piano_Roll::zoom(bool z) {
	if (_zoomed == z) return;
	_zoomed = z;

	float scroll_x = scroll_x_max() > 0 ? xposition() / (float) scroll_x_max() : 0.0f;
	float scroll_y = scroll_y_max() > 0 ? yposition() / (float) scroll_y_max() : 0.0f;

	_piano_timeline.calc_sizes();
	_piano_timeline._keys.calc_sizes();

	set_timeline_width();
	_piano_timeline.h(NUM_OCTAVES * octave_height());

	scroll_to((int)(scroll_x * scroll_x_max()), (int)(scroll_y * scroll_y_max()));
	sticky_keys();

	if (_following) {
		focus_cursor();
	}
}

void Piano_Roll::set_size(int W, int H) {
	if (W != w() || H != h()) {
		size(W, H);
		set_timeline_width();
		if (xposition() > scroll_x_max()) {
			scroll_to(scroll_x_max(), yposition());
			sticky_keys();
		}
		if (yposition() > scroll_y_max()) {
			scroll_to(xposition(), scroll_y_max());
		}
	}
}

void Piano_Roll::set_timeline_width() {
	_piano_timeline.w(std::max(WHITE_KEY_WIDTH + _song_length * tick_width(), w() - scrollbar.w()));
	int32_t last_note_x = get_last_note_x();
	int width = last_note_x + w() - scrollbar.w() - WHITE_KEY_WIDTH;
	if (width > _piano_timeline.w()) {
		_piano_timeline.w(width);
	}
}

void Piano_Roll::set_timeline(const Song &song) {
	_channel_1_loop_tick = song.channel_1_loop_tick();
	_channel_2_loop_tick = song.channel_2_loop_tick();
	_channel_3_loop_tick = song.channel_3_loop_tick();
	_channel_4_loop_tick = song.channel_4_loop_tick();
	_channel_1_end_tick = song.channel_1_end_tick();
	_channel_2_end_tick = song.channel_2_end_tick();
	_channel_3_end_tick = song.channel_3_end_tick();
	_channel_4_end_tick = song.channel_4_end_tick();

	_song_length = get_song_length();

	_piano_timeline.begin();
	build_note_view(_piano_timeline._channel_1_loops, _piano_timeline._channel_1_calls, _channel_1_notes, song.channel_1_commands(), _song_length, NOTE_RED);
	build_note_view(_piano_timeline._channel_2_loops, _piano_timeline._channel_2_calls, _channel_2_notes, song.channel_2_commands(), _song_length, NOTE_BLUE);
	build_note_view(_piano_timeline._channel_3_loops, _piano_timeline._channel_3_calls, _channel_3_notes, song.channel_3_commands(), _song_length, NOTE_GREEN);
	build_note_view(_piano_timeline._channel_4_loops, _piano_timeline._channel_4_calls, _channel_4_notes, song.channel_4_commands(), _song_length, NOTE_BROWN);
	_piano_timeline.end();

	_piano_timeline.set_channel_1(_channel_1_notes);
	_piano_timeline.set_channel_2(_channel_2_notes);
	_piano_timeline.set_channel_3(_channel_3_notes);
	_piano_timeline.set_channel_4(_channel_4_notes);

	set_timeline_width();
}

void Piano_Roll::set_active_channel_timeline(const Song &song) {
	_piano_timeline.begin();
	if (selected_channel() == 1) {
		_piano_timeline.clear_channel_1();
		_channel_1_notes.clear();
		build_note_view(_piano_timeline._channel_1_loops, _piano_timeline._channel_1_calls, _channel_1_notes, song.channel_1_commands(), _song_length, NOTE_RED);
		_piano_timeline.set_channel_1(_channel_1_notes);
		_piano_timeline.set_channel_detail(
			_piano_timeline._channel_1_notes,
			_piano_timeline._channel_1_loops,
			_piano_timeline._channel_1_calls,
			_piano_timeline._channel_1_flags,
			2
		);
	}
	else if (selected_channel() == 2) {
		_piano_timeline.clear_channel_2();
		_channel_2_notes.clear();
		build_note_view(_piano_timeline._channel_2_loops, _piano_timeline._channel_2_calls, _channel_2_notes, song.channel_2_commands(), _song_length, NOTE_BLUE);
		_piano_timeline.set_channel_2(_channel_2_notes);
		_piano_timeline.set_channel_detail(
			_piano_timeline._channel_2_notes,
			_piano_timeline._channel_2_loops,
			_piano_timeline._channel_2_calls,
			_piano_timeline._channel_2_flags,
			2
		);
	}
	else if (selected_channel() == 3) {
		_piano_timeline.clear_channel_3();
		_channel_3_notes.clear();
		build_note_view(_piano_timeline._channel_3_loops, _piano_timeline._channel_3_calls, _channel_3_notes, song.channel_3_commands(), _song_length, NOTE_GREEN);
		_piano_timeline.set_channel_3(_channel_3_notes);
		_piano_timeline.set_channel_detail(
			_piano_timeline._channel_3_notes,
			_piano_timeline._channel_3_loops,
			_piano_timeline._channel_3_calls,
			_piano_timeline._channel_3_flags,
			2
		);
	}
	else if (selected_channel() == 4) {
		_piano_timeline.clear_channel_4();
		_channel_4_notes.clear();
		build_note_view(_piano_timeline._channel_4_loops, _piano_timeline._channel_4_calls, _channel_4_notes, song.channel_4_commands(), _song_length, NOTE_BROWN);
		_piano_timeline.set_channel_4(_channel_4_notes);
		_piano_timeline.set_channel_detail(
			_piano_timeline._channel_4_notes,
			_piano_timeline._channel_4_loops,
			_piano_timeline._channel_4_calls,
			_piano_timeline._channel_4_flags,
			2
		);
	}
	_piano_timeline.end();

	set_timeline_width();
	scroll_to(std::min(xposition(), scroll_x_max()), yposition());
	sticky_keys();
}

void Piano_Roll::set_active_channel_selection(const std::set<int32_t> &selection) {
	auto channel = _piano_timeline.active_channel_boxes();
	if (!channel) return;

	for (int32_t index : selection) {
		channel->at(index)->selected(true);
	}

	refresh_note_properties();
}

void Piano_Roll::select_note_at_tick() {
	_piano_timeline.select_note_at_tick(*_piano_timeline.active_channel_boxes(), _tick);
}

void Piano_Roll::build_note_view(
	std::vector<Loop_Box *> &loops,
	std::vector<Call_Box *> &calls,
	std::vector<Note_View> &notes,
	const std::vector<Command> &commands,
	int32_t end_tick,
	Fl_Color color
) {
	int32_t tick = 0;

	Note_View note;
	note.octave = 8;
	note.speed = 1;
	note.drumkit = -1;

	bool restarted = false;
	Loop_Box *loop = nullptr;
	Call_Box *call = nullptr;
	bool speed_set_in_call = false;

	auto command_itr = commands.begin();

	std::stack<std::pair<decltype(command_itr), int32_t>> loop_stack;
	std::stack<decltype(command_itr)> call_stack;
	std::set<std::string> visited_labels_during_call;
	std::map<std::string, int32_t> label_positions;

	std::set<std::string> loop_targets;
	for (const Command &command : commands) {
		if (command.type == Command_Type::SOUND_LOOP && command.sound_loop.loop_count > 1) {
			loop_targets.insert(command.target);
		}
	}
	const auto is_loop_target = [&](const std::vector<std::string> &labels) {
		for (const std::string &label : labels) {
			if (loop_targets.count(label) > 0) {
				return true;
			}
		}
		return false;
	};

	while (command_itr != commands.end() && (tick < end_tick || (!restarted && tick == end_tick && (loop_stack.size() > 0 || call_stack.size() > 0)))) {
		for (const std::string &label : command_itr->labels) {
			label_positions.insert({ label, tick });
			if (call_stack.size() > 0) {
				visited_labels_during_call.insert(label);
			}
		}
		if (!restarted && is_loop_target(command_itr->labels)) {
			note.index = command_itr - commands.begin();
			loop = new Loop_Box(note, tick, 0, 0, 0, 0);
			loop->box(FL_BORDER_FRAME);
			loop->color(fl_lighter(color));
			loops.push_back(loop);
		}

		// TODO: handle all other commands...
		if (command_itr->type == Command_Type::NOTE) {
			note.length = command_itr->note.length;
			note.pitch = command_itr->note.pitch;
			tick += note.length * note.speed;
			if (tick > end_tick) {
				assert(restarted);
				note.length = note.length * note.speed - (tick - end_tick);
				note.speed = 1;
			}
			note.index = command_itr - commands.begin();
			note.ghost = restarted;
			notes.push_back(note);

			note.slide_duration = 0;
			note.slide_octave = 0;
			note.slide_pitch = Pitch::REST;

			if (loop) {
				loop->set_end_tick(tick);
				if (compare_pitch(note.pitch, note.octave, loop->max_pitch(), loop->max_octave()) > 0) {
					loop->set_max_pitch(note.pitch, note.octave);
				}
				if (compare_pitch(note.pitch, note.octave, loop->min_pitch(), loop->min_octave()) < 0) {
					loop->set_min_pitch(note.pitch, note.octave);
				}
			}
			if (call) {
				call->set_end_tick(tick);
				if (compare_pitch(note.pitch, note.octave, call->max_pitch(), call->max_octave()) > 0) {
					call->set_max_pitch(note.pitch, note.octave);
				}
				if (compare_pitch(note.pitch, note.octave, call->min_pitch(), call->min_octave()) < 0) {
					call->set_min_pitch(note.pitch, note.octave);
				}
				if (speed_set_in_call) {
					call->add_unambiguous_ticks(note.length * note.speed);
				}
				else {
					call->add_ambiguous_ticks(note.length);
				}
			}
		}
		else if (command_itr->type == Command_Type::DRUM_NOTE) {
			note.length = command_itr->drum_note.length;
			note.pitch = (Pitch)command_itr->drum_note.instrument;
			tick += note.length * note.speed;
			if (tick > end_tick) {
				assert(restarted);
				note.length = note.length * note.speed - (tick - end_tick);
				note.speed = 1;
			}
			note.index = command_itr - commands.begin();
			note.ghost = restarted;
			notes.push_back(note);

			if (loop) {
				loop->set_end_tick(tick);
				if (compare_pitch(note.pitch, note.octave, loop->max_pitch(), loop->max_octave()) > 0) {
					loop->set_max_pitch(note.pitch, note.octave);
				}
				if (compare_pitch(note.pitch, note.octave, loop->min_pitch(), loop->min_octave()) < 0) {
					loop->set_min_pitch(note.pitch, note.octave);
				}
			}
			if (call) {
				call->set_end_tick(tick);
				if (compare_pitch(note.pitch, note.octave, call->max_pitch(), call->max_octave()) > 0) {
					call->set_max_pitch(note.pitch, note.octave);
				}
				if (compare_pitch(note.pitch, note.octave, call->min_pitch(), call->min_octave()) < 0) {
					call->set_min_pitch(note.pitch, note.octave);
				}
				if (speed_set_in_call) {
					call->add_unambiguous_ticks(note.length * note.speed);
				}
				else {
					call->add_ambiguous_ticks(note.length);
				}
			}
		}
		else if (command_itr->type == Command_Type::REST) {
			note.length = command_itr->rest.length;
			note.pitch = Pitch::REST;
			tick += note.length * note.speed;
			if (tick > end_tick) {
				assert(restarted);
				note.length = note.length * note.speed - (tick - end_tick);
				note.speed = 1;
			}
			note.index = command_itr - commands.begin();
			note.ghost = restarted;
			notes.push_back(note);

			if (loop) {
				loop->set_end_tick(tick);
			}
			if (call) {
				call->set_end_tick(tick);
				if (speed_set_in_call) {
					call->add_unambiguous_ticks(note.length * note.speed);
				}
				else {
					call->add_ambiguous_ticks(note.length);
				}
			}
		}
		else if (command_itr->type == Command_Type::OCTAVE) {
			note.octave = command_itr->octave.octave;
		}
		else if (command_itr->type == Command_Type::NOTE_TYPE) {
			note.speed = command_itr->note_type.speed;
			note.volume = command_itr->note_type.volume;
			note.fade = command_itr->note_type.fade;
			if (call) speed_set_in_call = true;
		}
		else if (command_itr->type == Command_Type::DRUM_SPEED) {
			note.speed = command_itr->drum_speed.speed;
			if (call) speed_set_in_call = true;
		}
		else if (command_itr->type == Command_Type::TRANSPOSE) {
			note.transpose_octaves = command_itr->transpose.num_octaves;
			note.transpose_pitches = command_itr->transpose.num_pitches;
		}
		else if (command_itr->type == Command_Type::TEMPO) {
			note.tempo = command_itr->tempo.tempo;
		}
		else if (command_itr->type == Command_Type::DUTY_CYCLE) {
			note.duty = command_itr->duty_cycle.duty;
		}
		else if (command_itr->type == Command_Type::VOLUME_ENVELOPE) {
			note.volume = command_itr->volume_envelope.volume;
			note.fade = command_itr->volume_envelope.fade;
		}
		else if (command_itr->type == Command_Type::PITCH_SLIDE) {
			note.slide_duration = command_itr->pitch_slide.duration;
			note.slide_octave = command_itr->pitch_slide.octave;
			note.slide_pitch = command_itr->pitch_slide.pitch;
		}
		else if (command_itr->type == Command_Type::VIBRATO) {
			note.vibrato_delay = command_itr->vibrato.delay;
			note.vibrato_extent = command_itr->vibrato.extent;
			note.vibrato_rate = command_itr->vibrato.rate;
		}
		else if (command_itr->type == Command_Type::TOGGLE_NOISE) {
			note.drumkit = command_itr->toggle_noise.drumkit;
		}
		else if (command_itr->type == Command_Type::SOUND_JUMP) {
			if (
				!label_positions.count(command_itr->target) ||
				(call_stack.size() > 0 && !visited_labels_during_call.count(command_itr->target))
			) {
				command_itr = find_note_with_label(commands, command_itr->target);
				continue;
			}
			if (tick < end_tick) {
				restarted = true;
				command_itr = find_note_with_label(commands, command_itr->target);
				continue;
			}
			break; // song is finished
		}
		else if (command_itr->type == Command_Type::SOUND_LOOP) {
			if (loop) {
				note.index = command_itr - commands.begin();
				loop->set_end_note_view(note);
				loop = nullptr;
			}
			if (loop_stack.size() > 0 && loop_stack.top().first == command_itr) {
				loop_stack.top().second -= 1;
				if (loop_stack.top().second == 0) {
					loop_stack.pop();
				}
				else {
					command_itr = find_note_with_label(commands, command_itr->target);
					continue;
				}
			}
			else {
				if (command_itr->sound_loop.loop_count == 0) {
					if (
						!label_positions.count(command_itr->target) ||
						(call_stack.size() > 0 && !visited_labels_during_call.count(command_itr->target))
					) {
						command_itr = find_note_with_label(commands, command_itr->target);
						continue;
					}
					if (tick < end_tick) {
						restarted = true;
						command_itr = find_note_with_label(commands, command_itr->target);
						continue;
					}
					break; // song is finished
				}
				else if (command_itr->sound_loop.loop_count > 1) {
					// nested loops not allowed
					assert(loop_stack.size() == 0);

					loop_stack.emplace(command_itr, command_itr->sound_loop.loop_count - 1);
					command_itr = find_note_with_label(commands, command_itr->target);
					continue;
				}
			}
		}
		else if (command_itr->type == Command_Type::SOUND_CALL) {
			// nested calls not allowed
			assert(call_stack.size() == 0);

			if (!restarted) {
				note.index = command_itr - commands.begin();
				call = new Call_Box(note, tick, 0, 0, 0, 0);
				call->box(FL_BORDER_FRAME);
				call->color(fl_darker(color));
				calls.push_back(call);
				speed_set_in_call = false;
			}

			call_stack.push(command_itr);
			command_itr = find_note_with_label(commands, command_itr->target);
			continue;
		}
		else if (command_itr->type == Command_Type::SOUND_RET) {
			if (call) {
				note.index = command_itr - commands.begin();
				call->set_end_note_view(note);
				call = nullptr;
			}
			if (call_stack.size() == 0) {
				break; // song is finished
			}
			else {
				command_itr = call_stack.top();
				call_stack.pop();
				visited_labels_during_call.clear();
			}
		}
		else if (command_itr->type == Command_Type::LOAD_WAVE) {
			if (note.wave >= 0x0f) {
				note.wave = command_itr->load_wave.wave;
			}
		}
		else if (command_itr->type == Command_Type::INC_OCTAVE) {
			note.octave += 1;
			if (note.octave > 8) {
				note.octave = 1;
			}
		}
		else if (command_itr->type == Command_Type::DEC_OCTAVE) {
			note.octave -= 1;
			if (note.octave < 1) {
				note.octave = 8;
			}
		}
		else if (command_itr->type == Command_Type::SPEED) {
			note.speed = command_itr->speed.speed;
			if (call) speed_set_in_call = true;
		}
		else if (command_itr->type == Command_Type::CHANNEL_VOLUME) {
			note.volume = command_itr->channel_volume.volume;
		}
		else if (command_itr->type == Command_Type::FADE_WAVE) {
			note.fade = command_itr->fade_wave.fade;
		}
		++command_itr;
	}

	const auto tick_to_x_pos = [&](int32_t tick) {
		return _piano_timeline.x() + WHITE_KEY_WIDTH + tick * tick_width();
	};
	const auto pitch_to_y_pos = [&](Pitch pitch, int32_t octave) {
		return _piano_timeline.y() + (NUM_OCTAVES - octave) * octave_height() + (NUM_NOTES_PER_OCTAVE - (size_t)(pitch)) * note_row_height();
	};
	const auto resize_wrappers = [&](auto &wrappers) {
		for (Wrapper_Box *wrapper : wrappers) {
			if (
				wrapper->min_pitch() == Pitch::B_NAT && wrapper->min_octave() == 8 &&
				wrapper->max_pitch() == Pitch::C_NAT && wrapper->max_octave() == 1
			) {
				wrapper->set_min_pitch(Pitch::C_NAT, 1);
				wrapper->set_max_pitch(Pitch::B_NAT, 8);
			}
			int x_left =   tick_to_x_pos(wrapper->start_tick());
			int x_right =  tick_to_x_pos(wrapper->end_tick());
			int y_top =    pitch_to_y_pos(wrapper->max_pitch(), wrapper->max_octave());
			int y_bottom = pitch_to_y_pos(wrapper->min_pitch(), wrapper->min_octave());
			wrapper->resize(
				x_left,
				y_top,
				x_right - x_left,
				y_bottom + note_row_height() - y_top
			);
		}
	};

	resize_wrappers(loops);
	resize_wrappers(calls);
}

int32_t Piano_Roll::get_song_length() const {
	const int32_t loop_tick = get_loop_tick();
	const int32_t max_length = std::max({ _channel_1_end_tick, _channel_2_end_tick, _channel_3_end_tick, _channel_4_end_tick });

	if (loop_tick == -1) return max_length;

	int32_t song_length =
		loop_tick == _channel_1_loop_tick ? _channel_1_end_tick :
		loop_tick == _channel_2_loop_tick ? _channel_2_end_tick :
		loop_tick == _channel_3_loop_tick ? _channel_3_end_tick :
		_channel_4_end_tick;
	const int32_t body_length = song_length - loop_tick;

	const int32_t channel_1_body_length = _channel_1_loop_tick != -1 ? _channel_1_end_tick - _channel_1_loop_tick : 0;
	const int32_t channel_2_body_length = _channel_2_loop_tick != -1 ? _channel_2_end_tick - _channel_2_loop_tick : 0;
	const int32_t channel_3_body_length = _channel_3_loop_tick != -1 ? _channel_3_end_tick - _channel_3_loop_tick : 0;
	const int32_t channel_4_body_length = _channel_4_loop_tick != -1 ? _channel_4_end_tick - _channel_4_loop_tick : 0;

	const int32_t channel_1_offset = channel_1_body_length ? (loop_tick - _channel_1_loop_tick) % channel_1_body_length : 0;
	const int32_t channel_2_offset = channel_2_body_length ? (loop_tick - _channel_2_loop_tick) % channel_2_body_length : 0;
	const int32_t channel_3_offset = channel_3_body_length ? (loop_tick - _channel_3_loop_tick) % channel_3_body_length : 0;
	const int32_t channel_4_offset = channel_4_body_length ? (loop_tick - _channel_4_loop_tick) % channel_4_body_length : 0;

	const auto channels_aligned = [&]() {
		return
			(channel_1_body_length == 0 || (song_length - _channel_1_loop_tick) % channel_1_body_length == channel_1_offset) &&
			(channel_2_body_length == 0 || (song_length - _channel_2_loop_tick) % channel_2_body_length == channel_2_offset) &&
			(channel_3_body_length == 0 || (song_length - _channel_3_loop_tick) % channel_3_body_length == channel_3_offset) &&
			(channel_4_body_length == 0 || (song_length - _channel_4_loop_tick) % channel_4_body_length == channel_4_offset);
	};

	while (song_length < max_length || !channels_aligned()) {
		song_length += body_length;
	}

	return song_length;
}

int32_t Piano_Roll::get_loop_tick() const {
	int32_t loop_tick = std::max({ _channel_1_loop_tick, _channel_2_loop_tick, _channel_3_loop_tick, _channel_4_loop_tick });
	return loop_tick;
}

int32_t Piano_Roll::get_last_note_x() const {
	const auto get_channel_last_note_x = [this](const std::vector<Note_Box *> &notes) {
		if (notes.size() > 0) {
			return notes.back()->x() - _piano_timeline.x();
		}
		return 0;
	};
	int32_t last_note_x = std::max({
		get_channel_last_note_x(_piano_timeline._channel_1_notes),
		get_channel_last_note_x(_piano_timeline._channel_2_notes),
		get_channel_last_note_x(_piano_timeline._channel_3_notes),
		get_channel_last_note_x(_piano_timeline._channel_4_notes),
	});
	return last_note_x;
}

void Piano_Roll::update_channel_detail(int channel_number) {
	_piano_timeline.set_channel_1_detail(channel_number == 1 ? 2 : channel_number == 0 ? 1 : 0);
	_piano_timeline.set_channel_2_detail(channel_number == 2 ? 2 : channel_number == 0 ? 1 : 0);
	_piano_timeline.set_channel_3_detail(channel_number == 3 ? 2 : channel_number == 0 ? 1 : 0);
	_piano_timeline.set_channel_4_detail(channel_number == 4 ? 2 : channel_number == 0 ? 1 : 0);
	refresh_note_properties();
}

void Piano_Roll::align_cursor() {
	if (_tick == -1) return;
	_tick = quantize_tick(_tick, true);
}

void Piano_Roll::clear() {
	_piano_timeline.clear();
	_piano_timeline.w(w() - scrollbar.w());
	_piano_timeline._keys.reset_channel_pitches();
	scroll_to(0, yposition());
	sticky_keys();
	_following = false;
	_paused = false;
	_tick = -1;

	_channel_1_notes.clear();
	_channel_2_notes.clear();
	_channel_3_notes.clear();
	_channel_4_notes.clear();

	_channel_1_loop_tick = -1;
	_channel_2_loop_tick = -1;
	_channel_3_loop_tick = -1;
	_channel_4_loop_tick = -1;
	_channel_1_end_tick = -1;
	_channel_2_end_tick = -1;
	_channel_3_end_tick = -1;
	_channel_4_end_tick = -1;
	_song_length = -1;
}

void Piano_Roll::start_following() {
	_following = true;
	_paused = false;
	_piano_timeline.reset_note_colors();
	_piano_timeline._keys.reset_channel_pitches();
	_piano_timeline._selection_region.x = -1;
	_piano_timeline._selection_region.y = -1;
	_piano_timeline._selection_region.w = 0;
	_piano_timeline._selection_region.h = 0;
	if (_tick == -1) {
		scroll_to(0, yposition());
		sticky_keys();
	}
	refresh_note_properties();
	redraw();
}

void Piano_Roll::unpause_following() {
	_following = true;
	_paused = false;
	_piano_timeline._selection_region.x = -1;
	_piano_timeline._selection_region.y = -1;
	_piano_timeline._selection_region.w = 0;
	_piano_timeline._selection_region.h = 0;
}

void Piano_Roll::stop_following() {
	_following = false;
	_paused = false;
	_tick = -1;
	_piano_timeline.reset_note_colors();
	_piano_timeline._keys.reset_channel_pitches();
	refresh_note_properties();
	redraw();
}

void Piano_Roll::pause_following() {
	_following = false;
	_paused = true;
}

void Piano_Roll::highlight_tick(int32_t t) {
	if (_tick == t) return; // no change
	_tick = t;

	int scroll_x_before = xposition();

	_piano_timeline.highlight_channel_1_tick(_tick, _channel_1_muted);
	_piano_timeline.highlight_channel_2_tick(_tick, _channel_2_muted);
	_piano_timeline.highlight_channel_3_tick(_tick, _channel_3_muted);
	_piano_timeline.highlight_channel_4_tick(_tick, _channel_4_muted);
	_piano_timeline._keys.update_key_colors();
	_piano_timeline._keys.redraw();

	focus_cursor();
	if (
		_tick / ticks_per_step() * ticks_per_step() != _piano_timeline._cursor_tick ||
		xposition() != scroll_x_before
	) {
		redraw();
	}
}

void Piano_Roll::focus_cursor(bool center) {
	int x_pos = (_tick / ticks_per_step() * ticks_per_step()) * tick_width();
	if ((_following && _continuous) || x_pos > xposition() + w() - WHITE_KEY_WIDTH * 2 || x_pos < xposition()) {
		int scroll_pos = center ? x_pos + WHITE_KEY_WIDTH - w() / 2 : x_pos;
		scroll_to(std::min(std::max(scroll_pos, 0), scroll_x_max()), yposition());
		sticky_keys();
	}
}

void Piano_Roll::sticky_keys() {
	_piano_timeline._keys.position(0, _piano_timeline._keys.y());
}

void Piano_Roll::scroll_to_y_max() {
	scroll_to(xposition(), scroll_y_max());
}

void Piano_Roll::scroll_to(int X, int Y) {
	OS_Scroll::scroll_to(X, Y);
	for (Fl_Widget *wgt : _correlates) {
		wgt->damage(1);
	}
}

int Piano_Roll::scroll_x_max() const {
	return _piano_timeline.w() - (w() - scrollbar.w());
}

int Piano_Roll::scroll_y_max() const {
	return _piano_timeline.h() - (h() - hscrollbar.h());
}

bool Piano_Roll::put_note(Song &song, Pitch pitch, int32_t octave, int32_t tick) {
	auto channel = _piano_timeline.active_channel_boxes();
	if (!channel) return false;

	auto view = active_channel_view();

	if (tick == -1) tick = _tick;
	if (tick == -1) return false;

	int32_t tick_offset = 0;

	const Note_View *note_view = find_note_view_at_tick(*view, tick, &tick_offset);

	if (!note_view || note_view->ghost) return false;

	int32_t index = note_view->index;
	int32_t speed = note_view->speed;
	int32_t old_octave = note_view->octave;
	bool set_drumkit = selected_channel() == 4 && note_view->drumkit == -1;

	const Note_View *prev_note = find_note_before_tick(*view, tick - tick_offset);

	if (octave == 0) {
		if (prev_note && (tick_offset == 0 || note_view->pitch == Pitch::REST)) {
			octave = prev_note->octave;
		}
		else {
			octave = old_octave;
		}
	}

	int32_t prev_length = 1;
	int32_t prev_speed = note_view->speed;
	if (prev_note && note_view->pitch == Pitch::REST) {
		prev_length = prev_note->length;
		prev_speed = prev_note->speed;
	}

	for (const Note_View &other : *view) {
		if (other.index == note_view->index && other.speed != note_view->speed) {
			prev_speed = 0;
			break;
		}
	}

	std::set<int32_t> selected_boxes;

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			selected_boxes.insert(note_itr - channel->begin());
		}
	}

	int32_t length = song.put_note(selected_channel(), selected_boxes, pitch, octave, old_octave, speed, prev_length, prev_speed, index, tick, tick_offset / speed, set_drumkit);
	set_active_channel_timeline(song);
	_piano_timeline.select_note_at_tick(*channel, tick);

	_tick = tick + length;
	focus_cursor(true);

	return true;
}

bool Piano_Roll::set_speed(Song &song, int32_t speed) {
	auto channel = _piano_timeline.active_channel_boxes();
	if (!channel) return false;

	auto view = active_channel_view();

	std::set<int32_t> selected_notes;
	std::set<int32_t> selected_boxes;

	const std::vector<Command> &commands = song.channel_commands(selected_channel());

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			Note_View note_view = note->note_view();
			auto command_itr = commands.begin() + note_view.index;

			int ticks_to_erase = (note_view.length * speed) - (note_view.length * note_view.speed);
			if (
				speed > note_view.speed &&
				!is_followed_by_n_ticks_of_rest(command_itr, commands.end(), ticks_to_erase, note_view.speed)
			) {
				return false;
			}

			for (Note_Box *other : *channel) {
				if (other->note_view().index == note_view.index && other->note_view().speed != note_view.speed) {
					return false; // volatile speed changes not allowed
				}
			}
			selected_notes.insert(note_view.index);
			selected_boxes.insert(note_itr - channel->begin());
		}
	}
	if (selected_notes.size() == 0) {
		return false;
	}

	song.set_speed(selected_channel(), selected_notes, selected_boxes, *view, speed);
	set_active_channel_timeline(song);
	set_active_channel_selection(selected_boxes);

	align_cursor();

	return true;
}

bool Piano_Roll::set_volume(Song &song, int32_t volume) {
	auto channel = _piano_timeline.active_channel_boxes();
	if (!channel) return false;

	auto view = active_channel_view();

	std::set<int32_t> selected_notes;
	std::set<int32_t> selected_boxes;

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			Note_View note_view = note->note_view();
			selected_notes.insert(note_view.index);
			selected_boxes.insert(note_itr - channel->begin());
		}
	}
	if (selected_notes.size() == 0) {
		return false;
	}

	song.set_volume(selected_channel(), selected_notes, selected_boxes, *view, volume);
	set_active_channel_timeline(song);
	set_active_channel_selection(selected_boxes);

	return true;
}

bool Piano_Roll::set_fade(Song &song, int32_t fade) {
	auto channel = _piano_timeline.active_channel_boxes();
	if (!channel) return false;

	auto view = active_channel_view();

	std::set<int32_t> selected_notes;
	std::set<int32_t> selected_boxes;

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			Note_View note_view = note->note_view();
			selected_notes.insert(note_view.index);
			selected_boxes.insert(note_itr - channel->begin());
		}
	}
	if (selected_notes.size() == 0) {
		return false;
	}

	song.set_fade(selected_channel(), selected_notes, selected_boxes, *view, fade);
	set_active_channel_timeline(song);
	set_active_channel_selection(selected_boxes);

	return true;
}

bool Piano_Roll::set_vibrato_delay(Song &song, int32_t delay) {
	auto channel = _piano_timeline.active_channel_boxes();
	if (!channel) return false;

	auto view = active_channel_view();

	std::set<int32_t> selected_notes;
	std::set<int32_t> selected_boxes;

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			Note_View note_view = note->note_view();
			selected_notes.insert(note_view.index);
			selected_boxes.insert(note_itr - channel->begin());
		}
	}
	if (selected_notes.size() == 0) {
		return false;
	}

	song.set_vibrato_delay(selected_channel(), selected_notes, selected_boxes, *view, delay);
	set_active_channel_timeline(song);
	set_active_channel_selection(selected_boxes);

	return true;
}

bool Piano_Roll::set_vibrato_extent(Song &song, int32_t extent) {
	auto channel = _piano_timeline.active_channel_boxes();
	if (!channel) return false;

	auto view = active_channel_view();

	std::set<int32_t> selected_notes;
	std::set<int32_t> selected_boxes;

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			Note_View note_view = note->note_view();
			selected_notes.insert(note_view.index);
			selected_boxes.insert(note_itr - channel->begin());
		}
	}
	if (selected_notes.size() == 0) {
		return false;
	}

	song.set_vibrato_extent(selected_channel(), selected_notes, selected_boxes, *view, extent);
	set_active_channel_timeline(song);
	set_active_channel_selection(selected_boxes);

	return true;
}

bool Piano_Roll::set_vibrato_rate(Song &song, int32_t rate) {
	auto channel = _piano_timeline.active_channel_boxes();
	if (!channel) return false;

	auto view = active_channel_view();

	std::set<int32_t> selected_notes;
	std::set<int32_t> selected_boxes;

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			Note_View note_view = note->note_view();
			selected_notes.insert(note_view.index);
			selected_boxes.insert(note_itr - channel->begin());
		}
	}
	if (selected_notes.size() == 0) {
		return false;
	}

	song.set_vibrato_rate(selected_channel(), selected_notes, selected_boxes, *view, rate);
	set_active_channel_timeline(song);
	set_active_channel_selection(selected_boxes);

	return true;
}

bool Piano_Roll::set_wave(Song &song, int32_t wave) {
	auto channel = _piano_timeline.active_channel_boxes();
	if (!channel) return false;

	auto view = active_channel_view();

	std::set<int32_t> selected_notes;
	std::set<int32_t> selected_boxes;

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			Note_View note_view = note->note_view();
			selected_notes.insert(note_view.index);
			selected_boxes.insert(note_itr - channel->begin());
		}
	}
	if (selected_notes.size() == 0) {
		return false;
	}

	song.set_wave(selected_channel(), selected_notes, selected_boxes, *view, wave);
	set_active_channel_timeline(song);
	set_active_channel_selection(selected_boxes);

	return true;
}

bool Piano_Roll::set_drumkit(Song &song, int32_t drumkit) {
	auto channel = _piano_timeline.active_channel_boxes();
	if (!channel) return false;

	std::set<int32_t> selected_notes;
	std::set<int32_t> selected_boxes;

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			Note_View note_view = note->note_view();
			selected_notes.insert(note_view.index);
			selected_boxes.insert(note_itr - channel->begin());
		}
	}
	if (selected_notes.size() == 0) {
		return false;
	}

	song.set_drumkit(selected_channel(), selected_notes, selected_boxes, drumkit);
	set_active_channel_timeline(song);
	set_active_channel_selection(selected_boxes);

	return true;
}

bool Piano_Roll::set_duty(Song &song, int32_t duty) {
	auto channel = _piano_timeline.active_channel_boxes();
	if (!channel) return false;

	std::set<int32_t> selected_notes;
	std::set<int32_t> selected_boxes;

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			Note_View note_view = note->note_view();
			selected_notes.insert(note_view.index);
			selected_boxes.insert(note_itr - channel->begin());
		}
	}
	if (selected_notes.size() == 0) {
		return false;
	}

	song.set_duty(selected_channel(), selected_notes, selected_boxes, duty);
	set_active_channel_timeline(song);
	set_active_channel_selection(selected_boxes);

	return true;
}

bool Piano_Roll::set_tempo(Song &song, int32_t tempo) {
	auto channel = _piano_timeline.active_channel_boxes();
	if (!channel) return false;

	std::set<int32_t> selected_notes;
	std::set<int32_t> selected_boxes;

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			Note_View note_view = note->note_view();
			selected_notes.insert(note_view.index);
			selected_boxes.insert(note_itr - channel->begin());
		}
	}
	if (selected_notes.size() == 0) {
		return false;
	}

	song.set_tempo(selected_channel(), selected_notes, selected_boxes, tempo);
	set_active_channel_timeline(song);
	set_active_channel_selection(selected_boxes);

	return true;
}

bool Piano_Roll::set_transpose_octaves(Song &song, int32_t octaves) {
	auto channel = _piano_timeline.active_channel_boxes();
	if (!channel) return false;

	auto view = active_channel_view();

	std::set<int32_t> selected_notes;
	std::set<int32_t> selected_boxes;

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			Note_View note_view = note->note_view();
			selected_notes.insert(note_view.index);
			selected_boxes.insert(note_itr - channel->begin());
		}
	}
	if (selected_notes.size() == 0) {
		return false;
	}

	song.set_transpose_octaves(selected_channel(), selected_notes, selected_boxes, *view, octaves);
	set_active_channel_timeline(song);
	set_active_channel_selection(selected_boxes);

	return true;
}

bool Piano_Roll::set_transpose_pitches(Song &song, int32_t pitches) {
	auto channel = _piano_timeline.active_channel_boxes();
	if (!channel) return false;

	auto view = active_channel_view();

	std::set<int32_t> selected_notes;
	std::set<int32_t> selected_boxes;

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			Note_View note_view = note->note_view();
			selected_notes.insert(note_view.index);
			selected_boxes.insert(note_itr - channel->begin());
		}
	}
	if (selected_notes.size() == 0) {
		return false;
	}

	song.set_transpose_pitches(selected_channel(), selected_notes, selected_boxes, *view, pitches);
	set_active_channel_timeline(song);
	set_active_channel_selection(selected_boxes);

	return true;
}

bool Piano_Roll::set_slide_duration(Song &song, int32_t duration) {
	auto channel = _piano_timeline.active_channel_boxes();
	if (!channel) return false;

	auto view = active_channel_view();

	std::set<int32_t> selected_notes;
	std::set<int32_t> selected_boxes;

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			Note_View note_view = note->note_view();
			selected_notes.insert(note_view.index);
			selected_boxes.insert(note_itr - channel->begin());
		}
	}
	if (selected_notes.size() == 0) {
		return false;
	}

	song.set_slide_duration(selected_channel(), selected_notes, selected_boxes, *view, duration);
	set_active_channel_timeline(song);
	set_active_channel_selection(selected_boxes);

	return true;
}

bool Piano_Roll::set_slide_octave(Song &song, int32_t octave) {
	auto channel = _piano_timeline.active_channel_boxes();
	if (!channel) return false;

	auto view = active_channel_view();

	std::set<int32_t> selected_notes;
	std::set<int32_t> selected_boxes;

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			Note_View note_view = note->note_view();
			selected_notes.insert(note_view.index);
			selected_boxes.insert(note_itr - channel->begin());
		}
	}
	if (selected_notes.size() == 0) {
		return false;
	}

	song.set_slide_octave(selected_channel(), selected_notes, selected_boxes, *view, octave);
	set_active_channel_timeline(song);
	set_active_channel_selection(selected_boxes);

	return true;
}

bool Piano_Roll::set_slide_pitch(Song &song, Pitch pitch) {
	auto channel = _piano_timeline.active_channel_boxes();
	if (!channel) return false;

	auto view = active_channel_view();

	std::set<int32_t> selected_notes;
	std::set<int32_t> selected_boxes;

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			Note_View note_view = note->note_view();
			selected_notes.insert(note_view.index);
			selected_boxes.insert(note_itr - channel->begin());
		}
	}
	if (selected_notes.size() == 0) {
		return false;
	}

	song.set_slide_pitch(selected_channel(), selected_notes, selected_boxes, *view, pitch);
	set_active_channel_timeline(song);
	set_active_channel_selection(selected_boxes);

	return true;
}

bool Piano_Roll::set_slide(Song &song, int32_t duration, int32_t octave, Pitch pitch) {
	auto channel = _piano_timeline.active_channel_boxes();
	if (!channel) return false;

	std::set<int32_t> selected_notes;
	std::set<int32_t> selected_boxes;

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			Note_View note_view = note->note_view();
			selected_notes.insert(note_view.index);
			selected_boxes.insert(note_itr - channel->begin());
		}
	}
	if (selected_notes.size() == 0) {
		return false;
	}

	song.set_slide(selected_channel(), selected_notes, selected_boxes, duration, octave, pitch);
	set_active_channel_timeline(song);
	set_active_channel_selection(selected_boxes);

	return true;
}

bool Piano_Roll::pitch_up(Song &song, bool dry_run) {
	auto channel = _piano_timeline.active_channel_boxes();
	if (!channel) return false;

	auto view = active_channel_view();

	std::set<int32_t> selected_notes;
	std::set<int32_t> selected_boxes;

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			Note_View note_view = note->note_view();
			if (note_view.octave == 8 && note_view.pitch == Pitch::B_NAT) {
				return false;
			}
			selected_notes.insert(note_view.index);
			selected_boxes.insert(note_itr - channel->begin());
		}
	}
	if (selected_notes.size() == 0) {
		return false;
	}
	if (dry_run) {
		return true;
	}

	song.pitch_up(selected_channel(), selected_notes, selected_boxes, *view);
	set_active_channel_timeline(song);
	set_active_channel_selection(selected_boxes);

	return true;
}

bool Piano_Roll::pitch_down(Song &song, bool dry_run) {
	auto channel = _piano_timeline.active_channel_boxes();
	if (!channel) return false;

	auto view = active_channel_view();

	std::set<int32_t> selected_notes;
	std::set<int32_t> selected_boxes;

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			Note_View note_view = note->note_view();
			if (note_view.octave == 1 && note_view.pitch == Pitch::C_NAT) {
				return false;
			}
			selected_notes.insert(note_view.index);
			selected_boxes.insert(note_itr - channel->begin());
		}
	}
	if (selected_notes.size() == 0) {
		return false;
	}
	if (dry_run) {
		return true;
	}

	song.pitch_down(selected_channel(), selected_notes, selected_boxes, *view);
	set_active_channel_timeline(song);
	set_active_channel_selection(selected_boxes);

	return true;
}

bool Piano_Roll::octave_up(Song &song, bool dry_run) {
	auto channel = _piano_timeline.active_channel_boxes();
	if (!channel) return false;

	auto view = active_channel_view();

	std::set<int32_t> selected_notes;
	std::set<int32_t> selected_boxes;

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			Note_View note_view = note->note_view();
			if (note_view.octave == 8) {
				return false;
			}
			selected_notes.insert(note_view.index);
			selected_boxes.insert(note_itr - channel->begin());
		}
	}
	if (selected_notes.size() == 0) {
		return false;
	}
	if (dry_run) {
		return true;
	}

	song.octave_up(selected_channel(), selected_notes, selected_boxes, *view);
	set_active_channel_timeline(song);
	set_active_channel_selection(selected_boxes);

	return true;
}

bool Piano_Roll::octave_down(Song &song, bool dry_run) {
	auto channel = _piano_timeline.active_channel_boxes();
	if (!channel) return false;

	auto view = active_channel_view();

	std::set<int32_t> selected_notes;
	std::set<int32_t> selected_boxes;

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			Note_View note_view = note->note_view();
			if (note_view.octave == 1) {
				return false;
			}
			selected_notes.insert(note_view.index);
			selected_boxes.insert(note_itr - channel->begin());
		}
	}
	if (selected_notes.size() == 0) {
		return false;
	}
	if (dry_run) {
		return true;
	}

	song.octave_down(selected_channel(), selected_notes, selected_boxes, *view);
	set_active_channel_timeline(song);
	set_active_channel_selection(selected_boxes);

	return true;
}

bool Piano_Roll::move_left(Song &song, bool dry_run) {
	auto channel = _piano_timeline.active_channel_boxes();
	if (!channel) return false;

	auto view = active_channel_view();

	std::set<int32_t> selected_notes;
	std::set<int32_t> selected_boxes;

	const std::vector<Command> &commands = song.channel_commands(selected_channel());

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			Note_View note_view = note->note_view();
			auto command_itr = commands.rbegin() + (commands.size() - 1 - note_view.index);

			const auto is_preceded_by_rest = [&](decltype(command_itr) itr) {
				++itr;
				while (itr != commands.rend()) {
					if (
						is_note_command(itr->type) &&
						selected_notes.count((itr.base() - 1) - commands.begin()) > 0
					) {
						return true;
					}
					if (itr->type == Command_Type::REST) {
						return true;
					}
					if (
						itr->labels.size() > 0 ||
						is_note_command(itr->type) ||
						is_global_command(itr->type) ||
						is_control_command(itr->type)
					) {
						return false;
					}
					++itr;
				}
				return false;
			};

			if (command_itr->labels.size() > 0 || !is_preceded_by_rest(command_itr)) {
				return false;
			}
			selected_notes.insert(note_view.index);
			selected_boxes.insert(note_itr - channel->begin());
		}
	}
	if (selected_notes.size() == 0) {
		return false;
	}
	if (dry_run) {
		return true;
	}

	song.move_left(selected_channel(), selected_notes, selected_boxes, *view);
	set_active_channel_timeline(song);
	set_active_channel_selection(selected_boxes);

	align_cursor();

	return true;
}

bool Piano_Roll::move_right(Song &song, bool dry_run) {
	auto channel = _piano_timeline.active_channel_boxes();
	if (!channel) return false;

	auto view = active_channel_view();

	std::set<int32_t> selected_notes;
	std::set<int32_t> selected_boxes;

	const std::vector<Command> &commands = song.channel_commands(selected_channel());

	for (auto note_itr = channel->rbegin(); note_itr != channel->rend(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			Note_View note_view = note->note_view();
			auto command_itr = commands.begin() + note_view.index;

			const auto is_followed_by_rest = [&](decltype(command_itr) itr) {
				++itr;
				while (itr != commands.end()) {
					if (
						is_note_command(itr->type) &&
						itr->labels.size() == 0 &&
						selected_notes.count(itr - commands.begin()) > 0
					) {
						return true;
					}
					if (
						itr->labels.size() > 0 ||
						is_note_command(itr->type) ||
						is_global_command(itr->type) ||
						is_control_command(itr->type)
					) {
						return false;
					}
					if (itr->type == Command_Type::REST) {
						return true;
					}
					++itr;
				}
				return false;
			};

			if (!is_followed_by_rest(command_itr)) {
				return false;
			}
			selected_notes.insert(note_view.index);
			selected_boxes.insert((note_itr.base() - 1) - channel->begin());
		}
	}
	if (selected_notes.size() == 0) {
		return false;
	}
	if (dry_run) {
		return true;
	}

	song.move_right(selected_channel(), selected_notes, selected_boxes, *view);
	set_active_channel_timeline(song);
	set_active_channel_selection(selected_boxes);

	align_cursor();

	return true;
}

bool Piano_Roll::shorten(Song &song, bool dry_run) {
	auto channel = _piano_timeline.active_channel_boxes();
	if (!channel) return false;

	std::set<int32_t> selected_notes;
	std::set<int32_t> selected_boxes;

	int32_t tick_adjustment = 0;

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			Note_View note_view = note->note_view();
			if (note_view.length == 1) {
				return false;
			}
			selected_notes.insert(note_view.index);
			selected_boxes.insert(note_itr - channel->begin());
			if (note->tick() + note_view.length * note_view.speed == _tick) {
				tick_adjustment = -note_view.speed;
			}
		}
	}
	if (selected_notes.size() == 0) {
		return false;
	}
	if (dry_run) {
		return true;
	}

	song.shorten(selected_channel(), selected_notes, selected_boxes, tick_adjustment ? _tick : -1);
	set_active_channel_timeline(song);
	set_active_channel_selection(selected_boxes);

	if (tick_adjustment) {
		_tick += tick_adjustment;
		focus_cursor(true);
		parent()->set_song_position(_tick);
	}

	return true;
}

bool Piano_Roll::lengthen(Song &song, bool dry_run) {
	auto channel = _piano_timeline.active_channel_boxes();
	if (!channel) return false;

	auto view = active_channel_view();

	std::set<int32_t> selected_notes;
	std::set<int32_t> selected_boxes;

	int32_t tick_adjustment = 0;

	const std::vector<Command> &commands = song.channel_commands(selected_channel());

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			Note_View note_view = note->note_view();
			auto command_itr = commands.begin() + note_view.index;

			if (
				note_view.length == 16 ||
				!is_followed_by_n_ticks_of_rest(command_itr, commands.end(), note_view.speed, note_view.speed)
			) {
				return false;
			}
			selected_notes.insert(note_view.index);
			selected_boxes.insert(note_itr - channel->begin());
			if (note->tick() + note_view.length * note_view.speed == _tick) {
				tick_adjustment = note_view.speed;
			}
		}
	}
	if (selected_notes.size() == 0) {
		return false;
	}
	if (dry_run) {
		return true;
	}

	song.lengthen(selected_channel(), selected_notes, selected_boxes, *view, tick_adjustment ? _tick : -1);
	set_active_channel_timeline(song);
	set_active_channel_selection(selected_boxes);

	if (tick_adjustment) {
		_tick += tick_adjustment;
		focus_cursor(true);
		parent()->set_song_position(_tick);
	}
	align_cursor();

	return true;
}

bool Piano_Roll::delete_selection(Song &song, bool dry_run) {
	auto channel = _piano_timeline.active_channel_boxes();
	if (!channel) return false;

	std::set<int32_t> selected_notes;
	std::set<int32_t> selected_boxes;

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			Note_View note_view = note->note_view();
			selected_notes.insert(note_view.index);
			selected_boxes.insert(note_itr - channel->begin());
		}
	}
	if (selected_notes.size() == 0) {
		return false;
	}
	if (dry_run) {
		return true;
	}

	song.delete_selection(selected_channel(), selected_notes, selected_boxes);
	set_active_channel_timeline(song);
	refresh_note_properties();

	return true;
}

bool Piano_Roll::snip_selection(Song &song, bool dry_run) {
	auto channel = _piano_timeline.active_channel_boxes();
	if (!channel) return false;

	std::set<int32_t> selected_notes;
	std::set<int32_t> selected_boxes;

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			Note_View note_view = note->note_view();
			selected_notes.insert(note_view.index);
			selected_boxes.insert(note_itr - channel->begin());
		}
	}
	if (selected_notes.size() == 0) {
		return false;
	}
	if (dry_run) {
		return true;
	}

	song.snip_selection(selected_channel(), selected_notes, selected_boxes);
	set_active_channel_timeline(song);
	refresh_note_properties();

	align_cursor();

	return true;
}

bool Piano_Roll::split_note(Song &song) {
	auto channel = _piano_timeline.active_channel_boxes();
	if (!channel) return false;

	auto view = active_channel_view();

	if (_tick == -1) return false;

	int32_t tick_offset = 0;

	const Note_View *note_view = find_note_view_at_tick(*view, _tick, &tick_offset);

	if (
		!note_view ||
		note_view->ghost ||
		note_view->pitch == Pitch::REST ||
		tick_offset == 0
	) {
		return false;
	}

	int32_t index = note_view->index;
	int32_t speed = note_view->speed;

	std::set<int32_t> selected_boxes;

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			selected_boxes.insert(note_itr - channel->begin());
		}
	}

	song.split_note(selected_channel(), selected_boxes, index, _tick, tick_offset / speed);
	set_active_channel_timeline(song);
	_piano_timeline.select_note_at_tick(*channel, _tick);

	focus_cursor(true);

	return true;
}

bool Piano_Roll::glue_note(Song &song) {
	auto channel = _piano_timeline.active_channel_boxes();
	if (!channel) return false;

	auto view = active_channel_view();
	const std::vector<Command> &commands = song.channel_commands(selected_channel());

	if (_tick == -1) return false;

	int32_t tick_offset = 0;

	const Note_View *note_view = find_note_view_at_tick(*view, _tick, &tick_offset);
	const Note_View *prev_note = find_note_view_at_tick(*view, _tick - 1);

	if (
		!note_view ||
		!prev_note ||
		note_view->ghost ||
		note_view->pitch == Pitch::REST ||
		note_view->pitch != prev_note->pitch ||
		note_view->octave != prev_note->octave ||
		note_view->speed != prev_note->speed ||
		note_view->index != prev_note->index + 1 ||
		note_view->length + prev_note->length > 16 ||
		commands[note_view->index].labels.size() > 0 ||
		tick_offset != 0
	) {
		return false;
	}

	int32_t index = note_view->index;

	std::set<int32_t> selected_boxes;

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			selected_boxes.insert(note_itr - channel->begin());
		}
	}

	song.glue_note(selected_channel(), selected_boxes, index, _tick);
	set_active_channel_timeline(song);
	_piano_timeline.select_note_at_tick(*channel, _tick);

	focus_cursor(true);

	return true;
}

bool Piano_Roll::resize_song(Song &song, const Song_Options_Dialog::Song_Options &options) {
	song.resize_song(options);
	clear();
	set_timeline(song);
	update_channel_detail(selected_channel());

	return true;
}

bool Piano_Roll::is_point_in_loop(int X, int Y) {
	auto loops = _piano_timeline.active_channel_loops();
	if (!loops) return false;

	for (const Loop_Box *loop : *loops) {
		if (X >= loop->x() && X < loop->x() + loop->w() && Y >= loop->y() && Y < loop->y() + loop->h()) {
			return true;
		}
	}

	return false;
}

bool Piano_Roll::is_point_in_call(int X, int Y) {
	auto calls = _piano_timeline.active_channel_calls();
	if (!calls) return false;

	for (const Call_Box *call : *calls) {
		if (X >= call->x() && X < call->x() + call->w() && Y >= call->y() && Y < call->y() + call->h()) {
			return true;
		}
	}

	return false;
}

bool Piano_Roll::reduce_loop(Song &song, bool dry_run) {
	auto loops = _piano_timeline.active_channel_loops();
	if (!loops) return false;

	int32_t loop_index = -1;
	int32_t loop_length = 0;
	Note_View start_view, end_view;
	for (size_t i = 0; i < loops->size(); ++i) {
		const Loop_Box *loop = (*loops)[i];
		if (_tick >= loop->start_tick() && _tick < loop->end_tick()) {
			while (i + 1 < loops->size() && (*loops)[i + 1]->end_note_view().index == loop->end_note_view().index) {
				i += 1;
				loop = (*loops)[i];
			}
			loop_index = loop->end_note_view().index;
			loop_length = loop->end_tick() - loop->start_tick();
			start_view = loop->start_note_view();
			end_view = loop->end_note_view();
			break;
		}
	}
	if (loop_index == -1) return false;
	if (dry_run) {
		return true;
	}

	auto channel = _piano_timeline.active_channel_boxes();

	std::set<int32_t> selected_boxes;

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			selected_boxes.insert(note_itr - channel->begin());
		}
	}

	song.reduce_loop(selected_channel(), selected_boxes, _tick, loop_index, loop_length, start_view, end_view);
	set_active_channel_timeline(song);
	refresh_note_properties();

	return true;
}

bool Piano_Roll::extend_loop(Song &song, bool dry_run) {
	auto loops = _piano_timeline.active_channel_loops();
	if (!loops) return false;

	const std::vector<Command> &commands = song.channel_commands(selected_channel());

	int32_t loop_index = -1;
	int32_t loop_length = 0;
	Note_View start_view, end_view;
	for (size_t i = 0; i < loops->size(); ++i) {
		const Loop_Box *loop = (*loops)[i];
		if (_tick >= loop->start_tick() && _tick < loop->end_tick()) {
			while (i + 1 < loops->size() && (*loops)[i + 1]->end_note_view().index == loop->end_note_view().index) {
				i += 1;
				loop = (*loops)[i];
			}
			loop_index = loop->end_note_view().index;
			loop_length = loop->end_tick() - loop->start_tick();
			start_view = loop->start_note_view();
			end_view = loop->end_note_view();
			break;
		}
	}
	if (
		loop_index == -1 ||
		!is_followed_by_n_ticks_of_rest(commands.begin() + loop_index, commands.end(), loop_length, end_view.speed)
	) {
		return false;
	}
	if (dry_run) {
		return true;
	}

	auto channel = _piano_timeline.active_channel_boxes();

	std::set<int32_t> selected_boxes;

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			selected_boxes.insert(note_itr - channel->begin());
		}
	}

	song.extend_loop(selected_channel(), selected_boxes, _tick, loop_index, loop_length, start_view, end_view);
	set_active_channel_timeline(song);
	refresh_note_properties();

	return true;
}

bool Piano_Roll::unroll_loop(Song &song, bool dry_run) {
	auto loops = _piano_timeline.active_channel_loops();
	if (!loops) return false;

	const std::vector<Command> &commands = song.channel_commands(selected_channel());

	int32_t loop_index = -1;
	Note_View start_view, end_view;
	for (size_t i = 0; i < loops->size(); ++i) {
		const Loop_Box *loop = (*loops)[i];
		if (_tick >= loop->start_tick() && _tick < loop->end_tick()) {
			while (i + 1 < loops->size() && (*loops)[i + 1]->end_note_view().index == loop->end_note_view().index) {
				i += 1;
				loop = (*loops)[i];
			}
			loop_index = loop->end_note_view().index;
			start_view = loop->start_note_view();
			end_view = loop->end_note_view();
			break;
		}
	}
	if (loop_index == -1) return false;

	std::vector<Command> snippet = copy_snippet(commands, start_view.index, end_view.index);
	for (Command &command : snippet) {
		command.labels.clear();
	}

	if (dry_run) {
		return true;
	}

	auto channel = _piano_timeline.active_channel_boxes();

	std::set<int32_t> selected_boxes;

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			selected_boxes.insert(note_itr - channel->begin());
		}
	}

	song.unroll_loop(selected_channel(), selected_boxes, _tick, loop_index, snippet);
	set_active_channel_timeline(song);
	set_active_channel_selection(selected_boxes);

	return true;
}

bool Piano_Roll::create_loop(Song &song, bool dry_run) {
	auto channel = _piano_timeline.active_channel_boxes();
	if (!channel) return false;

	const auto find_selection_start = [](std::vector<Note_Box *> *channel) {
		for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
			if ((*note_itr)->selected()) {
				return note_itr;
			}
		}
		return channel->end();
	};
	const auto find_selection_end = [](std::vector<Note_Box *> *channel) {
		for (auto note_itr = channel->rbegin(); note_itr != channel->rend(); ++note_itr) {
			if ((*note_itr)->selected()) {
				return note_itr.base() - 1;
			}
		}
		return channel->end();
	};

	const auto selection_start = find_selection_start(channel);
	const auto selection_end = find_selection_end(channel);

	if (selection_start == channel->end()) return false;

	Note_View start_view = (*selection_start)->note_view();
	Note_View end_view = (*selection_end)->note_view();

	int32_t t_left = (*selection_start)->tick();
	int32_t t_right = (*selection_end)->tick() + end_view.length * end_view.speed;

	auto loops = _piano_timeline.active_channel_loops();
	for (const Loop_Box *loop : *loops) {
		if (!(loop->end_tick() <= t_left || loop->start_tick() >= t_right)) {
			return false;
		}
	}

	int32_t start_index = start_view.index;
	int32_t end_index = end_view.index;

	auto calls = _piano_timeline.active_channel_calls();
	for (const Call_Box *call : *calls) {
		if (call->start_tick() == t_left && call->end_tick() <= t_right) {
			start_index = call->start_note_view().index;
			start_view = call->start_note_view();
		}
		if (call->end_tick() == t_right && call->start_tick() >= t_left) {
			end_index = call->start_note_view().index;
			end_view = call->end_note_view();
		}
	}

	for (Note_Box *other : *channel) {
		if (other->note_view().index == end_view.index && other->note_view().speed != end_view.speed) {
			return false;
		}
	}

	if (
		selected_channel() == 4 &&
		start_view.drumkit != end_view.drumkit &&
		(start_view.drumkit == -1 || end_view.drumkit == -1)
	) {
		return false;
	}

	const std::vector<Command> &commands = song.channel_commands(selected_channel());

	auto start_itr = commands.begin() + start_index;
	auto end_itr = commands.begin() + end_index;

	auto command_itr = start_itr;

	while (command_itr != end_itr) {
		if (
			command_itr == commands.end() ||
			command_itr->type == Command_Type::SOUND_JUMP ||
			command_itr->type == Command_Type::SOUND_LOOP ||
			command_itr->type == Command_Type::SOUND_RET
		) {
			return false;
		}
		++command_itr;
	}

	int32_t loop_length = calc_snippet_length(commands, start_itr, end_itr + 1, end_view);
	if (!is_followed_by_n_ticks_of_rest(end_itr, commands.end(), loop_length, end_view.speed)) {
		return false;
	}

	if (dry_run) {
		return true;
	}

	std::set<int32_t> selected_boxes;

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			selected_boxes.insert(note_itr - channel->begin());
		}
	}

	song.create_loop(selected_channel(), selected_boxes, t_left, start_index, end_index, loop_length, start_view, end_view);
	set_active_channel_timeline(song);
	refresh_note_properties();
	
	_tick = t_left;
	focus_cursor(true);
	parent()->set_song_position(_tick);

	return true;
}

bool Piano_Roll::delete_call(Song &song, bool dry_run) {
	auto calls = _piano_timeline.active_channel_calls();
	if (!calls) return false;

	int32_t call_index = -1;
	int32_t ambiguous_ticks = 0;
	int32_t unambiguous_ticks = 0;
	Note_View start_view, end_view;
	for (const Call_Box *call : *calls) {
		if (_tick >= call->start_tick() && _tick < call->end_tick()) {
			call_index = call->start_note_view().index;
			ambiguous_ticks = call->ambiguous_ticks();
			unambiguous_ticks = call->unambiguous_ticks();
			start_view = call->start_note_view();
			end_view = call->end_note_view();
			break;
		}
	}
	if (call_index == -1) return false;
	if (dry_run) {
		return true;
	}

	auto channel = _piano_timeline.active_channel_boxes();

	std::set<int32_t> selected_boxes;

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			selected_boxes.insert(note_itr - channel->begin());
		}
	}

	song.delete_call(selected_channel(), selected_boxes, _tick, call_index, ambiguous_ticks, unambiguous_ticks, start_view, end_view);
	set_active_channel_timeline(song);
	refresh_note_properties();

	return true;
}

bool Piano_Roll::unpack_call(Song &song, bool dry_run) {
	auto calls = _piano_timeline.active_channel_calls();
	if (!calls) return false;

	const std::vector<Command> &commands = song.channel_commands(selected_channel());

	int32_t call_index = -1;
	Note_View start_view, end_view;
	for (const Call_Box *call : *calls) {
		if (_tick >= call->start_tick() && _tick < call->end_tick()) {
			call_index = call->start_note_view().index;
			start_view = call->start_note_view();
			end_view = call->end_note_view();
			break;
		}
	}
	if (call_index == -1) return false;

	assert(commands[call_index].type == Command_Type::SOUND_CALL);
	std::string target_label = commands[call_index].target;

	std::vector<Command> snippet = copy_snippet(commands, find_note_with_label(commands, target_label) - commands.begin(), end_view.index);

	const auto snippet_contains_label = [](const std::vector<Command> &snippet, const std::string &label) {
		for (const Command &command : snippet) {
			if (std::count(RANGE(command.labels), label) > 0) {
				return true;
			}
		}
		return false;
	};

	std::set<std::string> loop_targets;
	for (const Command &command : snippet) {
		if (command.type == Command_Type::SOUND_LOOP) {
			if (!snippet_contains_label(snippet, command.target)) return false;
			loop_targets.insert(command.target);
		}
	}
	for (Command &command : snippet) {
		for (size_t i = command.labels.size() - 1; i < command.labels.size(); --i) {
			if (loop_targets.count(command.labels[i]) == 0) {
				command.labels.erase(command.labels.begin() + i);
			}
		}
	}
	std::string scope = get_scope(commands, call_index);
	int loop_number = 1;
	for (Command &command : snippet) {
		for (size_t i = 0; i < command.labels.size(); ++i) {
			std::string old_label = command.labels[i];
			std::string next_label = get_next_loop_label(commands, scope, loop_number);
			command.labels[i] = next_label;
			for (Command &other : snippet) {
				if (other.type == Command_Type::SOUND_LOOP && other.target == old_label) {
					other.target = next_label;
				}
			}
		}
	}

	if (dry_run) {
		return true;
	}

	auto channel = _piano_timeline.active_channel_boxes();

	std::set<int32_t> selected_boxes;

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			selected_boxes.insert(note_itr - channel->begin());
		}
	}

	song.unpack_call(selected_channel(), selected_boxes, _tick, call_index, snippet);
	set_active_channel_timeline(song);
	set_active_channel_selection(selected_boxes);

	return true;
}

int32_t Piano_Roll::quantize_tick(int32_t tick, bool round) {
	const auto view = active_channel_view();
	if (view && !_following && !_paused) {
		int32_t t_left = 0;
		for (const Note_View &note : *view) {
			int32_t t_right = t_left + note.length * note.speed;
			if (t_right > tick) {
				return t_left + (tick - t_left + (round ? (int32_t)(note.speed / 2.0f - 0.5f) : 0)) / note.speed * note.speed;
			}
			t_left = t_right;
		}
		return active_channel_length();
	}
	return tick / ticks_per_step() * ticks_per_step();
}

const Note_View *Piano_Roll::find_note_view_at_tick(const std::vector<Note_View> &view, int32_t tick, int32_t *tick_offset) {
	int32_t t_left = 0;
	for (const Note_View &note : view) {
		int32_t t_right = t_left + note.length * note.speed;
		if (t_right > tick) {
			if (tick_offset) *tick_offset = tick - t_left;
			return &note;
		}
		t_left = t_right;
	}
	return (const Note_View *)nullptr;
}

const Note_View *Piano_Roll::find_note_before_tick(const std::vector<Note_View> &view, int32_t tick) {
	const Note_View *last_note = nullptr;
	int32_t t_left = 0;
	for (const Note_View &note : view) {
		int32_t t_right = t_left + note.length * note.speed;
		if (t_right > tick) {
			return last_note;
		}
		if (note.pitch != Pitch::REST) {
			last_note = &note;
		}
		t_left = t_right;
	}
	return last_note;
}

std::vector<Note_View> *Piano_Roll::active_channel_view() {
	int active_channel = selected_channel();
	if (active_channel == 1) return &_channel_1_notes;
	if (active_channel == 2) return &_channel_2_notes;
	if (active_channel == 3) return &_channel_3_notes;
	if (active_channel == 4) return &_channel_4_notes;
	return nullptr;
}

int32_t Piano_Roll::active_channel_length() {
	int active_channel = selected_channel();
	if (active_channel == 1) return _channel_1_end_tick;
	if (active_channel == 2) return _channel_2_end_tick;
	if (active_channel == 3) return _channel_3_end_tick;
	if (active_channel == 4) return _channel_4_end_tick;
	return -1;
}

void Piano_Roll::scrollbar_cb(Fl_Scrollbar *sb, void *) {
	Piano_Roll *scroll = (Piano_Roll *)(sb->parent());
	scroll->scroll_to(scroll->xposition(), std::min(sb->value(), scroll->scroll_y_max()));
}

void Piano_Roll::hscrollbar_cb(Fl_Scrollbar *sb, void *) {
	Piano_Roll *scroll = (Piano_Roll *)(sb->parent());
	scroll->scroll_to(std::min(sb->value(), scroll->scroll_x_max()), scroll->yposition());
	scroll->sticky_keys();
	if (scroll->_following) {
		scroll->focus_cursor();
	}
	scroll->redraw();
}
