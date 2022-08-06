#ifndef SONG_H
#define SONG_H

#include <deque>
#include <vector>

#include "utils.h"
#include "command.h"
#include "parse-song.h"

#define MAX_HISTORY_SIZE 100

class Song {
protected:
	struct Song_State {
		enum class Action {
			PITCH_UP,
			PITCH_DOWN,
			OCTAVE_UP,
			OCTAVE_DOWN,
			DELETE_SELECTION,
			SNIP_SELECTION
		};
		int channel_number = 0;
		std::vector<Command> commands;
		Action action;
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
	Parsed_Song::Result _result = Parsed_Song::Result::SONG_NULL;
	bool _modified = false;
	std::deque<Song_State> _history, _future;
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
	inline bool can_undo(void) const { return !_history.empty(); }
	inline bool can_redo(void) const { return !_future.empty(); }
	inline const char *undo_action(void) const { return get_action_message(_history.back().action); }
	inline const char *redo_action(void) const { return get_action_message(_future.back().action); }
	inline bool loaded(void) const { return _loaded; }
	void clear();
	void remember(int channel_number, Song_State::Action action);
	int undo();
	int redo();
	Parsed_Song::Result read_song(const char *f);
	void new_song();
	bool write_song(const char *f);
	const char *error_message() const { return _error_message.c_str(); }

	const std::vector<Command> &channel_1_commands() const { return _channel_1_commands; }
	const std::vector<Command> &channel_2_commands() const { return _channel_2_commands; }
	const std::vector<Command> &channel_3_commands() const { return _channel_3_commands; }
	const std::vector<Command> &channel_4_commands() const { return _channel_4_commands; }

	void pitch_up(const int selected_channel, const std::set<int32_t> &selected_notes, const std::vector<Note_View> &view);
	void pitch_down(const int selected_channel, const std::set<int32_t> &selected_notes, const std::vector<Note_View> &view);
	void octave_up(const int selected_channel, const std::set<int32_t> &selected_notes, const std::vector<Note_View> &view);
	void octave_down(const int selected_channel, const std::set<int32_t> &selected_notes, const std::vector<Note_View> &view);
	void delete_selection(const int selected_channel, const std::set<int32_t> &selected_notes);
	void snip_selection(const int selected_channel, const std::set<int32_t> &selected_notes);
private:
	std::string commands_str(const std::vector<Command> &commands) const;
	std::string get_error_message(Parsed_Song parsed_song) const;
	const char *get_action_message(Song_State::Action action) const;

	std::vector<Command> &channel_commands(const int selected_channel);
};

#endif
