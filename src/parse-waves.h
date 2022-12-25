#ifndef PARSE_WAVES_H
#define PARSE_WAVES_H

#include <string>
#include <array>
#include <vector>

constexpr size_t NUM_WAVE_SAMPLES = 32;
typedef std::array<uint8_t, NUM_WAVE_SAMPLES> Wave;

class Parsed_Waves {
public:
	enum class Result { WAVES_OK, WAVES_BAD_FILE, WAVES_NULL };
private:
	std::string _waves_file;
	std::vector<Wave> _waves;
	Result _result = Result::WAVES_NULL;
public:
	Parsed_Waves(const char *d);
	inline ~Parsed_Waves() {}
	inline std::string waves_file(void) const { return _waves_file; }
	inline std::vector<Wave> &&waves(void) { return std::move(_waves); }
	inline Result result(void) const { return _result; }

	static Result parse_wave(std::istringstream &lss, Wave &wave, bool nybbles);
private:
	Result parse_waves(const char *d);
	Result try_parse_waves(const char *f);
};

#endif
