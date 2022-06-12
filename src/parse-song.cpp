#include <string>
#include <fstream>
#include <sstream>
#include <set>

#include "utils.h"
#include "parse-song.h"

Parsed_Song::Parsed_Song(const char *f) {
	parse_song(f);
}

static bool parse_value(std::string s, int32_t &v) {
	trim(s);
	if (!s.empty()) {
		int32_t scale = 1;
		if (s[0] == '-') {
			s.erase(0, 1);
			trim(s);
			if (s.empty()) return false;
			scale = -1;
		}
		if (s[0] == '$') {
			s.erase(0, 1);
			if (!is_hex(s)) return false;
			v = (int32_t)strtol(s.c_str(), NULL, 16) * scale;
		}
		else if (s[0] == '&') {
			s.erase(0, 1);
			if (!is_octal(s)) return false;
			v = (int32_t)strtol(s.c_str(), NULL, 8) * scale;
		}
		else if (s[0] == '%') {
			s.erase(0, 1);
			if (!is_binary(s)) return false;
			v = (int32_t)strtol(s.c_str(), NULL, 2) * scale;
		}
		else {
			if (!is_decimal(s)) return false;
			v = (int32_t)strtol(s.c_str(), NULL, 10) * scale;
		}
		return true;
	}
	return false;
}

static bool get_label(std::istringstream &iss, std::string &l, const std::string &scope = "") {
	iss >> l;
	rtrim(l, ":");
	trim(l);
	if (l.size() == 0) {
		return false;
	}
	if (l[0] == '.') {
		l = scope + l;
	}
	return true;
}

static bool get_number_and_label(std::istringstream &iss, int32_t &v, std::string &l, const std::string &scope = "") {
	std::getline(iss, l);
	size_t p = l.find(',');
	if (p == std::string::npos) {
		return false;
	}
	if (!parse_value(l.substr(0, p), v)) {
		return false;
	}
	l.erase(0, p + 1);
	trim(l);
	if (l.size() == 0) {
		return false;
	}
	if (l[0] == '.') {
		l = scope + l;
	}
	return true;
}

static bool get_number(std::istringstream &iss, int32_t &v) {
	std::string l;
	std::getline(iss, l);
	if (!parse_value(l, v)) {
		return false;
	}
	return true;
}

static bool get_number_and_number(std::istringstream &iss, int32_t &v1, int32_t &v2) {
	std::string l;
	std::getline(iss, l);
	size_t p = l.find(',');
	if (p == std::string::npos) {
		return false;
	}
	if (!parse_value(l.substr(0, p), v1)) {
		return false;
	}
	l.erase(0, p + 1);
	trim(l);
	if (l.size() == 0) {
		return false;
	}
	if (!parse_value(l, v2)) {
		return false;
	}
	return true;
}

static bool get_number_and_number_and_number(std::istringstream &iss, int32_t &v1, int32_t &v2, int32_t &v3) {
	std::string l;
	std::getline(iss, l);
	size_t p = l.find(',');
	if (p == std::string::npos) {
		return false;
	}
	if (!parse_value(l.substr(0, p), v1)) {
		return false;
	}
	l.erase(0, p + 1);
	trim(l);
	if (l.size() == 0) {
		return false;
	}

	p = l.find(',');
	if (p == std::string::npos) {
		return false;
	}
	if (!parse_value(l.substr(0, p), v2)) {
		return false;
	}
	l.erase(0, p + 1);
	trim(l);
	if (l.size() == 0) {
		return false;
	}

	if (!parse_value(l, v3)) {
		return false;
	}
	return true;
}

static bool get_number_and_number_and_number_and_number(std::istringstream &iss, int32_t &v1, int32_t &v2, int32_t &v3, int32_t &v4) {
	std::string l;
	std::getline(iss, l);
	size_t p = l.find(',');
	if (p == std::string::npos) {
		return false;
	}
	if (!parse_value(l.substr(0, p), v1)) {
		return false;
	}
	l.erase(0, p + 1);
	trim(l);
	if (l.size() == 0) {
		return false;
	}

	p = l.find(',');
	if (p == std::string::npos) {
		return false;
	}
	if (!parse_value(l.substr(0, p), v2)) {
		return false;
	}
	l.erase(0, p + 1);
	trim(l);
	if (l.size() == 0) {
		return false;
	}

	p = l.find(',');
	if (p == std::string::npos) {
		return false;
	}
	if (!parse_value(l.substr(0, p), v3)) {
		return false;
	}
	l.erase(0, p + 1);
	trim(l);
	if (l.size() == 0) {
		return false;
	}

	if (!parse_value(l, v4)) {
		return false;
	}
	return true;
}

