#ifndef IT_MODULE_H
#define IT_MODULE_H

#include <cstdint>
#include <vector>

#include <libopenmpt/libopenmpt.hpp>
#include <portaudiocpp/PortAudioCpp.hxx>

#include "song.h"

class IT_Module {
private:
	uint8_t *_data = nullptr;
	uint32_t _size = 0;

	openmpt::module *_mod = nullptr;

	portaudio::BlockingStream _stream;
	std::vector<float> _left;
	std::vector<float> _right;
	std::vector<float> _interleaved_buffer;
	bool _is_interleaved = false;
public:
	IT_Module(const Song &song);
	~IT_Module() noexcept;

	IT_Module(const IT_Module&) = delete;
	IT_Module& operator=(const IT_Module&) = delete;

	bool ready() const { return _stream.isOpen(); }
	bool is_playing() { return Pa_IsStreamActive(_stream.paStream()) == 1; }

	bool start() { return Pa_StartStream(_stream.paStream()) == paNoError; };
	bool stop() { return Pa_StopStream(_stream.paStream()) == paNoError; };
	void play();
private:
	bool try_open();
};

#endif
