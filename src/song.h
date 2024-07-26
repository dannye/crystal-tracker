#ifndef SONG_H
#define SONG_H

#include <deque>
#include <set>
#include <vector>

#include "utils.h"
#include "command.h"
#include "parse-song.h"
#include "option-dialogs.h"

#define MAX_HISTORY_SIZE 256

template<typename T>
inline int32_t itr_index(const typename std::vector<T> &vec, const typename std::vector<T>::const_iterator &itr) { return (int32_t)(itr - vec.begin()); }

std::string get_scope(const std::vector<Command> &commands, int32_t index);

std::string get_next_label(const std::vector<Command> &commands, const std::string &scope, const std::string &prefix, int &i);
std::string get_next_loop_label(const std::vector<Command> &commands, const std::string &scope, int &i);
std::string get_next_call_label(const std::vector<Command> &commands, const std::string &scope, int &i);

int count_label_references(const std::vector<Command> &commands, const std::string &label);
bool is_label_referenced(const std::vector<Command> &commands, const std::string &label);
void delete_label(std::vector<Command> &commands, const std::string &label);

std::vector<Command>::const_iterator find_note_with_label(const std::vector<Command> &commands, const std::string &label);

bool is_followed_by_n_ticks_of_rest(std::vector<Command>::const_iterator itr, std::vector<Command>::const_iterator end, int32_t n, int32_t speed);
bool is_followed_by_n_ticks_of_rest_no_speed_change(std::vector<Command>::const_iterator itr, std::vector<Command>::const_iterator end, int32_t n, int32_t speed);

std::vector<Command> copy_snippet(const std::vector<Command> &commands, int32_t start_index, int32_t end_index, bool copy_jumps = false);

struct Extra_Info {
	int32_t loop_index = 0;
	int32_t speed_at_loop = 1;
	int32_t volume_at_loop = 0;
	int32_t fade_at_loop = 0;
	int32_t drumkit_at_loop = -1;

	int32_t end_index = 0;
	int32_t speed_at_end = 1;
	int32_t volume_at_end = 0;
	int32_t fade_at_end = 0;
	int32_t drumkit_at_end = -1;
};

Parsed_Song::Result calc_channel_length(const std::vector<Command> &commands, int32_t &loop_tick, int32_t &end_tick, Extra_Info *info = nullptr);

int32_t calc_snippet_length(const std::vector<Command> &commands, const std::vector<Command>::const_iterator &start_itr, const std::vector<Command>::const_iterator &end_itr, const Note_View &start_view);

Note_View get_note_view(const std::vector<Command> &commands, int32_t index, int32_t min_tick = 0);

void postprocess(std::vector<Command> &commands);

void split_tempo_change_rests(std::vector<Command> &commands, const std::set<int32_t> &tempo_changes);

class Song {
public:
	struct Song_State {
		enum class Action {
			PUT_NOTE,
			FORMAT_PAINTER,
			SET_SPEED,
			SET_VOLUME,
			SET_FADE,
			SET_VIBRATO_DELAY,
			SET_VIBRATO_EXTENT,
			SET_VIBRATO_RATE,
			SET_WAVE,
			SET_DRUMKIT,
			SET_DUTY,
			SET_TEMPO,
			SET_TRANSPOSE_OCTAVES,
			SET_TRANSPOSE_PITCHES,
			SET_SLIDE_DURATION,
			SET_SLIDE_OCTAVE,
			SET_SLIDE_PITCH,
			SET_SLIDE,
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
			INSERT_REST,
			SPLIT_NOTE,
			GLUE_NOTE,
			POSTPROCESS_CHANNEL,
			REDUCE_LOOP,
			EXTEND_LOOP,
			UNROLL_LOOP,
			CREATE_LOOP,
			DELETE_CALL,
			UNPACK_CALL,
			CREATE_CALL,
			INSERT_CALL
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

	std::vector<std::string> _mixed_labels;

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
	Song_Options_Dialog::Song_Options get_options();
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
	const std::vector<std::string> &mixed_labels() const { return _mixed_labels; }

