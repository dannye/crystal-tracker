#include <cassert>
#include <cstdio>
#include <fstream>

#include "song.h"

Song::Song() {}

Song::~Song() {
	clear();
}

void Song::clear() {
	_song_name = "";
	_number_of_channels = 0;
	_channel_1_label = "";
	_channel_2_label = "";
	_channel_3_label = "";
	_channel_4_label = "";
	_channel_1_commands.clear();
	_channel_2_commands.clear();
	_channel_3_commands.clear();
	_channel_4_commands.clear();
	_result = Parsed_Song::Result::SONG_NULL;
	_modified = false;
	_mod_time = 0;
	_history.clear();
	_future.clear();
	_loaded = false;
}

void Song::remember(int channel_number, const std::set<int32_t> &selection, Song_State::Action action) {
	_future.clear();
	while (_history.size() >= MAX_HISTORY_SIZE) { _history.pop_front(); }

	std::vector<Command> &commands = channel_commands(channel_number);

	Song_State ss;
	ss.channel_number = channel_number;
	ss.commands = commands;
	ss.selection = selection;
	ss.action = action;
	_history.push_back(ss);
}

int Song::undo() {
	if (_history.empty()) { return 0; }
	while (_future.size() >= MAX_HISTORY_SIZE) { _future.pop_front(); }

	Song_State &prev = _history.back();
	std::vector<Command> &commands = channel_commands(prev.channel_number);

	Song_State ss;
	ss.channel_number = prev.channel_number;
	ss.commands = std::move(commands);
	ss.selection = std::move(prev.selection);
	ss.action = prev.action;
	_future.push_back(ss);

	commands = std::move(prev.commands);
	_history.pop_back();

	_modified = true;

	return prev.channel_number;
}

int Song::redo() {
	if (_future.empty()) { return 0; }
	while (_history.size() >= MAX_HISTORY_SIZE) { _history.pop_front(); }

	Song_State &next = _future.back();
	std::vector<Command> &commands = channel_commands(next.channel_number);

	Song_State ss;
	ss.channel_number = next.channel_number;
	ss.commands = std::move(commands);
	ss.selection = std::move(next.selection);
	ss.action = next.action;
	_history.push_back(ss);

	commands = std::move(next.commands);
	_future.pop_back();

	_modified = true;

	return next.channel_number;
}

Parsed_Song::Result Song::read_song(const char *f) {
	Parsed_Song data(f);
	if (data.result() != Parsed_Song::Result::SONG_OK) {
		_error_message = get_error_message(data);
		return (_result = data.result());
	}

	_song_name = data.song_name();
	_number_of_channels = data.number_of_channels();
	_channel_1_label = data.channel_1_label();
	_channel_2_label = data.channel_2_label();
	_channel_3_label = data.channel_3_label();
	_channel_4_label = data.channel_4_label();
	_channel_1_commands = data.channel_1_commands();
	_channel_2_commands = data.channel_2_commands();
	_channel_3_commands = data.channel_3_commands();
	_channel_4_commands = data.channel_4_commands();

	_mod_time = file_modified(f);

	_loaded = true;
	return (_result = Parsed_Song::Result::SONG_OK);
}

void Song::new_song() {
	_song_name = "NewSong";
	_number_of_channels = 4;
	_channel_1_label = "NewSong_Ch1";
	_channel_2_label = "NewSong_Ch2";
	_channel_3_label = "NewSong_Ch3";
	_channel_4_label = "NewSong_Ch4";
	_channel_1_commands = { Command(Command_Type::SOUND_RET, _channel_1_label) };
	_channel_2_commands = { Command(Command_Type::SOUND_RET, _channel_2_label) };
	_channel_3_commands = { Command(Command_Type::SOUND_RET, _channel_3_label) };
	_channel_4_commands = { Command(Command_Type::SOUND_RET, _channel_4_label) };

	_mod_time = 0;

	_loaded = true;
	_result = Parsed_Song::Result::SONG_OK;
}

