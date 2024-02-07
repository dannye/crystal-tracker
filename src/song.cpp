#include <algorithm>
#include <cassert>
#include <cstdio>
#include <fstream>
#include <map>
#include <stack>

#include "song.h"

std::string get_scope(const std::vector<Command> &commands, int32_t index) {
	for (auto command_itr = commands.rbegin() + (commands.size() - 1 - index); command_itr != commands.rend(); ++command_itr) {
		for (auto label_itr = command_itr->labels.rbegin(); label_itr != command_itr->labels.rend(); ++label_itr) {
			if (label_itr->find_first_of(".") == std::string::npos) {
				return *label_itr;
			}
		}
	}
	assert(false);
	return "";
}

std::string get_next_label(const std::vector<Command> &commands, const std::string &scope, const std::string &prefix, int &i) {
	const auto is_label_taken = [](const std::vector<Command> &commands, const std::string &label) {
		for (const Command &command : commands) {
			for (const std::string &l : command.labels) {
				if (l == label) {
					return true;
				}
			}
		}
		return false;
	};

	while (true) {
		std::string label = scope + "." + prefix + std::to_string(i);
		i += 1;
		if (is_label_taken(commands, label)) {
			continue;
		}
		return label;
	}
}

std::string get_next_loop_label(const std::vector<Command> &commands, const std::string &scope, int &i) {
	return get_next_label(commands, scope, "loop", i);
}

std::string get_next_call_label(const std::vector<Command> &commands, const std::string &scope, int &i) {
	return get_next_label(commands, scope, "sub", i);
}

bool is_label_referenced(const std::vector<Command> &commands, const std::string &label) {
	for (const Command &command : commands) {
		if (
			(command.type == Command_Type::SOUND_JUMP ||
			command.type == Command_Type::SOUND_LOOP ||
			command.type == Command_Type::SOUND_CALL) &&
			command.target == label
		) {
			return true;
		}
	}
	return false;
}

void delete_label(std::vector<Command> &commands, const std::string &label) {
	for (Command &command : commands) {
		for (auto l = command.labels.begin(); l != command.labels.end(); ++l) {
			if (*l == label) {
				command.labels.erase(l);
				return;
			}
		}
	}
}

std::vector<Command>::const_iterator find_note_with_label(const std::vector<Command> &commands, const std::string &label) {
	for (auto command_itr = commands.begin(); command_itr != commands.end(); ++command_itr) {
		if (std::count(RANGE(command_itr->labels), label) > 0) {
			return command_itr;
		}
	}
	assert(false);
	return commands.end();
}

bool is_followed_by_n_ticks_of_rest(std::vector<Command>::const_iterator itr, std::vector<Command>::const_iterator end, int32_t n, int32_t speed) {
	int32_t ticks = 0;
	++itr;
	while (itr != end) {
		if (
			itr->labels.size() > 0 ||
			is_note_command(itr->type) ||
			is_global_command(itr->type) ||
			is_control_command(itr->type)
		) {
			return false;
		}
		if (is_speed_command(itr->type)) {
			speed = itr->note_type.speed;
		}
		if (itr->type == Command_Type::REST) {
			ticks += speed * itr->rest.length;
			if (ticks >= n) {
				return true;
			}
		}
		++itr;
	}
	return false;
}

std::vector<Command> copy_snippet(const std::vector<Command> &commands, int32_t start_index, int32_t end_index, bool copy_jumps) {
	std::vector<Command> snippet;

	auto command_itr = commands.begin() + start_index;
	auto end_itr = commands.begin() + end_index;

	while (command_itr != commands.end() && command_itr != end_itr) {
		assert(command_itr->type != Command_Type::SOUND_RET);
		if (
			command_itr->type == Command_Type::SOUND_JUMP ||
			(command_itr->type == Command_Type::SOUND_LOOP && command_itr->sound_loop.loop_count == 0)
		) {
			if (copy_jumps) {
				snippet.push_back(*command_itr);
			}
			command_itr = find_note_with_label(commands, command_itr->target);
			continue;
		}
		snippet.push_back(*command_itr);
		++command_itr;
	}

	return snippet;
}

