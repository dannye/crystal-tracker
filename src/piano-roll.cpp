#include <cassert>
#include <map>
#include <set>
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

static auto find_note_with_label(const std::vector<Command> &commands, std::string label) {
	for (auto command_itr = commands.begin(); command_itr != commands.end(); ++command_itr) {
		if (command_itr->labels.count(label) > 0) {
			return command_itr;
		}
	}
	assert(false);
	return commands.end();
}

static void calc_channel_length(const std::vector<Command> &commands, int32_t &loop_tick, int32_t &end_tick) {
	int32_t tick = 0;
	loop_tick = -1;
	end_tick = -1;
	int32_t speed = 12;
	auto command_itr = commands.begin();
	std::stack<std::pair<decltype(command_itr), int32_t>> loop_stack;
	std::stack<decltype(command_itr)> call_stack;
	std::map<std::string, int32_t> label_positions;
	while (command_itr != commands.end()) {
		for (const std::string &label : command_itr->labels) {
			label_positions.insert({ label, tick });
		}
		if (command_itr->type == Command_Type::NOTE) {
			tick += command_itr->note.length * speed;
		}
		else if (command_itr->type == Command_Type::DRUM_NOTE) {
			tick += command_itr->drum_note.length * speed;
		}
		else if (command_itr->type == Command_Type::REST) {
			tick += command_itr->rest.length * speed;
		}
		else if (command_itr->type == Command_Type::NOTE_TYPE) {
			speed = command_itr->note_type.speed;
		}
		else if (command_itr->type == Command_Type::DRUM_SPEED) {
			speed = command_itr->drum_speed.speed;
		}
		else if (command_itr->type == Command_Type::SOUND_JUMP) {
			command_itr = find_note_with_label(commands, command_itr->target);
			continue;
		}
		else if (command_itr->type == Command_Type::SOUND_LOOP) {
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
					if (!label_positions.count(command_itr->target)) {
						command_itr = find_note_with_label(commands, command_itr->target);
						continue;
					}
					loop_tick = label_positions.at(command_itr->target);
					end_tick = tick;
					break; // song is finished
				}
				else if (command_itr->sound_loop.loop_count > 1) {
					loop_stack.emplace(command_itr, command_itr->sound_loop.loop_count - 1);
					command_itr = find_note_with_label(commands, command_itr->target);
					continue;
				}
			}
		}
		else if (command_itr->type == Command_Type::SOUND_CALL) {
			call_stack.push(command_itr);
			command_itr = find_note_with_label(commands, command_itr->target);
			continue;
		}
		else if (command_itr->type == Command_Type::SOUND_RET) {
			if (call_stack.size() == 0) {
				end_tick = tick;
				break; // song is finished
			}
			else {
				command_itr = call_stack.top();
				call_stack.pop();
			}
		}
		++command_itr;
	}
}

