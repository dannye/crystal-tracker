#include "it-module.h"

constexpr std::size_t BUFFER_SIZE = 2048;
constexpr std::int32_t SAMPLE_RATE = 48000;

IT_Module::IT_Module(const Song &song) : _buffer(BUFFER_SIZE * 2) {
	generate_it_module(song);

	_mod = new openmpt::module(_data);

	_is_interleaved = false;
	if (!try_open()) {
		_is_interleaved = true;
		try_open();
	}
}

IT_Module::~IT_Module() noexcept {
	if (_mod) {
		delete _mod;
		_mod = nullptr;
	}
}

void IT_Module::play() {
	if (!ready() || !is_playing()) return;

	std::size_t count = _is_interleaved ?
		_mod->read_interleaved_stereo(SAMPLE_RATE, BUFFER_SIZE, _buffer.data()) :
		_mod->read(SAMPLE_RATE, BUFFER_SIZE, _buffer.data(), _buffer.data() + BUFFER_SIZE);

	if (count == 0) {
		stop();
		return;
	}
	try {
		if (_is_interleaved) {
			_stream.write(_buffer.data(), static_cast<unsigned long>(count));
		}
		else {
			const float * const buffers[2] = { _buffer.data(), _buffer.data() + BUFFER_SIZE };
			_stream.write(buffers, static_cast<unsigned long>(count));
		}
	}
	catch (...) {}
}

bool IT_Module::try_open() {
	try {
		portaudio::System &portaudio = portaudio::System::instance();
		portaudio::DirectionSpecificStreamParameters outputstream_parameters(
			portaudio.defaultOutputDevice(),
			2,
			portaudio::FLOAT32,
			_is_interleaved,
			portaudio.defaultOutputDevice().defaultHighOutputLatency(),
			0
		);
		portaudio::StreamParameters stream_parameters(
			portaudio::DirectionSpecificStreamParameters::null(),
			outputstream_parameters,
			SAMPLE_RATE,
			paFramesPerBufferUnspecified,
			paNoFlag
		);
		_stream.open(stream_parameters);
		return true;
	}
	catch (...) {}
	return false;
}

static inline void put_int(std::vector<uint8_t> &data, const uint32_t v) {
    data.push_back(v >>  0);
    data.push_back(v >>  8);
    data.push_back(v >> 16);
    data.push_back(v >> 24);
}

static inline void put_short(std::vector<uint8_t> &data, const uint32_t v) {
    data.push_back(v >>  0);
    data.push_back(v >>  8);
}

static inline void patch_int(std::vector<uint8_t> &data, const uint32_t i, const uint32_t v) {
    data[i + 0] = (v >>  0);
    data[i + 1] = (v >>  8);
    data[i + 2] = (v >> 16);
    data[i + 3] = (v >> 24);
}

static std::vector<std::vector<uint8_t>> get_instruments(const Song &song) {
	std::vector<std::vector<uint8_t>> instruments;
	return std::move(instruments);
}

