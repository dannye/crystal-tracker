#ifndef SONG_H
#define SONG_H

#include <list>

#include "utils.h"
#include "command.h"
#include "parse-song.h"

class Song {
private:
	std::string _song_name;
	int32_t     _number_of_channels = 0;
	std::string _channel_1_label;
	std::string _channel_2_label;
	std::string _channel_3_label;
	std::string _channel_4_label;
	std::list<Command> _channel_1_commands;
	std::list<Command> _channel_2_commands;
	std::list<Command> _channel_3_commands;
	std::list<Command> _channel_4_commands;
	std::list<Note_View> _channel_1_timeline;
	std::list<Note_View> _channel_2_timeline;
	std::list<Note_View> _channel_3_timeline;
	std::list<Note_View> _channel_4_timeline;
	Parsed_Song::Result _result = Parsed_Song::Result::SONG_NULL;
	bool _modified = false;
	int64_t _mod_time = 0;
	bool _loaded = false;

	std::string _error_message;
public:
	Song();
	~Song();
	inline Parsed_Song::Result result(void) const { return _result; }
	inline bool modified(void) const { return _modified; }
	inline void modified(bool m) { _modified = m; }
	inline bool other_modified(const char *f) const { return file_modified(f) > _mod_time; }
	inline bool loaded(void) const { return _loaded; }
	void clear();
	Parsed_Song::Result read_song(const char *f);
	void new_song();
	bool write_song(const char *f);
	const char *error_message() const { return _error_message.c_str(); }

	const std::list<Note_View> &channel_1_timeline() const { return _channel_1_timeline; }
	const std::list<Note_View> &channel_2_timeline() const { return _channel_2_timeline; }
	const std::list<Note_View> &channel_3_timeline() const { return _channel_3_timeline; }
	const std::list<Note_View> &channel_4_timeline() const { return _channel_4_timeline; }
private:
	std::string commands_str(const std::list<Command> &commands) const;
	std::string get_error_message(Parsed_Song parsed_song);
};

#endif
