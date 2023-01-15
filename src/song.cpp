#include <algorithm>
#include <cassert>
#include <cstdio>
#include <fstream>
#include <map>
#include <stack>

#include "song.h"

std::vector<Command>::const_iterator find_note_with_label(const std::vector<Command> &commands, std::string label) {
	for (auto command_itr = commands.begin(); command_itr != commands.end(); ++command_itr) {
		if (std::count(RANGE(command_itr->labels), label) > 0) {
			return command_itr;
		}
	}
	assert(false);
	return commands.end();
}

Parsed_Song::Result calc_channel_length(const std::vector<Command> &commands, int32_t &loop_tick, int32_t &end_tick, Extra_Info *info) {
	int32_t tick = 0;
	loop_tick = -1;
	end_tick = -1;
	int32_t speed = 1;
	int32_t volume = 0;
	int32_t fade = 0;

	struct Label_Info {
		int32_t tick = 0;
		int32_t index = 0;
		int32_t speed = 0;
		int32_t volume = 0;
		int32_t fade = 0;
	};

	auto command_itr = commands.begin();

	std::stack<std::pair<decltype(command_itr), int32_t>> loop_stack;
	std::stack<decltype(command_itr)> call_stack;
	std::set<std::string> visited_labels_during_call;
	std::map<std::string, Label_Info> label_infos;

	while (command_itr != commands.end()) {
		for (const std::string &label : command_itr->labels) {
			label_infos.insert({ label, { tick, (int32_t)(command_itr - commands.begin()), speed, volume, fade } });
			if (call_stack.size() > 0) {
				visited_labels_during_call.insert(label);
			}
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
			volume = command_itr->note_type.volume;
			fade = command_itr->note_type.fade;
		}
		else if (command_itr->type == Command_Type::DRUM_SPEED) {
			speed = command_itr->drum_speed.speed;
		}
		else if (command_itr->type == Command_Type::VOLUME_ENVELOPE) {
			volume = command_itr->volume_envelope.volume;
			fade = command_itr->volume_envelope.fade;
		}
		else if (command_itr->type == Command_Type::SOUND_JUMP) {
			if (
				!label_infos.count(command_itr->target) ||
				(call_stack.size() > 0 && !visited_labels_during_call.count(command_itr->target))
			) {
				command_itr = find_note_with_label(commands, command_itr->target);
				continue;
			}
			const Label_Info &label_info = label_infos.at(command_itr->target);
			loop_tick = label_info.tick;
			end_tick = tick;
			if (info) {
				info->loop_index = label_info.index;
				info->speed_at_loop = label_info.speed;
				info->volume_at_loop = label_info.volume;
				info->fade_at_loop = label_info.fade;

				info->end_index = command_itr - commands.begin();
				info->speed_at_end = speed;
				info->volume_at_end = volume;
				info->fade_at_end = fade;
			}
			if (loop_tick == end_tick) {
				return Parsed_Song::Result::SONG_EMPTY_LOOP;
			}
			break; // song is finished
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
					if (
						!label_infos.count(command_itr->target) ||
						(call_stack.size() > 0 && !visited_labels_during_call.count(command_itr->target))
					) {
						command_itr = find_note_with_label(commands, command_itr->target);
						continue;
					}
					const Label_Info &label_info = label_infos.at(command_itr->target);
					loop_tick = label_info.tick;
					end_tick = tick;
					if (info) {
						info->loop_index = label_info.index;
						info->speed_at_loop = label_info.speed;
						info->volume_at_loop = label_info.volume;
						info->fade_at_loop = label_info.fade;

						info->end_index = command_itr - commands.begin();
						info->speed_at_end = speed;
						info->volume_at_end = volume;
						info->fade_at_end = fade;
					}
					if (loop_tick == end_tick) {
						return Parsed_Song::Result::SONG_EMPTY_LOOP;
					}
					break; // song is finished
				}
				else if (command_itr->sound_loop.loop_count > 1) {
					// nested loops not allowed
					if (loop_stack.size() > 0) {
						return Parsed_Song::Result::SONG_NESTED_LOOP;
					}

					loop_stack.emplace(command_itr, command_itr->sound_loop.loop_count - 1);
					command_itr = find_note_with_label(commands, command_itr->target);
					continue;
				}
			}
		}
		else if (command_itr->type == Command_Type::SOUND_CALL) {
			// nested calls not allowed
			if (call_stack.size() > 0) {
				return Parsed_Song::Result::SONG_NESTED_CALL;
			}

			call_stack.push(command_itr);
			command_itr = find_note_with_label(commands, command_itr->target);
			continue;
		}
		else if (command_itr->type == Command_Type::SOUND_RET) {
			if (call_stack.size() == 0) {
				end_tick = tick;
				if (info) {
					info->end_index = command_itr - commands.begin();
					info->speed_at_end = speed;
					info->volume_at_end = volume;
					info->fade_at_end = fade;
				}
				break; // song is finished
			}
			else {
				command_itr = call_stack.top();
				call_stack.pop();
				visited_labels_during_call.clear();
			}
		}
		else if (command_itr->type == Command_Type::SPEED) {
			speed = command_itr->speed.speed;
		}
		else if (command_itr->type == Command_Type::CHANNEL_VOLUME) {
			volume = command_itr->channel_volume.volume;
		}
		else if (command_itr->type == Command_Type::FADE_WAVE) {
			fade = command_itr->fade_wave.fade;
		}
		++command_itr;
	}

	return Parsed_Song::Result::SONG_OK;
}

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
	_channel_1_loop_tick = -1;
	_channel_2_loop_tick = -1;
	_channel_3_loop_tick = -1;
	_channel_4_loop_tick = -1;
	_channel_1_end_tick = -1;
	_channel_2_end_tick = -1;
	_channel_3_end_tick = -1;
	_channel_4_end_tick = -1;
	_waves.clear();
	_result = Parsed_Song::Result::SONG_NULL;
	_modified = false;
	_mod_time = 0;
	_history.clear();
	_future.clear();
	_loaded = false;
}

