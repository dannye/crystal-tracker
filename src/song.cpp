#include <cstdio>
#include <fstream>
#include <map>
#include <stack>

#include "song.h"

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

static std::list<Note_View> build_timeline(const std::list<Command> &commands, int32_t end_tick) {
	int32_t tick = 0;
	std::list<Note_View> timeline;
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
	_channel_1_timeline.clear();
	_channel_2_timeline.clear();
	_channel_3_timeline.clear();
	_channel_4_timeline.clear();
	_result = Parsed_Song::Result::SONG_NULL;
	_modified = false;
	_mod_time = 0;
	_loaded = false;
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
	calc_channel_length(_channel_1_commands, _channel_1_loop_tick, _channel_1_end_tick);
	calc_channel_length(_channel_2_commands, _channel_2_loop_tick, _channel_2_end_tick);
	calc_channel_length(_channel_3_commands, _channel_3_loop_tick, _channel_3_end_tick);
	calc_channel_length(_channel_4_commands, _channel_4_loop_tick, _channel_4_end_tick);
	int32_t song_length = std::max({ _channel_1_end_tick, _channel_2_end_tick, _channel_3_end_tick, _channel_4_end_tick });
	_channel_1_timeline = build_timeline(_channel_1_commands, song_length);
	_channel_2_timeline = build_timeline(_channel_2_commands, song_length);
	_channel_3_timeline = build_timeline(_channel_3_commands, song_length);
	_channel_4_timeline = build_timeline(_channel_4_commands, song_length);

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
	calc_channel_length(_channel_1_commands, _channel_1_loop_tick, _channel_1_end_tick);
	calc_channel_length(_channel_2_commands, _channel_2_loop_tick, _channel_2_end_tick);
	calc_channel_length(_channel_3_commands, _channel_3_loop_tick, _channel_3_end_tick);
	calc_channel_length(_channel_4_commands, _channel_4_loop_tick, _channel_4_end_tick);
	int32_t song_length = std::max({ _channel_1_end_tick, _channel_2_end_tick, _channel_3_end_tick, _channel_4_end_tick });
	_channel_1_timeline = build_timeline(_channel_1_commands, song_length);
	_channel_2_timeline = build_timeline(_channel_2_commands, song_length);
	_channel_3_timeline = build_timeline(_channel_3_commands, song_length);
	_channel_4_timeline = build_timeline(_channel_4_commands, song_length);

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

int32_t Song::get_loop_tick() const {
	// TODO: this naive approach works in many cases but this will need to be improved (see: mainmenu.asm, lookhiker.asm)
	int32_t loop_tick = std::max({ _channel_1_loop_tick, _channel_2_loop_tick, _channel_3_loop_tick, _channel_4_loop_tick });
	return loop_tick;
}

std::string Song::commands_str(const std::list<Command> &commands) const {
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

std::string Song::get_error_message(Parsed_Song parsed_song) {
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
