#include <map>
#include <stack>

#pragma warning(push, 0)
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#pragma warning(pop)

#include "piano-roll.h"

#include "themes.h"

static inline bool is_white_key(size_t i) {
	return !(i == 1 || i == 3 || i == 5 || i == 8 || i == 10);
}

static const auto find_note_with_label(const std::list<Command> &commands, std::string label) {
	for (auto command_itr = commands.begin(); command_itr != commands.end(); ++command_itr) {
		if (command_itr->labels.count(label) > 0) {
			return command_itr;
		}
	}
	return commands.end(); // if this happens, we've already messed up
}

static void calc_channel_length(const std::list<Command> &commands, int32_t &loop_tick, int32_t &end_tick) {
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
			tick += command_itr->note.length * speed / 2;
		}
		else if (command_itr->type == Command_Type::DRUM_NOTE) {
			tick += command_itr->drum_note.length * speed / 2;
		}
		else if (command_itr->type == Command_Type::REST) {
			tick += command_itr->rest.length * speed / 2;
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
					if (label_positions.count(command_itr->target)) {
						loop_tick = label_positions.at(command_itr->target);
					}
					end_tick = tick;
					break; // assume end of song for now
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

static std::vector<Note_View> build_timeline(const std::list<Command> &commands, int32_t end_tick) {
	int32_t tick = 0;
	std::vector<Note_View> timeline;
	Note_View note;
	note.octave = 1;
	note.speed = 12;
	note.volume = 0;
	note.fade = 0;
	note.delay = 0;
	note.extent = 0;
	note.rate = 0;
	auto command_itr = commands.begin();
	std::stack<std::pair<decltype(command_itr), int32_t>> loop_stack;
	std::stack<decltype(command_itr)> call_stack;
	while (command_itr != commands.end() && tick < end_tick) {
		// TODO: handle all other commands...
		if (command_itr->type == Command_Type::NOTE) {
			note.length = command_itr->note.length;
			note.pitch = command_itr->note.pitch;
			tick += note.length * note.speed / 2;
			if (tick > end_tick) {
				note.length -= (tick - end_tick) / (note.speed / 2);
			}
			timeline.push_back(note);
		}
		else if (command_itr->type == Command_Type::DRUM_NOTE) {
			note.length = command_itr->drum_note.length;
			note.pitch = (Pitch)command_itr->drum_note.instrument;
			tick += note.length * note.speed / 2;
			if (tick > end_tick) {
				note.length -= (tick - end_tick) / (note.speed / 2);
			}
			timeline.push_back(note);
		}
		else if (command_itr->type == Command_Type::REST) {
			note.length = command_itr->rest.length;
			note.pitch = Pitch::REST;
			tick += note.length * note.speed / 2;
			if (tick > end_tick) {
				note.length -= (tick - end_tick) / (note.speed / 2);
			}
			timeline.push_back(note);
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
					if (tick < end_tick) {
						command_itr = find_note_with_label(commands, command_itr->target);
						continue;
					}
					break; // assume end of song for now
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
				break; // song is finished
			}
			else {
				command_itr = call_stack.top();
				call_stack.pop();
			}
		}
		++command_itr;
	}
	return timeline;
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
	end();
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
	_channel_1_notes.clear();
	_channel_2_notes.clear();
	_channel_3_notes.clear();
	_channel_4_notes.clear();
	for (Note_Box *note : _channel_1_timeline) {
		delete note;
	}
	_channel_1_timeline.clear();
	for (Note_Box *note : _channel_2_timeline) {
		delete note;
	}
	_channel_2_timeline.clear();
	for (Note_Box *note : _channel_3_timeline) {
		delete note;
	}
	_channel_3_timeline.clear();
	for (Note_Box *note : _channel_4_timeline) {
		delete note;
	}
	_channel_4_timeline.clear();
}