Parsed_Song::Result calc_channel_length(
	const std::vector<Command> &commands,
	std::vector<Command>::const_iterator start_itr,
	std::vector<Command>::const_iterator end_itr,
	int32_t start_speed,
	int32_t start_drumkit,
	int32_t &loop_tick,
	int32_t &end_tick,
	Extra_Info *info = nullptr
) {
	int32_t tick = 0;
	loop_tick = -1;
	end_tick = -1;
	int32_t speed = start_speed;
	int32_t volume = 0;
	int32_t fade = 0;
	int32_t drumkit = start_drumkit;

	struct Label_Info {
		int32_t tick = 0;
		int32_t index = 0;
		int32_t speed = 0;
		int32_t volume = 0;
		int32_t fade = 0;
		int32_t drumkit = 0;
	};

	auto command_itr = start_itr;

	std::stack<std::pair<decltype(command_itr), int32_t>> loop_stack;
	std::stack<decltype(command_itr)> call_stack;
	std::set<std::string> visited_labels_during_call;
	std::map<std::string, Label_Info> label_infos;

	while (command_itr != end_itr) {
		for (const std::string &label : command_itr->labels) {
			label_infos.insert({ label, { tick, itr_index(commands, command_itr), speed, volume, fade, drumkit } });
			if (call_stack.size() > 0) {
				visited_labels_during_call.insert(label);
			}
		}

		if (command_itr->type == Command_Type::NOTE) {
			tick += command_itr->note.length * speed;
		}
		else if (command_itr->type == Command_Type::DRUM_NOTE) {
			if (drumkit == -1) {
				return Parsed_Song::Result::SONG_NO_DRUMKIT_SELECTED;
			}
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
		else if (command_itr->type == Command_Type::TOGGLE_NOISE) {
			if (drumkit == -1 && command_itr->toggle_noise.drumkit == -1) {
				return Parsed_Song::Result::SONG_TOGGLE_NOISE_ALREADY_DISABLED;
			}
			if (drumkit != -1 && command_itr->toggle_noise.drumkit != -1) {
				return Parsed_Song::Result::SONG_TOGGLE_NOISE_ALREADY_ENABLED;
			}
			drumkit = command_itr->toggle_noise.drumkit;
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
				info->drumkit_at_loop = label_info.drumkit;

				info->end_index = itr_index(commands, command_itr);
				info->speed_at_end = speed;
				info->volume_at_end = volume;
				info->fade_at_end = fade;
				info->drumkit_at_end = drumkit;
			}
			if (loop_tick == end_tick) {
				return Parsed_Song::Result::SONG_EMPTY_LOOP;
			}
			if (call_stack.size() > 0) {
				return Parsed_Song::Result::SONG_UNFINISHED_CALL;
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
						info->drumkit_at_loop = label_info.drumkit;

						info->end_index = itr_index(commands, command_itr);
						info->speed_at_end = speed;
						info->volume_at_end = volume;
						info->fade_at_end = fade;
						info->drumkit_at_end = drumkit;
					}
					if (loop_tick == end_tick) {
						return Parsed_Song::Result::SONG_EMPTY_LOOP;
					}
					if (call_stack.size() > 0) {
						return Parsed_Song::Result::SONG_UNFINISHED_CALL;
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
					info->loop_index = -1;
					info->speed_at_loop = -1;
					info->volume_at_loop = -1;
					info->fade_at_loop = -1;
					info->drumkit_at_loop = -1;

					info->end_index = itr_index(commands, command_itr);
					info->speed_at_end = speed;
					info->volume_at_end = volume;
					info->fade_at_end = fade;
					info->drumkit_at_end = drumkit;
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

	end_tick = tick;
	return Parsed_Song::Result::SONG_OK;
}

Parsed_Song::Result calc_channel_length(const std::vector<Command> &commands, int32_t &loop_tick, int32_t &end_tick, Extra_Info *info) {
	return calc_channel_length(commands, commands.begin(), commands.end(), 1, -1, loop_tick, end_tick, info);
}

int32_t calc_snippet_length(const std::vector<Command> &commands, const std::vector<Command>::const_iterator &start_itr, const std::vector<Command>::const_iterator &end_itr, const Note_View &start_view) {
	int32_t loop_tick, end_tick;
	Parsed_Song::Result r = calc_channel_length(commands, start_itr, end_itr, start_view.speed, start_view.drumkit, loop_tick, end_tick);
	assert(r == Parsed_Song::Result::SONG_OK);
	return end_tick;
}

Note_View get_note_view(const std::vector<Command> &commands, int32_t index) {
	int32_t tick = 0;

	Note_View note;
	note.octave = 8;
	note.speed = 1;

	auto command_itr = commands.begin();

	std::stack<std::pair<decltype(command_itr), int32_t>> loop_stack;
	std::stack<decltype(command_itr)> call_stack;
	std::set<std::string> visited_labels_during_call;
	std::map<std::string, int32_t> label_positions;

	while (command_itr != commands.end()) {
		for (const std::string &label : command_itr->labels) {
			label_positions.insert({ label, tick });
			if (call_stack.size() > 0) {
				visited_labels_during_call.insert(label);
			}
		}

		if (command_itr->type == Command_Type::NOTE) {
			note.length = command_itr->note.length;
			note.pitch = command_itr->note.pitch;
			tick += note.length * note.speed;
			note.index = itr_index(commands, command_itr);

			note.slide_duration = 0;
			note.slide_octave = 0;
			note.slide_pitch = Pitch::REST;

			if (note.index == index) {
				return note;
			}
		}
		else if (command_itr->type == Command_Type::DRUM_NOTE) {
			note.length = command_itr->drum_note.length;
			note.pitch = (Pitch)command_itr->drum_note.instrument;
			tick += note.length * note.speed;
			note.index = itr_index(commands, command_itr);

			if (note.index == index) {
				return note;
			}
		}
		else if (command_itr->type == Command_Type::REST) {
			note.length = command_itr->rest.length;
			note.pitch = Pitch::REST;
			tick += note.length * note.speed;
			note.index = itr_index(commands, command_itr);

			if (note.index == index) {
				return note;
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
						!label_positions.count(command_itr->target) ||
						(call_stack.size() > 0 && !visited_labels_during_call.count(command_itr->target))
					) {
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
		}
		else if (command_itr->type == Command_Type::CHANNEL_VOLUME) {
			note.volume = command_itr->channel_volume.volume;
		}
		else if (command_itr->type == Command_Type::FADE_WAVE) {
			note.fade = command_itr->fade_wave.fade;
		}
		++command_itr;
	}

	assert(false);
	return note;
}

// get the last note or rest that is before `end_tick` which is not part of a loop or a call
int32_t get_base_index(const std::vector<Command> &commands, int32_t start_tick, int32_t end_tick, int32_t &drumkit_at_base) {
	int32_t tick = 0;
	int32_t speed = 1;
	int32_t drumkit = -1;

	int32_t base_index = -1;
	drumkit_at_base = -1;

	auto command_itr = commands.begin();

	std::stack<std::pair<decltype(command_itr), int32_t>> loop_stack;
	std::stack<decltype(command_itr)> call_stack;
	std::set<std::string> visited_labels_during_call;
	std::map<std::string, int32_t> label_positions;

	while (command_itr != commands.end()) {
		for (const std::string &label : command_itr->labels) {
			label_positions.insert({ label, tick });
			if (call_stack.size() > 0) {
				visited_labels_during_call.insert(label);
			}
		}

		if (command_itr->type == Command_Type::NOTE) {
			tick += command_itr->note.length * speed;

			if (tick > start_tick && tick <= end_tick && loop_stack.size() == 0 && call_stack.size() == 0) {
				base_index = itr_index(commands, command_itr);
				drumkit_at_base = drumkit;
			}
			if (tick > end_tick || (tick == end_tick && loop_stack.size() == 0 && call_stack.size() == 0)) {
				return base_index;
			}
		}
		else if (command_itr->type == Command_Type::DRUM_NOTE) {
			tick += command_itr->drum_note.length * speed;

			if (tick > start_tick && tick <= end_tick && loop_stack.size() == 0 && call_stack.size() == 0) {
				base_index = itr_index(commands, command_itr);
				drumkit_at_base = drumkit;
			}
			if (tick > end_tick || (tick == end_tick && loop_stack.size() == 0 && call_stack.size() == 0)) {
				return base_index;
			}
		}
		else if (command_itr->type == Command_Type::REST) {
			tick += command_itr->rest.length * speed;

			if (tick > start_tick && tick <= end_tick && loop_stack.size() == 0 && call_stack.size() == 0) {
				base_index = itr_index(commands, command_itr);
				drumkit_at_base = drumkit;
			}
			if (tick > end_tick || (tick == end_tick && loop_stack.size() == 0 && call_stack.size() == 0)) {
				return base_index;
			}
		}
		else if (command_itr->type == Command_Type::NOTE_TYPE) {
			speed = command_itr->note_type.speed;
		}
		else if (command_itr->type == Command_Type::DRUM_SPEED) {
			speed = command_itr->drum_speed.speed;
		}
		else if (command_itr->type == Command_Type::TOGGLE_NOISE) {
			drumkit = command_itr->toggle_noise.drumkit;
		}
		else if (command_itr->type == Command_Type::SOUND_JUMP) {
			if (
				!label_positions.count(command_itr->target) ||
				(call_stack.size() > 0 && !visited_labels_during_call.count(command_itr->target))
			) {
				command_itr = find_note_with_label(commands, command_itr->target);
				continue;
			}
			break; // song is finished
		}
		else if (command_itr->type == Command_Type::SOUND_LOOP) {
			if (loop_stack.size() > 0 && loop_stack.top().first == command_itr) {
				loop_stack.top().second -= 1;
				if (loop_stack.top().second == 0) {
					loop_stack.pop();
					if (tick > start_tick && tick <= end_tick && loop_stack.size() == 0 && call_stack.size() == 0) {
						base_index = itr_index(commands, command_itr);
						drumkit_at_base = drumkit;
					}
					if (tick > end_tick || (tick == end_tick && loop_stack.size() == 0 && call_stack.size() == 0)) {
						return base_index;
					}
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
				visited_labels_during_call.clear();
				if (tick > start_tick && tick <= end_tick && loop_stack.size() == 0 && call_stack.size() == 0) {
					base_index = itr_index(commands, command_itr);
					drumkit_at_base = drumkit;
				}
				if (tick > end_tick || (tick == end_tick && loop_stack.size() == 0 && call_stack.size() == 0)) {
					return base_index;
				}
			}
		}
		else if (command_itr->type == Command_Type::SPEED) {
			speed = command_itr->speed.speed;
		}
		++command_itr;
	}

	return -1;
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

Song_Options_Dialog::Song_Options Song::get_options() {
	Song_Options_Dialog::Song_Options options;
	options.song_name = _song_name;
	options.looping = _channel_1_loop_tick != -1 || _channel_2_loop_tick != -1 || _channel_3_loop_tick != -1 || _channel_4_loop_tick != -1;
	options.channel_1 = _channel_1_end_tick != -1;
	options.channel_2 = _channel_2_end_tick != -1;
	options.channel_3 = _channel_3_end_tick != -1;
	options.channel_4 = _channel_4_end_tick != -1;
	options.channel_1_loop_tick = std::max(_channel_1_loop_tick, 0);
	options.channel_2_loop_tick = std::max(_channel_2_loop_tick, 0);
	options.channel_3_loop_tick = std::max(_channel_3_loop_tick, 0);
	options.channel_4_loop_tick = std::max(_channel_4_loop_tick, 0);
	options.channel_1_end_tick = std::max(_channel_1_end_tick, 0);
	options.channel_2_end_tick = std::max(_channel_2_end_tick, 0);
	options.channel_3_end_tick = std::max(_channel_3_end_tick, 0);
	options.channel_4_end_tick = std::max(_channel_4_end_tick, 0);
	options.result = Song_Options_Dialog::Result::RESULT_OK;
	return options;
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

struct Command_Indexes {
	int32_t rest_index = -1;
	int32_t octave_index = -1;
	int32_t speed_index = -1;
	int32_t transpose_index = -1;
	int32_t tempo_index = -1;
	int32_t duty_index = -1;
	int32_t vol_env_index = -1;
	int32_t slide_index = -1;
	int32_t vibrato_index = -1;
	int32_t toggle_noise_off_index = -1;
	int32_t toggle_noise_on_index = -1;
	int32_t toggle_noise_off_again_index = -1;
};

void shift_indexes(Command_Indexes &indexes, int32_t index) {
	if (indexes.rest_index > index) {
		indexes.rest_index -= 1;
	}
	if (indexes.octave_index > index) {
		indexes.octave_index -= 1;
	}
	if (indexes.speed_index > index) {
		indexes.speed_index -= 1;
	}
	if (indexes.transpose_index > index) {
		indexes.transpose_index -= 1;
	}
	if (indexes.tempo_index > index) {
		indexes.tempo_index -= 1;
	}
	if (indexes.duty_index > index) {
		indexes.duty_index -= 1;
	}
	if (indexes.vol_env_index > index) {
		indexes.vol_env_index -= 1;
	}
	if (indexes.slide_index > index) {
		indexes.slide_index -= 1;
	}
	if (indexes.vibrato_index > index) {
		indexes.vibrato_index -= 1;
	}
	if (indexes.toggle_noise_off_index > index) {
		indexes.toggle_noise_off_index -= 1;
	}
	if (indexes.toggle_noise_on_index > index) {
		indexes.toggle_noise_on_index -= 1;
	}
	if (indexes.toggle_noise_off_again_index > index) {
		indexes.toggle_noise_off_again_index -= 1;
	}
}

void postprocess(std::vector<Command> &commands) {
	bool deleted = false;
	do {
		Note_View view;
		Command_Indexes indexes;
		deleted = false;
		for (uint32_t i = 0; i < commands.size(); ++i) {
			if (
				commands[i].labels.size() > 0 ||
				is_control_command(commands[i].type)
			) {
				// on hard boundary, forget everything
				view = Note_View{};
				view.volume = -1;
				view.fade = -1;
				view.drumkit = -1;
				view.transpose_octaves = -1;
				view.transpose_pitches = -1;
				view.duty = -1;
				view.vibrato_delay = -1;
				view.vibrato_extent = -1;
				view.vibrato_rate = -1;
				indexes.rest_index = -1;
				indexes.octave_index = -1;
				indexes.speed_index = -1;
				indexes.transpose_index = -1;
				indexes.tempo_index = -1;
				indexes.duty_index = -1;
				indexes.vol_env_index = -1;
				indexes.slide_index = -1;
				indexes.vibrato_index = -1;
				indexes.toggle_noise_off_index = -1;
				indexes.toggle_noise_on_index = -1;
				indexes.toggle_noise_off_again_index = -1;
			}
			if (
				is_global_command(commands[i].type) ||
				is_speed_command(commands[i].type)
			) {
				// on tempo or speed change, don't tamper with previous rest
				indexes.rest_index = -1;
			}
			if (commands[i].type == Command_Type::NOTE_TYPE) {
				// on note type change, don't tamper with previous envelope
				indexes.vol_env_index = -1;
			}
			if (commands[i].type == Command_Type::REST) {
				// on rest, don't tamper with previous speed or tempo
				indexes.speed_index = -1;
				indexes.tempo_index = -1;
			}
			if (is_note_command(commands[i].type)) {
				// on note, don't tamper with any previous commands
				view.slide_duration = 0;
				view.slide_octave = 0;
				view.slide_pitch = Pitch::REST;
				indexes.rest_index = -1;
				indexes.octave_index = -1;
				indexes.speed_index = -1;
				indexes.transpose_index = -1;
				indexes.tempo_index = -1;
				indexes.duty_index = -1;
				indexes.vol_env_index = -1;
				indexes.slide_index = -1;
				indexes.vibrato_index = -1;
				indexes.toggle_noise_off_index = -1;
				indexes.toggle_noise_on_index = -1;
				indexes.toggle_noise_off_again_index = -1;
			}

			if (commands[i].type == Command_Type::REST) {
				if (indexes.rest_index == -1) {
					// track this rest to possibly add to later, if it is not maxed out
					if (commands[i].rest.length < 16) {
						indexes.rest_index = i;
					}
				}
				else {
					// if the previous rest and current rest can't fit into one rest command...
					if (commands[indexes.rest_index].rest.length + commands[i].rest.length > 16) {
						// then max out the previous rest, put the remainder into the current rest, and start tracking the current rest
						commands[i].rest.length = commands[indexes.rest_index].rest.length + commands[i].rest.length - 16;
						commands[indexes.rest_index].rest.length = 16;
						indexes.rest_index = i;
					}
					// otherwise, combine into the previous rest and delete the current rest
					else {
						deleted = true;
						commands[indexes.rest_index].rest.length = commands[indexes.rest_index].rest.length + commands[i].rest.length;
						assert(commands[i].labels.size() == 0);
						commands.erase(commands.begin() + i);
						i -= 1;

						// stop tracking the previous rest if it is now maxed out
						if (commands[indexes.rest_index].rest.length == 16) {
							indexes.rest_index = -1;
						}
					}
				}
			}

			else if (commands[i].type == Command_Type::OCTAVE) {
				if (indexes.octave_index == -1) {
					// if the octave isn't changing, delete it
					if (view.octave == commands[i].octave.octave) {
						deleted = true;
						assert(commands[i].labels.size() == 0);
						commands.erase(commands.begin() + i);
						i -= 1;
					}
					// otherwise, track it
					else {
						view.octave = commands[i].octave.octave;
						indexes.octave_index = i;
					}
				}
				else {
					// octave changed twice in a row, so delete the old one
					deleted = true;
					commands[indexes.octave_index + 1].labels.insert(commands[indexes.octave_index + 1].labels.begin(), RANGE(commands[indexes.octave_index].labels));
					commands.erase(commands.begin() + indexes.octave_index);
					i -= 1;
					shift_indexes(indexes, indexes.octave_index);

					view.octave = commands[i].octave.octave;
					indexes.octave_index = i;
				}
			}
			else if (commands[i].type == Command_Type::INC_OCTAVE || commands[i].type == Command_Type::DEC_OCTAVE) {
				view.octave = 0;
				indexes.octave_index = -1;
			}

			else if (commands[i].type == Command_Type::NOTE_TYPE) {
				if (indexes.speed_index == -1) {
					// if the note type isn't changing, delete it
					if (
						view.speed == commands[i].note_type.speed &&
						view.volume == commands[i].note_type.volume &&
						view.fade == commands[i].note_type.fade
					) {
						deleted = true;
						assert(commands[i].labels.size() == 0);
						commands.erase(commands.begin() + i);
						i -= 1;
					}
					// otherwise, track it
					else {
						view.speed = commands[i].note_type.speed;
						view.volume = commands[i].note_type.volume;
						view.fade = commands[i].note_type.fade;
						indexes.speed_index = i;
					}
				}
				else {
					// note type changed twice in a row, so delete the old one
					deleted = true;
					commands[indexes.speed_index + 1].labels.insert(commands[indexes.speed_index + 1].labels.begin(), RANGE(commands[indexes.speed_index].labels));
					commands.erase(commands.begin() + indexes.speed_index);
					i -= 1;
					shift_indexes(indexes, indexes.speed_index);

					view.speed = commands[i].note_type.speed;
					view.volume = commands[i].note_type.volume;
					view.fade = commands[i].note_type.fade;
					indexes.speed_index = i;
				}
			}
			else if (commands[i].type == Command_Type::DRUM_SPEED) {
				if (indexes.speed_index == -1) {
					// if the speed isn't changing, delete it
					if (view.speed == commands[i].drum_speed.speed) {
						deleted = true;
						assert(commands[i].labels.size() == 0);
						commands.erase(commands.begin() + i);
						i -= 1;
					}
					// otherwise, track it
					else {
						view.speed = commands[i].drum_speed.speed;
						indexes.speed_index = i;
					}
				}
				else {
					// speed changed twice in a row, so delete the old one
					deleted = true;
					commands[indexes.speed_index + 1].labels.insert(commands[indexes.speed_index + 1].labels.begin(), RANGE(commands[indexes.speed_index].labels));
					commands.erase(commands.begin() + indexes.speed_index);
					i -= 1;
					shift_indexes(indexes, indexes.speed_index);

					view.speed = commands[i].drum_speed.speed;
					indexes.speed_index = i;
				}
			}
			else if (commands[i].type == Command_Type::SPEED) {
				view.speed = commands[i].speed.speed;
				indexes.speed_index = i;
			}

			else if (commands[i].type == Command_Type::TRANSPOSE) {
				if (indexes.transpose_index == -1) {
					// if the transpose isn't changing, delete it
					if (
						view.transpose_octaves == commands[i].transpose.num_octaves &&
						view.transpose_pitches == commands[i].transpose.num_pitches
					) {
						deleted = true;
						assert(commands[i].labels.size() == 0);
						commands.erase(commands.begin() + i);
						i -= 1;
					}
					// otherwise, track it
					else {
						view.transpose_octaves = commands[i].transpose.num_octaves;
						view.transpose_pitches = commands[i].transpose.num_pitches;
						indexes.transpose_index = i;
					}
				}
				else {
					// transpose changed twice in a row, so delete the old one
					deleted = true;
					commands[indexes.transpose_index + 1].labels.insert(commands[indexes.transpose_index + 1].labels.begin(), RANGE(commands[indexes.transpose_index].labels));
					commands.erase(commands.begin() + indexes.transpose_index);
					i -= 1;
					shift_indexes(indexes, indexes.transpose_index);

					view.transpose_octaves = commands[i].transpose.num_octaves;
					view.transpose_pitches = commands[i].transpose.num_pitches;
					indexes.transpose_index = i;
				}
			}

			else if (commands[i].type == Command_Type::TEMPO) {
				if (indexes.tempo_index == -1) {
					// if the tempo isn't changing, delete it
					if (view.tempo == commands[i].tempo.tempo) {
						deleted = true;
						assert(commands[i].labels.size() == 0);
						commands.erase(commands.begin() + i);
						i -= 1;
					}
					// otherwise, track it
					else {
						view.tempo = commands[i].tempo.tempo;
						indexes.tempo_index = i;
					}
				}
				else {
					// tempo changed twice in a row, so delete the old one
					deleted = true;
					commands[indexes.tempo_index + 1].labels.insert(commands[indexes.tempo_index + 1].labels.begin(), RANGE(commands[indexes.tempo_index].labels));
					commands.erase(commands.begin() + indexes.tempo_index);
					i -= 1;
					shift_indexes(indexes, indexes.tempo_index);

					view.tempo = commands[i].tempo.tempo;
					indexes.tempo_index = i;
				}
			}

			else if (commands[i].type == Command_Type::DUTY_CYCLE) {
				if (indexes.duty_index == -1) {
					// if the duty isn't changing, delete it
					if (view.duty == commands[i].duty_cycle.duty) {
						deleted = true;
						assert(commands[i].labels.size() == 0);
						commands.erase(commands.begin() + i);
						i -= 1;
					}
					// otherwise, track it
					else {
						view.duty = commands[i].duty_cycle.duty;
						indexes.duty_index = i;
					}
				}
				else {
					// duty changed twice in a row, so delete the old one
					deleted = true;
					commands[indexes.duty_index + 1].labels.insert(commands[indexes.duty_index + 1].labels.begin(), RANGE(commands[indexes.duty_index].labels));
					commands.erase(commands.begin() + indexes.duty_index);
					i -= 1;
					shift_indexes(indexes, indexes.duty_index);

					view.duty = commands[i].duty_cycle.duty;
					indexes.duty_index = i;
				}
			}

			else if (commands[i].type == Command_Type::VOLUME_ENVELOPE) {
				if (indexes.vol_env_index == -1) {
					// if the envelope isn't changing, delete it
					if (
						view.volume == commands[i].volume_envelope.volume &&
						view.fade == commands[i].volume_envelope.fade
					) {
						deleted = true;
						assert(commands[i].labels.size() == 0);
						commands.erase(commands.begin() + i);
						i -= 1;
					}
					// otherwise, track it
					else {
						view.volume = commands[i].volume_envelope.volume;
						view.fade = commands[i].volume_envelope.fade;
						indexes.vol_env_index = i;

						// allow note type commands to delete this envelope too
						// TODO: if speed_index is already != -1, delete this envelope and copy
						//   the envelope parameters into the note type command.
						indexes.speed_index = i;
					}
				}
				else {
					// envelope changed twice in a row, so delete the old one
					deleted = true;
					commands[indexes.vol_env_index + 1].labels.insert(commands[indexes.vol_env_index + 1].labels.begin(), RANGE(commands[indexes.vol_env_index].labels));
					commands.erase(commands.begin() + indexes.vol_env_index);
					i -= 1;
					shift_indexes(indexes, indexes.vol_env_index);

					view.volume = commands[i].volume_envelope.volume;
					view.fade = commands[i].volume_envelope.fade;
					indexes.vol_env_index = i;

					// allow note type commands to delete this envelope too
					indexes.speed_index = i;
				}
			}
			else if (commands[i].type == Command_Type::CHANNEL_VOLUME) {
				view.volume = commands[i].channel_volume.volume;
				indexes.vol_env_index = i;
			}
			else if (commands[i].type == Command_Type::FADE_WAVE) {
				view.fade = commands[i].fade_wave.fade;
				indexes.vol_env_index = i;
			}

			else if (commands[i].type == Command_Type::PITCH_SLIDE) {
				if (indexes.slide_index == -1) {
					view.slide_duration = commands[i].pitch_slide.duration;
					view.slide_octave = commands[i].pitch_slide.octave;
					view.slide_pitch = commands[i].pitch_slide.pitch;
					indexes.slide_index = i;
				}
				else {
					deleted = true;
					commands[indexes.slide_index + 1].labels.insert(commands[indexes.slide_index + 1].labels.begin(), RANGE(commands[indexes.slide_index].labels));
					commands.erase(commands.begin() + indexes.slide_index);
					i -= 1;
					shift_indexes(indexes, indexes.slide_index);

					view.slide_duration = commands[i].pitch_slide.duration;
					view.slide_octave = commands[i].pitch_slide.octave;
					view.slide_pitch = commands[i].pitch_slide.pitch;
					indexes.slide_index = i;
				}
			}

			else if (commands[i].type == Command_Type::VIBRATO) {
				if (indexes.vibrato_index == -1) {
					// if the vibrato isn't changing, delete it
					if (
						view.vibrato_delay == commands[i].vibrato.delay &&
						view.vibrato_extent == commands[i].vibrato.extent &&
						view.vibrato_rate == commands[i].vibrato.rate
					) {
						deleted = true;
						assert(commands[i].labels.size() == 0);
						commands.erase(commands.begin() + i);
						i -= 1;
					}
					// otherwise, track it
					else {
						view.vibrato_delay = commands[i].vibrato.delay;
						view.vibrato_extent = commands[i].vibrato.extent;
						view.vibrato_rate = commands[i].vibrato.rate;
						indexes.vibrato_index = i;
					}
				}
				else {
					// vibrato changed twice in a row, so delete the old one
					deleted = true;
					commands[indexes.vibrato_index + 1].labels.insert(commands[indexes.vibrato_index + 1].labels.begin(), RANGE(commands[indexes.vibrato_index].labels));
					commands.erase(commands.begin() + indexes.vibrato_index);
					i -= 1;
					shift_indexes(indexes, indexes.vibrato_index);

					view.vibrato_delay = commands[i].vibrato.delay;
					view.vibrato_extent = commands[i].vibrato.extent;
					view.vibrato_rate = commands[i].vibrato.rate;
					indexes.vibrato_index = i;
				}
			}

			else if (commands[i].type == Command_Type::TOGGLE_NOISE) {
				// off command
				if (commands[i].toggle_noise.drumkit == -1) {
					if (indexes.toggle_noise_off_index == -1) {
						assert(indexes.toggle_noise_on_index == -1);
						assert(indexes.toggle_noise_off_again_index == -1);
						indexes.toggle_noise_off_index = i;
					}
					else {
						assert(indexes.toggle_noise_on_index != -1);
						assert(indexes.toggle_noise_off_again_index == -1);
						indexes.toggle_noise_off_again_index = i;
					}
				}
				// on command
				else {
					if (indexes.toggle_noise_on_index == -1) {
						if (indexes.toggle_noise_off_index == -1) {
							// probably the initial drumkit setting...
							view.drumkit = commands[i].toggle_noise.drumkit;
						}
						else {
							assert(indexes.toggle_noise_off_index != -1);
							assert(indexes.toggle_noise_off_again_index == -1);
							// if the drumkit isn't changing, delete it (off and on commands)
							if (view.drumkit == commands[i].toggle_noise.drumkit) {
								deleted = true;
								assert(commands[i].labels.size() == 0);
								commands.erase(commands.begin() + i);
								i -= 1;

								assert(commands[indexes.toggle_noise_off_index].labels.size() == 0);
								commands.erase(commands.begin() + indexes.toggle_noise_off_index);
								i -= 1;
								shift_indexes(indexes, indexes.toggle_noise_off_index);
								indexes.toggle_noise_off_index = -1;
							}
							// otherwise, track it
							else {
								view.drumkit = commands[i].toggle_noise.drumkit;
								indexes.toggle_noise_on_index = i;
							}
						}
					}
					else {
						assert(indexes.toggle_noise_off_index != -1);
						assert(indexes.toggle_noise_off_again_index != -1);
						// drumkit changed twice in a row, so delete the old one (off and on commands)
						deleted = true;
						assert(commands[indexes.toggle_noise_on_index].labels.size() == 0);
						commands.erase(commands.begin() + indexes.toggle_noise_on_index);
						i -= 1;
						shift_indexes(indexes, indexes.toggle_noise_on_index);

						commands[indexes.toggle_noise_off_index + 1].labels.insert(commands[indexes.toggle_noise_off_index + 1].labels.begin(), RANGE(commands[indexes.toggle_noise_off_index].labels));
						commands.erase(commands.begin() + indexes.toggle_noise_off_index);
						i -= 1;
						shift_indexes(indexes, indexes.toggle_noise_off_index);

						view.drumkit = commands[i].toggle_noise.drumkit;
						indexes.toggle_noise_off_index = indexes.toggle_noise_off_again_index;
						indexes.toggle_noise_on_index = i;
						indexes.toggle_noise_off_again_index = -1;
					}
				}
			}
		}
	} while (deleted);

	for (uint32_t i = 0; i < commands.size(); ++i) {
		if (commands[i].type == Command_Type::TEMPO && commands[i].tempo.tempo == 0) {
			commands[i + 1].labels.insert(commands[i + 1].labels.begin(), RANGE(commands[i].labels));
			commands.erase(commands.begin() + i);
			i -= 1;
		}
		else if (
			commands[i].type == Command_Type::PITCH_SLIDE &&
			(commands[i].pitch_slide.duration == 0 ||
			commands[i].pitch_slide.octave == 0 ||
			commands[i].pitch_slide.pitch == Pitch::REST)
		) {
			commands[i + 1].labels.insert(commands[i + 1].labels.begin(), RANGE(commands[i].labels));
			commands.erase(commands.begin() + i);
			i -= 1;
		}
	}
}

int32_t insert_ticks(int32_t selected_channel, std::vector<Command> &commands, int32_t ticks_to_insert, int32_t index, int32_t speed = 1, int32_t volume = 0, int32_t fade = 0) {
	int32_t inserted = 0;

	int32_t speed_ticks_to_insert = ticks_to_insert / speed;
	int32_t rem_ticks_to_insert = ticks_to_insert % speed;

	Command rest = Command(Command_Type::REST);
	rest.rest.length = 16;

	while (speed_ticks_to_insert) {
		if (speed_ticks_to_insert < 16) rest.rest.length = speed_ticks_to_insert;
		commands.insert(commands.begin() + index + inserted, rest);
		inserted += 1;
		speed_ticks_to_insert -= rest.rest.length;
	}
	if (rem_ticks_to_insert > 0) {
		Command command = Command(selected_channel == 4 ? Command_Type::DRUM_SPEED : Command_Type::NOTE_TYPE);
		command.note_type.speed = 1;
		command.note_type.volume = volume;
		command.note_type.fade = fade;
		commands.insert(commands.begin() + index + inserted, command);
		inserted += 1;

		rest.rest.length = rem_ticks_to_insert;
		commands.insert(commands.begin() + index + inserted, rest);
		inserted += 1;

		command.note_type.speed = speed;
		commands.insert(commands.begin() + index + inserted, command);
		inserted += 1;
	}

	return inserted;
}

void erase_ticks(int32_t selected_channel, std::vector<Command> &commands, int32_t ticks_to_erase, int32_t index, int32_t speed, int32_t volume, int32_t fade) {
	int32_t ticks = 0;
	index += 1;
	while (index < (int32_t)commands.size()) {
		assert(commands[index].labels.size() == 0);
		assert(!is_note_command(commands[index].type));
		assert(!is_global_command(commands[index].type));
		assert(!is_control_command(commands[index].type));
		if (is_speed_command(commands[index].type)) {
			speed = commands[index].note_type.speed;
		}
		if (commands[index].type == Command_Type::REST) {
			ticks += speed * commands[index].rest.length;
			commands.erase(commands.begin() + index);
			index -= 1;
			if (ticks > ticks_to_erase) {
				insert_ticks(selected_channel, commands, ticks - ticks_to_erase, index + 1, speed, volume, fade);
				break;
			}
			if (ticks == ticks_to_erase) {
				break;
			}
		}
		index += 1;
	}
}

void resize_channel(int32_t selected_channel, std::vector<Command> &commands, const std::string &channel_label, int32_t new_loop_tick, int32_t new_end_tick) {
	int32_t old_loop_tick, old_end_tick;
	Extra_Info original_info;
	calc_channel_length(commands, old_loop_tick, old_end_tick, &original_info);

	const int32_t original_intro_length = old_loop_tick != -1 ? old_loop_tick : 0;
	const int32_t final_intro_length    = new_loop_tick != -1 ? new_loop_tick : 0;
	int32_t ticks_to_insert_at_loop = final_intro_length - original_intro_length;

	const int32_t original_body_length = old_end_tick - original_intro_length;
	const int32_t final_body_length    = new_end_tick - final_intro_length;
	int32_t ticks_to_insert_at_end = final_body_length - original_body_length;

	Extra_Info info = original_info;

	if (ticks_to_insert_at_loop != 0) {
		int32_t drumkit_at_base;
		int32_t base_loop_index = get_base_index(commands, 0, ticks_to_insert_at_loop < 0 ? final_intro_length : original_intro_length, drumkit_at_base);
		if (base_loop_index == -1) {
			// no base could be found, so start from the beginning...
			base_loop_index = 0;
			// ...but advance to first potential note or rest
			while (
				commands[base_loop_index].type != Command_Type::NOTE &&
				commands[base_loop_index].type != Command_Type::DRUM_NOTE &&
				commands[base_loop_index].type != Command_Type::REST &&
				commands[base_loop_index].type != Command_Type::SOUND_CALL &&
				!(commands[base_loop_index].type == Command_Type::SOUND_LOOP && commands[base_loop_index].sound_loop.loop_count != 0)
			) {
				if (base_loop_index == original_info.loop_index) {
					base_loop_index = 0;
					drumkit_at_base = -1;
					break;
				}
				assert(commands[base_loop_index].type != Command_Type::SOUND_RET);
				if (
					commands[base_loop_index].type == Command_Type::SOUND_JUMP ||
					commands[base_loop_index].type == Command_Type::SOUND_LOOP
				) {
					int32_t next_index = itr_index(commands, find_note_with_label(commands, commands[base_loop_index].target));
					if (next_index == original_info.loop_index) break;
					base_loop_index = next_index;
					continue;
				}
				if (commands[base_loop_index].type == Command_Type::TOGGLE_NOISE) {
					drumkit_at_base = commands[base_loop_index].toggle_noise.drumkit;
				}
				if (base_loop_index + 1 == original_info.loop_index) break;
				base_loop_index += 1;
			}
		}
		else {
			// the first command after the last note that fits
			base_loop_index += 1;
		}

		if (original_info.loop_index == 0) {
			if (commands[original_info.end_index].target == channel_label) {
				// TODO: double-check that ".mainLoop" isn't already in use
				commands[0].labels.push_back(channel_label + ".mainLoop");
				commands[original_info.end_index].target = channel_label + ".mainLoop";
			}
			for (size_t i = commands[0].labels.size() - 1; i < commands[0].labels.size(); --i) {
				if (commands[0].labels[i] == channel_label) {
					commands[0].labels.erase(commands[0].labels.begin() + i);
				}
			}
		}

		int32_t loop_index = original_info.loop_index;

		int32_t insert_index = base_loop_index;
		int32_t speed_at_insert = original_info.speed_at_loop;
		int32_t volume_at_insert = original_info.volume_at_loop;
		int32_t fade_at_insert = original_info.fade_at_loop;

		bool reinitializing = base_loop_index == 0;

		if (reinitializing) {
			if (selected_channel != 4) {
				Command command = Command(Command_Type::NOTE_TYPE);
				command.note_type.speed = 12;
				command.note_type.volume = selected_channel == 3 ? 1 : 15;
				command.note_type.fade = 0;
				if (base_loop_index != loop_index) {
					command.labels = std::move(commands[base_loop_index].labels);
					commands[base_loop_index].labels.clear();
				}
				commands.insert(commands.begin() + base_loop_index, command);
				loop_index += 1;
				insert_index += 1;
				base_loop_index += 1;
				speed_at_insert = command.note_type.speed;
				volume_at_insert = command.note_type.volume;
				fade_at_insert = command.note_type.fade;
			}
			else {
				Command command = Command(Command_Type::TOGGLE_NOISE);
				command.toggle_noise.drumkit = 0;
				if (base_loop_index != loop_index) {
					command.labels = std::move(commands[base_loop_index].labels);
					commands[base_loop_index].labels.clear();
				}
				commands.insert(commands.begin() + base_loop_index, command);
				loop_index += 1;
				insert_index += 1;
				base_loop_index += 1;
				drumkit_at_base = command.toggle_noise.drumkit;

				command = Command(Command_Type::DRUM_SPEED);
				command.drum_speed.speed = 12;
				commands.insert(commands.begin() + base_loop_index, command);
				loop_index += 1;
				insert_index += 1;
				base_loop_index += 1;
				speed_at_insert = command.drum_speed.speed;
			}
		}

		// preserve loop speed/drumkit
		if (selected_channel != 4) {
			Command command = Command(Command_Type::NOTE_TYPE);
			command.note_type.speed = original_info.speed_at_loop;
			command.note_type.volume = original_info.volume_at_loop;
			command.note_type.fade = original_info.fade_at_loop;
			if (base_loop_index != loop_index) {
				command.labels = std::move(commands[base_loop_index].labels);
				commands[base_loop_index].labels.clear();
			}
			commands.insert(commands.begin() + base_loop_index, command);
			if (loop_index >= base_loop_index) loop_index += 1;
			if (!reinitializing) insert_index += 1;
			base_loop_index += 1;
		}
		else {
			if (drumkit_at_base != original_info.drumkit_at_loop) {
				if (drumkit_at_base != -1) {
					// turn off
					Command command = Command(Command_Type::TOGGLE_NOISE);
					command.toggle_noise.drumkit = -1;
					if (base_loop_index != loop_index) {
						command.labels = std::move(commands[base_loop_index].labels);
						commands[base_loop_index].labels.clear();
					}
					commands.insert(commands.begin() + base_loop_index, command);
					if (loop_index >= base_loop_index) loop_index += 1;
					if (!reinitializing) insert_index += 1;
					base_loop_index += 1;
				}
				if (original_info.drumkit_at_loop != -1) {
					// turn on
					Command command = Command(Command_Type::TOGGLE_NOISE);
					command.toggle_noise.drumkit = original_info.drumkit_at_loop;
					if (base_loop_index != loop_index) {
						command.labels = std::move(commands[base_loop_index].labels);
						commands[base_loop_index].labels.clear();
					}
					commands.insert(commands.begin() + base_loop_index, command);
					if (loop_index >= base_loop_index) loop_index += 1;
					if (!reinitializing) insert_index += 1;
					base_loop_index += 1;
				}
			}

			Command command = Command(Command_Type::DRUM_SPEED);
			command.drum_speed.speed = original_info.speed_at_loop;
			if (base_loop_index != loop_index) {
				command.labels = std::move(commands[base_loop_index].labels);
				commands[base_loop_index].labels.clear();
			}
			commands.insert(commands.begin() + base_loop_index, command);
			if (loop_index >= base_loop_index) loop_index += 1;
			if (!reinitializing) insert_index += 1;
			base_loop_index += 1;
		}

		if (original_info.loop_index == 0) {
			commands[0].labels.insert(commands[0].labels.begin(), channel_label);
		}

		while (loop_index != base_loop_index) {
			if (
				(commands[base_loop_index].type == Command_Type::SOUND_JUMP) ||
				(commands[base_loop_index].type == Command_Type::SOUND_LOOP && commands[base_loop_index].sound_loop.loop_count == 0)
			) {
				base_loop_index = itr_index(commands, find_note_with_label(commands, commands[base_loop_index].target));
				continue;
			}
			commands[base_loop_index + 1].labels.insert(commands[base_loop_index + 1].labels.begin(), RANGE(commands[base_loop_index].labels));
			commands.erase(commands.begin() + base_loop_index);
			if (loop_index > base_loop_index) {
				loop_index -= 1;
			}
			if (insert_index > base_loop_index) {
				insert_index -= 1;
			}
		}

		if (ticks_to_insert_at_loop < 0) {
			int32_t current_loop_tick, current_end_tick;
			calc_channel_length(commands, current_loop_tick, current_end_tick);
			assert(current_loop_tick < old_loop_tick);

			ticks_to_insert_at_loop = final_intro_length - current_loop_tick;
			assert(ticks_to_insert_at_loop >= 0);
		}

		insert_ticks(selected_channel, commands, ticks_to_insert_at_loop, insert_index, speed_at_insert, volume_at_insert, fade_at_insert);

		calc_channel_length(commands, old_loop_tick, old_end_tick, &info);
	}

	const int32_t current_intro_length = old_loop_tick != -1 ? old_loop_tick : 0;
	assert(current_intro_length == final_intro_length);

	const int32_t current_body_length = old_end_tick - current_intro_length;
	ticks_to_insert_at_end = final_body_length - current_body_length;

	if (ticks_to_insert_at_end != 0) {
		int32_t drumkit_at_base;
		int32_t base_end_index = get_base_index(commands, current_intro_length, ticks_to_insert_at_end < 0 ? current_intro_length + final_body_length : current_intro_length + current_body_length, drumkit_at_base);
		if (base_end_index == -1) {
			// no base could be found, so start from the beginning...
			base_end_index = std::max(info.loop_index, 0);
			drumkit_at_base = info.drumkit_at_loop;
			// ...but advance to first potential note or rest
			while (
				commands[base_end_index].type != Command_Type::NOTE &&
				commands[base_end_index].type != Command_Type::DRUM_NOTE &&
				commands[base_end_index].type != Command_Type::REST &&
				commands[base_end_index].type != Command_Type::SOUND_CALL &&
				!(commands[base_end_index].type == Command_Type::SOUND_LOOP && commands[base_end_index].sound_loop.loop_count != 0)
			) {
				if (base_end_index == info.end_index) {
					base_end_index = std::max(info.loop_index, 0);
					drumkit_at_base = info.drumkit_at_loop;
					break;
				}
				assert(commands[base_end_index].type != Command_Type::SOUND_RET);
				if (
					commands[base_end_index].type == Command_Type::SOUND_JUMP ||
					commands[base_end_index].type == Command_Type::SOUND_LOOP
				) {
					int32_t next_index = itr_index(commands, find_note_with_label(commands, commands[base_end_index].target));
					if (next_index == info.end_index) break;
					base_end_index = next_index;
					continue;
				}
				if (commands[base_end_index].type == Command_Type::TOGGLE_NOISE) {
					drumkit_at_base = commands[base_end_index].toggle_noise.drumkit;
				}
				if (base_end_index + 1 == info.end_index) break;
				base_end_index += 1;
			}
		}
		else {
			// the first command after the last note that fits
			base_end_index += 1;
		}

		if (info.end_index == 0) {
			for (size_t i = commands[0].labels.size() - 1; i < commands[0].labels.size(); --i) {
				if (commands[0].labels[i] == channel_label) {
					commands[0].labels.erase(commands[0].labels.begin() + i);
				}
			}
		}

		int32_t end_index = info.end_index;

		int32_t insert_index = base_end_index;
		int32_t speed_at_insert = info.speed_at_end;
		int32_t volume_at_insert = info.volume_at_end;
		int32_t fade_at_insert = info.fade_at_end;

		bool reinitializing = base_end_index == std::max(info.loop_index, 0);

		if (reinitializing) {
			if (selected_channel != 4) {
				Command command = Command(Command_Type::NOTE_TYPE);
				command.note_type.speed = 12;
				command.note_type.volume = selected_channel == 3 ? 1 : 15;
				command.note_type.fade = 0;
				if (base_end_index != end_index) {
					command.labels = std::move(commands[base_end_index].labels);
					commands[base_end_index].labels.clear();
				}
				commands.insert(commands.begin() + base_end_index, command);
				if (end_index >= base_end_index) end_index += 1;
				insert_index += 1;
				base_end_index += 1;
				speed_at_insert = command.note_type.speed;
				volume_at_insert = command.note_type.volume;
				fade_at_insert = command.note_type.fade;
			}
			else {
				if (info.loop_index == -1) {
					Command command = Command(Command_Type::TOGGLE_NOISE);
					command.toggle_noise.drumkit = 0;
					if (base_end_index != end_index) {
						command.labels = std::move(commands[base_end_index].labels);
						commands[base_end_index].labels.clear();
					}
					commands.insert(commands.begin() + base_end_index, command);
					if (end_index >= base_end_index) end_index += 1;
					insert_index += 1;
					base_end_index += 1;
					drumkit_at_base = command.toggle_noise.drumkit;
				}

				Command command = Command(Command_Type::DRUM_SPEED);
				command.drum_speed.speed = 12;
				if (base_end_index != end_index) {
					command.labels = std::move(commands[base_end_index].labels);
					commands[base_end_index].labels.clear();
				}
				commands.insert(commands.begin() + base_end_index, command);
				if (end_index >= base_end_index) end_index += 1;
				insert_index += 1;
				base_end_index += 1;
				speed_at_insert = command.drum_speed.speed;
			}
		}

		// preserve end speed/drumkit
		if (selected_channel != 4) {
			Command command = Command(Command_Type::NOTE_TYPE);
			command.note_type.speed = info.speed_at_end;
			command.note_type.volume = info.volume_at_end;
			command.note_type.fade = info.fade_at_end;
			if (base_end_index != end_index) {
				command.labels = std::move(commands[base_end_index].labels);
				commands[base_end_index].labels.clear();
			}
			commands.insert(commands.begin() + base_end_index, command);
			if (end_index >= base_end_index) end_index += 1;
			if (!reinitializing) insert_index += 1;
			base_end_index += 1;
		}
		else {
			if (drumkit_at_base != info.drumkit_at_end) {
				if (drumkit_at_base != -1) {
					// turn off
					Command command = Command(Command_Type::TOGGLE_NOISE);
					command.toggle_noise.drumkit = -1;
					if (base_end_index != end_index) {
						command.labels = std::move(commands[base_end_index].labels);
						commands[base_end_index].labels.clear();
					}
					commands.insert(commands.begin() + base_end_index, command);
					if (end_index >= base_end_index) end_index += 1;
					if (!reinitializing) insert_index += 1;
					base_end_index += 1;
				}
				if (info.drumkit_at_end != -1) {
					// turn on
					Command command = Command(Command_Type::TOGGLE_NOISE);
					command.toggle_noise.drumkit = info.drumkit_at_end;
					if (base_end_index != end_index) {
						command.labels = std::move(commands[base_end_index].labels);
						commands[base_end_index].labels.clear();
					}
					commands.insert(commands.begin() + base_end_index, command);
					if (end_index >= base_end_index) end_index += 1;
					if (!reinitializing) insert_index += 1;
					base_end_index += 1;
				}
			}

			Command command = Command(Command_Type::DRUM_SPEED);
			command.drum_speed.speed = info.speed_at_end;
			if (base_end_index != end_index) {
				command.labels = std::move(commands[base_end_index].labels);
				commands[base_end_index].labels.clear();
			}
			commands.insert(commands.begin() + base_end_index, command);
			if (end_index >= base_end_index) end_index += 1;
			if (!reinitializing) insert_index += 1;
			base_end_index += 1;
		}

		if (original_info.end_index == 0) {
			commands[0].labels.insert(commands[0].labels.begin(), channel_label);
		}

		while (end_index != base_end_index) {
			if (
				(commands[base_end_index].type == Command_Type::SOUND_JUMP) ||
				(commands[base_end_index].type == Command_Type::SOUND_LOOP && commands[base_end_index].sound_loop.loop_count == 0)
			) {
				base_end_index = itr_index(commands, find_note_with_label(commands, commands[base_end_index].target));
				continue;
			}
			commands[base_end_index + 1].labels.insert(commands[base_end_index + 1].labels.begin(), RANGE(commands[base_end_index].labels));
			commands.erase(commands.begin() + base_end_index);
			if (end_index > base_end_index) {
				end_index -= 1;
			}
			if (insert_index > base_end_index) {
				insert_index -= 1;
			}
		}

		if (ticks_to_insert_at_end < 0) {
			int32_t current_loop_tick, current_end_tick;
			calc_channel_length(commands, current_loop_tick, current_end_tick);
			assert(current_loop_tick == old_loop_tick);
			assert(current_end_tick < old_end_tick);

			const int32_t current_intro_length = current_loop_tick != -1 ? current_loop_tick : 0;
			assert(current_intro_length == final_intro_length);

			const int32_t current_body_length = current_end_tick - current_intro_length;
			ticks_to_insert_at_end = final_body_length - current_body_length;
			assert(ticks_to_insert_at_end >= 0);
		}

		insert_ticks(selected_channel, commands, ticks_to_insert_at_end, insert_index, speed_at_insert, volume_at_insert, fade_at_insert);
	}
}

int32_t Song::put_note(const int selected_channel, const std::set<int32_t> &selected_boxes, Pitch pitch, int32_t octave, int32_t old_octave, int32_t old_speed, int32_t prev_length, int32_t prev_speed, int32_t index, int32_t tick, int32_t tick_offset, bool set_drumkit) {
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

	int32_t length = 1;
	int32_t speed = old_speed;

	if (tick_offset == 0) {
		commands[index].type = new_type;
		commands[index].note.length = length;
		commands[index].note.pitch = pitch;
	}
	else {
		commands[index].note.length = tick_offset;

		Command command = Command(new_type);
		command.note.length = length;
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

	if (octave != old_octave) {
		Command command = Command(Command_Type::OCTAVE);
		command.octave.octave = octave;
		command.labels = std::move(commands[index].labels);
		commands[index].labels.clear();
		commands.insert(commands.begin() + index, command);
		index += 1;

		command.octave.octave = old_octave;
		command.labels.clear();
		commands.insert(commands.begin() + index + 1, command);
	}

	if (set_drumkit) {
		Command command = Command(Command_Type::TOGGLE_NOISE);
		command.toggle_noise.drumkit = 0;
		command.labels = std::move(commands[index].labels);
		commands[index].labels.clear();
		commands.insert(commands.begin() + index, command);
		index += 1;

		command.toggle_noise.drumkit = -1;
		command.labels.clear();
		commands.insert(commands.begin() + index + 1, command);
	}

	if (prev_speed != 0 && (length != prev_length || speed != prev_speed)) {
		Note_View note_view = get_note_view(commands, index);
		assert(note_view.speed == old_speed);

		int32_t current_duration = length * speed;
		int32_t desired_duration = prev_length * prev_speed;
		int32_t min_desired_duration = length * prev_speed;

		int32_t final_length = length;
		int32_t final_speed = speed;
		int32_t final_duration = current_duration;

		if (desired_duration <= current_duration || is_followed_by_n_ticks_of_rest(commands.begin() + index, commands.end(), desired_duration - current_duration, note_view.speed)) {
			final_length = prev_length;
			final_speed = prev_speed;
			final_duration = desired_duration;
		}
		else if (
			speed != prev_speed &&
			(min_desired_duration <= current_duration || is_followed_by_n_ticks_of_rest(commands.begin() + index, commands.end(), min_desired_duration - current_duration, note_view.speed))
		) {
			final_speed = prev_speed;
			final_duration = min_desired_duration;
		}

		length = final_length;
		commands[index].note.length = length;

		if (speed != final_speed) {
			speed = final_speed;
			Command command = Command(selected_channel == 4 ? Command_Type::DRUM_SPEED : Command_Type::NOTE_TYPE);
			command.note_type.speed = speed;
			command.note_type.volume = note_view.volume;
			command.note_type.fade = note_view.fade;
			command.labels = std::move(commands[index].labels);
			commands[index].labels.clear();
			commands.insert(commands.begin() + index, command);
			index += 1;

			command.note_type.speed = note_view.speed;
			command.labels.clear();
			commands.insert(commands.begin() + index + 1, command);
			index += 1;
		}

		if (final_duration < current_duration) {
			insert_ticks(selected_channel, commands, current_duration - final_duration, index + 1, note_view.speed, note_view.volume, note_view.fade);
		}
		if (final_duration > current_duration) {
			erase_ticks(selected_channel, commands, final_duration - current_duration, index, note_view.speed, note_view.volume, note_view.fade);
		}
	}

	postprocess(commands);

	_modified = true;

	return length * speed;
}

void Song::set_speed(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view, int32_t speed) {
	remember(selected_channel, selected_boxes, Song_State::Action::SET_SPEED);
	std::vector<Command> &commands = channel_commands(selected_channel);

	for (auto note_itr = selected_notes.rbegin(); note_itr != selected_notes.rend(); ++note_itr) {
		if (selected_channel == 4) {
			Note_View note_view = find_note_view(view, *note_itr);

			Command command = Command(Command_Type::DRUM_SPEED);
			command.drum_speed.speed = note_view.speed;
			commands.insert(commands.begin() + *note_itr + 1, command);

			if (speed < note_view.speed) {
				int ticks_to_insert = (note_view.length * note_view.speed) - (note_view.length * speed);
				insert_ticks(selected_channel, commands, ticks_to_insert, *note_itr + 1, speed, 0, 0);
			}
			else if (speed > note_view.speed) {
				int ticks_to_erase = (note_view.length * speed) - (note_view.length * note_view.speed);
				erase_ticks(selected_channel, commands, ticks_to_erase, *note_itr, note_view.speed, 0, 0);
			}

			command.drum_speed.speed = speed;
			command.labels = std::move(commands[*note_itr].labels);
			commands[*note_itr].labels.clear();
			commands.insert(commands.begin() + *note_itr, command);
		}
		else {
			Note_View note_view = find_note_view(view, *note_itr);

			Command command = Command(Command_Type::NOTE_TYPE);
			command.note_type.speed = note_view.speed;
			command.note_type.volume = note_view.volume;
			command.note_type.fade = note_view.fade;
			commands.insert(commands.begin() + *note_itr + 1, command);

			if (speed < note_view.speed) {
				int ticks_to_insert = (note_view.length * note_view.speed) - (note_view.length * speed);
				insert_ticks(selected_channel, commands, ticks_to_insert, *note_itr + 1, speed, note_view.volume, note_view.fade);
			}
			else if (speed > note_view.speed) {
				int ticks_to_erase = (note_view.length * speed) - (note_view.length * note_view.speed);
				erase_ticks(selected_channel, commands, ticks_to_erase, *note_itr, note_view.speed, note_view.volume, note_view.fade);
			}

			command.note_type.speed = speed;
			command.labels = std::move(commands[*note_itr].labels);
			commands[*note_itr].labels.clear();
			commands.insert(commands.begin() + *note_itr, command);
		}
	}

	postprocess(commands);

	_modified = true;
}

void Song::set_volume(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view, int32_t volume) {
	remember(selected_channel, selected_boxes, Song_State::Action::SET_VOLUME);
	std::vector<Command> &commands = channel_commands(selected_channel);

	for (auto note_itr = selected_notes.rbegin(); note_itr != selected_notes.rend(); ++note_itr) {
		Note_View note_view = find_note_view(view, *note_itr);
		Command command = Command(Command_Type::VOLUME_ENVELOPE);
		command.volume_envelope.volume = volume;
		command.volume_envelope.fade = note_view.fade;
		command.labels = std::move(commands[*note_itr].labels);
		commands[*note_itr].labels.clear();
		commands.insert(commands.begin() + *note_itr, command);
	}

	postprocess(commands);

	_modified = true;
}

void Song::set_fade(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view, int32_t fade) {
	remember(selected_channel, selected_boxes, Song_State::Action::SET_FADE);
	std::vector<Command> &commands = channel_commands(selected_channel);

	for (auto note_itr = selected_notes.rbegin(); note_itr != selected_notes.rend(); ++note_itr) {
		Note_View note_view = find_note_view(view, *note_itr);
		Command command = Command(Command_Type::VOLUME_ENVELOPE);
		command.volume_envelope.volume = note_view.volume;
		command.volume_envelope.fade = fade;
		command.labels = std::move(commands[*note_itr].labels);
		commands[*note_itr].labels.clear();
		commands.insert(commands.begin() + *note_itr, command);
	}

	postprocess(commands);

	_modified = true;
}

void Song::set_vibrato_delay(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view, int32_t delay) {
	remember(selected_channel, selected_boxes, Song_State::Action::SET_VIBRATO_DELAY);
	std::vector<Command> &commands = channel_commands(selected_channel);

	for (auto note_itr = selected_notes.rbegin(); note_itr != selected_notes.rend(); ++note_itr) {
		Note_View note_view = find_note_view(view, *note_itr);
		Command command = Command(Command_Type::VIBRATO);
		command.vibrato.delay = delay;
		command.vibrato.extent = note_view.vibrato_extent;
		command.vibrato.rate = note_view.vibrato_rate;
		command.labels = std::move(commands[*note_itr].labels);
		commands[*note_itr].labels.clear();
		commands.insert(commands.begin() + *note_itr, command);
	}

	postprocess(commands);

	_modified = true;
}

void Song::set_vibrato_extent(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view, int32_t extent) {
	remember(selected_channel, selected_boxes, Song_State::Action::SET_VIBRATO_EXTENT);
	std::vector<Command> &commands = channel_commands(selected_channel);

	for (auto note_itr = selected_notes.rbegin(); note_itr != selected_notes.rend(); ++note_itr) {
		Note_View note_view = find_note_view(view, *note_itr);
		Command command = Command(Command_Type::VIBRATO);
		command.vibrato.delay = note_view.vibrato_delay;
		command.vibrato.extent = extent;
		command.vibrato.rate = note_view.vibrato_rate;
		command.labels = std::move(commands[*note_itr].labels);
		commands[*note_itr].labels.clear();
		commands.insert(commands.begin() + *note_itr, command);
	}

	postprocess(commands);

	_modified = true;
}

void Song::set_vibrato_rate(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view, int32_t rate) {
	remember(selected_channel, selected_boxes, Song_State::Action::SET_VIBRATO_RATE);
	std::vector<Command> &commands = channel_commands(selected_channel);

	for (auto note_itr = selected_notes.rbegin(); note_itr != selected_notes.rend(); ++note_itr) {
		Note_View note_view = find_note_view(view, *note_itr);
		Command command = Command(Command_Type::VIBRATO);
		command.vibrato.delay = note_view.vibrato_delay;
		command.vibrato.extent = note_view.vibrato_extent;
		command.vibrato.rate = rate;
		command.labels = std::move(commands[*note_itr].labels);
		commands[*note_itr].labels.clear();
		commands.insert(commands.begin() + *note_itr, command);
	}

	postprocess(commands);

	_modified = true;
}

void Song::set_wave(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view, int32_t wave) {
	remember(selected_channel, selected_boxes, Song_State::Action::SET_WAVE);
	std::vector<Command> &commands = channel_commands(selected_channel);

	for (auto note_itr = selected_notes.rbegin(); note_itr != selected_notes.rend(); ++note_itr) {
		Note_View note_view = find_note_view(view, *note_itr);
		Command command = Command(Command_Type::VOLUME_ENVELOPE);
		command.volume_envelope.volume = note_view.volume;
		command.volume_envelope.wave = wave;
		command.labels = std::move(commands[*note_itr].labels);
		commands[*note_itr].labels.clear();
		commands.insert(commands.begin() + *note_itr, command);
	}

	postprocess(commands);

	_modified = true;
}

void Song::set_drumkit(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, int32_t drumkit) {
	remember(selected_channel, selected_boxes, Song_State::Action::SET_DRUMKIT);
	std::vector<Command> &commands = channel_commands(selected_channel);

	for (auto note_itr = selected_notes.rbegin(); note_itr != selected_notes.rend(); ++note_itr) {
		Command command = Command(Command_Type::TOGGLE_NOISE);
		command.toggle_noise.drumkit = -1;
		command.labels = std::move(commands[*note_itr].labels);
		commands[*note_itr].labels.clear();
		commands.insert(commands.begin() + *note_itr, command);

		command = Command(Command_Type::TOGGLE_NOISE);
		command.toggle_noise.drumkit = drumkit;
		commands.insert(commands.begin() + *note_itr + 1, command);
	}

	postprocess(commands);

	_modified = true;
}

void Song::set_duty(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, int32_t duty) {
	remember(selected_channel, selected_boxes, Song_State::Action::SET_DUTY);
	std::vector<Command> &commands = channel_commands(selected_channel);

	for (auto note_itr = selected_notes.rbegin(); note_itr != selected_notes.rend(); ++note_itr) {
		Command command = Command(Command_Type::DUTY_CYCLE);
		command.duty_cycle.duty = duty;
		command.labels = std::move(commands[*note_itr].labels);
		commands[*note_itr].labels.clear();
		commands.insert(commands.begin() + *note_itr, command);
	}

	postprocess(commands);

	_modified = true;
}

void Song::set_tempo(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, int32_t tempo) {
	remember(selected_channel, selected_boxes, Song_State::Action::SET_TEMPO);
	std::vector<Command> &commands = channel_commands(selected_channel);

	for (auto note_itr = selected_notes.rbegin(); note_itr != selected_notes.rend(); ++note_itr) {
		Command command = Command(Command_Type::TEMPO);
		command.tempo.tempo = tempo;
		command.labels = std::move(commands[*note_itr].labels);
		commands[*note_itr].labels.clear();
		commands.insert(commands.begin() + *note_itr, command);
	}

	postprocess(commands);

	_modified = true;
}

void Song::set_transpose_octaves(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view, int32_t octaves) {
	remember(selected_channel, selected_boxes, Song_State::Action::SET_TRANSPOSE_OCTAVES);
	std::vector<Command> &commands = channel_commands(selected_channel);

	for (auto note_itr = selected_notes.rbegin(); note_itr != selected_notes.rend(); ++note_itr) {
		Note_View note_view = find_note_view(view, *note_itr);
		Command command = Command(Command_Type::TRANSPOSE);
		command.transpose.num_octaves = octaves;
		command.transpose.num_pitches = note_view.transpose_pitches;
		command.labels = std::move(commands[*note_itr].labels);
		commands[*note_itr].labels.clear();
		commands.insert(commands.begin() + *note_itr, command);
	}

	postprocess(commands);

	_modified = true;
}

void Song::set_transpose_pitches(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view, int32_t pitches) {
	remember(selected_channel, selected_boxes, Song_State::Action::SET_TRANSPOSE_PITCHES);
	std::vector<Command> &commands = channel_commands(selected_channel);

	for (auto note_itr = selected_notes.rbegin(); note_itr != selected_notes.rend(); ++note_itr) {
		Note_View note_view = find_note_view(view, *note_itr);
		Command command = Command(Command_Type::TRANSPOSE);
		command.transpose.num_octaves = note_view.transpose_octaves;
		command.transpose.num_pitches = pitches;
		command.labels = std::move(commands[*note_itr].labels);
		commands[*note_itr].labels.clear();
		commands.insert(commands.begin() + *note_itr, command);
	}

	postprocess(commands);

	_modified = true;
}

void Song::set_slide_duration(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view, int32_t duration) {
	remember(selected_channel, selected_boxes, Song_State::Action::SET_SLIDE_DURATION);
	std::vector<Command> &commands = channel_commands(selected_channel);

	for (auto note_itr = selected_notes.rbegin(); note_itr != selected_notes.rend(); ++note_itr) {
		Note_View note_view = find_note_view(view, *note_itr);
		Command command = Command(Command_Type::PITCH_SLIDE);
		command.pitch_slide.duration = duration;
		command.pitch_slide.octave = note_view.slide_octave;
		command.pitch_slide.pitch = note_view.slide_pitch;
		command.labels = std::move(commands[*note_itr].labels);
		commands[*note_itr].labels.clear();
		commands.insert(commands.begin() + *note_itr, command);
	}

	postprocess(commands);

	_modified = true;
}

void Song::set_slide_octave(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view, int32_t octave) {
	remember(selected_channel, selected_boxes, Song_State::Action::SET_SLIDE_OCTAVE);
	std::vector<Command> &commands = channel_commands(selected_channel);

	for (auto note_itr = selected_notes.rbegin(); note_itr != selected_notes.rend(); ++note_itr) {
		Note_View note_view = find_note_view(view, *note_itr);
		Command command = Command(Command_Type::PITCH_SLIDE);
		command.pitch_slide.duration = note_view.slide_duration;
		command.pitch_slide.octave = octave;
		command.pitch_slide.pitch = note_view.slide_pitch;
		command.labels = std::move(commands[*note_itr].labels);
		commands[*note_itr].labels.clear();
		commands.insert(commands.begin() + *note_itr, command);
	}

	postprocess(commands);

	_modified = true;
}

void Song::set_slide_pitch(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view, Pitch pitch) {
	remember(selected_channel, selected_boxes, Song_State::Action::SET_SLIDE_PITCH);
	std::vector<Command> &commands = channel_commands(selected_channel);

	for (auto note_itr = selected_notes.rbegin(); note_itr != selected_notes.rend(); ++note_itr) {
		Note_View note_view = find_note_view(view, *note_itr);
		Command command = Command(Command_Type::PITCH_SLIDE);
		command.pitch_slide.duration = note_view.slide_duration;
		command.pitch_slide.octave = note_view.slide_octave;
		command.pitch_slide.pitch = pitch;
		command.labels = std::move(commands[*note_itr].labels);
		commands[*note_itr].labels.clear();
		commands.insert(commands.begin() + *note_itr, command);
	}

	postprocess(commands);

	_modified = true;
}

void Song::set_slide(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, int32_t duration, int32_t octave, Pitch pitch) {
	remember(selected_channel, selected_boxes, Song_State::Action::SET_SLIDE);
	std::vector<Command> &commands = channel_commands(selected_channel);

	for (auto note_itr = selected_notes.rbegin(); note_itr != selected_notes.rend(); ++note_itr) {
		Command command = Command(Command_Type::PITCH_SLIDE);
		command.pitch_slide.duration = duration;
		command.pitch_slide.octave = octave;
		command.pitch_slide.pitch = pitch;
		command.labels = std::move(commands[*note_itr].labels);
		commands[*note_itr].labels.clear();
		commands.insert(commands.begin() + *note_itr, command);
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

void Song::move_left(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view) {
	remember(selected_channel, selected_boxes, Song_State::Action::MOVE_LEFT);
	std::vector<Command> &commands = channel_commands(selected_channel);

	int32_t offset = 0;

	for (auto note_itr = selected_notes.begin(); note_itr != selected_notes.end(); ++note_itr) {
		auto command_itr = commands.rbegin() + (commands.size() - 1 - (*note_itr + offset));
		assert(commands[*note_itr + offset].labels.size() == 0);
		Note_View note_view = find_note_view(view, *note_itr);

		const auto find_preceding_rest = [&](decltype(command_itr) itr) {
			++itr;
			while (itr != commands.rend()) {
				if (itr->type == Command_Type::REST) {
					return itr_index(commands, itr.base() - 1);
				}
				++itr;
			}
			assert(false);
			return itr_index(commands, commands.end());
		};

		int32_t rest_index = find_preceding_rest(command_itr);
		Note_View rest_view = get_note_view(commands, rest_index);

		Command command = Command(selected_channel == 4 ? Command_Type::DRUM_SPEED : Command_Type::NOTE_TYPE);
		command.note_type.speed = rest_view.speed;
		command.note_type.volume = note_view.volume;
		command.note_type.fade = note_view.fade;
		commands.insert(commands.begin() + (*note_itr + offset) + 1, command);

		command.type = Command_Type::REST;
		command.rest.length = 1;
		commands.insert(commands.begin() + (*note_itr + offset) + 2, command);

		command.type = selected_channel == 4 ? Command_Type::DRUM_SPEED : Command_Type::NOTE_TYPE;
		command.note_type.speed = note_view.speed;
		command.note_type.volume = note_view.volume;
		command.note_type.fade = note_view.fade;
		commands.insert(commands.begin() + (*note_itr + offset) + 3, command);

		if (commands[rest_index].rest.length == 1) {
			commands[rest_index + 1].labels.insert(commands[rest_index + 1].labels.begin(), RANGE(commands[rest_index].labels));
			commands.erase(commands.begin() + rest_index);
			offset += 2;
		}
		else {
			commands[rest_index].rest.length -= 1;
			offset += 3;
		}
	}

	postprocess(commands);

	_modified = true;
}

void Song::move_right(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view) {
	remember(selected_channel, selected_boxes, Song_State::Action::MOVE_RIGHT);
	std::vector<Command> &commands = channel_commands(selected_channel);

	for (auto note_itr = selected_notes.rbegin(); note_itr != selected_notes.rend(); ++note_itr) {
		auto command_itr = commands.begin() + *note_itr;
		Note_View note_view = find_note_view(view, *note_itr);

		const auto find_following_rest = [&](decltype(command_itr) itr) {
			++itr;
			while (itr != commands.end()) {
				if (itr->type == Command_Type::REST) {
					return itr_index(commands, itr);
				}
				++itr;
			}
			assert(false);
			return itr_index(commands, commands.end());
		};

		int32_t rest_index = find_following_rest(command_itr);
		Note_View rest_view = get_note_view(commands, rest_index);

		assert(commands[rest_index].labels.size() == 0);
		if (commands[rest_index].rest.length == 1) {
			commands.erase(commands.begin() + rest_index);
		}
		else {
			commands[rest_index].rest.length -= 1;
		}

		Command command = Command(selected_channel == 4 ? Command_Type::DRUM_SPEED : Command_Type::NOTE_TYPE);
		command.note_type.speed = rest_view.speed;
		command.note_type.volume = note_view.volume;
		command.note_type.fade = note_view.fade;
		command.labels = std::move(commands[*note_itr].labels);
		commands[*note_itr].labels.clear();
		commands.insert(commands.begin() + *note_itr, command);
		command.labels.clear();

		command.type = Command_Type::REST;
		command.rest.length = 1;
		commands.insert(commands.begin() + *note_itr + 1, command);

		command.type = selected_channel == 4 ? Command_Type::DRUM_SPEED : Command_Type::NOTE_TYPE;
		command.note_type.speed = note_view.speed;
		command.note_type.volume = note_view.volume;
		command.note_type.fade = note_view.fade;
		commands.insert(commands.begin() + *note_itr + 2, command);
	}

	postprocess(commands);

	_modified = true;
}

void Song::shorten(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, int32_t tick) {
	remember(selected_channel, selected_boxes, Song_State::Action::SHORTEN, tick);
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

void Song::lengthen(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view, int32_t tick) {
	remember(selected_channel, selected_boxes, Song_State::Action::LENGTHEN, tick);
	std::vector<Command> &commands = channel_commands(selected_channel);

	for (auto note_itr = selected_notes.rbegin(); note_itr != selected_notes.rend(); ++note_itr) {
		Note_View note_view = find_note_view(view, *note_itr);
		commands[*note_itr].note.length += 1;
		erase_ticks(selected_channel, commands, note_view.speed, *note_itr, note_view.speed, note_view.volume, note_view.fade);
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

	resize_channel(selected_channel, commands, channel_label(selected_channel), channel_loop_tick(selected_channel), channel_end_tick(selected_channel));

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
		commands[index].note.pitch == commands[index - 1].note.pitch &&
		commands[index].note.length + commands[index - 1].note.length <= 16 &&
		commands[index].labels.size() == 0
	);

	commands[index - 1].note.length += commands[index].note.length;
	commands.erase(commands.begin() + index);

	postprocess(commands);

	_modified = true;
}

void Song::resize_song(const Song_Options_Dialog::Song_Options &options) {
	_history.clear();
	_future.clear();

	if (options.channel_1) {
		std::vector<Command> &commands = channel_commands(1);
		resize_channel(1, commands, channel_label(1), options.looping ? options.channel_1_loop_tick : -1, options.channel_1_end_tick);
		postprocess(commands);
		if (options.looping) _channel_1_loop_tick = options.channel_1_loop_tick;
		_channel_1_end_tick  = options.channel_1_end_tick;
	}
	if (options.channel_2) {
		std::vector<Command> &commands = channel_commands(2);
		resize_channel(2, commands, channel_label(2), options.looping ? options.channel_2_loop_tick : -1, options.channel_2_end_tick);
		postprocess(commands);
		if (options.looping) _channel_2_loop_tick = options.channel_2_loop_tick;
		_channel_2_end_tick  = options.channel_2_end_tick;
	}
	if (options.channel_3) {
		std::vector<Command> &commands = channel_commands(3);
		resize_channel(3, commands, channel_label(3), options.looping ? options.channel_3_loop_tick : -1, options.channel_3_end_tick);
		postprocess(commands);
		if (options.looping) _channel_3_loop_tick = options.channel_3_loop_tick;
		_channel_3_end_tick  = options.channel_3_end_tick;
	}
	if (options.channel_4) {
		std::vector<Command> &commands = channel_commands(4);
		resize_channel(4, commands, channel_label(4), options.looping ? options.channel_4_loop_tick : -1, options.channel_4_end_tick);
		postprocess(commands);
		if (options.looping) _channel_4_loop_tick = options.channel_4_loop_tick;
		_channel_4_end_tick  = options.channel_4_end_tick;
	}

	_modified = true;
}

void Song::reduce_loop(const int selected_channel, const std::set<int32_t> &selected_boxes, int32_t tick, int32_t loop_index, int32_t loop_length, Note_View start_view, Note_View end_view) {
	remember(selected_channel, selected_boxes, Song_State::Action::REDUCE_LOOP, tick);
	std::vector<Command> &commands = channel_commands(selected_channel);

	assert(commands[loop_index].type == Command_Type::SOUND_LOOP);
	assert(commands[loop_index].sound_loop.loop_count > 1);
	assert(end_view.speed != 0);

	commands[loop_index].sound_loop.loop_count -= 1;

	insert_ticks(selected_channel, commands, loop_length, loop_index + 1, end_view.speed, end_view.volume, end_view.fade);

	if (commands[loop_index].sound_loop.loop_count == 1) {
		std::string target = commands[loop_index].target;

		commands[loop_index + 1].labels.insert(commands[loop_index + 1].labels.begin(), RANGE(commands[loop_index].labels));
		commands.erase(commands.begin() + loop_index);

		if (target != channel_label(selected_channel) && target.find_first_of(".") != std::string::npos && !is_label_referenced(commands, target)) {
			delete_label(commands, target);
		}
	}

	postprocess(commands);

	_modified = true;
}

void Song::extend_loop(const int selected_channel, const std::set<int32_t> &selected_boxes, int32_t tick, int32_t loop_index, int32_t loop_length, Note_View start_view, Note_View end_view) {
	remember(selected_channel, selected_boxes, Song_State::Action::EXTEND_LOOP, tick);
	std::vector<Command> &commands = channel_commands(selected_channel);

	assert(commands[loop_index].type == Command_Type::SOUND_LOOP);
	assert(commands[loop_index].sound_loop.loop_count > 1);
	assert(end_view.speed != 0);

	commands[loop_index].sound_loop.loop_count += 1;

	erase_ticks(selected_channel, commands, loop_length, loop_index, end_view.speed, end_view.volume, end_view.fade);

	postprocess(commands);

	_modified = true;
}

void Song::unroll_loop(const int selected_channel, const std::set<int32_t> &selected_boxes, int32_t tick, int32_t loop_index, const std::vector<Command> &snippet) {
	remember(selected_channel, selected_boxes, Song_State::Action::UNROLL_LOOP, tick);
	std::vector<Command> &commands = channel_commands(selected_channel);

	assert(commands[loop_index].type == Command_Type::SOUND_LOOP);
	assert(commands[loop_index].sound_loop.loop_count > 1);

	commands[loop_index].sound_loop.loop_count -= 1;

	commands.insert(commands.begin() + (loop_index + 1), RANGE(snippet));

	if (commands[loop_index].sound_loop.loop_count == 1) {
		std::string target = commands[loop_index].target;

		commands[loop_index + 1].labels.insert(commands[loop_index + 1].labels.begin(), RANGE(commands[loop_index].labels));
		commands.erase(commands.begin() + loop_index);

		if (target != channel_label(selected_channel) && target.find_first_of(".") != std::string::npos && !is_label_referenced(commands, target)) {
			delete_label(commands, target);
		}
	}

	postprocess(commands);

	_modified = true;
}

void Song::create_loop(const int selected_channel, const std::set<int32_t> &selected_boxes, int32_t tick, int32_t start_index, int32_t end_index, int32_t loop_length, Note_View start_view, Note_View end_view) {
	remember(selected_channel, selected_boxes, Song_State::Action::CREATE_LOOP, tick);
	std::vector<Command> &commands = channel_commands(selected_channel);

	assert(end_view.speed != 0);

	std::string scope = get_scope(commands, start_index);
	int loop_number = 1;
	std::string loop_label = get_next_loop_label(commands, scope, loop_number);
	commands[start_index].labels.push_back(loop_label);

	if (start_view.octave != end_view.octave) {
		Command command = Command(Command_Type::OCTAVE);
		command.octave.octave = start_view.octave;
		commands.insert(commands.begin() + end_index + 1, command);
		end_index += 1;
	}

	Command loop = Command(Command_Type::SOUND_LOOP);
	loop.target = loop_label;
	loop.sound_loop.loop_count = 2;
	commands.insert(commands.begin() + end_index + 1, loop);

	erase_ticks(selected_channel, commands, loop_length, end_index + 1, end_view.speed, end_view.volume, end_view.fade);

	postprocess(commands);

	_modified = true;
}

void Song::delete_call(const int selected_channel, const std::set<int32_t> &selected_boxes, int32_t tick, int32_t call_index, int32_t ambiguous_ticks, int32_t unambiguous_ticks, Note_View start_view, Note_View end_view) {
	remember(selected_channel, selected_boxes, Song_State::Action::DELETE_CALL, tick);
	std::vector<Command> &commands = channel_commands(selected_channel);

	assert(commands[call_index].type == Command_Type::SOUND_CALL);
	assert(start_view.speed != 0);
	assert(end_view.speed != 0);

	insert_ticks(selected_channel, commands, unambiguous_ticks, call_index + 1, end_view.speed, end_view.volume, end_view.fade);

	if (selected_channel != 4) {
		int32_t index = call_index + 1;
		if (end_view.speed != start_view.speed || (ambiguous_ticks && unambiguous_ticks)) {
			Command command = Command(Command_Type::NOTE_TYPE);
			command.note_type.speed = end_view.speed;
			command.note_type.volume = end_view.volume;
			command.note_type.fade = end_view.fade;
			commands.insert(commands.begin() + index, command);
			index += 1;
		}

		Command command = Command(Command_Type::OCTAVE);
		command.octave.octave = end_view.octave;
		commands.insert(commands.begin() + index, command);
		index += 1;
	}
	else {
		int32_t index = call_index + 1;
		if (start_view.drumkit != end_view.drumkit) {
			if (start_view.drumkit != -1) {
				// turn off
				Command command = Command(Command_Type::TOGGLE_NOISE);
				command.toggle_noise.drumkit = -1;
				commands.insert(commands.begin() + index, command);
				index += 1;
			}
			if (end_view.drumkit != -1) {
				// turn on
				Command command = Command(Command_Type::TOGGLE_NOISE);
				command.toggle_noise.drumkit = end_view.drumkit;
				commands.insert(commands.begin() + index, command);
				index += 1;
			}
		}

		if (end_view.speed != start_view.speed || (ambiguous_ticks && unambiguous_ticks)) {
			Command command = Command(Command_Type::DRUM_SPEED);
			command.drum_speed.speed = end_view.speed;
			commands.insert(commands.begin() + index, command);
			index += 1;
		}

		Command command = Command(Command_Type::OCTAVE);
		command.octave.octave = end_view.octave;
		commands.insert(commands.begin() + index, command);
		index += 1;
	}

	insert_ticks(selected_channel, commands, ambiguous_ticks, call_index + 1);

	commands[call_index + 1].labels.insert(commands[call_index + 1].labels.begin(), RANGE(commands[call_index].labels));
	commands.erase(commands.begin() + call_index); // TODO: delete snippet if call target is now unreferenced

	postprocess(commands);

	_modified = true;
}

void Song::unpack_call(const int selected_channel, const std::set<int32_t> &selected_boxes, int32_t tick, int32_t call_index, const std::vector<Command> &snippet) {
	remember(selected_channel, selected_boxes, Song_State::Action::UNPACK_CALL, tick);
	std::vector<Command> &commands = channel_commands(selected_channel);

	assert(commands[call_index].type == Command_Type::SOUND_CALL);

	commands.insert(commands.begin() + (call_index + 1), RANGE(snippet));

	commands[call_index + 1].labels.insert(commands[call_index + 1].labels.begin(), RANGE(commands[call_index].labels));
	commands.erase(commands.begin() + call_index); // TODO: delete snippet if call target is now unreferenced

	postprocess(commands);

	_modified = true;
}

void Song::create_call(const int selected_channel, const std::set<int32_t> &selected_boxes, int32_t tick, int32_t start_index, int32_t end_index, const std::vector<Command> &snippet) {
	remember(selected_channel, selected_boxes, Song_State::Action::CREATE_CALL, tick);
	std::vector<Command> &commands = channel_commands(selected_channel);

	assert(snippet.size() > 0 && snippet[0].labels.size() > 0);

	commands.insert(commands.end(), RANGE(snippet));

	Command call = Command(Command_Type::SOUND_CALL);
	call.target = snippet[0].labels[0];
	call.labels = std::move(commands[start_index].labels);
	commands.insert(commands.begin() + end_index + 1, call);

	for (int32_t i = end_index; i >= start_index; --i) {
		commands.erase(commands.begin() + i);
	}

	postprocess(commands);

	_modified = true;
}

void Song::insert_call(const int selected_channel, const std::set<int32_t> &selected_boxes, int32_t tick, int32_t tick_offset, int32_t insert_index, const std::string &target_label, int32_t call_length, Note_View insert_view, Note_View start_view, Note_View end_view) {
	remember(selected_channel, selected_boxes, Song_State::Action::INSERT_CALL, tick);
	std::vector<Command> &commands = channel_commands(selected_channel);

	assert(commands[insert_index].type == Command_Type::REST);

	int32_t tail_length = commands[insert_index].rest.length - tick_offset / insert_view.speed;
	assert(tail_length > 0);

	if (tail_length < commands[insert_index].rest.length) {
		Command command = Command(Command_Type::REST);
		command.rest.length = tail_length;
		commands[insert_index].rest.length -= tail_length;
		commands.insert(commands.begin() + insert_index + 1, command);
		insert_index += 1;
	}

	if (start_view.octave != insert_view.octave) {
		Command command = Command(Command_Type::OCTAVE);
		command.octave.octave = start_view.octave;
		command.labels = std::move(commands[insert_index].labels);
		commands[insert_index].labels.clear();
		commands.insert(commands.begin() + insert_index, command);
		insert_index += 1;
	}

	if (start_view.speed != insert_view.speed) {
		Command command = Command(selected_channel == 4 ? Command_Type::DRUM_SPEED : Command_Type::NOTE_TYPE);
		command.note_type.speed = start_view.speed;
		command.note_type.volume = start_view.volume;
		command.note_type.fade = start_view.fade;
		command.labels = std::move(commands[insert_index].labels);
		commands[insert_index].labels.clear();
		commands.insert(commands.begin() + insert_index, command);
		insert_index += 1;
	}

	if (selected_channel == 4 && start_view.drumkit != insert_view.drumkit) {
		if (insert_view.drumkit != -1) {
			// turn off
			Command command = Command(Command_Type::TOGGLE_NOISE);
			command.toggle_noise.drumkit = -1;
			command.labels = std::move(commands[insert_index].labels);
			commands[insert_index].labels.clear();
			commands.insert(commands.begin() + insert_index, command);
			insert_index += 1;
		}
		if (start_view.drumkit != -1) {
			// turn on
			Command command = Command(Command_Type::TOGGLE_NOISE);
			command.toggle_noise.drumkit = start_view.drumkit;
			command.labels = std::move(commands[insert_index].labels);
			commands[insert_index].labels.clear();
			commands.insert(commands.begin() + insert_index, command);
			insert_index += 1;
		}
	}

	Command call = Command(Command_Type::SOUND_CALL);
	call.target = target_label;
	call.labels = std::move(commands[insert_index].labels);
	commands[insert_index].labels.clear();
	commands.insert(commands.begin() + insert_index, call);

	{
		Command command = Command(Command_Type::OCTAVE);
		command.octave.octave = insert_view.octave;
		commands.insert(commands.begin() + insert_index + 1, command);
		insert_index += 1;
	}

	{
		Command command = Command(selected_channel == 4 ? Command_Type::DRUM_SPEED : Command_Type::NOTE_TYPE);
		command.note_type.speed = insert_view.speed;
		command.note_type.volume = insert_view.volume;
		command.note_type.fade = insert_view.fade;
		commands.insert(commands.begin() + insert_index + 1, command);
		insert_index += 1;
	}

	if (selected_channel == 4 && insert_view.drumkit != end_view.drumkit) {
		if (end_view.drumkit != -1) {
			// turn off
			Command command = Command(Command_Type::TOGGLE_NOISE);
			command.toggle_noise.drumkit = -1;
			commands.insert(commands.begin() + insert_index + 1, command);
			insert_index += 1;
		}
		if (insert_view.drumkit != -1) {
			// turn on
			Command command = Command(Command_Type::TOGGLE_NOISE);
			command.toggle_noise.drumkit = insert_view.drumkit;
			commands.insert(commands.begin() + insert_index + 1, command);
			insert_index += 1;
		}
	}

	erase_ticks(selected_channel, commands, call_length, insert_index, insert_view.speed, insert_view.volume, insert_view.fade);

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
			if (command.toggle_noise.drumkit != -1) {
				str = str + " " + std::to_string(command.toggle_noise.drumkit);
			}
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
	case Parsed_Song::Result::SONG_UNFINISHED_CALL:
		return "Channel " + std::to_string(parsed_song.channel_number()) +
			": Unfinished call.";
	case Parsed_Song::Result::SONG_NO_DRUMKIT_SELECTED:
		return "Channel " + std::to_string(parsed_song.channel_number()) +
			": drum_note: no drumkit selected.";
	case Parsed_Song::Result::SONG_TOGGLE_NOISE_ALREADY_DISABLED:
		return "Channel " + std::to_string(parsed_song.channel_number()) +
			": toggle_noise: noise already disabled.";
	case Parsed_Song::Result::SONG_TOGGLE_NOISE_ALREADY_ENABLED:
		return "Channel " + std::to_string(parsed_song.channel_number()) +
			": toggle_noise: noise already enabled.";
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
	case Song_State::Action::SET_SPEED:
		return "Set speed";
	case Song_State::Action::SET_VOLUME:
		return "Set volume";
	case Song_State::Action::SET_FADE:
		return "Set fade";
	case Song_State::Action::SET_VIBRATO_DELAY:
		return "Set vibrato delay";
	case Song_State::Action::SET_VIBRATO_EXTENT:
		return "Set vibrato depth";
	case Song_State::Action::SET_VIBRATO_RATE:
		return "Set vibrato rate";
	case Song_State::Action::SET_WAVE:
		return "Set wave";
	case Song_State::Action::SET_DRUMKIT:
		return "Set drumkit";
	case Song_State::Action::SET_DUTY:
		return "Set duty";
	case Song_State::Action::SET_TEMPO:
		return "Set tempo";
	case Song_State::Action::SET_TRANSPOSE_OCTAVES:
		return "Set transpose octaves";
	case Song_State::Action::SET_TRANSPOSE_PITCHES:
		return "Set transpose pitches";
	case Song_State::Action::SET_SLIDE_DURATION:
		return "Set slide duration";
	case Song_State::Action::SET_SLIDE_OCTAVE:
		return "Set slide octave";
	case Song_State::Action::SET_SLIDE_PITCH:
		return "Set slide pitch";
	case Song_State::Action::SET_SLIDE:
		return "Set slide";
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
	case Song_State::Action::REDUCE_LOOP:
		return "Reduce loop";
	case Song_State::Action::EXTEND_LOOP:
		return "Extend loop";
	case Song_State::Action::UNROLL_LOOP:
		return "Unroll loop";
	case Song_State::Action::CREATE_LOOP:
		return "Create loop";
	case Song_State::Action::DELETE_CALL:
		return "Delete call";
	case Song_State::Action::UNPACK_CALL:
		return "Unpack call";
	case Song_State::Action::CREATE_CALL:
		return "Create call";
	case Song_State::Action::INSERT_CALL:
		return "Insert call";
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

const std::string &Song::channel_label(const int selected_channel) const {
	assert(selected_channel >= 1 && selected_channel <= 4);
	if (selected_channel == 1) {
		return _channel_1_label;
	}
	else if (selected_channel == 2) {
		return _channel_2_label;
	}
	else if (selected_channel == 3) {
		return _channel_3_label;
	}
	else {
		return _channel_4_label;
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

int32_t Song::max_drumkit_id() const {
	int32_t max_drumkit = -1;
	for (const Command &command : _channel_4_commands) {
		if (
			command.type == Command_Type::TOGGLE_NOISE &&
			command.toggle_noise.drumkit > max_drumkit
		) {
			max_drumkit = command.toggle_noise.drumkit;
		}
	}
	return max_drumkit;
}