bool Song::write_song(const char *f) {
	std::ofstream ofs;
	open_ofstream(ofs, f);
	if (!ofs.good()) { return false; }

	ofs << _song_name << ":\n";
	ofs << "\tchannel_count " << _number_of_channels << '\n';
	if (_channel_1_label.size()) {
		ofs << "\tchannel 1, " << _channel_1_label << '\n';
	}
	if (_channel_2_label.size()) {
		ofs << "\tchannel 2, " << _channel_2_label << '\n';
	}
	if (_channel_3_label.size()) {
		ofs << "\tchannel 3, " << _channel_3_label << '\n';
	}
	if (_channel_4_label.size()) {
		ofs << "\tchannel 4, " << _channel_4_label << '\n';
	}
	if (_channel_1_label.size()) {
		ofs << '\n' << commands_str(_channel_1_commands);
	}
	if (_channel_2_label.size()) {
		ofs << '\n' << commands_str(_channel_2_commands);
	}
	if (_channel_3_label.size()) {
		ofs << '\n' << commands_str(_channel_3_commands);
	}
	if (_channel_4_label.size()) {
		ofs << '\n' << commands_str(_channel_4_commands);
	}
	ofs.close();

	_mod_time = file_modified(f);
	return true;
}

Note_View find_note_view(const std::vector<Note_View> &view, int32_t index) {
	for (const Note_View &note : view) {
		if (note.index == index) {
			return note;
		}
	}
	assert(false);
	return Note_View{};
}

void postprocess(std::vector<Command> &commands) {
	bool deleted = false;
	do {
		Note_View view;
		int32_t rest_index = -1;
		int32_t octave_index = -1;
		deleted = false;
		for (uint32_t i = 0; i < commands.size(); ++i) {
			if (commands[i].labels.size() > 0 || is_control_command(commands[i].type)) {
				view = Note_View{};
				rest_index = -1;
				octave_index = -1;
			}

			if (is_note_command(commands[i].type)) {
				rest_index = -1;
				octave_index = -1;
			}

			else if (commands[i].type == Command_Type::REST) {
				if (rest_index == -1) {
					if (commands[i].rest.length < 16) {
						rest_index = i;
					}
				}
				else {
					if (commands[rest_index].rest.length + commands[i].rest.length > 16) {
						commands[i].rest.length = commands[rest_index].rest.length + commands[i].rest.length - 16;
						commands[rest_index].rest.length = 16;
						rest_index = i;
					}
					else {
						commands[rest_index].rest.length = commands[rest_index].rest.length + commands[i].rest.length;
						commands.erase(commands.begin() + i);
						i -= 1;

						if (commands[rest_index].rest.length == 16) {
							rest_index = -1;
						}
					}
				}
			}

			else if (commands[i].type == Command_Type::OCTAVE) {
				if (octave_index == -1) {
					if (view.octave == commands[i].octave.octave) {
						deleted = true;
						commands.erase(commands.begin() + i);
						i -= 1;
					}
					else {
						view.octave = commands[i].octave.octave;
						octave_index = i;
					}
				}
				else {
					deleted = true;
					// TODO: these label's order is probably not being preserved...
					commands[octave_index + 1].labels.insert(commands[octave_index].labels.begin(), commands[octave_index].labels.end());
					commands.erase(commands.begin() + octave_index);
					i -= 1;
					if (rest_index > octave_index) {
						rest_index -= 1;
					}

					view.octave = commands[i].octave.octave;
					octave_index = i;
				}
			}
		}
	} while (deleted);
}

void Song::pitch_up(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view) {
	remember(selected_channel, selected_boxes, Song_State::Action::PITCH_UP);
	std::vector<Command> &commands = channel_commands(selected_channel);

	for (auto note_itr = selected_notes.rbegin(); note_itr != selected_notes.rend(); ++note_itr) {
		if (commands[*note_itr].note.pitch == Pitch::B_NAT) {
			commands[*note_itr].note.pitch = Pitch::C_NAT;

			Note_View note_view = find_note_view(view, *note_itr);
			Command command = Command(Command_Type::OCTAVE);
			command.octave.octave = note_view.octave;
			commands.insert(commands.begin() + *note_itr + 1, command);

			command.octave.octave = note_view.octave + 1;
			command.labels = std::move(commands[*note_itr].labels);
			commands.insert(commands.begin() + *note_itr, command);
		}
		else {
			commands[*note_itr].note.pitch = (Pitch)((int)commands[*note_itr].note.pitch + 1);
		}
	}

	postprocess(commands);

	_modified = true;
}