void Piano_Timeline::set_timeline(const Song &song) {
	calc_channel_length(song.channel_1_commands(), _channel_1_loop_tick, _channel_1_end_tick);
	calc_channel_length(song.channel_2_commands(), _channel_2_loop_tick, _channel_2_end_tick);
	calc_channel_length(song.channel_3_commands(), _channel_3_loop_tick, _channel_3_end_tick);
	calc_channel_length(song.channel_4_commands(), _channel_4_loop_tick, _channel_4_end_tick);
	int32_t song_length = std::max({ _channel_1_end_tick, _channel_2_end_tick, _channel_3_end_tick, _channel_4_end_tick });
	_channel_1_notes = build_timeline(song.channel_1_commands(), song_length);
	_channel_2_notes = build_timeline(song.channel_2_commands(), song_length);
	_channel_3_notes = build_timeline(song.channel_3_commands(), song_length);
	_channel_4_notes = build_timeline(song.channel_4_commands(), song_length);

	set_channel_timeline(_channel_1_timeline, _channel_1_notes, NOTE_RED);
	set_channel_timeline(_channel_2_timeline, _channel_2_notes, NOTE_BLUE);
	set_channel_timeline(_channel_3_timeline, _channel_3_notes, NOTE_GREEN);
	set_channel_timeline(_channel_4_timeline, _channel_4_notes, NOTE_BROWN);
}

int32_t Piano_Timeline::get_loop_tick() const {
	// TODO: this naive approach works in many cases but this will need to be improved (see: mainmenu.asm, lookhiker.asm)
	int32_t loop_tick = std::max({ _channel_1_loop_tick, _channel_2_loop_tick, _channel_3_loop_tick, _channel_4_loop_tick });
	return loop_tick;
}

void Piano_Timeline::reset_note_colors() {
	for (Note_Box *note : _channel_1_timeline) {
		note->color(NOTE_RED);
	}
	for (Note_Box *note : _channel_2_timeline) {
		note->color(NOTE_BLUE);
	}
	for (Note_Box *note : _channel_3_timeline) {
		note->color(NOTE_GREEN);
	}
	for (Note_Box *note : _channel_4_timeline) {
		note->color(NOTE_BROWN);
	}
}