void Song::remember(int channel_number, const std::set<int32_t> &selection, Song_State::Action action, int tick) {
	_future.clear();
	while (_history.size() >= MAX_HISTORY_SIZE) { _history.pop_front(); }

	std::vector<Command> &commands = channel_commands(channel_number);

	Song_State ss;
	ss.tick = tick;
	ss.channel_number = channel_number;
	ss.commands = commands;
	ss.selection = selection;
	ss.action = action;
	_history.push_back(ss);
}

void Song::undo() {
	if (_history.empty()) { return; }
	while (_future.size() >= MAX_HISTORY_SIZE) { _future.pop_front(); }

	Song_State &prev = _history.back();
	std::vector<Command> &commands = channel_commands(prev.channel_number);

	Song_State ss;
	ss.tick = prev.tick;
	ss.channel_number = prev.channel_number;
	ss.commands = std::move(commands);
	ss.selection = std::move(prev.selection);
	ss.action = prev.action;
	_future.push_back(ss);

	commands = std::move(prev.commands);
	_history.pop_back();

	_modified = true;
}

void Song::redo() {
	if (_future.empty()) { return; }
	while (_history.size() >= MAX_HISTORY_SIZE) { _history.pop_front(); }

	Song_State &next = _future.back();
	std::vector<Command> &commands = channel_commands(next.channel_number);

	Song_State ss;
	ss.tick = next.tick;
	ss.channel_number = next.channel_number;
	ss.commands = std::move(commands);
	ss.selection = std::move(next.selection);
	ss.action = next.action;
	_history.push_back(ss);

	commands = std::move(next.commands);
	_future.pop_back();

	_modified = true;
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
	_channel_1_loop_tick = data.channel_1_loop_tick();
	_channel_2_loop_tick = data.channel_2_loop_tick();
	_channel_3_loop_tick = data.channel_3_loop_tick();
	_channel_4_loop_tick = data.channel_4_loop_tick();
	_channel_1_end_tick = data.channel_1_end_tick();
	_channel_2_end_tick = data.channel_2_end_tick();
	_channel_3_end_tick = data.channel_3_end_tick();
	_channel_4_end_tick = data.channel_4_end_tick();
	_waves = data.waves();

	_mod_time = file_modified(f);

	_loaded = true;
	return (_result = Parsed_Song::Result::SONG_OK);
}

