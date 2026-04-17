#ifndef PARSE_WAVES_H
#define PARSE_WAVES_H

#include <cstdint>
#include <string>
#include <array>
#include <vector>

constexpr size_t NUM_WAVE_SAMPLES = 32;
typedef std::array<uint8_t, NUM_WAVE_SAMPLES> Wave;

struct Waves {
	std::string waves_file;
	std::string waves_label;
	std::vector<Wave> waves;
	int32_t num_waves = 0;
	bool uses_dn = false;
};

class Parsed_Waves {
public:
	enum class Result {
		WAVES_OK,
		WAVES_BAD_FILE,
		WAVES_NO_WAVES,
		WAVES_UNRECOGNIZED_MACRO,
		WAVES_TOO_FEW_SAMPLES,
		WAVES_TOO_MANY_SAMPLES,
		WAVES_INVALID_VALUE,
		WAVES_NULL
	};
private:
	std::string _waves_file;
	std::string _waves_label;
	std::vector<Wave> _waves;
	int32_t _num_parsed_waves = 0;
	bool _uses_dn = false;
	Result _result = Result::WAVES_NULL;

	// for error reporting
	int32_t _line_number = 0;
public:
	Parsed_Waves(const char *d);
	inline ~Parsed_Waves() {}
	inline std::string waves_file(void) const { return _waves_file; }
	inline std::string waves_label(void) const { return _waves_label; }
	inline std::vector<Wave> &&waves(void) { return std::move(_waves); }
	inline int32_t num_parsed_waves(void) const { return _num_parsed_waves; }
	inline bool uses_dn(void) const { return _uses_dn; }
	inline Result result(void) const { return _result; }

	std::string get_error_message() const;

	static Result parse_wave(std::istringstream &lss, Wave &wave, bool nybbles);
private:
	Result parse_waves(const char *d);
	Result try_parse_waves(const char *f);
};

#endif