void Song::pitch_down(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view) {
	remember(selected_channel, selected_boxes, Song_State::Action::PITCH_DOWN);
	std::vector<Command> &commands = channel_commands(selected_channel);

	for (auto note_itr = selected_notes.rbegin(); note_itr != selected_notes.rend(); ++note_itr) {
		if (commands[*note_itr].note.pitch == Pitch::C_NAT) {
			commands[*note_itr].note.pitch = Pitch::B_NAT;

			Note_View note_view = find_note_view(view, *note_itr);
			Command command = Command(Command_Type::OCTAVE);
			command.octave.octave = note_view.octave;
			commands.insert(commands.begin() + *note_itr + 1, command);

			command.octave.octave = note_view.octave - 1;
			command.labels = std::move(commands[*note_itr].labels);
			commands.insert(commands.begin() + *note_itr, command);
		}
		else {
			commands[*note_itr].note.pitch = (Pitch)((int)commands[*note_itr].note.pitch - 1);
		}
	}

	postprocess(commands);

	_modified = true;
}

void Song::octave_up(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view) {
	remember(selected_channel, selected_boxes, Song_State::Action::OCTAVE_UP);
	std::vector<Command> &commands = channel_commands(selected_channel);

	for (auto note_itr = selected_notes.rbegin(); note_itr != selected_notes.rend(); ++note_itr) {
		Note_View note_view = find_note_view(view, *note_itr);
		Command command = Command(Command_Type::OCTAVE);
		command.octave.octave = note_view.octave;
		commands.insert(commands.begin() + *note_itr + 1, command);

		command.octave.octave = note_view.octave + 1;
		command.labels = std::move(commands[*note_itr].labels);
		commands.insert(commands.begin() + *note_itr, command);
	}

	postprocess(commands);

	_modified = true;
}

void Song::octave_down(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view) {
	remember(selected_channel, selected_boxes, Song_State::Action::OCTAVE_DOWN);
	std::vector<Command> &commands = channel_commands(selected_channel);

	for (auto note_itr = selected_notes.rbegin(); note_itr != selected_notes.rend(); ++note_itr) {
		Note_View note_view = find_note_view(view, *note_itr);
		Command command = Command(Command_Type::OCTAVE);
		command.octave.octave = note_view.octave;
		commands.insert(commands.begin() + *note_itr + 1, command);

		command.octave.octave = note_view.octave - 1;
		command.labels = std::move(commands[*note_itr].labels);
		commands.insert(commands.begin() + *note_itr, command);
	}

	postprocess(commands);

	_modified = true;
}

void Song::move_left(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes) {
	remember(selected_channel, selected_boxes, Song_State::Action::MOVE_LEFT);
	std::vector<Command> &commands = channel_commands(selected_channel);

	int32_t offset = 0;

	for (auto note_itr = selected_notes.begin(); note_itr != selected_notes.end(); ++note_itr) {
		auto command_itr = commands.rbegin() + (commands.size() - 1 - (*note_itr + offset));

		const auto find_preceding_rest = [&](decltype(command_itr) itr) {
			++itr;
			while (itr != commands.rend()) {
				if (itr->type == Command_Type::REST) {
					return (itr.base() - 1) - commands.begin();
				}
				++itr;
			}
			assert(false);
			return commands.end() - commands.begin();
		};

		int32_t rest_index = find_preceding_rest(command_itr);

		Command command = Command(Command_Type::REST);
		command.rest.length = 1;
		commands.insert(commands.begin() + (*note_itr + offset) + 1, command);
		if (commands[rest_index].rest.length == 1) {
			// TODO: these label's order is probably not being preserved...
			commands[rest_index + 1].labels.insert(commands[rest_index].labels.begin(), commands[rest_index].labels.end());
			commands.erase(commands.begin() + rest_index);
		}
		else {
			commands[rest_index].rest.length -= 1;
			offset += 1;
		}
	}

	postprocess(commands);

	_modified = true;
}