	int32_t put_note(const int selected_channel, const std::set<int32_t> &selected_boxes, Pitch pitch, int32_t octave, int32_t old_octave, int32_t old_speed, int32_t prev_length, int32_t prev_speed, int32_t index, int32_t tick, int32_t tick_offset, bool set_drumkit);
	void apply_format_painter(const int selected_channel, const std::set<int32_t> &selected_boxes, const Note_View &view, int32_t index, int32_t tick);
	void set_speed(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view, int32_t speed);
	void set_volume(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view, int32_t volume);
	void set_fade(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view, int32_t fade);
	void set_vibrato_delay(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view, int32_t delay);
	void set_vibrato_extent(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view, int32_t extent);
	void set_vibrato_rate(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view, int32_t rate);
	void set_wave(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view, int32_t wave);
	void set_drumkit(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, int32_t drumkit);
	void set_duty(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, int32_t duty);
	void set_tempo(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, int32_t tempo);
	void set_transpose_octaves(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view, int32_t octaves);
	void set_transpose_pitches(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view, int32_t pitches);
	void set_slide_duration(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view, int32_t duration);
	void set_slide_octave(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view, int32_t octave);
	void set_slide_pitch(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view, Pitch pitch);
	void set_slide(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, int32_t duration, int32_t octave, Pitch pitch);

	void pitch_up(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view);
	void pitch_down(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view);
	void octave_up(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view);
	void octave_down(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view);
	void move_left(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view);
	void move_right(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view);
	void shorten(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, int32_t tick);
	void lengthen(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes, const std::vector<Note_View> &view, int32_t tick);
	void delete_selection(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes);
	void snip_selection(const int selected_channel, const std::set<int32_t> &selected_notes, const std::set<int32_t> &selected_boxes);
	void insert_rest(const int selected_channel, const std::set<int32_t> &selected_boxes, int32_t index, int32_t tick, int32_t tick_offset);
	void split_note(const int selected_channel, const std::set<int32_t> &selected_boxes, int32_t index, int32_t tick, int32_t tick_offset);
	void glue_note(const int selected_channel, const std::set<int32_t> &selected_boxes, int32_t index, int32_t tick);

	void postprocess_channel(const int selected_channel, const std::set<int32_t> &selected_boxes);
	void resize_song(const Song_Options_Dialog::Song_Options &options);

	void reduce_loop(const int selected_channel, const std::set<int32_t> &selected_boxes, int32_t tick, int32_t loop_index, int32_t loop_length, Note_View start_view, Note_View end_view);
	void extend_loop(const int selected_channel, const std::set<int32_t> &selected_boxes, int32_t tick, int32_t loop_index, int32_t loop_length, Note_View start_view, Note_View end_view);
	void unroll_loop(const int selected_channel, const std::set<int32_t> &selected_boxes, int32_t tick, int32_t loop_index, const std::vector<Command> &snippet);
	void create_loop(const int selected_channel, const std::set<int32_t> &selected_boxes, int32_t tick, int32_t start_index, int32_t end_index, int32_t loop_length, Note_View start_view, Note_View end_view);
	void delete_call(const int selected_channel, const std::set<int32_t> &selected_boxes, int32_t tick, int32_t call_index, int32_t ambiguous_ticks, int32_t unambiguous_ticks, Note_View start_view, Note_View end_view, int32_t start_index, int32_t end_index);
	void unpack_call(const int selected_channel, const std::set<int32_t> &selected_boxes, int32_t tick, int32_t call_index, const std::vector<Command> &snippet, int32_t start_index, int32_t end_index);
	void create_call(const int selected_channel, const std::set<int32_t> &selected_boxes, int32_t tick, int32_t start_index, int32_t end_index, const std::vector<Command> &snippet);
	void insert_call(const int selected_channel, const std::set<int32_t> &selected_boxes, int32_t tick, int32_t tick_offset, int32_t insert_index, const std::string &target_label, int32_t call_length, Note_View insert_view, Note_View start_view, Note_View end_view);

	std::vector<Command> &channel_commands(const int selected_channel);
	const std::string &channel_label(const int selected_channel) const;
	int32_t channel_loop_tick(const int selected_channel) const;
	int32_t channel_end_tick(const int selected_channel) const;

	int32_t max_wave_id() const;
	int32_t max_drumkit_id() const;
private:
	std::string commands_str(const std::vector<Command> &commands, int32_t channel_number) const;
	std::string get_error_message(const Parsed_Song &parsed_song) const;
	const char *get_action_message(Song_State::Action action) const;
};

#endif
