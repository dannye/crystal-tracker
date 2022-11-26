#ifndef IT_MODULE_H
#define IT_MODULE_H

#include <cstdint>
#include <vector>

#include <libopenmpt/libopenmpt_ext.hpp>
#include <portaudiocpp/PortAudioCpp.hxx>

#include "parse-waves.h"
#include "piano-roll.h"

constexpr uint32_t ROWS_PER_PATTERN = 192;

class IT_Module {
private:
	std::vector<uint8_t> _data;

	openmpt::module_ext *_mod = nullptr;

	portaudio::BlockingStream _stream;
	std::vector<float> _buffer;
	bool _is_interleaved = false;

	int32_t _current_pattern = 0;
	int32_t _current_row = 0;

	bool _paused = false;
public:
	IT_Module(const Piano_Roll &song, const std::vector<Wave> &waves, int32_t loop_tick = -1);
	~IT_Module() noexcept;

	IT_Module(const IT_Module&) = delete;
	IT_Module& operator=(const IT_Module&) = delete;

	bool ready() const { return _stream.isOpen(); }
	bool playing() { return Pa_IsStreamActive(_stream.paStream()) == 1; }
	bool paused() const { return _paused; }
	bool stopped() { return !playing() && !paused(); }
	bool looping() { return _mod->get_repeat_count() == -1; }

	bool start()   { _paused = false; return Pa_StartStream(_stream.paStream()) == paNoError; }
	bool stop()    { _paused = false; return Pa_StopStream(_stream.paStream())  == paNoError; }
	bool pause()   { _paused = true;  return Pa_StopStream(_stream.paStream())  == paNoError; }
	void play();

	void mute_channel(int32_t channel, bool mute);

	int32_t current_tick() const { return (_current_pattern * ROWS_PER_PATTERN + _current_row) * 2; }
	void set_tick(int32_t tick);
private:
	bool try_open();
	void generate_it_module(const Piano_Roll &song, const std::vector<Wave> &waves, int32_t loop_tick = -1);
};

#endif
