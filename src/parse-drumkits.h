#ifndef PARSE_DRUMKITS_H
#define PARSE_DRUMKITS_H

#include <string>
#include <array>
#include <vector>

struct Noise_Note {
	int32_t length;

	int32_t volume;
	int32_t envelope_direction;
	int32_t sweep_pace;

	int32_t clock_shift;
	int32_t lfsr_width;
	int32_t clock_divider;
};

struct Drum {
	std::string label;
	std::vector<Noise_Note> noise_notes;
};

constexpr size_t NUM_DRUMS_PER_DRUMKIT = 13;

struct Drumkit {
	std::string label;
	std::array<int32_t, NUM_DRUMS_PER_DRUMKIT> drums;
};

class Parsed_Drumkits {
public:
	enum class Result { DRUMKITS_OK, DRUMKITS_BAD_FILE, DRUMKITS_NULL };
private:
	std::string _drumkits_file;
	std::vector<Drumkit> _drumkits;
	std::vector<Drum> _drums;
	int32_t _num_parsed_drumkits = 0;
	int32_t _num_parsed_drums = 0;
	Result _result = Result::DRUMKITS_NULL;
public:
	Parsed_Drumkits(const char *d);
	inline ~Parsed_Drumkits() {}
	inline std::string drumkits_file(void) const { return _drumkits_file; }
	inline std::vector<Drumkit> &&drumkits(void) { return std::move(_drumkits); }
	inline std::vector<Drum> &&drums(void) { return std::move(_drums); }
	inline int32_t num_parsed_drumkits(void) const { return _num_parsed_drumkits; }
	inline int32_t num_parsed_drums(void) const { return _num_parsed_drums; }
	inline Result result(void) const { return _result; }
private:
	Result parse_drumkits(const char *d);
	Result try_parse_drumkits(const char *f);
};

#endif