static bool get_bool_and_bool(std::istringstream &iss, int32_t &b1, int32_t &b2) {
	std::string l;
	std::getline(iss, l);
	size_t p = l.find(',');
	if (p == std::string::npos) {
		return false;
	}
	std::string s = l.substr(0, p);
	if (s == "TRUE") {
		b1 = true;
	}
	else if (s == "FALSE") {
		b1 = false;
	}
	else {
		return false;
	}
	l.erase(0, p + 1);
	trim(l);
	if (l.size() == 0) {
		return false;
	}
	if (l == "TRUE") {
		b2 = true;
	}
	else if (l == "FALSE") {
		b2 = false;
	}
	else {
		return false;
	}
	return true;
}

static bool get_pitch_from_string(const std::string &s, Pitch &p) {
	for (size_t i = 1; i <= NUM_PITCHES; ++i) {
		if (s == PITCH_NAMES[i]) {
			p = (Pitch)i;
			return true;
		}
	}
	return false;
}

static bool get_pitch_and_number(std::istringstream &iss, Pitch &pitch, int32_t &v) {
	std::string l;
	std::getline(iss, l);
	size_t p = l.find(',');
	if (p == std::string::npos) {
		return false;
	}
	std::string s = l.substr(0, p);
	trim(s);
	if (!get_pitch_from_string(s, pitch)) {
		return false;
	}
	l.erase(0, p + 1);
	trim(l);
	if (l.size() == 0) {
		return false;
	}
	if (!parse_value(l, v)) {
		return false;
	}
	return true;
}

static bool get_number_and_number_and_pitch(std::istringstream &iss, int32_t &v1, int32_t &v2, Pitch &pitch) {
	std::string l;
	std::getline(iss, l);
	size_t p = l.find(',');
	if (p == std::string::npos) {
		return false;
	}
	if (!parse_value(l.substr(0, p), v1)) {
		return false;
	}
	l.erase(0, p + 1);
	trim(l);
	if (l.size() == 0) {
		return false;
	}

	p = l.find(',');
	if (p == std::string::npos) {
		return false;
	}
	if (!parse_value(l.substr(0, p), v2)) {
		return false;
	}
	l.erase(0, p + 1);
	trim(l);
	if (l.size() == 0) {
		return false;
	}

	if (!get_pitch_from_string(l, pitch)) {
		return false;
	}
	return true;
}

