#include <string>
#include <fstream>
#include <sstream>
#include <set>

#include "utils.h"
#include "parse-song.h"

Parsed_Song::Parsed_Song(const char *f) : _song_name(), _number_of_channels(0),
	_channel_1_label(), _channel_2_label(), _channel_3_label(), _channel_4_label(),
	_channel_1_commands(), _channel_2_commands(), _channel_3_commands(), _channel_4_commands(),
	_result(Result::SONG_NULL) {
	parse_song(f);
}

// TODO: need to allow for negatives
static bool parse_value(std::string s, uint32_t &v) {
	trim(s);
	size_t c = s.length();
	if (!s.empty()) {
		// this is a bit too trusting...
		if (s[0] == '$') {
			s.erase(0, 1);
			v = (uint32_t)strtol(s.c_str(), NULL, 16);
		}
		else if (s[0] == '%') {
			s.erase(0, 1);
			v = (uint32_t)strtol(s.c_str(), NULL, 2);
		}
		else if (s[c-1] == 'h' || s[c-1] == 'H') {
			s.erase(c - 1);
			v = (uint32_t)strtol(s.c_str(), NULL, 16);
		}
		else {
			v = (uint32_t)strtol(s.c_str(), NULL, 10);
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

static bool get_number_and_label(std::istringstream &iss, uint32_t &v, std::string &l, const std::string &scope = "") {
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

Parsed_Song::Result Parsed_Song::parse_song(const char *f) {
	std::ifstream ifs;
	open_ifstream(ifs, f);
	if (!ifs.good()) {
		return (_result = Result::SONG_BAD_FILE);
	}

	enum class Step { LOOKING_FOR_HEADER, READING_HEADER, LOOKING_FOR_CHANNEL, READING_CHANNEL, DONE };

	Step step = Step::LOOKING_FOR_HEADER;
	uint32_t current_channel = 0;
	std::string current_channel_label;
	std::list<Command> *current_channel_commands;
	std::string current_scope;

	std::set<std::string> visited_labels;
	std::set<std::string> unvisited_labels;

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
				std::string number_of_channels_str;
				std::getline(lss, number_of_channels_str);
				if (!parse_value(number_of_channels_str, _number_of_channels)) {
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
				uint32_t channel_number = 0;
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
					uint32_t num_labelled_channels = 0;
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
			if (indented) continue;
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
				if (macro == "note") {
					current_channel_commands->push_back({ Command_Type::NOTE });
				}
				else if (macro == "drum_note") {
					current_channel_commands->push_back({ Command_Type::DRUM_NOTE });
				}
				else if (macro == "rest") {
					current_channel_commands->push_back({ Command_Type::REST });
				}
				else if (macro == "octave") {
					current_channel_commands->push_back({ Command_Type::OCTAVE });
				}
				else if (macro == "note_type") {
					current_channel_commands->push_back({ Command_Type::NOTE_TYPE });
				}
				else if (macro == "drum_speed") {
					current_channel_commands->push_back({ Command_Type::DRUM_SPEED });
				}
				else if (macro == "transpose") {
					current_channel_commands->push_back({ Command_Type::TRANSPOSE });
				}
				else if (macro == "tempo") {
					current_channel_commands->push_back({ Command_Type::TEMPO });
				}
				else if (macro == "duty_cycle") {
					current_channel_commands->push_back({ Command_Type::DUTY_CYCLE });
				}
				else if (macro == "volume_envelope") {
					current_channel_commands->push_back({ Command_Type::VOLUME_ENVELOPE });
				}
				else if (macro == "duty_cycle_pattern") {
					current_channel_commands->push_back({ Command_Type::DUTY_CYCLE_PATTERN });
				}
				else if (macro == "pitch_slide") {
					current_channel_commands->push_back({ Command_Type::PITCH_SLIDE });
				}
				else if (macro == "vibrato") {
					current_channel_commands->push_back({ Command_Type::VIBRATO });
				}
				else if (macro == "toggle_noise") {
					current_channel_commands->push_back({ Command_Type::TOGGLE_NOISE });
				}
				else if (macro == "force_stereo_panning") {
					current_channel_commands->push_back({ Command_Type::FORCE_STEREO_PANNING });
				}
				else if (macro == "volume") {
					current_channel_commands->push_back({ Command_Type::VOLUME });
				}
				else if (macro == "pitch_offset") {
					current_channel_commands->push_back({ Command_Type::PITCH_OFFSET });
				}
				else if (macro == "stereo_panning") {
					current_channel_commands->push_back({ Command_Type::STEREO_PANNING });
				}
				else if (macro == "sound_jump") {
					std::string label;
					if (!get_label(lss, label, current_scope)) {
						return (_result = Result::SONG_BAD_FILE);
					}

					if (!visited_labels.count(label)) {
						unvisited_labels.insert(label);
					}

					current_channel_commands->push_back({ Command_Type::SOUND_JUMP });
					done_with_branch = true;
				}
				else if (macro == "sound_loop") {
					uint32_t loop_count = 0;
					std::string label;
					if (!get_number_and_label(lss, loop_count, label, current_scope)) {
						return (_result = Result::SONG_BAD_FILE);
					}
					if (loop_count < 0 || loop_count > 255) {
						return (_result = Result::SONG_BAD_FILE);
					}

					if (!visited_labels.count(label)) {
						unvisited_labels.insert(label);
					}

					current_channel_commands->push_back({ Command_Type::SOUND_LOOP });
					if (loop_count == 0) {
						done_with_branch = true;
					}
				}
				else if (macro == "sound_call") {
					std::string label;
					if (!get_label(lss, label, current_scope)) {
						return (_result = Result::SONG_BAD_FILE);
					}

					if (!visited_labels.count(label)) {
						unvisited_labels.insert(label);
					}

					current_channel_commands->push_back({ Command_Type::SOUND_CALL });
				}
				else if (macro == "sound_ret") {
					current_channel_commands->push_back({ Command_Type::SOUND_RET });
					done_with_branch = true;
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
