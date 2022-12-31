#include <cstring>

#pragma warning(push, 0)
#include <FL/filename.H>
#pragma warning(pop)

#include "parse-waves.h"

#include "utils.h"

Parsed_Waves::Parsed_Waves(const char *d) {
	parse_waves(d);
}

Parsed_Waves::Result Parsed_Waves::parse_wave(std::istringstream &lss, Wave &wave, bool nybbles) {
	size_t i = 0;
	for (std::string token; std::getline(lss, token, ',');) {
		if (i >= NUM_WAVE_SAMPLES) {
			return Result::WAVES_BAD_FILE;
		}

		int32_t v;
		if (!parse_value(token, v)) {
			return Result::WAVES_BAD_FILE;
		}
		if (nybbles) {
			if (v < 0 || v > 15) {
				return Result::WAVES_BAD_FILE;
			}
			wave[i++] = v;
		}
		else {
			if (v < 0 || v > 255) {
				return Result::WAVES_BAD_FILE;
			}
			int32_t hi = v >> 4;
			int32_t lo = v & 0x0F;
			wave[i++] = hi;
			wave[i++] = lo;
		}
	}
	if (i != NUM_WAVE_SAMPLES) {
		return Result::WAVES_BAD_FILE;
	}
	return Result::WAVES_OK;
}

Parsed_Waves::Result Parsed_Waves::parse_waves(const char *d) {
	char waves_file[FL_PATH_MAX] = {};

	// first, try crysaudio/wave_samples.asm
	strcpy(waves_file, d);
	strcat(waves_file, DIR_SEP "crysaudio" DIR_SEP "wave_samples.asm");
	if (try_parse_waves(waves_file) == Parsed_Waves::Result::WAVES_OK) {
		return _result;
	}
	// second, try audio/wave_samples.asm
	strcpy(waves_file, d);
	strcat(waves_file, DIR_SEP "audio" DIR_SEP "wave_samples.asm");
	if (try_parse_waves(waves_file) == Parsed_Waves::Result::WAVES_OK) {
		return _result;
	}
	// third, try wave_samples.asm
	strcpy(waves_file, d);
	strcat(waves_file, DIR_SEP "wave_samples.asm");
	if (try_parse_waves(waves_file) == Parsed_Waves::Result::WAVES_OK) {
		return _result;
	}

	return _result;
}

Parsed_Waves::Result Parsed_Waves::try_parse_waves(const char *f) {
	_waves_file = f;
	_waves.clear();
	_num_parsed_waves = 0;
	_result = Result::WAVES_NULL;

	std::ifstream ifs;
	open_ifstream(ifs, f);
	if (!ifs.good()) {
		return (_result = Result::WAVES_BAD_FILE);
	}

	while (ifs.good()) {
		std::string line;
		std::getline(ifs, line);
		remove_comment(line);
		rtrim(line);
		if (line.size() == 0) { continue; }
		std::istringstream lss(line);

		std::string macro;
		if (!leading_macro(lss, macro)) { continue; }
		bool nybbles = equals_ignore_case(macro, "dn");
		if (!nybbles && !equals_ignore_case(macro, "db")) { continue; }

		Wave wave;
		Result r = parse_wave(lss, wave, nybbles);
		if (r != Result::WAVES_OK) {
			return (_result = r);
		}
		_waves.push_back(wave);
	}

	_num_parsed_waves = (int32_t)_waves.size();
	_waves.resize(16);

	if (_num_parsed_waves == 0) {
		return (_result = Result::WAVES_BAD_FILE);
	}
	return (_result = Result::WAVES_OK);
}