Parsed_Song::Result Parsed_Song::parse_song(const char *f) {
	std::ifstream ifs;
	open_ifstream(ifs, f);
	if (!ifs.good()) {
		return (_result = Result::SONG_BAD_FILE);
	}

	enum class Step { LOOKING_FOR_HEADER, READING_HEADER, LOOKING_FOR_CHANNEL, READING_CHANNEL, DONE };

	Step step = Step::LOOKING_FOR_HEADER;
	int32_t current_channel = 0;
	std::string current_channel_label;
	std::list<Command> *current_channel_commands;
	std::string current_scope;

	std::set<std::string> visited_labels;
	std::set<std::string> unvisited_labels;
	std::set<std::string> buffered_labels;

	while (ifs.good()) {
		std::string line;
		std::getline(ifs, line);
		remove_comment(line);
		rtrim(line);
		if (line.size() == 0) { continue; }
		bool indented = is_indented(line);
		std::istringstream lss(line);

		if (step == Step::LOOKING_FOR_HEADER) {
			if (indented) {
				return (_result = Result::SONG_BAD_FILE);
			}
			if (!get_label(lss, _song_name)) {
				return (_result = Result::SONG_BAD_FILE);
			}
			step = Step::READING_HEADER;
		}

		else if (step == Step::READING_HEADER) {
			if (!indented) { continue; } // maybe a distracting label or something
			if (_number_of_channels == 0) {
				std::string macro;
				if (!leading_macro(lss, macro, "channel_count")) {
					return (_result = Result::SONG_BAD_FILE);
				}
				if (!get_number(lss, _number_of_channels)) {
					return (_result = Result::SONG_BAD_FILE);
				}
				if (_number_of_channels < 1 || _number_of_channels > 4) {
					return (_result = Result::SONG_BAD_FILE);
				}
			}
			else {
				std::string macro;
				if (!leading_macro(lss, macro, "channel")) {
					return (_result = Result::SONG_BAD_FILE);
				}
				int32_t channel_number = 0;
				std::string label;
				if (!get_number_and_label(lss, channel_number, label)) {
					return (_result = Result::SONG_BAD_FILE);
				}
				if (channel_number < 1 || channel_number > 4) {
					return (_result = Result::SONG_BAD_FILE);
				}
				if (channel_number == 1) _channel_1_label = label;
				if (channel_number == 2) _channel_2_label = label;
				if (channel_number == 3) _channel_3_label = label;
				if (channel_number == 4) _channel_4_label = label;
				current_channel += 1;
				if (current_channel == _number_of_channels) {
					int32_t num_labelled_channels = 0;
					if (_channel_1_label.size() > 0) num_labelled_channels += 1;
					if (_channel_2_label.size() > 0) num_labelled_channels += 1;
					if (_channel_3_label.size() > 0) num_labelled_channels += 1;
					if (_channel_4_label.size() > 0) num_labelled_channels += 1;
					if (num_labelled_channels != _number_of_channels) {
						return (_result = Result::SONG_BAD_FILE);
					}
					if (_channel_1_label.size() > 0) {
						current_channel = 1;
						current_channel_label = _channel_1_label;
						current_channel_commands = &_channel_1_commands;
					}
					else if (_channel_2_label.size() > 0) {
						current_channel = 2;
						current_channel_label = _channel_2_label;
						current_channel_commands = &_channel_2_commands;
					}
					else if (_channel_3_label.size() > 0) {
						current_channel = 3;
						current_channel_label = _channel_3_label;
						current_channel_commands = &_channel_3_commands;
					}
					else { // if (_channel_4_label.size() > 0)
						current_channel = 4;
						current_channel_label = _channel_4_label;
						current_channel_commands = &_channel_4_commands;
					}
					current_scope = "";
					visited_labels.clear();
					unvisited_labels.clear();
					ifs.seekg(0);
					step = Step::LOOKING_FOR_CHANNEL;
				}
			}
		}

		else if (step == Step::LOOKING_FOR_CHANNEL) {
			if (indented) {
				buffered_labels.clear();
				continue;
			}
			std::string label;
			if (!get_label(lss, label)) {
				continue;
			}
			if (label[0] == '.') {
				label = current_scope + label;
			}
			else {
				current_scope = label;
			}
			buffered_labels.insert(label);
			if (label == current_channel_label) {
				visited_labels.insert(label);
				unvisited_labels.erase(label);
				step = Step::READING_CHANNEL;
			}
		}

		else if (step == Step::READING_CHANNEL) {
			bool done_with_branch = false;
			if (!indented) {
				std::string label;
				if (!get_label(lss, label)) {
					continue;
				}
				if (label[0] == '.') {
					label = current_scope + label;
				}
				else {
					current_scope = label;
				}
				buffered_labels.insert(label);
				if (visited_labels.count(label)) {
					// been here done that
					done_with_branch = true;
				}
				else {
					visited_labels.insert(label);
					unvisited_labels.erase(label);
				}
				continue;
			}
			else {
				std::string macro;
				if (!leading_macro(lss, macro)) {
					return (_result = Result::SONG_BAD_FILE);
				}
				Command command;
				command.labels = std::move(buffered_labels);
				if (macro == "note") {
					if (current_channel == 4) {
						return (_result = Result::SONG_BAD_FILE);
					}
					command.type = Command_Type::NOTE;
					if (!get_pitch_and_number(lss, command.note.pitch, command.note.length)) {
						return (_result = Result::SONG_BAD_FILE);
					}
					if (command.note.length < 1 || command.note.length > 16) {
						return (_result = Result::SONG_BAD_FILE);
					}
					current_channel_commands->push_back(command);
				}

				else if (macro == "drum_note") {
					if (current_channel != 4) {
						return (_result = Result::SONG_BAD_FILE);
					}
					command.type = Command_Type::DRUM_NOTE;
					if (!get_number_and_number(lss, command.drum_note.instrument, command.drum_note.length)) {
						return (_result = Result::SONG_BAD_FILE);
					}
					if (command.drum_note.instrument < 1 || command.drum_note.instrument > 12) {
						return (_result = Result::SONG_BAD_FILE);
					}
					if (command.drum_note.length < 1 || command.drum_note.length > 16) {
						return (_result = Result::SONG_BAD_FILE);
					}
					current_channel_commands->push_back(command);
				}

				else if (macro == "rest") {
					command.type = Command_Type::REST;
					if (!get_number(lss, command.rest.length)) {
						return (_result = Result::SONG_BAD_FILE);
					}
					if (command.rest.length < 1 || command.rest.length > 16) {
						return (_result = Result::SONG_BAD_FILE);
					}
					current_channel_commands->push_back(command);
				}

				else if (macro == "octave") {
					// too many real songs violate this rule
					/* if (current_channel == 4) {
						return (_result = Result::SONG_BAD_FILE);
					} */
					command.type = Command_Type::OCTAVE;
					if (!get_number(lss, command.octave.octave)) {
						return (_result = Result::SONG_BAD_FILE);
					}
					if (command.octave.octave < 1 || command.octave.octave > 8) {
						return (_result = Result::SONG_BAD_FILE);
					}
					current_channel_commands->push_back(command);
				}

				else if (macro == "note_type") {
					if (current_channel == 4) {
						return (_result = Result::SONG_BAD_FILE);
					}
					if (current_channel == 3) {
						command.type = Command_Type::NOTE_TYPE;
						if (!get_number_and_number_and_number(lss, command.note_type.speed, command.note_type.volume, command.note_type.wave)) {
							return (_result = Result::SONG_BAD_FILE);
						}
						// the allowed speeds for now...
						if (
							// command.note_type.speed != 1 &&
							command.note_type.speed != 2 &&
							// command.note_type.speed != 3 &&
							command.note_type.speed != 4 &&
							command.note_type.speed != 6 &&
							command.note_type.speed != 8 &&
							command.note_type.speed != 12
						) {
							return (_result = Result::SONG_BAD_FILE);
						}
						if (command.note_type.volume < 0 || command.note_type.volume > 3) {
							return (_result = Result::SONG_BAD_FILE);
						}
						if (command.note_type.wave < 0 || command.note_type.wave > 15) {
							return (_result = Result::SONG_BAD_FILE);
						}
						current_channel_commands->push_back(command);
					}
					else {
						command.type = Command_Type::NOTE_TYPE;
						if (!get_number_and_number_and_number(lss, command.note_type.speed, command.note_type.volume, command.note_type.fade)) {
							return (_result = Result::SONG_BAD_FILE);
						}
						// the allowed speeds for now...
						if (
							// command.note_type.speed != 1 &&
							command.note_type.speed != 2 &&
							// command.note_type.speed != 3 &&
							command.note_type.speed != 4 &&
							command.note_type.speed != 6 &&
							command.note_type.speed != 8 &&
							command.note_type.speed != 12
						) {
							return (_result = Result::SONG_BAD_FILE);
						}
						if (command.note_type.volume < 0 || command.note_type.volume > 15) {
							return (_result = Result::SONG_BAD_FILE);
						}
						if (command.note_type.fade == 8) command.note_type.fade = 0; // 8 is used in place of 0
						if (command.note_type.fade < -7 || command.note_type.fade > 7) {
							return (_result = Result::SONG_BAD_FILE);
						}
						current_channel_commands->push_back(command);
					}
				}

				else if (macro == "drum_speed") {
					if (current_channel != 4) {
						return (_result = Result::SONG_BAD_FILE);
					}
					command.type = Command_Type::DRUM_SPEED;
					if (!get_number(lss, command.drum_speed.speed)) {
						return (_result = Result::SONG_BAD_FILE);
					}
					// the allowed speeds for now...
					if (
						// command.drum_speed.speed != 1 &&
						command.drum_speed.speed != 2 &&
						// command.drum_speed.speed != 3 &&
						command.drum_speed.speed != 4 &&
						command.drum_speed.speed != 6 &&
						command.drum_speed.speed != 8 &&
						command.drum_speed.speed != 12
					) {
						return (_result = Result::SONG_BAD_FILE);
					}
					current_channel_commands->push_back(command);
				}

				else if (macro == "transpose") {
					if (current_channel == 4) {
						return (_result = Result::SONG_BAD_FILE);
					}
					command.type = Command_Type::TRANSPOSE;
					if (!get_number_and_number(lss, command.transpose.num_octaves, command.transpose.num_pitches)) {
						return (_result = Result::SONG_BAD_FILE);
					}
					if (command.transpose.num_octaves < 0 || command.transpose.num_octaves > 7) {
						return (_result = Result::SONG_BAD_FILE);
					}
					if (command.transpose.num_pitches < 0 || command.transpose.num_pitches > 24) {
						return (_result = Result::SONG_BAD_FILE);
					}
					current_channel_commands->push_back(command);
				}

				else if (macro == "tempo") {
					command.type = Command_Type::TEMPO;
					if (!get_number(lss, command.tempo.tempo)) {
						return (_result = Result::SONG_BAD_FILE);
					}
					if (command.tempo.tempo < 1 || command.tempo.tempo > 1024) {
						return (_result = Result::SONG_BAD_FILE);
					}
					current_channel_commands->push_back(command);
				}

				else if (macro == "duty_cycle") {
					if (current_channel != 1 && current_channel != 2) {
						return (_result = Result::SONG_BAD_FILE);
					}
					command.type = Command_Type::DUTY_CYCLE;
					if (!get_number(lss, command.duty_cycle.duty)) {
						return (_result = Result::SONG_BAD_FILE);
					}
					if (command.duty_cycle.duty < 0 || command.duty_cycle.duty > 3) {
						return (_result = Result::SONG_BAD_FILE);
					}
					current_channel_commands->push_back(command);
				}

				else if (macro == "volume_envelope") {
					if (current_channel == 4) {
						return (_result = Result::SONG_BAD_FILE);
					}
					if (current_channel == 3) {
						command.type = Command_Type::VOLUME_ENVELOPE;
						if (!get_number_and_number(lss, command.volume_envelope.volume, command.volume_envelope.wave)) {
							return (_result = Result::SONG_BAD_FILE);
						}
						if (command.volume_envelope.volume < 0 || command.volume_envelope.volume > 3) {
							return (_result = Result::SONG_BAD_FILE);
						}
						if (command.volume_envelope.wave < 0 || command.volume_envelope.wave > 15) {
							return (_result = Result::SONG_BAD_FILE);
						}
						current_channel_commands->push_back(command);
					}
					else {
						command.type = Command_Type::VOLUME_ENVELOPE;
						if (!get_number_and_number(lss, command.volume_envelope.volume, command.volume_envelope.fade)) {
							return (_result = Result::SONG_BAD_FILE);
						}
						if (command.volume_envelope.volume < 0 || command.volume_envelope.volume > 15) {
							return (_result = Result::SONG_BAD_FILE);
						}
						if (command.volume_envelope.fade == 8) command.volume_envelope.fade = 0; // 8 is used in place of 0
						if (command.volume_envelope.fade < -7 || command.volume_envelope.fade > 7) {
							return (_result = Result::SONG_BAD_FILE);
						}
						current_channel_commands->push_back(command);
					}
				}

				else if (macro == "pitch_sweep") {
					if (current_channel != 1) {
						return (_result = Result::SONG_BAD_FILE);
					}
					command.type = Command_Type::PITCH_SWEEP;
					if (!get_number_and_number(lss, command.pitch_sweep.duration, command.pitch_sweep.pitch_change)) {
						return (_result = Result::SONG_BAD_FILE);
					}
					if (command.pitch_sweep.duration < 0 || command.pitch_sweep.duration > 15) {
						return (_result = Result::SONG_BAD_FILE);
					}
					if (command.pitch_sweep.pitch_change == 8) command.pitch_sweep.pitch_change = 0; // 8 is used in place of 0
					if (command.pitch_sweep.pitch_change < -7 || command.pitch_sweep.pitch_change > 7) {
						return (_result = Result::SONG_BAD_FILE);
					}
					current_channel_commands->push_back(command);
				}

				else if (macro == "duty_cycle_pattern") {
					if (current_channel != 1 && current_channel != 2) {
						return (_result = Result::SONG_BAD_FILE);
					}
					command.type = Command_Type::DUTY_CYCLE_PATTERN;
					if (!get_number_and_number_and_number_and_number(lss, command.duty_cycle_pattern.duty1, command.duty_cycle_pattern.duty2, command.duty_cycle_pattern.duty3, command.duty_cycle_pattern.duty4)) {
						return (_result = Result::SONG_BAD_FILE);
					}
					if (command.duty_cycle_pattern.duty1 < 0 || command.duty_cycle_pattern.duty1 > 3) {
						return (_result = Result::SONG_BAD_FILE);
					}
					if (command.duty_cycle_pattern.duty2 < 0 || command.duty_cycle_pattern.duty2 > 3) {
						return (_result = Result::SONG_BAD_FILE);
					}
					if (command.duty_cycle_pattern.duty3 < 0 || command.duty_cycle_pattern.duty3 > 3) {
						return (_result = Result::SONG_BAD_FILE);
					}
					if (command.duty_cycle_pattern.duty4 < 0 || command.duty_cycle_pattern.duty4 > 3) {
						return (_result = Result::SONG_BAD_FILE);
					}
					current_channel_commands->push_back(command);
				}

				else if (macro == "pitch_slide") {
					if (current_channel != 1) {
						return (_result = Result::SONG_BAD_FILE);
					}
					command.type = Command_Type::PITCH_SLIDE;
					if (!get_number_and_number_and_pitch(lss, command.pitch_slide.duration, command.pitch_slide.octave, command.pitch_slide.pitch)) {
						return (_result = Result::SONG_BAD_FILE);
					}
					if (command.pitch_slide.duration < 1 || command.pitch_slide.duration > 256) {
						return (_result = Result::SONG_BAD_FILE);
					}
					if (command.pitch_slide.octave < 1 || command.pitch_slide.octave > 8) {
						return (_result = Result::SONG_BAD_FILE);
					}
					current_channel_commands->push_back(command);
				}

				else if (macro == "vibrato") {
					if (current_channel == 4) {
						return (_result = Result::SONG_BAD_FILE);
					}
					command.type = Command_Type::VIBRATO;
					if (!get_number_and_number_and_number(lss, command.vibrato.delay, command.vibrato.extent, command.vibrato.rate)) {
						return (_result = Result::SONG_BAD_FILE);
					}
					if (command.vibrato.delay < 0 || command.vibrato.delay > 255) {
						return (_result = Result::SONG_BAD_FILE);
					}
					if (command.vibrato.extent < 0 || command.vibrato.extent > 15) {
						return (_result = Result::SONG_BAD_FILE);
					}
					if (command.vibrato.rate < 0 || command.vibrato.rate > 15) {
						return (_result = Result::SONG_BAD_FILE);
					}
					current_channel_commands->push_back(command);
				}

				else if (macro == "toggle_noise") {
					if (current_channel != 4) {
						return (_result = Result::SONG_BAD_FILE);
					}
					command.type = Command_Type::TOGGLE_NOISE;
					if (!get_number(lss, command.toggle_noise.drumkit)) {
						return (_result = Result::SONG_BAD_FILE);
					}
					if (command.toggle_noise.drumkit < 0 || command.toggle_noise.drumkit > 6) {
						return (_result = Result::SONG_BAD_FILE);
					}
					current_channel_commands->push_back(command);
				}

				else if (macro == "force_stereo_panning") {
					command.type = Command_Type::FORCE_STEREO_PANNING;
					if (!get_bool_and_bool(lss, command.force_stereo_panning.left, command.force_stereo_panning.right)) {
						return (_result = Result::SONG_BAD_FILE);
					}
					current_channel_commands->push_back(command);
				}

				else if (macro == "volume") {
					command.type = Command_Type::VOLUME;
					if (!get_number_and_number(lss, command.volume.left, command.volume.right)) {
						return (_result = Result::SONG_BAD_FILE);
					}
					if (command.volume.left < 0 || command.volume.left > 7) {
						return (_result = Result::SONG_BAD_FILE);
					}
					if (command.volume.right < 0 || command.volume.right > 7) {
						return (_result = Result::SONG_BAD_FILE);
					}
					current_channel_commands->push_back(command);
				}

				else if (macro == "pitch_offset") {
					if (current_channel == 4) {
						return (_result = Result::SONG_BAD_FILE);
					}
					command.type = Command_Type::PITCH_OFFSET;
					if (!get_number(lss, command.pitch_offset.offset)) {
						return (_result = Result::SONG_BAD_FILE);
					}
					if (command.pitch_offset.offset < 0 || command.pitch_offset.offset > 65535) {
						return (_result = Result::SONG_BAD_FILE);
					}
					current_channel_commands->push_back(command);
				}

				else if (macro == "stereo_panning") {
					command.type = Command_Type::STEREO_PANNING;
					if (!get_bool_and_bool(lss, command.stereo_panning.left, command.stereo_panning.right)) {
						return (_result = Result::SONG_BAD_FILE);
					}
					current_channel_commands->push_back(command);
				}

				else if (macro == "sound_jump") {
					command.type = Command_Type::SOUND_JUMP;
					if (!get_label(lss, command.target, current_scope)) {
						return (_result = Result::SONG_BAD_FILE);
					}

					if (!visited_labels.count(command.target)) {
						unvisited_labels.insert(command.target);
					}

					current_channel_commands->push_back(command);
					done_with_branch = true;
				}

				else if (macro == "sound_loop") {
					command.type = Command_Type::SOUND_LOOP;
					if (!get_number_and_label(lss, command.sound_loop.loop_count, command.target, current_scope)) {
						return (_result = Result::SONG_BAD_FILE);
					}
					if (command.sound_loop.loop_count < 0 || command.sound_loop.loop_count > 255) {
						return (_result = Result::SONG_BAD_FILE);
					}

					if (!visited_labels.count(command.target)) {
						unvisited_labels.insert(command.target);
					}

					current_channel_commands->push_back(command);
					if (command.sound_loop.loop_count == 0) {
						done_with_branch = true;
					}
				}

				else if (macro == "sound_call") {
					command.type = Command_Type::SOUND_CALL;
					if (!get_label(lss, command.target, current_scope)) {
						return (_result = Result::SONG_BAD_FILE);
					}

					if (!visited_labels.count(command.target)) {
						unvisited_labels.insert(command.target);
					}

					current_channel_commands->push_back(command);
				}

				else if (macro == "sound_ret") {
					command.type = Command_Type::SOUND_RET;
					current_channel_commands->push_back(command);
					done_with_branch = true;
				}

				else if (macro == "toggle_perfect_pitch") {
					// TODO
				}

				else if (macro == "load_wave") {
					// TODO
				}

				else if (macro == "inc_octave") {
					// TODO
				}

				else if (macro == "dec_octave") {
					// TODO
				}

				else if (macro == "notetype0") {
					// TODO
				}

				else if (macro == "notetype1") {
					// TODO
				}

				else if (macro == "notetype2") {
					// TODO
				}

				else {
					// unknown command
					return (_result = Result::SONG_BAD_FILE);
				}
			}

			if (done_with_branch && unvisited_labels.size() == 0) {
				if (_channel_2_label.size() > 0 && current_channel < 2) {
					current_channel = 2;
					current_channel_label = _channel_2_label;
					current_channel_commands = &_channel_2_commands;
					current_scope = "";
					visited_labels.clear();
					unvisited_labels.clear();
					buffered_labels.clear();
					ifs.seekg(0);
					step = Step::LOOKING_FOR_CHANNEL;
				}
				else if (_channel_3_label.size() > 0 && current_channel < 3) {
					current_channel = 3;
					current_channel_label = _channel_3_label;
					current_channel_commands = &_channel_3_commands;
					current_scope = "";
					visited_labels.clear();
					unvisited_labels.clear();
					buffered_labels.clear();
					ifs.seekg(0);
					step = Step::LOOKING_FOR_CHANNEL;
				}
				else if (_channel_4_label.size() > 0 && current_channel < 4) {
					current_channel = 4;
					current_channel_label = _channel_4_label;
					current_channel_commands = &_channel_4_commands;
					current_scope = "";
					visited_labels.clear();
					unvisited_labels.clear();
					buffered_labels.clear();
					ifs.seekg(0);
					step = Step::LOOKING_FOR_CHANNEL;
				}
				else {
					step = Step::DONE;
					break;
				}
			}
			else if (done_with_branch) {
				current_channel_label = *unvisited_labels.begin();
				current_scope = "";
				buffered_labels.clear();
				ifs.seekg(0);
				step = Step::LOOKING_FOR_CHANNEL;
			}
		}
	}

	if (step != Step::DONE) {
		return (_result = Result::SONG_BAD_FILE);
	}

	return (_result = Result::SONG_OK);
}
