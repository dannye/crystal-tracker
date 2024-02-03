#include <cstring>

#pragma warning(push, 0)
#include <FL/filename.H>
#pragma warning(pop)

#include "parse-drumkits.h"

#include "utils.h"

Parsed_Drumkits::Parsed_Drumkits(const char *d) {
	parse_drumkits(d);
}

Parsed_Drumkits::Result Parsed_Drumkits::parse_drumkits(const char *d) {
	char drumkits_file[FL_PATH_MAX] = {};

	// first, try crysaudio/drumkits.asm
	strcpy(drumkits_file, d);
	strcat(drumkits_file, DIR_SEP "crysaudio" DIR_SEP "drumkits.asm");
	if (try_parse_drumkits(drumkits_file) == Parsed_Drumkits::Result::DRUMKITS_OK) {
		return _result;
	}
	// second, try audio/drumkits.asm
	strcpy(drumkits_file, d);
	strcat(drumkits_file, DIR_SEP "audio" DIR_SEP "drumkits.asm");
	if (try_parse_drumkits(drumkits_file) == Parsed_Drumkits::Result::DRUMKITS_OK) {
		return _result;
	}
	// third, try audio/drumkits_0f.asm (for pinball)
	strcpy(drumkits_file, d);
	strcat(drumkits_file, DIR_SEP "audio" DIR_SEP "drumkits_0f.asm");
	if (try_parse_drumkits(drumkits_file) == Parsed_Drumkits::Result::DRUMKITS_OK) {
		return _result;
	}
	// fourth, try drumkits.asm
	strcpy(drumkits_file, d);
	strcat(drumkits_file, DIR_SEP "drumkits.asm");
	if (try_parse_drumkits(drumkits_file) == Parsed_Drumkits::Result::DRUMKITS_OK) {
		return _result;
	}

	return _result;
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

static int32_t find_drum(const std::vector<Drum> &drums, const std::string &label) {
	for (size_t i = 0; i < drums.size(); ++i) {
		if (drums[i].label == label) {
			return (int32_t)i;
		}
	}
	return -1;
}

static bool leading_pointer(std::istringstream &lss) {
	std::string macro;
	return leading_macro(lss, macro) && (equals_ignore_case(macro, "dw") || equals_ignore_case(macro, "dr"));
}

Parsed_Drumkits::Result Parsed_Drumkits::try_parse_drumkits(const char *f) {
	_drumkits_file = f;
	_drumkits.clear();
	_drums.clear();
	_num_parsed_drumkits = 0;
	_num_parsed_drums = 0;
	_result = Result::DRUMKITS_NULL;

	std::ifstream ifs;
	open_ifstream(ifs, f);
	if (!ifs.good()) {
		return (_result = Result::DRUMKITS_BAD_FILE);
	}

	enum class Step {
		LOOKING_FOR_DRUMKITS,
		READING_DRUMKITS,
		LOOKING_FOR_DRUMKIT,
		READING_DRUMKIT,
		LOOKING_FOR_DRUM,
		READING_DRUM,
		DONE
	};

	Step step = Step::LOOKING_FOR_DRUMKITS;
	auto drumkit_itr = _drumkits.begin();
	size_t drumkit_index = 0;
	auto drum_itr = _drums.begin();

	while (ifs.good()) {
		std::string line;
		std::getline(ifs, line);
		remove_comment(line);
		rtrim(line);
		if (line.size() == 0) { continue; }
		bool indented = is_indented(line);
		std::istringstream lss(line);

		if (step == Step::LOOKING_FOR_DRUMKITS) {
			if (indented) {
				return (_result = Result::DRUMKITS_BAD_FILE);
			}
			std::string dummy;
			if (!get_label(lss, dummy)) {
				return (_result = Result::DRUMKITS_BAD_FILE);
			}
			step = Step::READING_DRUMKITS;
		}

		else if (step == Step::READING_DRUMKITS) {
			if (!indented) {
				if (_drumkits.size() == 0) {
					return (_result = Result::DRUMKITS_BAD_FILE);
				}
				drumkit_itr = _drumkits.begin();
				drumkit_index = 0;
				ifs.seekg(0);
				step = Step::LOOKING_FOR_DRUMKIT;
				continue;
			}
			if (!leading_pointer(lss)) {
				return (_result = Result::DRUMKITS_BAD_FILE);
			}
			Drumkit drumkit;
			if (!get_label(lss, drumkit.label)) {
				return (_result = Result::DRUMKITS_BAD_FILE);
			}
			_drumkits.push_back(drumkit);
		}

		else if (step == Step::LOOKING_FOR_DRUMKIT) {
			if (indented) { continue; }
			std::string label;
			if (!get_label(lss, label)) {
				continue;
			}
			if (label == drumkit_itr->label) {
				step = Step::READING_DRUMKIT;
			}
		}

		else if (step == Step::READING_DRUMKIT) {
			if (!indented) { continue; }
			if (!leading_pointer(lss)) {
				return (_result = Result::DRUMKITS_BAD_FILE);
			}
			std::string label;
			if (!get_label(lss, label)) {
				return (_result = Result::DRUMKITS_BAD_FILE);
			}
			int32_t drum_index = find_drum(_drums, label);
			if (drum_index == -1) {
				drumkit_itr->drums[drumkit_index] = (int32_t)_drums.size();
				Drum drum;
				drum.label = label;
				_drums.push_back(drum);
			}
			else {
				drumkit_itr->drums[drumkit_index] = drum_index;
			}
			drumkit_index += 1;
			if (drumkit_index == NUM_DRUMS_PER_DRUMKIT) {
				drumkit_itr += 1;
				if (drumkit_itr == _drumkits.end()) {
					drum_itr = _drums.begin();
					ifs.seekg(0);
					step = Step::LOOKING_FOR_DRUM;
				}
				else {
					drumkit_index = 0;
					ifs.seekg(0);
					step = Step::LOOKING_FOR_DRUMKIT;
				}
			}
		}

		else if (step == Step::LOOKING_FOR_DRUM) {
			if (indented) { continue; }
			std::string label;
			if (!get_label(lss, label)) {
				continue;
			}
			if (label == drum_itr->label) {
				step = Step::READING_DRUM;
			}
		}

		else if (step == Step::READING_DRUM) {
			if (!indented) { continue; }
			std::string macro;
			if (!leading_macro(lss, macro)) {
				return (_result = Result::DRUMKITS_BAD_FILE);
			}
			if (macro == "noise_note") {
				int32_t length, volume, fade, frequency;
				if (!get_number_and_number_and_number_and_number(lss, length, volume, fade, frequency)) {
					return (_result = Result::DRUMKITS_BAD_FILE);
				}
				if (length < 0 || length > 255) {
					return (_result = Result::DRUMKITS_BAD_FILE);
				}
				if (volume < 0 || volume > 15) {
					return (_result = Result::DRUMKITS_BAD_FILE);
				}
				if (fade == 8) fade = 0; // 8 is used in place of 0
				if (fade < -7 || fade > 7) {
					return (_result = Result::DRUMKITS_BAD_FILE);
				}
				if (frequency < 0 || frequency > 255) {
					return (_result = Result::DRUMKITS_BAD_FILE);
				}
				Noise_Note noise_note;
				noise_note.length = length;
				noise_note.volume = volume;
				if (fade < 0) {
					noise_note.envelope_direction = 1;
					noise_note.sweep_pace = fade * -1;
				}
				else {
					noise_note.envelope_direction = 0;
					noise_note.sweep_pace = fade;
				}
				noise_note.clock_shift = frequency >> 4;
				noise_note.lfsr_width = (frequency >> 3) & 1;
				noise_note.clock_divider = frequency & 0b111;
				drum_itr->noise_notes.push_back(noise_note);
			}
			else if (macro == "sound_ret") {
				drum_itr += 1;
				if (drum_itr == _drums.end()) {
					step = Step::DONE;
					break;
				}
				else {
					ifs.seekg(0);
					step = Step::LOOKING_FOR_DRUM;
				}
			}
			else {
				return (_result = Result::DRUMKITS_BAD_FILE);
			}
		}
	}

	_num_parsed_drumkits = (int32_t)_drumkits.size();
	if (_drumkits.size() > 256) {
		_drumkits.resize(256);
	}

	_num_parsed_drums = (int32_t)_drums.size();
	if (_drums.size() > 64) {
		_drums.resize(64);
	}

	if (step != Step::DONE) {
		return (_result = Result::DRUMKITS_BAD_FILE);
	}
	return (_result = Result::DRUMKITS_OK);
}