static std::vector<std::vector<uint8_t>> get_samples(const Song &song) {
	const uint32_t sample_filename_length = 12;
	const uint32_t sample_global_volume = 64;
	const uint32_t sample_flags = 0b00010001;
	const uint32_t sample_default_volume = 64;
	const uint32_t sample_name_length = 26;
	const uint32_t sample_default_panning = 32;
	const uint32_t sample_length = 64;
	const uint32_t sample_loop_begin = 0;
	const uint32_t sample_loop_end = 64;
	const uint32_t sample_speed = 67772;
	const uint32_t sample_sustain_loop_begin = 0;
	const uint32_t sample_sustain_loop_end = 0;
	const uint32_t sample_vibrato_speed = 0;
	const uint32_t sample_vibrato_depth = 0;
	const uint32_t sample_vibrato_rate = 0;
	const uint32_t sample_vibrato_waveform = 0;

	std::vector<std::vector<uint8_t>> samples;

	// one hard-coded square sample
	{
		std::vector<uint8_t> sample;

		// sample header, 80 bytes
		{
			sample.push_back('I');
			sample.push_back('M');
			sample.push_back('P');
			sample.push_back('S');

			for (uint32_t i = 0; i < sample_filename_length; ++i) {
				sample.push_back('\0');
			}

			sample.push_back(0); // unused
			sample.push_back(sample_global_volume);
			sample.push_back(sample_flags);
			sample.push_back(sample_default_volume);

			for (uint32_t i = 0; i < sample_name_length; ++i) {
				sample.push_back('\0');
			}

			sample.push_back(1); // ???
			sample.push_back(sample_default_panning);

			put_int(sample, sample_length);
			put_int(sample, sample_loop_begin);
			put_int(sample, sample_loop_end);
			put_int(sample, sample_speed);
			put_int(sample, sample_sustain_loop_begin);
			put_int(sample, sample_sustain_loop_end);
			put_int(sample, 0); // sample offset (index 72)

			sample.push_back(sample_vibrato_speed);
			sample.push_back(sample_vibrato_depth);
			sample.push_back(sample_vibrato_rate);
			sample.push_back(sample_vibrato_waveform);
		}

		// sample (50% square)
		for (uint32_t i = 0; i < sample_length / 2; ++i) {
			sample.push_back(127);
		}
		for (uint32_t i = 0; i < sample_length / 2; ++i) {
			sample.push_back(0);
		}

		samples.push_back(std::move(sample));
	}

	return std::move(samples);
}

static std::vector<std::vector<uint8_t>> get_patterns(const Song &song) {
	const uint32_t pattern_num_rows = 64;

	std::vector<std::vector<uint8_t>> patterns;

	{
		std::vector<uint8_t> pattern;

		const uint8_t example_pattern[] = {
			0x81, 0x03, 0x4C, 0x01, 0x82, 0x03, 0x4C, 0x01, 0x00, 0x81, 0x04, 0x00, 0x82, 0x04, 0x00, 0x00,
			0x81, 0x21, 0x47, 0x82, 0x21, 0x50, 0x00, 0x81, 0x40, 0x82, 0x40, 0x00, 0x81, 0x21, 0x49, 0x82,
			0x21, 0x4E, 0x00, 0x81, 0x40, 0x82, 0x40, 0x00, 0x81, 0x21, 0x47, 0x82, 0x21, 0x51, 0x00, 0x01,
			0x4C, 0x02, 0x50, 0x00, 0x81, 0x40, 0x82, 0x40, 0x00, 0x00, 0x00, 0x00, 0x81, 0x30, 0x00, 0x00,
			0x81, 0x40, 0x00, 0x00, 0x81, 0x21, 0x4B, 0x82, 0x21, 0x4B, 0x00, 0x81, 0x40, 0x82, 0x40, 0x00,
			0x81, 0x21, 0x47, 0x82, 0x21, 0x4E, 0x00, 0x81, 0x40, 0x82, 0x40, 0x00, 0x81, 0x21, 0x49, 0x82,
			0x21, 0x4C, 0x00, 0x81, 0x40, 0x82, 0x40, 0x00, 0x81, 0x21, 0x47, 0x82, 0x21, 0x50, 0x00, 0x01,
			0x4B, 0x02, 0x4E, 0x00, 0x81, 0x40, 0x82, 0x40, 0x00, 0x00, 0x00, 0x00, 0x82, 0x21, 0x47, 0x00,
			0x00, 0x82, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00
		};
		const uint32_t example_pattern_size = sizeof(example_pattern);

		put_short(pattern, example_pattern_size);
		put_short(pattern, pattern_num_rows);
		put_short(pattern, 0); // unused
		put_short(pattern, 0); // unused
		for (uint32_t i = 0; i < example_pattern_size; ++i) {
			pattern.push_back(example_pattern[i]);
		}

		patterns.push_back(std::move(pattern));
	}

	return std::move(patterns);
}

static std::size_t get_total_size(const std::vector<std::vector<uint8_t>> &data) {
	std::size_t size = 0;
	for (const auto &v : data) {
		size += v.size();
	}
	return size;
}