void Song::move_right(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes) {
	remember(selected_channel, selected_boxes, Song_State::Action::MOVE_RIGHT);
	std::vector<Command> &commands = channel_commands(selected_channel);

	for (auto note_itr = selected_notes.rbegin(); note_itr != selected_notes.rend(); ++note_itr) {
		auto command_itr = commands.begin() + *note_itr;

		const auto find_following_rest = [&](decltype(command_itr) itr) {
			++itr;
			while (itr != commands.end()) {
				if (itr->type == Command_Type::REST) {
					return itr - commands.begin();
				}
				++itr;
			}
			assert(false);
			return commands.end() - commands.begin();
		};

		int32_t rest_index = find_following_rest(command_itr);

		assert(commands[rest_index].labels.size() == 0);
		if (commands[rest_index].rest.length == 1) {
			commands.erase(commands.begin() + rest_index);
		}
		else {
			commands[rest_index].rest.length -= 1;
		}

		// TODO: this potentially puts a rest between note settings and note which is maybe not ideal?
		Command command = Command(Command_Type::REST);
		command.labels = std::move(commands[*note_itr].labels);
		command.rest.length = 1;
		commands.insert(commands.begin() + *note_itr, command);
	}

	postprocess(commands);

	_modified = true;
}

void Song::shorten(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes) {
	remember(selected_channel, selected_boxes, Song_State::Action::SHORTEN);
	std::vector<Command> &commands = channel_commands(selected_channel);

	for (auto note_itr = selected_notes.rbegin(); note_itr != selected_notes.rend(); ++note_itr) {
		Command command = Command(Command_Type::REST);
		command.rest.length = 1;
		commands.insert(commands.begin() + *note_itr + 1, command);
		commands[*note_itr].note.length -= 1;
	}

	postprocess(commands);

	_modified = true;
}

void Song::lengthen(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes) {
	remember(selected_channel, selected_boxes, Song_State::Action::LENGTHEN);
	std::vector<Command> &commands = channel_commands(selected_channel);

	for (auto note_itr = selected_notes.rbegin(); note_itr != selected_notes.rend(); ++note_itr) {
		auto command_itr = commands.begin() + *note_itr;

		const auto find_following_rest = [&](decltype(command_itr) itr) {
			++itr;
			while (itr != commands.end()) {
				if (itr->type == Command_Type::REST) {
					return itr - commands.begin();
				}
				++itr;
			}
			assert(false);
			return commands.end() - commands.begin();
		};

		int32_t rest_index = find_following_rest(command_itr);

		assert(commands[rest_index].labels.size() == 0);
		if (commands[rest_index].rest.length == 1) {
			commands.erase(commands.begin() + rest_index);
		}
		else {
			commands[rest_index].rest.length -= 1;
		}

		commands[*note_itr].note.length += 1;
	}

	postprocess(commands);

	_modified = true;
}

void Song::delete_selection(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes) {
	remember(selected_channel, selected_boxes, Song_State::Action::DELETE_SELECTION);
	std::vector<Command> &commands = channel_commands(selected_channel);

	for (auto note_itr = selected_notes.rbegin(); note_itr != selected_notes.rend(); ++note_itr) {
		commands[*note_itr].type = Command_Type::REST;
	}

	postprocess(commands);

	_modified = true;
}

void Song::snip_selection(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes) {
	remember(selected_channel, selected_boxes, Song_State::Action::SNIP_SELECTION);
	std::vector<Command> &commands = channel_commands(selected_channel);

	for (auto note_itr = selected_notes.rbegin(); note_itr != selected_notes.rend(); ++note_itr) {
		// TODO: these label's order is probably not being preserved...
		commands[*note_itr + 1].labels.insert(commands[*note_itr].labels.begin(), commands[*note_itr].labels.end());
		commands.erase(commands.begin() + *note_itr);
	}

	postprocess(commands);

	_modified = true;
}

