#include <cstdio>

#include "song.h"
#include "parse-song.h"

Song::Song() : _song_name(), _number_of_channels(0),
	_channel_1_label(), _channel_2_label(), _channel_3_label(), _channel_4_label(),
	_channel_1_commands(), _channel_2_commands(), _channel_3_commands(), _channel_4_commands(),
	_result(Result::SONG_NULL), _loaded(false) {}

Song::~Song() {
	clear();
}

void Song::clear() {
	_song_name = "";
	_number_of_channels = 0;
	_channel_1_label = "";
	_channel_2_label = "";
	_channel_3_label = "";
	_channel_4_label = "";
	_channel_1_commands.clear();
	_channel_2_commands.clear();
	_channel_3_commands.clear();
	_channel_4_commands.clear();
	_result = Result::SONG_NULL;
	_loaded = false;
}

Song::Result Song::read_song(const char *f) {
	Parsed_Song data(f);
	if (data.result() != Parsed_Song::Result::SONG_OK) {
		return (_result = Result::SONG_BAD_FILE); // cannot parse file
	}

	_song_name = data.song_name();
	_number_of_channels = data.number_of_channels();
	_channel_1_label = data.channel_1_label();
	_channel_2_label = data.channel_2_label();
	_channel_3_label = data.channel_3_label();
	_channel_4_label = data.channel_4_label();
	_channel_1_commands = data.channel_1_commands();
	_channel_2_commands = data.channel_2_commands();
	_channel_3_commands = data.channel_3_commands();
	_channel_4_commands = data.channel_4_commands();

	_loaded = true;
	return (_result = Result::SONG_OK);
}

std::string Song::commands_str(const std::list<Command> &commands) const {
	std::string str;
	for (const Command &command : commands) {
		str += COMMAND_NAMES[(uint32_t)command.command_type];
		str += "\n";
	}
	return str;
}

const char *Song::error_message(Result result) {
	switch (result) {
	case Result::SONG_OK:
		return "OK.";
	case Result::SONG_BAD_FILE:
		return "Not a valid song file.";
	case Result::SONG_NULL:
		return "No *.asm file chosen.";
	default:
		return "Unspecified error.";
	}
}
