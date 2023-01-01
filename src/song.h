#ifndef SONG_H
#define SONG_H

#include <deque>
#include <set>
#include <vector>

#include "utils.h"
#include "command.h"
#include "parse-song.h"
#include "option-dialogs.h"

#define MAX_HISTORY_SIZE 100

std::vector<Command>::const_iterator find_note_with_label(const std::vector<Command> &commands, std::string label);

struct Extra_Info {
	int32_t loop_index = 0;
	int32_t speed_at_loop = 1;
	int32_t volume_at_loop = 0;
	int32_t fade_at_loop = 0;

	int32_t end_index = 0;
	int32_t speed_at_end = 1;
	int32_t volume_at_end = 0;
	int32_t fade_at_end = 0;
};

Parsed_Song::Result calc_channel_length(const std::vector<Command> &commands, int32_t &loop_tick, int32_t &end_tick, Extra_Info *info = nullptr);

class Song {
public:
	struct Song_State {
		enum class Action {
			PUT_NOTE,
			PITCH_UP,
			PITCH_DOWN,
			OCTAVE_UP,
			OCTAVE_DOWN,
			MOVE_LEFT,
			MOVE_RIGHT,
			SHORTEN,
			LENGTHEN,
			DELETE_SELECTION,
			SNIP_SELECTION,
			SPLIT_NOTE,
			GLUE_NOTE
		};
		int tick = -1;
		int channel_number = 0;
		std::vector<Command> commands;
		std::set<int32_t> selection;
		Action action = {};
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
	inline int undo_tick(void) const { return _history.back().tick; }
	inline int redo_tick(void) const { return _future.back().tick; }
	inline int undo_channel_number(void) const { return _history.back().channel_number; }
	inline int redo_channel_number(void) const { return _future.back().channel_number; }
	inline std::set<int32_t> undo_selection(void) const { return _history.back().selection; }
	inline std::set<int32_t> redo_selection(void) const { return _future.back().selection; }
	inline Song_State::Action undo_action(void) const { return _history.back().action; }
	inline Song_State::Action redo_action(void) const { return _future.back().action; }
	inline const char *undo_action_message(void) const { return get_action_message(_history.back().action); }
	inline const char *redo_action_message(void) const { return get_action_message(_future.back().action); }
	inline bool loaded(void) const { return _loaded; }
	void clear();
	void remember(int channel_number, const std::set<int32_t> &selection, Song_State::Action action, int tick = -1);
	void undo();
	void redo();
	Parsed_Song::Result read_song(const char *f);
	void new_song(Song_Options_Dialog::Song_Options options);
	bool write_song(const char *f);
	const char *error_message() const { return _error_message.c_str(); }

	const std::vector<Command> &channel_1_commands() const { return _channel_1_commands; }
	const std::vector<Command> &channel_2_commands() const { return _channel_2_commands; }
	const std::vector<Command> &channel_3_commands() const { return _channel_3_commands; }
	const std::vector<Command> &channel_4_commands() const { return _channel_4_commands; }
	int32_t channel_1_loop_tick(void) const { return _channel_1_loop_tick; }
	int32_t channel_2_loop_tick(void) const { return _channel_2_loop_tick; }
	int32_t channel_3_loop_tick(void) const { return _channel_3_loop_tick; }
	int32_t channel_4_loop_tick(void) const { return _channel_4_loop_tick; }
	int32_t channel_1_end_tick(void) const { return _channel_1_end_tick; }
	int32_t channel_2_end_tick(void) const { return _channel_2_end_tick; }
	int32_t channel_3_end_tick(void) const { return _channel_3_end_tick; }
	int32_t channel_4_end_tick(void) const { return _channel_4_end_tick; }
	const std::vector<Wave> &waves() const { return _waves; }

	void put_note(const int selected_channel, const std::set<int32_t> &selected_boxes, Pitch pitch, int32_t index, int32_t tick, int32_t tick_offset);
	void pitch_up(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view);
	void pitch_down(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view);
	void octave_up(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view);
	void octave_down(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view);
	void move_left(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes);
	void move_right(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes);
	void shorten(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes);
	void lengthen(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes);
	void delete_selection(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes);
	void snip_selection(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes);
	void split_note(const int selected_channel, const std::set<int32_t> &selected_boxes, int32_t index, int32_t tick, int32_t tick_offset);
	void glue_note(const int selected_channel, const std::set<int32_t> &selected_boxes, int32_t index, int32_t tick);

	std::vector<Command> &channel_commands(const int selected_channel);
	int32_t channel_loop_tick(const int selected_channel) const;
	int32_t channel_end_tick(const int selected_channel) const;

	int32_t max_wave_id() const;
private:
	std::string commands_str(const std::vector<Command> &commands, int32_t channel_number) const;
	std::string get_error_message(Parsed_Song parsed_song) const;
	const char *get_action_message(Song_State::Action action) const;
};

#endif
