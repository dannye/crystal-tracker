#include <string>
#include <fstream>
#include <sstream>

#include "utils.h"
#include "parse-song.h"

Parsed_Song::Parsed_Song(const char *f) : _song_name(), _number_of_channels(0),
	_channel_1_label(), _channel_2_label(), _channel_3_label(), _channel_4_label(),
	_channel_1_commands(), _channel_2_commands(), _channel_3_commands(), _channel_4_commands(),
	_result(Result::SONG_NULL) {
	parse_song(f);
}

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

static std::string get_label(std::istringstream &iss) {
	std::string label;
	iss >> label;
	rtrim(label, ":");
	return label;
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
			_song_name = get_label(lss);
			if (_song_name.size() == 0) {
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
				std::string params;
				std::getline(lss, params);
				size_t p = params.find(',');
				if (p == std::string::npos) {
					return (_result = Result::SONG_BAD_FILE);
				}
				uint32_t channel_number = 0;
				if (!parse_value(params.substr(0, p), channel_number)) {
					return (_result = Result::SONG_BAD_FILE);
				}
				if (channel_number < 1 || channel_number > 4) {
					return (_result = Result::SONG_BAD_FILE);
				}
				params.erase(0, p + 1);
				trim(params);
				if (params.size() == 0) {
					return (_result = Result::SONG_BAD_FILE);
				}
				if (channel_number == 1) _channel_1_label = params;
				if (channel_number == 2) _channel_2_label = params;
				if (channel_number == 3) _channel_3_label = params;
				if (channel_number == 4) _channel_4_label = params;
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
					ifs.seekg(0);
					step = Step::LOOKING_FOR_CHANNEL;
				}
			}
		}

		else if (step == Step::LOOKING_FOR_CHANNEL) {
			if (indented) continue;
			std::string label = get_label(lss);
			if (label == current_channel_label) {
				current_scope = label;
				step = Step::READING_CHANNEL;
			}
		}

		else if (step == Step::READING_CHANNEL) {
			if (!indented) {
				std::string label = get_label(lss);
				if (label.size() > 0 && label[0] != '.') {
					current_scope = label;
				}
				continue;
			}
			bool done_with_channel = false;
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
				current_channel_commands->push_back({ Command_Type::SOUND_JUMP });
				done_with_channel = true;
			}
			else if (macro == "sound_loop") {
				current_channel_commands->push_back({ Command_Type::SOUND_LOOP });
				done_with_channel = true;
			}
			else if (macro == "sound_call") {
				current_channel_commands->push_back({ Command_Type::SOUND_CALL });
			}
			else if (macro == "sound_ret") {
				current_channel_commands->push_back({ Command_Type::SOUND_RET });
				done_with_channel = true;
			}
			else {
				// unknown command
				return (_result = Result::SONG_BAD_FILE);
			}

			if (done_with_channel) {
				if (_channel_2_label.size() > 0 && current_channel < 2) {
					current_channel = 2;
					current_channel_label = _channel_2_label;
					current_channel_commands = &_channel_2_commands;
					ifs.seekg(0);
					step = Step::LOOKING_FOR_CHANNEL;
				}
				else if (_channel_3_label.size() > 0 && current_channel < 3) {
					current_channel = 3;
					current_channel_label = _channel_3_label;
					current_channel_commands = &_channel_3_commands;
					ifs.seekg(0);
					step = Step::LOOKING_FOR_CHANNEL;
				}
				else if (_channel_4_label.size() > 0 && current_channel < 4) {
					current_channel = 4;
					current_channel_label = _channel_4_label;
					current_channel_commands = &_channel_4_commands;
					ifs.seekg(0);
					step = Step::LOOKING_FOR_CHANNEL;
				}
				else {
					step = Step::DONE;
					break;
				}
			}
		}
	}

	if (step != Step::DONE) {
		return (_result = Result::SONG_BAD_FILE);
	}

	return (_result = Result::SONG_OK);
}