void Piano_Timeline::set_channel_timeline(std::vector<Note_Box *> &timeline, const std::vector<Note_View> &notes, Fl_Color color) {
	begin();
	int x_pos = x() + WHITE_KEY_WIDTH;
	for (const Note_View &note : notes) {
		int width = TIME_STEP_WIDTH * note.length * note.speed / DEFAULT_SPEED;
		if (note.pitch != Pitch::REST) {
			int y_pos = y() + (NUM_OCTAVES - note.octave) * OCTAVE_HEIGHT + (NUM_PITCHES - (size_t)note.pitch) * NOTE_ROW_HEIGHT;
			Note_Box *box = new Note_Box(x_pos, y_pos, width, NOTE_ROW_HEIGHT);
			box->box(FL_BORDER_BOX);
			box->color(color);
			timeline.push_back(box);
		}
		x_pos += width;
	}
	end();

	if (timeline.size() > 0) {
		const int width = timeline.back()->x() + parent()->w() - ((Fl_Scroll *)parent())->scrollbar.w() - WHITE_KEY_WIDTH;
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

Note_Box *Piano_Timeline::get_note_at_tick(std::vector<Note_Box *> &timeline, int32_t tick) {
	int x_pos = tick * TIME_STEP_WIDTH + WHITE_KEY_WIDTH;
	for (Note_Box *note : timeline) {
		int note_left = note->x() - note->parent()->x();
		int note_right = note_left + note->w();
		if (note_left <= x_pos && x_pos < note_right) {
			return note;
		}
	}
	return nullptr;
}

void Piano_Timeline::toggle_channel_box_type(std::vector<Note_Box *> &timeline) {
	if (timeline.size() == 0) return;

	Fl_Boxtype box = timeline.front()->box() == FL_BORDER_BOX ? FL_BORDER_FRAME : FL_BORDER_BOX;
	for (Note_Box *note : timeline) {
		note->box(box);
	}
	redraw();
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

		int x_pos = x() + WHITE_KEY_WIDTH;
		const size_t num_dividers = (w() - WHITE_KEY_WIDTH) / TIME_STEP_WIDTH + 1;
		for (size_t i = 0; i < num_dividers; ++i) {
			fl_yxline(x_pos - 1, y(), y() + h());
			fl_yxline(x_pos, y(), y() + h());
			x_pos += TIME_STEP_WIDTH;
		}

		Piano_Roll *p = (Piano_Roll *)parent();
		x_pos = x() + p->_tick * TIME_STEP_WIDTH + WHITE_KEY_WIDTH;
		fl_color(FL_YELLOW);
		fl_yxline(x_pos, y(), y() + h());
	}
	draw_children();
}

Piano_Roll::Piano_Roll(int x, int y, int w, int h, const char *l) : Fl_Scroll(x, y, w, h, l) {
	type(BOTH_ALWAYS);
	_piano_timeline = new Piano_Timeline(x, y, (w - scrollbar.w()) * 2, OCTAVE_HEIGHT * NUM_OCTAVES);
	end();

	hscrollbar.callback((Fl_Callback *)hscrollbar_cb);
}

Piano_Roll::~Piano_Roll() noexcept {
	if (_piano_timeline) {
		delete _piano_timeline;
		_piano_timeline = nullptr;
	}
}

int Piano_Roll::handle(int event) {
	switch (event) {
	case FL_PUSH:
		if (_following) {
			_realtime = !_realtime;
			return 1;
		}
		break;
	case FL_MOUSEWHEEL:
		if (Fl::event_shift()) {
			std::swap(Fl::e_dx, Fl::e_dy);
		}
		break;
	}
	return Fl_Scroll::handle(event);
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
	_piano_timeline->w((w() - scrollbar.w()) * 2);
	scroll_to(0, yposition());
	_piano_timeline->_keys->position(0, _piano_timeline->_keys->y());
	_following = false;
	_tick = -1;
}

void Piano_Roll::start_following() {
	_following = true;
	_tick = -1;
	scroll_to(0, yposition());
	_piano_timeline->_keys->position(0, _piano_timeline->_keys->y());
	redraw();
}

void Piano_Roll::unpause_following() {
	_following = true;
}

void Piano_Roll::stop_following() {
	_following = false;
	_tick = -1;
	_piano_timeline->reset_note_colors();
	redraw();
}

void Piano_Roll::pause_following() {
	_following = false;
}

void Piano_Roll::highlight_tick(int32_t t) {
	if (_tick == t) return; // no change
	_tick = t;

	Note_Box *note = _piano_timeline->get_channel_1_note_at_tick(_tick);
	if (note) {
		note->color(fl_lighter(NOTE_RED)); // maybe subclass box to also draw() a frame
	}
	note = _piano_timeline->get_channel_2_note_at_tick(_tick);
	if (note) {
		note->color(fl_lighter(NOTE_BLUE));
	}
	note = _piano_timeline->get_channel_3_note_at_tick(_tick);
	if (note) {
		note->color(fl_lighter(NOTE_GREEN));
	}
	note = _piano_timeline->get_channel_4_note_at_tick(_tick);
	if (note) {
		note->color(fl_lighter(NOTE_BROWN));
	}

	int x_pos = _tick * TIME_STEP_WIDTH;
	if (_realtime || x_pos > xposition() + w() - WHITE_KEY_WIDTH * 2 || x_pos < xposition()) {
		if (x_pos > _piano_timeline->w() - (w() - scrollbar.w())) {
			scroll_to(_piano_timeline->w() - (w() - scrollbar.w()), yposition());
		}
		else {
			scroll_to(x_pos, yposition());
		}
		_piano_timeline->_keys->position(0, _piano_timeline->_keys->y());
	}
	redraw();
}

void Piano_Roll::hscrollbar_cb(Fl_Scrollbar *sb, void *) {
	Piano_Roll *scroll = (Piano_Roll *)(sb->parent());
	scroll->scroll_to(sb->value(), scroll->yposition());
	scroll->_piano_timeline->_keys->position(0, scroll->_piano_timeline->_keys->y());
	scroll->redraw();
}