void Song::new_song(Song_Options_Dialog::Song_Options options) {
	auto build_channel = [](std::vector<Command> &commands, const std::string &channel_label, int channel_number, bool first_channel, int32_t loop_tick, int32_t end_tick) {
		Command command;
		command.labels.push_back(channel_label);

		if (first_channel) {
			command.type = Command_Type::TEMPO;
			command.tempo.tempo = 256;
			commands.push_back(command);
			command.labels.clear();

			command.type = Command_Type::VOLUME;
			command.volume.left = 7;
			command.volume.right = 7;
			commands.push_back(command);
		}
		if (channel_number == 1 || channel_number == 2) {
			command.type = Command_Type::NOTE_TYPE;
			command.note_type.speed = 12;
			command.note_type.volume = 15;
			command.note_type.fade = 0;
			commands.push_back(command);
			command.labels.clear();
		}
		else if (channel_number == 3) {
			command.type = Command_Type::NOTE_TYPE;
			command.note_type.speed = 12;
			command.note_type.volume = 1;
			command.note_type.wave = 0;
			commands.push_back(command);
			command.labels.clear();
		}
		else {
			command.type = Command_Type::TOGGLE_NOISE;
			command.toggle_noise.drumkit = 0;
			commands.push_back(command);
			command.labels.clear();

			command.type = Command_Type::DRUM_SPEED;
			command.drum_speed.speed = 12;
			commands.push_back(command);
		}

		auto insert_ticks = [channel_number](std::vector<Command> &commands, Command &command, int32_t ticks_to_insert) {
			int32_t speed_ticks_to_insert = ticks_to_insert / 12;
			int32_t rem_ticks_to_insert = ticks_to_insert % 12;

			command.type = Command_Type::REST;
			command.rest.length = 16;

			while (speed_ticks_to_insert) {
				if (speed_ticks_to_insert < 16) command.rest.length = speed_ticks_to_insert;
				commands.push_back(command);
				command.labels.clear();
				speed_ticks_to_insert -= command.rest.length;
			}
			if (rem_ticks_to_insert > 0) {
				if (channel_number == 1 || channel_number == 2) {
					command.type = Command_Type::NOTE_TYPE;
					command.note_type.speed = 1;
					command.note_type.volume = 15;
					command.note_type.fade = 0;
				}
				else if (channel_number == 3) {
					command.type = Command_Type::NOTE_TYPE;
					command.note_type.speed = 1;
					command.note_type.volume = 1;
					command.note_type.fade = 0;
				}
				else {
					command.type = Command_Type::DRUM_SPEED;
					command.drum_speed.speed = 1;
				}
				commands.push_back(command);
				command.labels.clear();

				command.type = Command_Type::REST;
				command.rest.length = rem_ticks_to_insert;
				commands.push_back(command);

				if (channel_number == 1 || channel_number == 2) {
					command.type = Command_Type::NOTE_TYPE;
					command.note_type.speed = 12;
					command.note_type.volume = 15;
					command.note_type.fade = 0;
				}
				else if (channel_number == 3) {
					command.type = Command_Type::NOTE_TYPE;
					command.note_type.speed = 12;
					command.note_type.volume = 1;
					command.note_type.fade = 0;
				}
				else {
					command.type = Command_Type::DRUM_SPEED;
					command.drum_speed.speed = 12;
				}
				commands.push_back(command);
			}
		};

		if (loop_tick != -1) {
			insert_ticks(commands, command, loop_tick);
			command.labels.push_back(channel_label + ".mainLoop");
		}

		int32_t body_length = loop_tick != -1 ? end_tick - loop_tick : end_tick;
		insert_ticks(commands, command, body_length);

		if (loop_tick != -1) {
			command.type = Command_Type::SOUND_LOOP;
			command.target = channel_label + ".mainLoop";
			command.sound_loop.loop_count = 0;
			commands.push_back(command);
		}
		else {
			command.type = Command_Type::SOUND_RET;
			commands.push_back(command);
		}
	};

	_song_name = "Music_" + options.song_name;
	_number_of_channels = 0;
	if (options.channel_1) {
		_number_of_channels += 1;
		_channel_1_label = _song_name + "_Ch1";
		if (options.looping) _channel_1_loop_tick = options.channel_1_loop_tick;
		_channel_1_end_tick = options.channel_1_end_tick;
	}
	if (options.channel_2) {
		_number_of_channels += 1;
		_channel_2_label = _song_name + "_Ch2";
		if (options.looping) _channel_2_loop_tick = options.channel_2_loop_tick;
		_channel_2_end_tick = options.channel_2_end_tick;
	}
	if (options.channel_3) {
		_number_of_channels += 1;
		_channel_3_label = _song_name + "_Ch3";
		if (options.looping) _channel_3_loop_tick = options.channel_3_loop_tick;
		_channel_3_end_tick = options.channel_3_end_tick;
	}
	if (options.channel_4) {
		_number_of_channels += 1;
		_channel_4_label = _song_name + "_Ch4";
		if (options.looping) _channel_4_loop_tick = options.channel_4_loop_tick;
		_channel_4_end_tick = options.channel_4_end_tick;
	}

	bool first_channel = true;
	if (options.channel_1) {
		build_channel(_channel_1_commands, _channel_1_label, 1, first_channel, _channel_1_loop_tick, _channel_1_end_tick);
		first_channel = false;
	}
	if (options.channel_2) {
		build_channel(_channel_2_commands, _channel_2_label, 2, first_channel, _channel_2_loop_tick, _channel_2_end_tick);
		first_channel = false;
	}
	if (options.channel_3) {
		build_channel(_channel_3_commands, _channel_3_label, 3, first_channel, _channel_3_loop_tick, _channel_3_end_tick);
		first_channel = false;
	}
	if (options.channel_4) {
		build_channel(_channel_4_commands, _channel_4_label, 4, first_channel, _channel_4_loop_tick, _channel_4_end_tick);
		first_channel = false;
	}

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
		ofs << '\n' << commands_str(_channel_1_commands, 1);
	}
	if (_channel_2_label.size()) {
		ofs << '\n' << commands_str(_channel_2_commands, 2);
	}
	if (_channel_3_label.size()) {
		ofs << '\n' << commands_str(_channel_3_commands, 3);
	}
	if (_channel_4_label.size()) {
		ofs << '\n' << commands_str(_channel_4_commands, 4);
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
						assert(commands[i].labels.size() == 0);
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
						assert(commands[i].labels.size() == 0);
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
					commands[octave_index + 1].labels.insert(commands[octave_index + 1].labels.begin(), RANGE(commands[octave_index].labels));
					commands.erase(commands.begin() + octave_index);
					i -= 1;
					if (rest_index > octave_index) {
						rest_index -= 1;
					}

					view.octave = commands[i].octave.octave;
					octave_index = i;
				}
			}
			else if (commands[i].type == Command_Type::INC_OCTAVE || commands[i].type == Command_Type::DEC_OCTAVE) {
				view.octave = 0;
				octave_index = -1;
			}
		}
	} while (deleted);
}

