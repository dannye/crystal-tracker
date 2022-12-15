#ifndef PARSE_SONG_H
#define PARSE_SONG_H

#include <vector>

#include "command.h"

class Parsed_Song {
public:
	enum class Result {
		SONG_OK,
		SONG_BAD_FILE,
		SONG_INVALID_HEADER,
		SONG_TOO_COMPLEX,
		SONG_ENDED_PREMATURELY,
		SONG_UNRECOGNIZED_LABEL,
		SONG_ILLEGAL_MACRO,
		SONG_UNRECOGNIZED_MACRO,
		SONG_INVALID_MACRO_ARGUMENT,
		SONG_UNSUPPORTED_MACRO_ARGUMENT,
		SONG_NULL
	};
private:
	std::string _song_name;
	int32_t     _number_of_channels = 0;
	std::string _channel_1_label;
	std::string _channel_2_label;
	std::string _channel_3_label;
	std::string _channel_4_label;
	std::vector<Command> _channel_1_commands;
	std::vector<Command> _channel_2_commands;
	std::vector<Command> _channel_3_commands;
	std::vector<Command> _channel_4_commands;
	Result _result = Result::SONG_NULL;

	// for error reporting
	int32_t _line_number = 0;
	int32_t _channel_number = 0;
	std::string _label;
public:
	Parsed_Song(const char *f);
	inline ~Parsed_Song() {}
	inline std::string song_name(void) const { return _song_name; }
	inline int32_t number_of_channels(void) const { return _number_of_channels; }
	inline std::string channel_1_label(void) const { return _channel_1_label; }
	inline std::string channel_2_label(void) const { return _channel_2_label; }
	inline std::string channel_3_label(void) const { return _channel_3_label; }
	inline std::string channel_4_label(void) const { return _channel_4_label; }
	inline std::vector<Command> &&channel_1_commands(void) { return std::move(_channel_1_commands); }
	inline std::vector<Command> &&channel_2_commands(void) { return std::move(_channel_2_commands); }
	inline std::vector<Command> &&channel_3_commands(void) { return std::move(_channel_3_commands); }
	inline std::vector<Command> &&channel_4_commands(void) { return std::move(_channel_4_commands); }
	inline Result result(void) const { return _result; }
	inline int32_t line_number(void) const { return _line_number; }
	inline int32_t channel_number(void) const { return _channel_number; }
	inline const std::string &label(void) const { return _label; }
private:
	Result parse_song(const char *f);
};

#endif