std::string Song::commands_str(const std::vector<Command> &commands) const {
	const auto to_local_label = [](const std::string &label) {
		std::size_t dot = label.find_first_of(".");
		return dot != std::string::npos ? &label[dot] : label.c_str();
	};

	std::string str;
	for (const Command &command : commands) {
		for (const std::string &label : command.labels) {
			str = str + to_local_label(label) + ":\n";
		}
		str = str + "\t" + COMMAND_NAMES[(uint32_t)command.type];
		if (command.type == Command_Type::NOTE) {
			str = str + " " + PITCH_NAMES[(uint32_t)command.note.pitch] + ", " + std::to_string(command.note.length);
		}
		else if (command.type == Command_Type::DRUM_NOTE) {
			str = str + " " + std::to_string(command.drum_note.instrument) + ", " + std::to_string(command.drum_note.length);
		}
		else if (command.type == Command_Type::REST) {
			str = str + " " + std::to_string(command.rest.length);
		}
		else if (command.type == Command_Type::OCTAVE) {
			str = str + " " + std::to_string(command.octave.octave);
		}
		else if (command.type == Command_Type::NOTE_TYPE) {
			str = str + " " + std::to_string(command.note_type.speed) + ", " + std::to_string(command.note_type.volume) + ", " + std::to_string(command.note_type.fade);
		}
		else if (command.type == Command_Type::DRUM_SPEED) {
			str = str + " " + std::to_string(command.drum_speed.speed);
		}
		else if (command.type == Command_Type::TRANSPOSE) {
			str = str + " " + std::to_string(command.transpose.num_octaves) + ", " + std::to_string(command.transpose.num_pitches);
		}
		else if (command.type == Command_Type::TEMPO) {
			str = str + " " + std::to_string(command.tempo.tempo);
		}
		else if (command.type == Command_Type::DUTY_CYCLE) {
			str = str + " " + std::to_string(command.duty_cycle.duty);
		}
		else if (command.type == Command_Type::VOLUME_ENVELOPE) {
			str = str + " " + std::to_string(command.volume_envelope.volume) + ", " + std::to_string(command.volume_envelope.fade);
		}
		else if (command.type == Command_Type::PITCH_SWEEP) {
			str = str + " " + std::to_string(command.pitch_sweep.duration) + ", " + std::to_string(command.pitch_sweep.pitch_change);
		}
		else if (command.type == Command_Type::DUTY_CYCLE_PATTERN) {
			str = str + " " + std::to_string(command.duty_cycle_pattern.duty1) + ", " + std::to_string(command.duty_cycle_pattern.duty2) + ", " + std::to_string(command.duty_cycle_pattern.duty3) + ", " + std::to_string(command.duty_cycle_pattern.duty4);
		}
		else if (command.type == Command_Type::PITCH_SLIDE) {
			str = str + " " + std::to_string(command.pitch_slide.duration) + ", " + std::to_string(command.pitch_slide.octave) + ", " + PITCH_NAMES[(uint32_t)command.pitch_slide.pitch];
		}
		else if (command.type == Command_Type::VIBRATO) {
			str = str + " " + std::to_string(command.vibrato.delay) + ", " + std::to_string(command.vibrato.extent) + ", " + std::to_string(command.vibrato.rate);
		}
		else if (command.type == Command_Type::TOGGLE_NOISE) {
			str = str + " " + std::to_string(command.toggle_noise.drumkit);
		}
		else if (command.type == Command_Type::FORCE_STEREO_PANNING) {
			str = str + " " + (command.force_stereo_panning.left ? "TRUE" : "FALSE") + ", " + (command.force_stereo_panning.right ? "TRUE" : "FALSE");
		}
		else if (command.type == Command_Type::VOLUME) {
			str = str + " " + std::to_string(command.volume.left) + ", " + std::to_string(command.volume.right);
		}
		else if (command.type == Command_Type::PITCH_OFFSET) {
			str = str + " " + std::to_string(command.pitch_offset.offset);
		}
		else if (command.type == Command_Type::STEREO_PANNING) {
			str = str + " " + (command.stereo_panning.left ? "TRUE" : "FALSE") + ", " + (command.stereo_panning.right ? "TRUE" : "FALSE");
		}
		else if (command.type == Command_Type::SOUND_JUMP) {
			str = str + " " + to_local_label(command.target);
			str = str + "\n";
		}
		else if (command.type == Command_Type::SOUND_LOOP) {
			str = str + " " + std::to_string(command.sound_loop.loop_count) + ", " + to_local_label(command.target);
			if (command.sound_loop.loop_count == 0) {
				str = str + "\n";
			}
		}
		else if (command.type == Command_Type::SOUND_CALL) {
			str = str + " " + to_local_label(command.target);
		}
		else if (command.type == Command_Type::SOUND_RET) {
			str = str + "\n";
		}
		str += "\n";
	}
	rtrim(str);
	str += "\n";
	return str;
}