void Song::put_note(const int selected_channel, const std::set<int32_t> &selected_boxes, Pitch pitch, int32_t index, int32_t tick, int32_t tick_offset) {
	remember(selected_channel, selected_boxes, Song_State::Action::PUT_NOTE, tick);
	std::vector<Command> &commands = channel_commands(selected_channel);

	Command_Type new_type =
		pitch == Pitch::REST ? Command_Type::REST :
		selected_channel == 4 ? Command_Type::DRUM_NOTE :
		Command_Type::NOTE;

	assert(
		commands[index].type == Command_Type::NOTE ||
		commands[index].type == Command_Type::DRUM_NOTE ||
		commands[index].type == Command_Type::REST
	);

	Command_Type old_type = commands[index].type;
	int32_t old_length = commands[index].note.length;
	Pitch old_pitch = commands[index].note.pitch;

	if (tick_offset == 0) {
		commands[index].type = new_type;
		commands[index].note.pitch = pitch;
		commands[index].note.length = 1;
	}
	else {
		commands[index].note.length = tick_offset;

		Command command = Command(new_type);
		command.note.length = 1;
		command.note.pitch = pitch;
		commands.insert(commands.begin() + index + 1, command);

		old_length -= tick_offset;
		index += 1;
	}

	if (old_length > 1) {
		Command command = Command(old_type);
		command.note.length = old_length - 1;
		command.note.pitch = old_pitch;
		commands.insert(commands.begin() + index + 1, command);
	}

	postprocess(commands);

	_modified = true;
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
			commands[*note_itr].labels.clear();
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
			commands[*note_itr].labels.clear();
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
		commands[*note_itr].labels.clear();
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
		commands[*note_itr].labels.clear();
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
		assert(commands[*note_itr + offset].labels.size() == 0);

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
			commands[rest_index + 1].labels.insert(commands[rest_index + 1].labels.begin(), RANGE(commands[rest_index].labels));
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
		commands[*note_itr].labels.clear();
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
		commands[*note_itr + 1].labels.insert(commands[*note_itr + 1].labels.begin(), RANGE(commands[*note_itr].labels));
		commands.erase(commands.begin() + *note_itr);
	}

	int32_t loop_tick = channel_loop_tick(selected_channel);
	int32_t end_tick = channel_end_tick(selected_channel);

	int32_t new_loop_tick, new_end_tick;
	Extra_Info info;
	calc_channel_length(commands, new_loop_tick, new_end_tick, &info);
	assert(new_loop_tick != -1 || loop_tick == -1);
	assert(new_loop_tick <= loop_tick);
	assert(new_end_tick != -1);
	assert(new_end_tick < end_tick);

	auto insert_ticks = [selected_channel](std::vector<Command> &commands, int32_t ticks_to_insert, int32_t index, int32_t speed, int32_t volume, int32_t fade) {
		int32_t speed_ticks_to_insert = ticks_to_insert / speed;
		int32_t rem_ticks_to_insert = ticks_to_insert % speed;

		Command rest = Command(Command_Type::REST);
		rest.rest.length = 16;

		while (speed_ticks_to_insert) {
			if (speed_ticks_to_insert < 16) rest.rest.length = speed_ticks_to_insert;
			commands.insert(commands.begin() + index, rest);
			speed_ticks_to_insert -= rest.rest.length;
		}
		if (rem_ticks_to_insert > 0) {
			Command command = Command(selected_channel == 4 ? Command_Type::DRUM_SPEED : Command_Type::NOTE_TYPE);
			command.note_type.speed = speed;
			command.note_type.volume = volume;
			command.note_type.fade = fade;
			commands.insert(commands.begin() + index, command);

			rest.rest.length = rem_ticks_to_insert;
			commands.insert(commands.begin() + index, rest);

			command.note_type.speed = 1;
			commands.insert(commands.begin() + index, command);
		}
	};

	int32_t ticks_to_insert_at_loop = loop_tick - new_loop_tick;
	int32_t ticks_to_insert_at_end = end_tick - new_end_tick - ticks_to_insert_at_loop;

	insert_ticks(commands, ticks_to_insert_at_end,  info.end_index,  info.speed_at_end,  info.volume_at_end,  info.fade_at_end);
	insert_ticks(commands, ticks_to_insert_at_loop, info.loop_index, info.speed_at_loop, info.volume_at_loop, info.fade_at_loop);

	// TODO: merge redundant note_type commands
	postprocess(commands);

	_modified = true;
}

