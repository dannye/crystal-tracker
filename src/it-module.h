#ifndef IT_MODULE_H
#define IT_MODULE_H

#include <cstdint>
#include <array>
#include <vector>

#include <libopenmpt/libopenmpt_ext.hpp>
#include <portaudiocpp/PortAudioCpp.hxx>

#include "command.h"
#include "parse-waves.h"
#include "parse-drumkits.h"

constexpr std::size_t BUFFER_SIZE = 2048;
constexpr std::int32_t SAMPLE_RATE = 48000;

constexpr uint32_t ROWS_PER_PATTERN = 192;

constexpr uint32_t NOISE_SAMPLE_SPEED_FACTOR = 4;

constexpr float UNITS_PER_MINUTE = 256.0f /* units per frame */ * (262144.0f / 4389.0f) /* frames per second */ * 60.0f /* seconds per minute */;

class IT_Module {
private:
	std::vector<uint8_t> _data;
	int32_t _tempo_change_wrong_channel = -1;
	int32_t _tempo_change_mid_note = -1;

	openmpt::module_ext *_mod = nullptr;

	portaudio::BlockingStream _stream;
	std::array<float, BUFFER_SIZE * 2> _buffer;
	bool _is_interleaved = false;

	int32_t _current_pattern = 0;
	int32_t _current_row = 0;

	bool _paused = false;
public:
	IT_Module();
	IT_Module(
		const std::vector<Note_View> &channel_1_notes,
		const std::vector<Note_View> &channel_2_notes,
		const std::vector<Note_View> &channel_3_notes,
		const std::vector<Note_View> &channel_4_notes,
		const std::vector<Wave> &waves,
		const std::vector<Drumkit> &drumkits,
		const std::vector<std::vector<uint8_t>> &drums,
		int32_t loop_tick = -1
	);
	~IT_Module() noexcept;

	IT_Module(const IT_Module&) = delete;
	IT_Module& operator=(const IT_Module&) = delete;

	bool export_file(const char *f);

	std::string get_warnings() { return _mod->get_metadata("warnings"); }
	int32_t tempo_change_wrong_channel() const { return _tempo_change_wrong_channel; }
	int32_t tempo_change_mid_note() const { return _tempo_change_mid_note; }

	bool ready() const { return _stream.isOpen(); }
	bool playing() { return Pa_IsStreamActive(_stream.paStream()) == 1; }
	bool paused() const { return _paused; }
	bool stopped() { return !playing() && !paused(); }
	bool looping() { return _mod->get_repeat_count() == -1; }

	bool start()   { _paused = false; return Pa_StartStream(_stream.paStream()) == paNoError; }
	bool stop()    { _paused = false; return Pa_StopStream(_stream.paStream())  == paNoError; }
	bool pause()   { _paused = true;  return Pa_StopStream(_stream.paStream())  == paNoError; }
	bool play();

	void mute_channel(int32_t channel, bool mute);

	int32_t play_note(Pitch pitch, int32_t octave);
	void stop_note(int32_t channel);

	int32_t current_tick() const { return _current_pattern * ROWS_PER_PATTERN + _current_row; }
	void set_tick(int32_t tick);

	double get_position_seconds();
	double get_duration_seconds();
private:
	bool try_open();
	std::vector<std::vector<uint8_t>> get_instruments();
	std::vector<std::vector<uint8_t>> get_samples(const std::vector<Wave> &waves, const std::vector<std::vector<uint8_t>> &drums);
	std::vector<std::vector<uint8_t>> get_patterns(
		const std::vector<Note_View> &channel_1_notes,
		const std::vector<Note_View> &channel_2_notes,
		const std::vector<Note_View> &channel_3_notes,
		const std::vector<Note_View> &channel_4_notes,
		const std::vector<Drumkit> &drumkits,
		int32_t loop_tick,
		int32_t num_inline_waves
	);
	void generate_it_module(
		const std::vector<Note_View> &channel_1_notes = {},
		const std::vector<Note_View> &channel_2_notes = {},
		const std::vector<Note_View> &channel_3_notes = {},
		const std::vector<Note_View> &channel_4_notes = {},
		const std::vector<Wave> &waves = {},
		const std::vector<Drumkit> &drumkits = {},
		const std::vector<std::vector<uint8_t>> &drums = {},
		int32_t loop_tick = -1
	);
};

std::vector<std::vector<uint8_t>> generate_noise_samples(const std::vector<Drum> &drums);

#endif