std::string Song::get_error_message(Parsed_Song parsed_song) const {
	switch (parsed_song.result()) {
	case Parsed_Song::Result::SONG_OK:
		return "OK.";
	case Parsed_Song::Result::SONG_BAD_FILE:
		return "Cannot open song file.";
	case Parsed_Song::Result::SONG_INVALID_HEADER:
		return "Invalid song header.";
	case Parsed_Song::Result::SONG_ENDED_PREMATURELY:
		return "Channel " + std::to_string(parsed_song.channel_number()) +
			": File ended prematurely.";
	case Parsed_Song::Result::SONG_UNRECOGNIZED_LABEL:
		return "Channel " + std::to_string(parsed_song.channel_number()) +
			": Unrecognized label: " + parsed_song.label();
	case Parsed_Song::Result::SONG_ILLEGAL_MACRO:
		return "Line " + std::to_string(parsed_song.line_number()) +
			": Channel " + std::to_string(parsed_song.channel_number()) +
			": Illegal macro for channel.";
	case Parsed_Song::Result::SONG_UNRECOGNIZED_MACRO:
		return "Line " + std::to_string(parsed_song.line_number()) +
			": Channel " + std::to_string(parsed_song.channel_number()) +
			": Unrecognized macro.";
	case Parsed_Song::Result::SONG_INVALID_MACRO_ARGUMENT:
		return "Line " + std::to_string(parsed_song.line_number()) +
			": Channel " + std::to_string(parsed_song.channel_number()) +
			": Invalid macro argument.";
	case Parsed_Song::Result::SONG_NULL:
		return "No *.asm file chosen.";
	default:
		return "Unspecified error.";
	}
}

const char *Song::get_action_message(Song_State::Action action) const {
	switch (action) {
	case Song_State::Action::PITCH_UP:
		return "Pitch up";
	case Song_State::Action::PITCH_DOWN:
		return "Pitch down";
	case Song_State::Action::OCTAVE_UP:
		return "Octave up";
	case Song_State::Action::OCTAVE_DOWN:
		return "Octave down";
	case Song_State::Action::MOVE_LEFT:
		return "Move left";
	case Song_State::Action::MOVE_RIGHT:
		return "Move right";
	case Song_State::Action::SHORTEN:
		return "Shorten";
	case Song_State::Action::LENGTHEN:
		return "Lengthen";
	case Song_State::Action::DELETE_SELECTION:
		return "Delete selection";
	case Song_State::Action::SNIP_SELECTION:
		return "Snip selection";
	default:
		return "Unspecified action";
	}
}

std::vector<Command> &Song::channel_commands(const int selected_channel) {
	assert(selected_channel >= 1 && selected_channel <= 4);
	if (selected_channel == 1) {
		return _channel_1_commands;
	}
	else if (selected_channel == 2) {
		return _channel_2_commands;
	}
	else if (selected_channel == 3) {
		return _channel_3_commands;
	}
	else {
		return _channel_4_commands;
	}
}
