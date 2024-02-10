#ifndef PARSE_SONG_H
#define PARSE_SONG_H

#include <vector>

#include "command.h"
#include "parse-waves.h"

class Parsed_Song {
public:
	enum class Result {
		SONG_OK,
		SONG_BAD_FILE,
		SONG_INVALID_HEADER,
		SONG_EMPTY_LOOP,
		SONG_NESTED_LOOP,
		SONG_NESTED_CALL,
		SONG_UNFINISHED_LOOP,
		SONG_UNFINISHED_CALL,
		SONG_NO_DRUMKIT_SELECTED,
		SONG_TOGGLE_NOISE_ALREADY_DISABLED,
		SONG_TOGGLE_NOISE_ALREADY_ENABLED,
		SONG_ENDED_PREMATURELY,
		SONG_UNRECOGNIZED_LABEL,
		SONG_UNSUPPORTED_KEYWORD,
		SONG_ILLEGAL_MACRO,
		SONG_UNRECOGNIZED_MACRO,
		SONG_INVALID_MACRO_ARGUMENT,
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
	int32_t _channel_1_loop_tick = -1;
	int32_t _channel_2_loop_tick = -1;
	int32_t _channel_3_loop_tick = -1;
	int32_t _channel_4_loop_tick = -1;
	int32_t _channel_1_end_tick = -1;
	int32_t _channel_2_end_tick = -1;
	int32_t _channel_3_end_tick = -1;
	int32_t _channel_4_end_tick = -1;

	std::vector<Wave> _waves;

	Result _result = Result::SONG_NULL;

	// for error reporting
	int32_t _line_number = 0;
	int32_t _channel_number = 0;
	std::string _label;
	std::vector<std::string> _mixed_labels;
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
	inline int32_t channel_1_loop_tick(void) const { return _channel_1_loop_tick; }
	inline int32_t channel_2_loop_tick(void) const { return _channel_2_loop_tick; }
	inline int32_t channel_3_loop_tick(void) const { return _channel_3_loop_tick; }
	inline int32_t channel_4_loop_tick(void) const { return _channel_4_loop_tick; }
	inline int32_t channel_1_end_tick(void) const { return _channel_1_end_tick; }
	inline int32_t channel_2_end_tick(void) const { return _channel_2_end_tick; }
	inline int32_t channel_3_end_tick(void) const { return _channel_3_end_tick; }
	inline int32_t channel_4_end_tick(void) const { return _channel_4_end_tick; }
	inline std::vector<Wave> &&waves(void) { return std::move(_waves); }
	inline Result result(void) const { return _result; }
	inline int32_t line_number(void) const { return _line_number; }
	inline int32_t channel_number(void) const { return _channel_number; }
	inline const std::string &label(void) const { return _label; }
	inline std::vector<std::string> &&mixed_labels(void) { return std::move(_mixed_labels); }
private:
	Result parse_song(const char *f);
};

#endif
