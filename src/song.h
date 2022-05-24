#ifndef SONG_H
#define SONG_H

#include <list>

#include "utils.h"
#include "command.h"

class Song {
public:
	enum class Result { SONG_OK, SONG_BAD_FILE, SONG_NULL };
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
	bool _loaded;
public:
	Song();
	~Song();
	inline Result result(void) const { return _result; }
	inline bool loaded(void) const { return _loaded; }
	void clear();
	Result read_song(const char *f);

	std::string channel_1_commands_str() { return commands_str(_channel_1_commands); }
	std::string channel_2_commands_str() { return commands_str(_channel_2_commands); }
	std::string channel_3_commands_str() { return commands_str(_channel_3_commands); }
	std::string channel_4_commands_str() { return commands_str(_channel_4_commands); }
private:
	std::string commands_str(const std::list<Command> &) const;
public:
	static const char *error_message(Result result);
};

#endif
