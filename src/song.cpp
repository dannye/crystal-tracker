#include <cstdio>
#include <stack>

#include "song.h"
#include "parse-song.h"

const auto find_note_with_label(const std::list<Command> &commands, std::string label) {
	for (auto command_itr = commands.begin(); command_itr != commands.end(); ++command_itr) {
		if (command_itr->labels.count(label) > 0) {
			return command_itr;
		}
	}
	return commands.end(); // if this happens, we've already messed up
}

static std::list<Note_View> build_timeline(const std::list<Command> &commands) {
	std::list<Note_View> timeline;
	int32_t octave = 1;
	int32_t speed = 12;
	auto command_itr = commands.begin();
	std::stack<std::pair<decltype(command_itr), int32_t>> loop_stack;
	std::stack<decltype(command_itr)> call_stack;
	while (command_itr != commands.end()) {
		// TODO: handle all other commands...
		if (command_itr->type == Command_Type::NOTE) {
			timeline.push_back(Note_View{ command_itr->note.length, command_itr->note.pitch, octave, speed });
		}
		else if (command_itr->type == Command_Type::DRUM_NOTE) {
			timeline.push_back(Note_View{ command_itr->drum_note.length, (Pitch)(command_itr->drum_note.instrument % NUM_PITCHES), command_itr->drum_note.instrument / (int32_t)NUM_PITCHES + 1, speed });
		}
		else if (command_itr->type == Command_Type::REST) {
			timeline.push_back(Note_View{ command_itr->rest.length, Pitch::REST, octave, speed });
		}
		else if (command_itr->type == Command_Type::OCTAVE) {
			octave = command_itr->octave.octave;
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
	return std::move(timeline);
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
	_result = Result::SONG_NULL;
	_loaded = false;
}

Song::Result Song::read_song(const char *f) {
	Parsed_Song data(f);
	if (data.result() != Parsed_Song::Result::SONG_OK) {
		return (_result = Result::SONG_BAD_FILE); // cannot parse file
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
	_channel_1_timeline = build_timeline(_channel_1_commands);
	_channel_2_timeline = build_timeline(_channel_2_commands);
	_channel_3_timeline = build_timeline(_channel_3_commands);
	_channel_4_timeline = build_timeline(_channel_4_commands);

	_loaded = true;
	return (_result = Result::SONG_OK);
}

std::string Song::commands_str(const std::list<Command> &commands) const {
	std::string str;
	for (const Command &command : commands) {
		str += COMMAND_NAMES[(uint32_t)command.type];
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
			str = str + " " + std::to_string(command.force_stereo_panning.left) + ", " + std::to_string(command.force_stereo_panning.right);
		}
		else if (command.type == Command_Type::VOLUME) {
			str = str + " " + std::to_string(command.volume.left) + ", " + std::to_string(command.volume.right);
		}
		else if (command.type == Command_Type::PITCH_OFFSET) {
			str = str + " " + std::to_string(command.pitch_offset.offset);
		}
		else if (command.type == Command_Type::STEREO_PANNING) {
			str = str + " " + std::to_string(command.stereo_panning.left) + ", " + std::to_string(command.stereo_panning.right);
		}
		else if (command.type == Command_Type::SOUND_JUMP) {
			str = str + " " + command.target;
		}
		else if (command.type == Command_Type::SOUND_LOOP) {
			str = str + " " + std::to_string(command.sound_loop.loop_count) + ", " + command.target;
		}
		else if (command.type == Command_Type::SOUND_CALL) {
			str = str + " " + command.target;
		}
		str += "\n";
	}
	return str;
}

const char *Song::error_message(Result result) {
	// TODO: add specific error messages for each kind of parsing error
	switch (result) {
	case Result::SONG_OK:
		return "OK.";
	case Result::SONG_BAD_FILE:
		return "Not a valid song file.";
	case Result::SONG_NULL:
		return "No *.asm file chosen.";
	default:
		return "Unspecified error.";
	}
}