void Song::split_note(const int selected_channel, const std::set<int32_t> &selected_boxes, int32_t index, int32_t tick, int32_t tick_offset) {
	remember(selected_channel, selected_boxes, Song_State::Action::SPLIT_NOTE, tick);
	std::vector<Command> &commands = channel_commands(selected_channel);

	assert(
		commands[index].type == Command_Type::NOTE ||
		commands[index].type == Command_Type::DRUM_NOTE
	);
	assert(tick_offset != 0);

	Command command = Command(commands[index].type);
	command.note.length = commands[index].note.length - tick_offset;
	command.note.pitch = commands[index].note.pitch;
	commands[index].note.length = tick_offset;
	commands.insert(commands.begin() + index + 1, command);

	postprocess(commands);

	_modified = true;
}

void Song::glue_note(const int selected_channel, const std::set<int32_t> &selected_boxes, int32_t index, int32_t tick) {
	remember(selected_channel, selected_boxes, Song_State::Action::GLUE_NOTE, tick);
	std::vector<Command> &commands = channel_commands(selected_channel);

	assert(index > 0);
	assert(
		commands[index].type == Command_Type::NOTE ||
		commands[index].type == Command_Type::DRUM_NOTE
	);
	assert(
		commands[index].type == commands[index - 1].type &&
		commands[index].note.pitch  == commands[index - 1].note.pitch &&
		commands[index].note.length + commands[index - 1].note.length <= 16 &&
		commands[index].labels.size() == 0
	);

	commands[index - 1].note.length += commands[index].note.length;
	commands.erase(commands.begin() + index);

	postprocess(commands);

	_modified = true;
}

