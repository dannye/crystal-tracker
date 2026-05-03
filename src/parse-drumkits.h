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

struct Drumkits {
	std::string drumkits_file;
	std::string drumkits_label;
	std::vector<Drumkit> drumkits;
	std::vector<Drum> drums;
	bool uses_dr = false;
};

class Parsed_Drumkits {
public:
	enum class Result {
		DRUMKITS_OK,
		DRUMKITS_BAD_FILE,
		DRUMKITS_INVALID_DRUMKITS_TABLE,
		DRUMKITS_DUPLICATE_DRUMKIT,
		DRUMKITS_INVALID_DRUMKIT,
		DRUMKITS_DRUMKIT_ENDED_PREMATURELY,
		DRUMKITS_UNRECOGNIZED_DRUMKIT,
		DRUMKITS_DRUM_ENDED_PREMATURELY,
		DRUMKITS_UNRECOGNIZED_DRUM,
		DRUMKITS_UNRECOGNIZED_MACRO,
		DRUMKITS_INVALID_MACRO_ARGUMENT,
		DRUMKITS_NULL
	};
private:
	std::string _drumkits_file;
	std::string _drumkits_label;
	std::vector<Drumkit> _drumkits;
	std::vector<Drum> _drums;
	int32_t _num_parsed_drumkits = 0;
	bool _uses_dr = false;
	Result _result = Result::DRUMKITS_NULL;

	// for error reporting
	int32_t _line_number = 0;
	std::string _label;
public:
	Parsed_Drumkits(const char *d);
	inline ~Parsed_Drumkits() {}
	inline std::string drumkits_file(void) const { return _drumkits_file; }
	inline std::string drumkits_label(void) const { return _drumkits_label; }
	inline std::vector<Drumkit> &&drumkits(void) { return std::move(_drumkits); }
	inline std::vector<Drum> &&drums(void) { return std::move(_drums); }
	inline int32_t num_parsed_drumkits(void) const { return _num_parsed_drumkits; }
	inline bool uses_dr(void) const { return _uses_dr; }
	inline Result result(void) const { return _result; }

	std::string get_error_message() const;
private:
	Result parse_drumkits(const char *d);
	Result try_parse_drumkits(const char *f);
};

#endif