void Note_Box::draw() {
	draw_box();
	if (_ghost) {
		draw_box(FL_BORDER_FRAME, NOTE_GHOST);
		if (box() != FL_BORDER_FRAME && x() + w() > 1) {
			draw_box(FL_BORDER_FRAME, x() + 1, y() + 1, w() - 2, h() - 2, NOTE_GHOST);
			draw_box(FL_BORDER_FRAME, x() + 2, y() + 2, w() - 4, h() - 4, NOTE_GHOST);
		}
	}
	else if (_selected) {
		draw_box(FL_BORDER_FRAME, FL_FOREGROUND_COLOR);
		if (box() != FL_BORDER_FRAME && x() + w() > 1) {
			draw_box(FL_BORDER_FRAME, x() + 1, y() + 1, w() - 2, h() - 2, FL_WHITE);
			draw_box(FL_BORDER_FRAME, x() + 2, y() + 2, w() - 4, h() - 4, FL_WHITE);
		}
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

void White_Key_Box::draw() {
	draw_box();
	draw_label(x() + BLACK_KEY_WIDTH, y(), w() - BLACK_KEY_WIDTH, h());
}

Piano_Keys::Piano_Keys(int x, int y, int w, int h, const char *l) : Fl_Group(x, y, w, h, l) {
	for (size_t _y = 0; _y < NUM_OCTAVES; ++_y) {
		for (size_t _x = 0; _x < NUM_NOTES_PER_OCTAVE; ++_x) {
			size_t i = _y * NUM_NOTES_PER_OCTAVE + _x;
			if (NOTE_KEYS[_x].white) {
				_notes[i] = new White_Key_Box(x, y, 0, 0, NOTE_KEYS[_x].label);
				_notes[i]->box(FL_BORDER_BOX);
				_notes[i]->color(FL_BACKGROUND2_COLOR);
				_notes[i]->labelcolor(FL_FOREGROUND_COLOR);
			}
			else {
				_notes[i] = new Fl_Box(x, y, 0, 0, NOTE_KEYS[_x].label);
				_notes[i]->box(FL_BORDER_BOX);
				_notes[i]->color(FL_FOREGROUND_COLOR);
				_notes[i]->labelcolor(FL_BACKGROUND2_COLOR);
			}
		}
	}
	end();
	calc_sizes();
}

Piano_Keys::~Piano_Keys() noexcept {
	for (size_t i = 0; i < NUM_NOTES_PER_OCTAVE * NUM_OCTAVES; ++i) {
		if (_notes[i]) {
			delete _notes[i];
			_notes[i] = nullptr;
		}
	}
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

	int y_top = _notes[0]->y();

	for (size_t _y = 0; _y < NUM_OCTAVES; ++_y) {
		int y_pos = octave_height * _y;
		for (size_t _x = 0; _x < NUM_NOTES_PER_OCTAVE; ++_x) {
			size_t i = _y * NUM_NOTES_PER_OCTAVE + _x;
			int delta = zoomed ? NOTE_KEYS[_x].delta1 : NOTE_KEYS[_x].delta2;
			if (NOTE_KEYS[_x].white) {
				_notes[i]->resize(
					_notes[i]->x(),
					y_top + y_pos + NOTE_KEYS[_x].y * white_key_height + white_delta,
					WHITE_KEY_WIDTH,
					white_key_height + delta
				);
				white_delta += delta;
			}
			else {
				_notes[i]->resize(
					_notes[i]->x(),
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

void Piano_Keys::highlight_key(Pitch pitch, int32_t octave, Fl_Color color) {
	size_t _y = NUM_OCTAVES - octave;
	size_t _x = PITCH_TO_KEY_INDEX[(size_t)pitch - 1];
	size_t i = _y * NUM_NOTES_PER_OCTAVE + _x;
	_notes[i]->color(color);
}

void Piano_Keys::reset_key_colors() {
	for (size_t _y = 0; _y < NUM_OCTAVES; ++_y) {
		for (size_t _x = 0; _x < NUM_NOTES_PER_OCTAVE; ++_x) {
			size_t i = _y * NUM_NOTES_PER_OCTAVE + _x;
			if (NOTE_KEYS[_x].white) {
				_notes[i]->color(FL_BACKGROUND2_COLOR);
			}
			else {
				_notes[i]->color(FL_FOREGROUND_COLOR);
			}
		}
	}
}

Piano_Timeline::Piano_Timeline(int x, int y, int w, int h, const char *l) : Fl_Group(x, y, w, h, l) {
	_keys = new Piano_Keys(x, y, WHITE_KEY_WIDTH, NUM_OCTAVES * parent()->octave_height());
	end();
}

Piano_Timeline::~Piano_Timeline() noexcept {
	clear();
	if (_keys) {
		delete _keys;
		_keys = nullptr;
	}
}

void Piano_Timeline::clear() {
	clear_channel_1();
	clear_channel_2();
	clear_channel_3();
	clear_channel_4();
}

void Piano_Timeline::clear_channel_1() {
	for (Note_Box *note : _channel_1_notes) {
		delete note;
	}
	_channel_1_notes.clear();

	for (Loop_Box *loop : _channel_1_loops) {
		delete loop;
	}
	_channel_1_loops.clear();

	for (Call_Box *call : _channel_1_calls) {
		delete call;
	}
	_channel_1_calls.clear();
}

void Piano_Timeline::clear_channel_2() {
	for (Note_Box *note : _channel_2_notes) {
		delete note;
	}
	_channel_2_notes.clear();

	for (Loop_Box *loop : _channel_2_loops) {
		delete loop;
	}
	_channel_2_loops.clear();

	for (Call_Box *call : _channel_2_calls) {
		delete call;
	}
	_channel_2_calls.clear();
}

void Piano_Timeline::clear_channel_3() {
	for (Note_Box *note : _channel_3_notes) {
		delete note;
	}
	_channel_3_notes.clear();

	for (Loop_Box *loop : _channel_3_loops) {
		delete loop;
	}
	_channel_3_loops.clear();

	for (Call_Box *call : _channel_3_calls) {
		delete call;
	}
	_channel_3_calls.clear();
}

void Piano_Timeline::clear_channel_4() {
	for (Note_Box *note : _channel_4_notes) {
		delete note;
	}
	_channel_4_notes.clear();

	for (Loop_Box *loop : _channel_4_loops) {
		delete loop;
	}
	_channel_4_loops.clear();

	for (Call_Box *call : _channel_4_calls) {
		delete call;
	}
	_channel_4_calls.clear();
}

inline int Piano_Timeline::selected_channel() const {
	return parent()->selected_channel();
}

void Piano_Timeline::calc_sizes() {
	const int octave_height = parent()->octave_height();
	const int note_row_height = parent()->note_row_height();
	const int tick_width = parent()->tick_width();

	const auto tick_to_x_pos = [&](int32_t tick) {
		return x() + WHITE_KEY_WIDTH + tick * tick_width;
	};
	const auto pitch_to_y_pos = [&](Pitch pitch, int32_t octave) {
		return y() + (NUM_OCTAVES - octave) * octave_height + (NUM_PITCHES - (size_t)(pitch)) * note_row_height;
	};

	const auto resize_notes = [&](std::vector<Note_Box *> &notes) {
		for (Note_Box *note : notes) {
			note->resize(
				tick_to_x_pos(note->tick()),
				pitch_to_y_pos(note->note_view().pitch, note->note_view().octave),
				note->note_view().length * note->note_view().speed * tick_width,
				note_row_height
			);
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

	redraw();
}

int Piano_Timeline::handle(int event) {
	switch (event) {
	case FL_PUSH:
		if (parent()->handle_mouse_click(event)) {
			return 1;
		}
		if (Fl::event_button() == FL_LEFT_MOUSE) {
			if (handle_note_selection(event)) {
				return 1;
			}
		}
		break;
	case FL_DRAG:
		if (parent()->handle_mouse_click(event)) {
			return 1;
		}
		break;
	}
	return Fl_Group::handle(event);
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
				_keys->redraw();
			}
		}
	}

	bool clicked_note = false;
	for (Note_Box *note : *channel) {
		if (!note->ghost() && Fl::event_inside(note)) {
			note->selected(!note->selected() || !Fl::event_command());
			note->redraw();
			_keys->redraw();
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

	return clicked_note;
}

bool Piano_Timeline::select_all() {
	auto channel = active_channel_boxes();
	if (!channel) return false;

	bool note_selected = false;
	for (Note_Box *note : *channel) {
		if (!note->selected()) {
			note->selected(true);
			note->redraw();
			_keys->redraw();
			note_selected = true;
		}
	}

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
			_keys->redraw();
			note_deselected = true;
		}
	}

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

void Piano_Timeline::highlight_tick(std::vector<Note_Box *> &notes, int32_t tick, bool muted, Fl_Color color) {
	int x_pos = tick * parent()->tick_width() + WHITE_KEY_WIDTH;
	for (Note_Box *note : notes) {
		int note_left = note->x() - x();
		int note_right = note_left + note->w();
		if (note_left > x_pos) {
			return;
		}
		if (note_right > x_pos && !muted) {
			const Note_View &view = note->note_view();
			_keys->highlight_key(view.pitch, view.octave, color);
		}
		note->color(color);
	}
}

void Piano_Timeline::select_note_at_tick(std::vector<Note_Box *> &notes, int32_t tick) {
	int x_pos = tick * parent()->tick_width() + WHITE_KEY_WIDTH;
	for (Note_Box *note : notes) {
		int note_left = note->x() - x();
		int note_right = note_left + note->w();
		if (note_left > x_pos) {
			return;
		}
		if (note_right > x_pos) {
			note->selected(true);
			return;
		}
	}
}

void Piano_Timeline::set_channel(std::vector<Note_Box *> &channel, const std::vector<Note_View> &notes, Fl_Color color) {
	const int octave_height = parent()->octave_height();
	const int note_row_height = parent()->note_row_height();
	const int tick_width = parent()->tick_width();

	const auto tick_to_x_pos = [&](int32_t tick) {
		return x() + WHITE_KEY_WIDTH + tick * tick_width;
	};
	const auto pitch_to_y_pos = [&](Pitch pitch, int32_t octave) {
		return y() + (NUM_OCTAVES - octave) * octave_height + (NUM_PITCHES - (size_t)(pitch)) * note_row_height;
	};

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
				note_row_height
			);
			box->box(FL_BORDER_BOX);
			box->color(color);
			box->ghost(note.ghost);
			channel.push_back(box);
		}
		tick += note.length * note.speed;
	}
	end();

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

void Piano_Timeline::set_channel_detailed(
	std::vector<Note_Box *> &notes,
	std::vector<Loop_Box *> &loops,
	std::vector<Call_Box *> &calls,
	bool detailed
) {
	Fl_Boxtype note_box = detailed ? FL_BORDER_BOX : FL_BORDER_FRAME;
	Fl_Boxtype group_box = detailed ? FL_BORDER_FRAME : FL_NO_BOX;
	for (Note_Box *note : notes) {
		note->box(note_box);
	}
	for (Loop_Box *loop : loops) {
		loop->box(group_box);
	}
	for (Call_Box *call : calls) {
		call->box(group_box);
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

void Piano_Timeline::draw() {
	OS::Theme theme = OS::current_theme();
	bool dark = OS::is_dark_theme(theme);
	bool hc = theme == OS::Theme::HIGH_CONTRAST;
	Fl_Color light_row = hc ? fl_darker(FL_BACKGROUND2_COLOR) : dark ? ALT_LIGHT_ROW : FL_LIGHT1;
	Fl_Color dark_row = hc ? FL_BACKGROUND2_COLOR : dark ? ALT_DARK_ROW : FL_DARK2;
	Fl_Color row_divider = hc ? FL_BACKGROUND2_COLOR : FL_DARK2;
	Fl_Color cursor_color = dark ? FL_YELLOW : FL_MAGENTA;
	if (damage() & ~FL_DAMAGE_CHILD) {
		Piano_Roll *p = parent();
		const int note_row_height = p->note_row_height();
		const int tick_width = p->tick_width();

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
		int time_step_width = tick_width * TICKS_PER_STEP;
		const size_t num_dividers = (w() - WHITE_KEY_WIDTH) / time_step_width + 1;
		for (size_t i = 0; i < num_dividers; ++i) {
			fl_color(FL_DARK3);
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

		int32_t tick = p->tick();
		if (tick != -1 && (parent()->following() || parent()->paused())) {
			tick = tick / TICKS_PER_STEP * TICKS_PER_STEP;
		}
		x_pos = x() + tick * tick_width + WHITE_KEY_WIDTH;
		fl_color(cursor_color);
		fl_yxline(x_pos - 1, y(), y() + h());
		fl_yxline(x_pos, y(), y() + h());
	}
	draw_children();
}

Piano_Roll::Piano_Roll(int x, int y, int w, int h, const char *l) : Fl_Scroll(x, y, w, h, l) {
	type(BOTH_ALWAYS);
	_piano_timeline = new Piano_Timeline(x, y, w - scrollbar.w(), NUM_OCTAVES * octave_height());
	end();

	hscrollbar.callback((Fl_Callback *)hscrollbar_cb);
}

Piano_Roll::~Piano_Roll() noexcept {
	if (_piano_timeline) {
		delete _piano_timeline;
		_piano_timeline = nullptr;
	}
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

int Piano_Roll::handle(int event) {
	switch (event) {
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
			int x_pos = xposition() - (w() - WHITE_KEY_WIDTH * 2);
			if (x_pos < 0) {
				scroll_to(0, yposition());
			}
			else {
				scroll_to(x_pos, yposition());
			}
			sticky_keys();
			redraw();
			return 1;
		}
		else if (Fl::event_key() == FL_Page_Down) {
			int x_pos = xposition() + (w() - WHITE_KEY_WIDTH * 2);
			if (x_pos > scroll_x_max()) {
				scroll_to(scroll_x_max(), yposition());
			}
			else {
				scroll_to(x_pos, yposition());
			}
			sticky_keys();
			redraw();
			return 1;
		}
		else if (Fl::event_key() == FL_Home) {
			scroll_to(0, yposition());
			sticky_keys();
			redraw();
			return 1;
		}
		else if (Fl::event_key() == FL_End) {
			scroll_to(scroll_x_max(), yposition());
			sticky_keys();
			redraw();
			return 1;
		}

		if (Fl::event_key() == FL_Escape) {
			_tick = -1;
			redraw();
		}
		break;
	}
	return Fl_Scroll::handle(event);
}

bool Piano_Roll::handle_mouse_click(int event) {
	if (
		(Fl::event_button() == FL_MIDDLE_MOUSE || (Fl::event_command() && Fl::event_button() == FL_RIGHT_MOUSE)) &&
		(!_following || event == FL_PUSH)
	) {
		int32_t t = (Fl::event_x() - _piano_timeline->x() - WHITE_KEY_WIDTH) / tick_width();
		t = quantize_tick(t);

		if (_tick != t && 0 <= t && t < _song_length) {
			_tick = t;
			parent()->set_song_position(_tick);
			redraw();
		}

		return true;
	}
	else if (_following && event == FL_PUSH) {
		toggle_follow_mode();
		return true;
	}
	return false;
}

void Piano_Roll::zoom(bool z) {
	if (_zoomed == z) return;
	_zoomed = z;

	float scroll_x = scroll_x_max() > 0 ? hscrollbar.value() / (float) scroll_x_max() : 0.0f;
	float scroll_y = scroll_y_max() > 0 ? scrollbar.value() / (float) scroll_y_max() : 0.0f;

	_piano_timeline->calc_sizes();
	_piano_timeline->_keys->calc_sizes();

	set_timeline_width();
	_piano_timeline->h(NUM_OCTAVES * octave_height());

	scroll_to(scroll_x * scroll_x_max(), scroll_y * scroll_y_max());
	sticky_keys();

	if (_following) {
		focus_cursor();
	}
}

void Piano_Roll::set_size(int W, int H) {
	if (W != w() || H != h()) {
		size(W, H);
		set_timeline_width();
		if (hscrollbar.value() > scroll_x_max()) {
			scroll_to(scroll_x_max(), yposition());
			sticky_keys();
		}
		if (scrollbar.value() > scroll_y_max()) {
			scroll_to(xposition(), scroll_y_max());
		}
	}
}

void Piano_Roll::set_timeline_width() {
	_piano_timeline->w(std::max(WHITE_KEY_WIDTH + _song_length * tick_width(), w() - scrollbar.w()));
	int32_t last_note_x = get_last_note_x();
	int width = last_note_x + w() - scrollbar.w() - WHITE_KEY_WIDTH;
	if (width > _piano_timeline->w()) {
		_piano_timeline->w(width);
	}
}

bool Piano_Roll::set_timeline(const Song &song) {
	calc_channel_length(song.channel_1_commands(), _channel_1_loop_tick, _channel_1_end_tick);
	calc_channel_length(song.channel_2_commands(), _channel_2_loop_tick, _channel_2_end_tick);
	calc_channel_length(song.channel_3_commands(), _channel_3_loop_tick, _channel_3_end_tick);
	calc_channel_length(song.channel_4_commands(), _channel_4_loop_tick, _channel_4_end_tick);

	_song_length = get_song_length();

	bool success = true;
	success = success && build_note_view(_piano_timeline->_channel_1_loops, _piano_timeline->_channel_1_calls, _channel_1_notes, song.channel_1_commands(), _song_length, NOTE_RED);
	success = success && build_note_view(_piano_timeline->_channel_2_loops, _piano_timeline->_channel_2_calls, _channel_2_notes, song.channel_2_commands(), _song_length, NOTE_BLUE);
	success = success && build_note_view(_piano_timeline->_channel_3_loops, _piano_timeline->_channel_3_calls, _channel_3_notes, song.channel_3_commands(), _song_length, NOTE_GREEN);
	success = success && build_note_view(_piano_timeline->_channel_4_loops, _piano_timeline->_channel_4_calls, _channel_4_notes, song.channel_4_commands(), _song_length, NOTE_BROWN);

	if (!success) {
		clear();
		return false;
	}

	_piano_timeline->set_channel_1(_channel_1_notes);
	_piano_timeline->set_channel_2(_channel_2_notes);
	_piano_timeline->set_channel_3(_channel_3_notes);
	_piano_timeline->set_channel_4(_channel_4_notes);

	set_timeline_width();

	return true;
}

void Piano_Roll::set_active_channel_timeline(const Song &song) {
	int32_t song_length = get_song_length();
	// TODO: if song length has changed, rebuild all channel timelines

	if (selected_channel() == 1) {
		_piano_timeline->clear_channel_1();
		_channel_1_notes.clear();
		build_note_view(_piano_timeline->_channel_1_loops, _piano_timeline->_channel_1_calls, _channel_1_notes, song.channel_1_commands(), song_length, NOTE_RED);
		_piano_timeline->set_channel_1(_channel_1_notes);
	}
	else if (selected_channel() == 2) {
		_piano_timeline->clear_channel_2();
		_channel_2_notes.clear();
		build_note_view(_piano_timeline->_channel_2_loops, _piano_timeline->_channel_2_calls, _channel_2_notes, song.channel_2_commands(), song_length, NOTE_BLUE);
		_piano_timeline->set_channel_2(_channel_2_notes);
	}
	else if (selected_channel() == 3) {
		_piano_timeline->clear_channel_3();
		_channel_3_notes.clear();
		build_note_view(_piano_timeline->_channel_3_loops, _piano_timeline->_channel_3_calls, _channel_3_notes, song.channel_3_commands(), song_length, NOTE_GREEN);
		_piano_timeline->set_channel_3(_channel_3_notes);
	}
	else if (selected_channel() == 4) {
		_piano_timeline->clear_channel_4();
		_channel_4_notes.clear();
		build_note_view(_piano_timeline->_channel_4_loops, _piano_timeline->_channel_4_calls, _channel_4_notes, song.channel_4_commands(), song_length, NOTE_BROWN);
		_piano_timeline->set_channel_4(_channel_4_notes);
	}
}

void Piano_Roll::set_active_channel_selection(const std::set<int32_t> &selection) {
	auto channel = _piano_timeline->active_channel_boxes();
	if (!channel) return;

	for (int32_t index : selection) {
		channel->at(index)->selected(true);
	}
}

#define TICK_TO_X_POS(tick) (_piano_timeline->x() + WHITE_KEY_WIDTH + (tick) * tick_width())
#define PITCH_TO_Y_POS(pitch, octave) (_piano_timeline->y() + (NUM_OCTAVES - (octave)) * octave_height() + (NUM_PITCHES - (size_t)(pitch)) * note_row_height())

bool Piano_Roll::build_note_view(
	std::vector<Loop_Box *> &loops,
	std::vector<Call_Box *> &calls,
	std::vector<Note_View> &notes,
	const std::vector<Command> &commands,
	int32_t end_tick,
	Fl_Color color
) {
	int32_t tick = 0;

	Note_View note;
	note.octave = 1;
	note.speed = 12;
	note.volume = 0;
	note.fade = 0;
	note.delay = 0;
	note.extent = 0;
	note.rate = 0;
	note.ghost = false;

	bool restarted = false;
	bool loop_pending = false;
	Loop_Box *loop = nullptr;
	Call_Box *call = nullptr;

	auto command_itr = commands.begin();

	std::stack<std::pair<decltype(command_itr), int32_t>> loop_stack;
	std::stack<decltype(command_itr)> call_stack;
	std::map<std::string, int32_t> label_positions;

	while (command_itr != commands.end() && tick < end_tick) {
		for (const std::string &label : command_itr->labels) {
			label_positions.insert({ label, tick });
		}
		// TODO: handle all other commands...
		if (command_itr->type == Command_Type::NOTE) {
			note.length = command_itr->note.length;
			note.pitch = command_itr->note.pitch;
			tick += note.length * note.speed;
			if (tick > end_tick) {
				note.length -= (tick - end_tick) / note.speed;
			}
			note.index = command_itr - commands.begin();
			note.ghost = restarted;
			notes.push_back(note);

			int note_x2 = TICK_TO_X_POS(tick);
			int note_y1 = PITCH_TO_Y_POS(note.pitch, note.octave);
			int note_y2 = note_y1 + note_row_height();
			if (loop) {
				loop->size(note_x2 - loop->x(), loop->h());
				loop->set_end_tick(tick);
				if (note_y1 < loop->y()) {
					int loop_y2 = loop->y() + loop->h();
					loop->resize(loop->x(), note_y1, loop->w(), loop_y2 - note_y1);
					loop->set_max_pitch(note.pitch, note.octave);
				}
				if (note_y2 - loop->y() > loop->h()) {
					loop->size(loop->w(), note_y2 - loop->y());
					loop->set_min_pitch(note.pitch, note.octave);
				}
			}
			if (call) {
				call->size(note_x2 - call->x(), call->h());
				call->set_end_tick(tick);
				if (note_y1 < call->y()) {
					int call_y2 = call->y() + call->h();
					call->resize(call->x(), note_y1, call->w(), call_y2 - note_y1);
					call->set_max_pitch(note.pitch, note.octave);
				}
				if (note_y2 - call->y() > call->h()) {
					call->size(call->w(), note_y2 - call->y());
					call->set_min_pitch(note.pitch, note.octave);
				}
			}
		}
		else if (command_itr->type == Command_Type::DRUM_NOTE) {
			note.length = command_itr->drum_note.length;
			note.pitch = (Pitch)command_itr->drum_note.instrument;
			tick += note.length * note.speed;
			if (tick > end_tick) {
				note.length -= (tick - end_tick) / note.speed;
			}
			note.index = command_itr - commands.begin();
			note.ghost = restarted;
			notes.push_back(note);

			int note_x2 = TICK_TO_X_POS(tick);
			int note_y1 = PITCH_TO_Y_POS(note.pitch, note.octave);
			int note_y2 = note_y1 + note_row_height();
			if (loop) {
				loop->size(note_x2 - loop->x(), loop->h());
				loop->set_end_tick(tick);
				if (note_y1 < loop->y()) {
					int loop_y2 = loop->y() + loop->h();
					loop->resize(loop->x(), note_y1, loop->w(), loop_y2 - note_y1);
					loop->set_max_pitch(note.pitch, note.octave);
				}
				if (note_y2 - loop->y() > loop->h()) {
					loop->size(loop->w(), note_y2 - loop->y());
					loop->set_min_pitch(note.pitch, note.octave);
				}
			}
			if (call) {
				call->size(note_x2 - call->x(), call->h());
				call->set_end_tick(tick);
				if (note_y1 < call->y()) {
					int call_y2 = call->y() + call->h();
					call->resize(call->x(), note_y1, call->w(), call_y2 - note_y1);
					call->set_max_pitch(note.pitch, note.octave);
				}
				if (note_y2 - call->y() > call->h()) {
					call->size(call->w(), note_y2 - call->y());
					call->set_min_pitch(note.pitch, note.octave);
				}
			}
		}
		else if (command_itr->type == Command_Type::REST) {
			note.length = command_itr->rest.length;
			note.pitch = Pitch::REST;
			tick += note.length * note.speed;
			if (tick > end_tick) {
				note.length -= (tick - end_tick) / note.speed;
			}
			note.index = command_itr - commands.begin();
			note.ghost = restarted;
			notes.push_back(note);

			int note_x2 = TICK_TO_X_POS(tick);
			if (loop) {
				loop->size(note_x2 - loop->x(), loop->h());
				loop->set_end_tick(tick);
			}
			if (call) {
				call->size(note_x2 - call->x(), call->h());
				call->set_end_tick(tick);
			}
		}
		else if (command_itr->type == Command_Type::OCTAVE) {
			note.octave = command_itr->octave.octave;
		}
		else if (command_itr->type == Command_Type::NOTE_TYPE) {
			note.speed = command_itr->note_type.speed;
			note.volume = command_itr->note_type.volume;
			note.fade = command_itr->note_type.fade;
		}
		else if (command_itr->type == Command_Type::DRUM_SPEED) {
			note.speed = command_itr->drum_speed.speed;
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
		else if (command_itr->type == Command_Type::VIBRATO) {
			note.delay = command_itr->vibrato.delay;
			note.extent = command_itr->vibrato.extent;
			note.rate = command_itr->vibrato.rate;
		}
		else if (command_itr->type == Command_Type::SOUND_JUMP) {
			command_itr = find_note_with_label(commands, command_itr->target);
			continue;
		}
		else if (command_itr->type == Command_Type::SOUND_LOOP) {
			if (loop_stack.size() > 0 && loop_stack.top().first == command_itr) {
				if (loop_pending) {
					loop->position(loop->x() - loop->w(), loop->y());
					_piano_timeline->begin();
					Loop_Box *first_loop = loop;
					loop = new Loop_Box(loop->x() + loop->w(), loop->y(), loop->w(), loop->h());
					loop->set_start_tick(first_loop->start_tick());
					loop->set_end_tick(first_loop->end_tick());
					first_loop->set_start_tick(first_loop->start_tick() - (loop->end_tick() - loop->start_tick()));
					first_loop->set_end_tick(first_loop->end_tick() - (loop->end_tick() - loop->start_tick()));
					loop->set_min_pitch(first_loop->min_pitch(), first_loop->min_octave());
					loop->set_max_pitch(first_loop->max_pitch(), first_loop->max_octave());
					loop->box(FL_BORDER_FRAME);
					loop->color(fl_lighter(color));
					loops.push_back(loop);
					_piano_timeline->end();
					loop_pending = false;
				}

				loop_stack.top().second -= 1;
				if (loop_stack.top().second == 0) {
					loop = nullptr;
					loop_stack.pop();
				}
				else {
					if (!restarted) {
						_piano_timeline->begin();
						int loop_x1 = TICK_TO_X_POS(tick);
						int loop_y1 = PITCH_TO_Y_POS(Pitch::C_NAT, 1);
						int loop_y2 = PITCH_TO_Y_POS(Pitch::B_NAT, 8);
						loop = new Loop_Box(loop_x1, loop_y1, 0, loop_y2 - loop_y1);
						loop->set_start_tick(tick);
						loop->box(FL_BORDER_FRAME);
						loop->color(fl_lighter(color));
						loops.push_back(loop);
						_piano_timeline->end();
					}

					command_itr = find_note_with_label(commands, command_itr->target);
					continue;
				}
			}
			else {
				if (command_itr->sound_loop.loop_count == 0) {
					if (!label_positions.count(command_itr->target)) {
						command_itr = find_note_with_label(commands, command_itr->target);
						continue;
					}
					if (tick < end_tick) {
						restarted = true;
						// TODO: this gets trapped in an infinite loop if the channel's body advances by 0 ticks
						command_itr = find_note_with_label(commands, command_itr->target);
						continue;
					}
					break; // song is finished
				}
				else if (command_itr->sound_loop.loop_count > 1) {
					// nested loops not supported
					if (loop_stack.size() > 0) {
						return false;
					}

					if (!restarted) {
						_piano_timeline->begin();
						loop_pending = true;
						int loop_x1 = TICK_TO_X_POS(tick);
						int loop_y1 = PITCH_TO_Y_POS(Pitch::C_NAT, 1);
						int loop_y2 = PITCH_TO_Y_POS(Pitch::B_NAT, 8);
						loop = new Loop_Box(loop_x1, loop_y1, 0, loop_y2 - loop_y1);
						loop->set_start_tick(tick);
						loop->box(FL_BORDER_FRAME);
						loop->color(fl_lighter(color));
						loops.push_back(loop);
						_piano_timeline->end();
					}

					loop_stack.emplace(command_itr, command_itr->sound_loop.loop_count - 1);
					command_itr = find_note_with_label(commands, command_itr->target);
					continue;
				}
			}
		}
		else if (command_itr->type == Command_Type::SOUND_CALL) {
			// nested calls not supported
			if (call_stack.size() > 0) {
				return false;
			}

			if (!restarted) {
				_piano_timeline->begin();
				int call_x1 = TICK_TO_X_POS(tick);
				int call_y1 = PITCH_TO_Y_POS(Pitch::C_NAT, 1);
				int call_y2 = PITCH_TO_Y_POS(Pitch::B_NAT, 8);
				call = new Call_Box(call_x1, call_y1, 0, call_y2 - call_y1);
				call->set_start_tick(tick);
				call->box(FL_BORDER_FRAME);
				call->color(fl_darker(color));
				calls.push_back(call);
				_piano_timeline->end();
			}

			call_stack.push(command_itr);
			command_itr = find_note_with_label(commands, command_itr->target);
			continue;
		}
		else if (command_itr->type == Command_Type::SOUND_RET) {
			call = nullptr;
			if (call_stack.size() == 0) {
				break; // song is finished
			}
			else {
				command_itr = call_stack.top();
				call_stack.pop();
			}
		}
		++command_itr;
	}

	if (loop_pending) {
		loop->position(loop->x() - loop->w(), loop->y());
		_piano_timeline->begin();
		Loop_Box *first_loop = loop;
		loop = new Loop_Box(loop->x() + loop->w(), loop->y(), loop->w(), loop->h());
		loop->set_start_tick(first_loop->start_tick());
		loop->set_end_tick(first_loop->end_tick());
		first_loop->set_start_tick(first_loop->start_tick() - (loop->end_tick() - loop->start_tick()));
		first_loop->set_end_tick(first_loop->end_tick() - (loop->end_tick() - loop->start_tick()));
		loop->set_min_pitch(first_loop->min_pitch(), first_loop->min_octave());
		loop->set_max_pitch(first_loop->max_pitch(), first_loop->max_octave());
		loop->box(FL_BORDER_FRAME);
		loop->color(fl_lighter(color));
		loops.push_back(loop);
		_piano_timeline->end();
		loop_pending = false;
	}

	return true;
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
	const auto get_channel_last_note_x = [this](std::vector<Note_Box *> &notes) {
		if (notes.size() > 0) {
			return notes.back()->x() - _piano_timeline->x();
		}
		return 0;
	};
	int32_t last_note_x = std::max({
		get_channel_last_note_x(_piano_timeline->_channel_1_notes),
		get_channel_last_note_x(_piano_timeline->_channel_2_notes),
		get_channel_last_note_x(_piano_timeline->_channel_3_notes),
		get_channel_last_note_x(_piano_timeline->_channel_4_notes),
	});
	return last_note_x;
}

void Piano_Roll::align_cursor() {
	_tick = quantize_tick(_tick, true);
}

void Piano_Roll::clear() {
	_piano_timeline->clear();
	_piano_timeline->w(w() - scrollbar.w());
	_piano_timeline->_keys->reset_key_colors();
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
	_piano_timeline->reset_note_colors();
	_piano_timeline->_keys->reset_key_colors();
	if (_tick == -1) {
		scroll_to(0, yposition());
		sticky_keys();
	}
	redraw();
}

void Piano_Roll::unpause_following() {
	_following = true;
	_paused = false;
}

void Piano_Roll::stop_following() {
	_following = false;
	_paused = false;
	_tick = -1;
	_piano_timeline->reset_note_colors();
	_piano_timeline->_keys->reset_key_colors();
	redraw();
}

void Piano_Roll::pause_following() {
	_following = false;
	_paused = true;
}

void Piano_Roll::highlight_tick(int32_t t) {
	if (_tick == t) return; // no change
	_tick = t;

	_piano_timeline->_keys->reset_key_colors();

	_piano_timeline->highlight_channel_1_tick(_tick, _channel_1_muted);
	_piano_timeline->highlight_channel_2_tick(_tick, _channel_2_muted);
	_piano_timeline->highlight_channel_3_tick(_tick, _channel_3_muted);
	_piano_timeline->highlight_channel_4_tick(_tick, _channel_4_muted);

	focus_cursor();
	redraw();
}

void Piano_Roll::focus_cursor() {
	int x_pos = (_tick / TICKS_PER_STEP * TICKS_PER_STEP) * tick_width();
	if ((_following && _realtime) || x_pos > xposition() + w() - WHITE_KEY_WIDTH * 2 || x_pos < xposition()) {
		if (x_pos > scroll_x_max()) {
			scroll_to(scroll_x_max(), yposition());
		}
		else {
			scroll_to(x_pos, yposition());
		}
		sticky_keys();
	}
}

void Piano_Roll::sticky_keys() {
	_piano_timeline->_keys->position(0, _piano_timeline->_keys->y());
}

int Piano_Roll::scroll_x_max() const {
	return _piano_timeline->w() - (w() - scrollbar.w());
}

int Piano_Roll::scroll_y_max() const {
	return _piano_timeline->h() - (h() - hscrollbar.h());
}

bool Piano_Roll::put_note(Song &song, Pitch pitch) {
	auto channel = _piano_timeline->active_channel_boxes();
	if (!channel) return false;

	auto view = active_channel_view();

	if (_tick == -1) return false;

	int32_t tick_offset = 0;

	const auto find_note_view_at_tick = [&tick_offset](const std::vector<Note_View> &view, int32_t tick) {
		int32_t t_left = 0;
		for (const Note_View &note : view) {
			int32_t t_right = t_left + note.length * note.speed;
			if (t_right > tick && !note.ghost) {
				tick_offset = tick - t_left;
				return &note;
			}
			t_left = t_right;
		}
		return (const Note_View *)nullptr;
	};

	const Note_View *note_view = find_note_view_at_tick(*view, _tick);

	if (!note_view) return false;

	int32_t index = note_view->index;
	int32_t speed = note_view->speed;

	std::set<int32_t> selected_boxes;

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			selected_boxes.insert(note_itr - channel->begin());
		}
	}

	song.put_note(selected_channel(), selected_boxes, pitch, index, tick_offset / speed);
	set_active_channel_timeline(song);
	_piano_timeline->select_note_at_tick(*channel, _tick);

	_tick += speed;
	focus_cursor();

	return true;
}

bool Piano_Roll::pitch_up(Song &song) {
	auto channel = _piano_timeline->active_channel_boxes();
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

	song.pitch_up(selected_channel(), selected_notes, selected_boxes, *view);
	set_active_channel_timeline(song);
	set_active_channel_selection(selected_boxes);

	return true;
}

bool Piano_Roll::pitch_down(Song &song) {
	auto channel = _piano_timeline->active_channel_boxes();
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

	song.pitch_down(selected_channel(), selected_notes, selected_boxes, *view);
	set_active_channel_timeline(song);
	set_active_channel_selection(selected_boxes);

	return true;
}

bool Piano_Roll::octave_up(Song &song) {
	auto channel = _piano_timeline->active_channel_boxes();
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

	song.octave_up(selected_channel(), selected_notes, selected_boxes, *view);
	set_active_channel_timeline(song);
	set_active_channel_selection(selected_boxes);

	return true;
}

bool Piano_Roll::octave_down(Song &song) {
	auto channel = _piano_timeline->active_channel_boxes();
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

	song.octave_down(selected_channel(), selected_notes, selected_boxes, *view);
	set_active_channel_timeline(song);
	set_active_channel_selection(selected_boxes);

	return true;
}

bool Piano_Roll::move_left(Song &song) {
	auto channel = _piano_timeline->active_channel_boxes();
	if (!channel) return false;

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
					if (itr->type == Command_Type::REST) {
						return true;
					}
					if (
						is_note_command(itr->type) &&
						selected_notes.count((itr.base() - 1) - commands.begin()) > 0
					) {
						return true;
					}
					if (!is_note_setting_command(itr->type) || itr->labels.size() > 0) {
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

	song.move_left(selected_channel(), selected_notes, selected_boxes);
	set_active_channel_timeline(song);
	set_active_channel_selection(selected_boxes);

	return true;
}

bool Piano_Roll::move_right(Song &song) {
	auto channel = _piano_timeline->active_channel_boxes();
	if (!channel) return false;

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
					if (itr->type == Command_Type::REST && itr->labels.size() == 0) {
						return true;
					}
					if (
						is_note_command(itr->type) &&
						itr->labels.size() == 0 &&
						selected_notes.count(itr - commands.begin()) > 0
					) {
						return true;
					}
					if (!is_note_setting_command(itr->type) || itr->labels.size() > 0) {
						return false;
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

	song.move_right(selected_channel(), selected_notes, selected_boxes);
	set_active_channel_timeline(song);
	set_active_channel_selection(selected_boxes);

	return true;
}

bool Piano_Roll::shorten(Song &song) {
	auto channel = _piano_timeline->active_channel_boxes();
	if (!channel) return false;

	std::set<int32_t> selected_notes;
	std::set<int32_t> selected_boxes;

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			Note_View note_view = note->note_view();
			if (note_view.length == 1) {
				return false;
			}
			selected_notes.insert(note_view.index);
			selected_boxes.insert(note_itr - channel->begin());
		}
	}
	if (selected_notes.size() == 0) {
		return false;
	}

	song.shorten(selected_channel(), selected_notes, selected_boxes);
	set_active_channel_timeline(song);
	set_active_channel_selection(selected_boxes);

	return true;
}

bool Piano_Roll::lengthen(Song &song) {
	auto channel = _piano_timeline->active_channel_boxes();
	if (!channel) return false;

	std::set<int32_t> selected_notes;
	std::set<int32_t> selected_boxes;

	const std::vector<Command> &commands = song.channel_commands(selected_channel());

	for (auto note_itr = channel->begin(); note_itr != channel->end(); ++note_itr) {
		Note_Box *note = *note_itr;
		if (note->selected()) {
			Note_View note_view = note->note_view();
			auto command_itr = commands.begin() + note_view.index;

			const auto is_followed_by_rest = [&](decltype(command_itr) itr) {
				++itr;
				while (itr != commands.end()) {
					if (itr->type == Command_Type::REST && itr->labels.size() == 0) {
						return true;
					}
					if (!is_note_setting_command(itr->type) || itr->labels.size() > 0) {
						return false;
					}
					++itr;
				}
				return false;
			};

			if (note_view.length == 16 || !is_followed_by_rest(command_itr)) {
				return false;
			}
			selected_notes.insert(note_view.index);
			selected_boxes.insert(note_itr - channel->begin());
		}
	}
	if (selected_notes.size() == 0) {
		return false;
	}

	song.lengthen(selected_channel(), selected_notes, selected_boxes);
	set_active_channel_timeline(song);
	set_active_channel_selection(selected_boxes);

	return true;
}

bool Piano_Roll::delete_selection(Song &song) {
	auto channel = _piano_timeline->active_channel_boxes();
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

	song.delete_selection(selected_channel(), selected_notes, selected_boxes);
	set_active_channel_timeline(song);

	return true;
}

bool Piano_Roll::snip_selection(Song &song) {
	auto channel = _piano_timeline->active_channel_boxes();
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

	song.snip_selection(selected_channel(), selected_notes, selected_boxes);
	set_active_channel_timeline(song);

	return true;
}

int32_t Piano_Roll::quantize_tick(int32_t tick, bool round) {
	const auto view = active_channel_view();
	if (view && !_following && !_paused) {
		int32_t t_left = 0;
		for (const Note_View &note : *view) {
			int32_t t_right = t_left + note.length * note.speed;
			if (t_right > tick) {
				return t_left + (tick - t_left + (round ? note.speed / 2 - 1 : 0)) / note.speed * note.speed;
			}
			t_left = t_right;
		}
	}
	return tick / TICKS_PER_STEP * TICKS_PER_STEP;
}

std::vector<Note_View> *Piano_Roll::active_channel_view() {
	int active_channel = selected_channel();
	if (active_channel == 1) return &_channel_1_notes;
	if (active_channel == 2) return &_channel_2_notes;
	if (active_channel == 3) return &_channel_3_notes;
	if (active_channel == 4) return &_channel_4_notes;
	return nullptr;
}

void Piano_Roll::hscrollbar_cb(Fl_Scrollbar *sb, void *) {
	Piano_Roll *scroll = (Piano_Roll *)(sb->parent());
	scroll->scroll_to(sb->value(), scroll->yposition());
	scroll->sticky_keys();
	scroll->redraw();
}