void IT_Module::generate_it_module(const Song &song) {
	const uint32_t song_name_length = 26;
	const uint32_t pattern_row_highlight = 0x1004; // ???
	const uint32_t tracker_version = 0x5130;
	const uint32_t compatible_version = 0x0214;
	const uint32_t flags = 0b01001001;
	const uint32_t special = 0b00000110;

	const uint32_t global_volume = 128;
	const uint32_t mix_volume = 48;
	const uint32_t initial_speed = 6;
	const uint32_t initial_tempo = 125;
	const uint32_t panning_separation = 128;
	const uint32_t pitch_wheel_depth = 0;
	const uint32_t default_channel_panning = 32;
	const uint32_t channel_disabled = 0b10000000;
	const uint32_t max_num_channels = 64;
	const uint32_t default_channel_volume = 64;

	const uint32_t sample_header_size = 80;

	std::vector<std::vector<uint8_t>> instruments = get_instruments(song);
	std::vector<std::vector<uint8_t>> samples = get_samples(song);
	std::vector<std::vector<uint8_t>> patterns = get_patterns(song);

	const uint32_t number_of_orders = patterns.size() + 1;
	const uint32_t number_of_instruments = instruments.size();
	const uint32_t number_of_samples = samples.size();
	const uint32_t number_of_patterns = patterns.size();

	const uint32_t header_size = 192;
	const uint32_t instruments_start = header_size +
		number_of_orders +
		number_of_instruments * 4 +
		number_of_samples * 4 +
		number_of_patterns * 4;
	const uint32_t samples_start = instruments_start + get_total_size(instruments);
	const uint32_t patterns_start = samples_start + get_total_size(samples);
	const uint32_t total_size = patterns_start + get_total_size(patterns);

	_data.reserve(total_size);

	// header, 192 bytes
	{
		_data.push_back('I');
		_data.push_back('M');
		_data.push_back('P');
		_data.push_back('M');

		for (uint32_t i = 0; i < song_name_length; ++i) {
			_data.push_back('\0');
		}

		put_short(_data, pattern_row_highlight);
		put_short(_data, number_of_orders);
		put_short(_data, number_of_instruments);
		put_short(_data, number_of_samples);
		put_short(_data, number_of_patterns);
		put_short(_data, tracker_version);
		put_short(_data, compatible_version);
		put_short(_data, flags);
		put_short(_data, special);

		_data.push_back(global_volume);
		_data.push_back(mix_volume);
		_data.push_back(initial_speed);
		_data.push_back(initial_tempo);
		_data.push_back(panning_separation);
		_data.push_back(pitch_wheel_depth);

		put_short(_data, 0); // message length
		put_int(_data, 0); // message offset
		put_int(_data, 0); // reserved

		_data.push_back(default_channel_panning);
		_data.push_back(default_channel_panning);
		_data.push_back(default_channel_panning);
		_data.push_back(default_channel_panning);
		for (uint32_t i = 0; i < max_num_channels - 4; ++i) {
			_data.push_back(default_channel_panning | channel_disabled);
		}
		for (uint32_t i = 0; i < max_num_channels; ++i) {
			_data.push_back(default_channel_volume);
		}
	}

	// orders
	for (uint32_t i = 0; i < number_of_orders - 1; ++i) {
		_data.push_back(i);
	}
	_data.push_back(0xff);

	// instrument offsets
	uint32_t instrument_offset = instruments_start;
	for (const auto &instrument : instruments) {
		put_int(_data, instrument_offset);
		instrument_offset += instrument.size();
	}
	// sample offsets
	uint32_t sample_offset = samples_start;
	for (auto &sample : samples) {
		put_int(_data, sample_offset);

		// fix inner sample offset
		patch_int(sample, 72, sample_offset + sample_header_size);

		sample_offset += sample.size();
	}
	// pattern offsets
	uint32_t pattern_offset = patterns_start;
	for (const auto &pattern : patterns) {
		put_int(_data, pattern_offset);
		pattern_offset += pattern.size();
	}

	// instruments
	for (const auto &instrument : instruments) {
		_data.insert(_data.end(), instrument.begin(), instrument.end());
	}
	// samples
	for (const auto &sample : samples) {
		_data.insert(_data.end(), sample.begin(), sample.end());
	}
	// patterns
	for (const auto &pattern : patterns) {
		_data.insert(_data.end(), pattern.begin(), pattern.end());
	}
}
