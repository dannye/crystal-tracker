#ifndef PARSE_SONG_H
#define PARSE_SONG_H

#include <list>

#include "command.h"

class Parsed_Song {
public:
	enum class Result { SONG_OK, SONG_BAD_FILE, SONG_OVERFLOW, SONG_NULL };
private:
	std::string _song_name;
	uint32_t    _number_of_channels;
	std::string _channel_1_label;
	std::string _channel_2_label;
	std::string _channel_3_label;
	std::string _channel_4_label;
	std::list<Command> _channel_1_commands;
	std::list<Command> _channel_2_commands;
	std::list<Command> _channel_3_commands;
	std::list<Command> _channel_4_commands;
	Result _result;
public:
	Parsed_Song(const char *f);
	inline ~Parsed_Song() {}
	inline std::string song_name(void) const { return _song_name; }
	inline uint32_t number_of_channels(void) const { return _number_of_channels; }
	inline std::string channel_1_label(void) const { return _channel_1_label; }
	inline std::string channel_2_label(void) const { return _channel_2_label; }
	inline std::string channel_3_label(void) const { return _channel_3_label; }
	inline std::string channel_4_label(void) const { return _channel_4_label; }
	inline std::list<Command> channel_1_commands(void) const { return _channel_1_commands; }
	inline std::list<Command> channel_2_commands(void) const { return _channel_2_commands; }
	inline std::list<Command> channel_3_commands(void) const { return _channel_3_commands; }
	inline std::list<Command> channel_4_commands(void) const { return _channel_4_commands; }
	inline Result result(void) const { return _result; }
private:
	Result parse_song(const char *f);
};

#endif