std::string Song::commands_str(const std::vector<Command> &commands, int32_t channel_number) const {
	const auto to_local_label = [](const std::string &label, const std::string &scope) {
		std::size_t dot = label.find_first_of(".");
		return dot != std::string::npos && label.substr(0, dot) == scope ? &label[dot] : label.c_str();
	};

	std::string current_scope;

	std::string str;
	for (const Command &command : commands) {
		for (const std::string &label : command.labels) {
			std::size_t dot = label.find_first_of(".");
			if (dot == std::string::npos) {
				current_scope = label;
			}
			str = str + to_local_label(label, current_scope) + ":\n";
		}

		str = str + "\t" + COMMAND_NAMES[(uint32_t)command.type];

		if (command.type == Command_Type::NOTE) {
			str = str + " " + PITCH_NAMES[(uint32_t)command.note.pitch] +
						", " + std::to_string(command.note.length);
		}

		else if (command.type == Command_Type::DRUM_NOTE) {
			str = str + " " + std::to_string(command.drum_note.instrument) +
						", " + std::to_string(command.drum_note.length);
		}

		else if (command.type == Command_Type::REST) {
			str = str + " " + std::to_string(command.rest.length);
		}

		else if (command.type == Command_Type::OCTAVE) {
			str = str + " " + std::to_string(command.octave.octave);
		}

		else if (command.type == Command_Type::NOTE_TYPE) {
			str = str + " " + std::to_string(command.note_type.speed) +
						", " + std::to_string(command.note_type.volume) +
						", " + ((channel_number == 1 || channel_number == 2) && command.note_type.fade == 0 ? "8" : std::to_string(command.note_type.fade));
		}

		else if (command.type == Command_Type::DRUM_SPEED) {
			str = str + " " + std::to_string(command.drum_speed.speed);
		}

		else if (command.type == Command_Type::TRANSPOSE) {
			str = str + " " + std::to_string(command.transpose.num_octaves) +
						", " + std::to_string(command.transpose.num_pitches);
		}

		else if (command.type == Command_Type::TEMPO) {
			str = str + " " + std::to_string(command.tempo.tempo);
		}

		else if (command.type == Command_Type::DUTY_CYCLE) {
			str = str + " " + std::to_string(command.duty_cycle.duty);
		}

		else if (command.type == Command_Type::VOLUME_ENVELOPE) {
			str = str + " " + std::to_string(command.volume_envelope.volume) +
						", " + ((channel_number == 1 || channel_number == 2) && command.volume_envelope.fade == 0 ? "8" : std::to_string(command.volume_envelope.fade));
		}

		else if (command.type == Command_Type::PITCH_SWEEP) {
			str = str + " " + std::to_string(command.pitch_sweep.duration) +
						", " + (command.pitch_sweep.pitch_change == 0 ? "8" : std::to_string(command.pitch_sweep.pitch_change));
		}

		else if (command.type == Command_Type::DUTY_CYCLE_PATTERN) {
			str = str + " " + std::to_string(command.duty_cycle_pattern.duty1) +
						", " + std::to_string(command.duty_cycle_pattern.duty2) +
						", " + std::to_string(command.duty_cycle_pattern.duty3) +
						", " + std::to_string(command.duty_cycle_pattern.duty4);
		}

		else if (command.type == Command_Type::PITCH_SLIDE) {
			str = str + " " + std::to_string(command.pitch_slide.duration) +
						", " + std::to_string(command.pitch_slide.octave) +
						", " + PITCH_NAMES[(uint32_t)command.pitch_slide.pitch];
		}

		else if (command.type == Command_Type::VIBRATO) {
			str = str + " " + std::to_string(command.vibrato.delay) +
						", " + std::to_string(command.vibrato.extent) +
						", " + std::to_string(command.vibrato.rate);
		}

		else if (command.type == Command_Type::TOGGLE_NOISE) {
			// TODO: handle true toggling
			str = str + " " + std::to_string(command.toggle_noise.drumkit);
		}

		else if (command.type == Command_Type::FORCE_STEREO_PANNING) {
			str = str + " " + (command.force_stereo_panning.left ? "TRUE" : "FALSE") +
						", " + (command.force_stereo_panning.right ? "TRUE" : "FALSE");
		}

		else if (command.type == Command_Type::VOLUME) {
			str = str + " " + std::to_string(command.volume.left) +
						", " + std::to_string(command.volume.right);
		}

		else if (command.type == Command_Type::PITCH_OFFSET) {
			str = str + " " + std::to_string(command.pitch_offset.offset);
		}

		else if (command.type == Command_Type::STEREO_PANNING) {
			str = str + " " + (command.stereo_panning.left ? "TRUE" : "FALSE") +
						", " + (command.stereo_panning.right ? "TRUE" : "FALSE");
		}

		else if (command.type == Command_Type::SOUND_JUMP) {
			str = str + " " + to_local_label(command.target, current_scope);
			str = str + "\n";
		}

		else if (command.type == Command_Type::SOUND_LOOP) {
			str = str + " " + std::to_string(command.sound_loop.loop_count) +
						", " + to_local_label(command.target, current_scope);
			if (command.sound_loop.loop_count == 0) {
				str = str + "\n";
			}
		}

		else if (command.type == Command_Type::SOUND_CALL) {
			str = str + " " + to_local_label(command.target, current_scope);
		}

		else if (command.type == Command_Type::SOUND_RET) {
			str = str + "\n";
		}

		else if (command.type == Command_Type::TOGGLE_PERFECT_PITCH) {}

		else if (command.type == Command_Type::LOAD_WAVE) {
			const Wave &wave = _waves[command.load_wave.wave - 0x10];
			for (size_t i = 0; i < NUM_WAVE_SAMPLES; ++i) {
				str = str + " " + std::to_string(wave[i]);
				if (i != NUM_WAVE_SAMPLES - 1) {
					str = str + ",";
				}
			}
		}

		else if (command.type == Command_Type::INC_OCTAVE) {}

		else if (command.type == Command_Type::DEC_OCTAVE) {}

		else if (command.type == Command_Type::SPEED) {
			str = str + " " + std::to_string(command.speed.speed);
		}

		else if (command.type == Command_Type::CHANNEL_VOLUME) {
			str = str + " " + std::to_string(command.channel_volume.volume);
		}

		else if (command.type == Command_Type::FADE_WAVE) {
			str = str + " " + ((channel_number == 1 || channel_number == 2) && command.fade_wave.fade == 0 ? "8" : std::to_string(command.fade_wave.fade));
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
	case Parsed_Song::Result::SONG_EMPTY_LOOP:
		return "Channel " + std::to_string(parsed_song.channel_number()) +
			": Empty infinite loop.";
	case Parsed_Song::Result::SONG_NESTED_LOOP:
		return "Channel " + std::to_string(parsed_song.channel_number()) +
			": Nested loops not allowed.";
	case Parsed_Song::Result::SONG_NESTED_CALL:
		return "Channel " + std::to_string(parsed_song.channel_number()) +
			": Nested calls not allowed.";
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
	case Song_State::Action::PUT_NOTE:
		return "Put note";
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
	case Song_State::Action::SPLIT_NOTE:
		return "Split note";
	case Song_State::Action::GLUE_NOTE:
		return "Glue note";
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

int32_t Song::channel_loop_tick(const int selected_channel) const {
	assert(selected_channel >= 1 && selected_channel <= 4);
	if (selected_channel == 1) {
		return _channel_1_loop_tick;
	}
	else if (selected_channel == 2) {
		return _channel_2_loop_tick;
	}
	else if (selected_channel == 3) {
		return _channel_3_loop_tick;
	}
	else {
		return _channel_4_loop_tick;
	}
}

int32_t Song::channel_end_tick(const int selected_channel) const {
	assert(selected_channel >= 1 && selected_channel <= 4);
	if (selected_channel == 1) {
		return _channel_1_end_tick;
	}
	else if (selected_channel == 2) {
		return _channel_2_end_tick;
	}
	else if (selected_channel == 3) {
		return _channel_3_end_tick;
	}
	else {
		return _channel_4_end_tick;
	}
}

int32_t Song::max_wave_id() const {
	int32_t max_wave = -1;
	bool inline_waves = _waves.size() > 0;
	for (const Command &command : _channel_3_commands) {
		if (
			command.type == Command_Type::NOTE_TYPE &&
			command.note_type.wave > max_wave &&
			(command.note_type.wave != 0x0f || !inline_waves)
		) {
			max_wave = command.note_type.wave;
		}
		else if (
			command.type == Command_Type::VOLUME_ENVELOPE &&
			command.volume_envelope.wave > max_wave &&
			(command.volume_envelope.wave != 0x0f || !inline_waves)
		) {
			max_wave = command.volume_envelope.wave;
		}
		else if (
			command.type == Command_Type::FADE_WAVE &&
			command.fade_wave.wave > max_wave &&
			(command.fade_wave.wave != 0x0f || !inline_waves)
		) {
			max_wave = command.fade_wave.wave;
		}
	}
	return max_wave;
}
