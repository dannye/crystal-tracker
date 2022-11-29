#include "it-module.h"

#include <cmath>

constexpr std::size_t BUFFER_SIZE = 2048;
constexpr std::int32_t SAMPLE_RATE = 48000;

IT_Module::IT_Module(const Piano_Roll &song, const std::vector<Wave> &waves, int32_t loop_tick) : _buffer(BUFFER_SIZE * 2) {
	generate_it_module(song, waves, loop_tick);

	_mod = new openmpt::module_ext(_data);
	if (loop_tick != -1) {
		_mod->set_repeat_count(-1);
	}

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
	if (!ready() || !playing()) return;

	std::size_t count = _is_interleaved ?
		_mod->read_interleaved_stereo(SAMPLE_RATE, BUFFER_SIZE, _buffer.data()) :
		_mod->read(SAMPLE_RATE, BUFFER_SIZE, _buffer.data(), _buffer.data() + BUFFER_SIZE);
	_current_pattern = _mod->get_current_pattern();
	_current_row = _mod->get_current_row();

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

void IT_Module::mute_channel(int32_t channel, bool mute) {
	if (channel - 1 < _mod->get_num_channels()) {
		openmpt::ext::interactive *interactive = static_cast<openmpt::ext::interactive *>(_mod->get_interface(openmpt::ext::interactive_id));
		interactive->set_channel_mute_status(channel - 1, mute);
	}
}

void IT_Module::set_tick(int32_t tick) {
	if (!ready()) return;

	_mod->set_position_order_row((tick / 2) / ROWS_PER_PATTERN, (tick / 2) % ROWS_PER_PATTERN);
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

static std::vector<std::vector<uint8_t>> get_instruments() {
	std::vector<std::vector<uint8_t>> instruments;
	return instruments;
}

static std::vector<std::vector<uint8_t>> get_samples(const std::vector<Wave> &waves) {
	const uint32_t sample_filename_length = 12;
	const uint32_t sample_global_volume = 64;
	const uint32_t sample_flags = 0b00010001;
	const uint32_t sample_default_volume = 64;
	const uint32_t sample_name_length = 26;
	const uint32_t sample_default_panning = 32;
	const uint32_t sample_length = 64;
	const uint32_t sample_loop_begin = 0;
	const uint32_t sample_loop_end = 64;
	const uint32_t sample_speed = 33886;
	const uint32_t sample_sustain_loop_begin = 0;
	const uint32_t sample_sustain_loop_end = 0;
	const uint32_t sample_vibrato_speed = 0;
	const uint32_t sample_vibrato_depth = 0;
	const uint32_t sample_vibrato_rate = 0;
	const uint32_t sample_vibrato_waveform = 0;

	std::vector<std::vector<uint8_t>> samples;

	// sample header, 80 bytes
	auto sample_header = [&](std::vector<uint8_t> &sample) {
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
	};

	// four hard-coded square samples (duty cycles)
	{
		std::vector<uint8_t> sample;

		sample_header(sample);

		// sample (12.5% square)
		for (uint32_t i = 0; i < sample_length / 2 * 1/8; ++i) {
			sample.push_back(0);
		}
		for (uint32_t i = 0; i < sample_length / 2 * 7/8; ++i) {
			sample.push_back(127);
		}
		for (uint32_t i = 0; i < sample_length / 2 * 1/8; ++i) {
			sample.push_back(0);
		}
		for (uint32_t i = 0; i < sample_length / 2 * 7/8; ++i) {
			sample.push_back(127);
		}

		samples.push_back(std::move(sample));
	}
	{
		std::vector<uint8_t> sample;

		sample_header(sample);

		// sample (25% square)
		for (uint32_t i = 0; i < sample_length / 2 * 1/4; ++i) {
			sample.push_back(0);
		}
		for (uint32_t i = 0; i < sample_length / 2 * 3/4; ++i) {
			sample.push_back(127);
		}
		for (uint32_t i = 0; i < sample_length / 2 * 1/4; ++i) {
			sample.push_back(0);
		}
		for (uint32_t i = 0; i < sample_length / 2 * 3/4; ++i) {
			sample.push_back(127);
		}

		samples.push_back(std::move(sample));
	}
	{
		std::vector<uint8_t> sample;

		sample_header(sample);

		// sample (50% square)
		for (uint32_t i = 0; i < sample_length / 2 * 1/2; ++i) {
			sample.push_back(0);
		}
		for (uint32_t i = 0; i < sample_length / 2 * 1/2; ++i) {
			sample.push_back(127);
		}
		for (uint32_t i = 0; i < sample_length / 2 * 1/2; ++i) {
			sample.push_back(0);
		}
		for (uint32_t i = 0; i < sample_length / 2 * 1/2; ++i) {
			sample.push_back(127);
		}

		samples.push_back(std::move(sample));
	}
	{
		std::vector<uint8_t> sample;

		sample_header(sample);

		// sample (75% square)
		for (uint32_t i = 0; i < sample_length / 2 * 3/4; ++i) {
			sample.push_back(0);
		}
		for (uint32_t i = 0; i < sample_length / 2 * 1/4; ++i) {
			sample.push_back(127);
		}
		for (uint32_t i = 0; i < sample_length / 2 * 3/4; ++i) {
			sample.push_back(0);
		}
		for (uint32_t i = 0; i < sample_length / 2 * 1/4; ++i) {
			sample.push_back(127);
		}

		samples.push_back(std::move(sample));
	}
	// dynamic wave samples
	for (const Wave &wave : waves) {
		std::vector<uint8_t> sample;

		sample_header(sample);

		for (uint32_t i = 0; i < NUM_WAVE_SAMPLES; ++i) {
			sample.push_back(wave[i] * 8);
			sample.push_back(wave[i] * 8);
		}

		samples.push_back(std::move(sample));
	}

	return samples;
}

static std::vector<std::vector<uint8_t>> get_patterns(const Piano_Roll &song, int32_t loop_tick) {
	std::vector<std::vector<uint8_t>> patterns;

	auto channel_1_itr = song.channel_1_notes().begin();
	auto channel_2_itr = song.channel_2_notes().begin();
	auto channel_3_itr = song.channel_3_notes().begin();
	auto channel_4_itr = song.channel_4_notes().begin();
	int32_t channel_1_note_length = 0;
	int32_t channel_2_note_length = 0;
	int32_t channel_3_note_length = 0;
	int32_t channel_4_note_length = 0;
	int32_t channel_1_note_duration = 0;
	int32_t channel_2_note_duration = 0;
	int32_t channel_3_note_duration = 0;

	Note_View channel_1_prev_note;
	Note_View channel_2_prev_note;
	Note_View channel_3_prev_note;
	Note_View channel_4_prev_note;

	auto song_finished = [&]() {
		return
			channel_1_itr == song.channel_1_notes().end() &&
			channel_2_itr == song.channel_2_notes().end() &&
			channel_3_itr == song.channel_3_notes().end() &&
			channel_4_itr == song.channel_4_notes().end() &&
			channel_1_note_length == 0 &&
			channel_2_note_length == 0 &&
			channel_3_note_length == 0 &&
			channel_4_note_length == 0;
	};

	auto channel_3_volume = [](int32_t volume) {
		return (uint8_t)(
			volume == 1 ? 64 :
			volume == 2 ? 32 :
			volume == 3 ? 16 :
			0
		);
	};

	auto convert_tempo = [](int32_t tempo) {
		// this is a purely empirical approximation.
		// i have no idea what the real formula is.
		int32_t bpm = (int32_t)(std::log2(tempo - 75.0) * -75.0 / 2.0 + 360.0);
		if (bpm < 32)  bpm = 32;
		if (bpm > 255) bpm = 255;
		return (uint8_t)bpm;
	};

	do {
		std::vector<uint8_t> pattern;

		std::vector<uint8_t> pattern_data;
		uint32_t row = 0;
		while (row < ROWS_PER_PATTERN && !song_finished()) {
			// NOTE: only even speeds are allowed for now to avoid complex time dilation
			if (channel_1_note_length == 0 && channel_1_itr != song.channel_1_notes().end()) {
				channel_1_note_length = channel_1_itr->length * channel_1_itr->speed / 2 - 1;
				channel_1_note_duration = 0;
				if (channel_1_itr->tempo != channel_1_prev_note.tempo) {
					pattern_data.push_back(0x85);
					pattern_data.push_back(0x08); // command
					pattern_data.push_back(0x14); // tempo
					pattern_data.push_back(convert_tempo(channel_1_itr->tempo));
				}
				if (channel_1_itr->pitch != Pitch::REST) {
					pattern_data.push_back(0x81);
					pattern_data.push_back(0x07); // note + sample + volume
					pattern_data.push_back(channel_1_itr->octave * 12 + (uint32_t)channel_1_itr->pitch - 1);
					pattern_data.push_back(channel_1_itr->duty + 1); // sample
					pattern_data.push_back((channel_1_itr->volume + 1) * 4); // volume
				}
				else {
					pattern_data.push_back(0x81);
					pattern_data.push_back(0x01); // note
					pattern_data.push_back(0xfe); // cut
				}
				channel_1_prev_note = *channel_1_itr;
				++channel_1_itr;
			}
			else if (channel_1_note_length > 0) {
				if (channel_1_prev_note.fade && row % 4 == 1) {
					pattern_data.push_back(0x81);
					pattern_data.push_back(0x08); // command
					if (
						channel_1_prev_note.rate &&
						channel_1_prev_note.extent &&
						channel_1_note_duration > channel_1_prev_note.delay
					) {
						pattern_data.push_back(0x0b); // fade+vibrato
					}
					else {
						pattern_data.push_back(0x04); // fade
					}
					if (channel_1_prev_note.fade > 0) {
						pattern_data.push_back(0xf0 | (10 - channel_1_prev_note.fade));
					}
					else {
						pattern_data.push_back(((10 + channel_1_prev_note.fade) << 4) | 0x0f);
					}
				}
				else if (
					channel_1_prev_note.rate &&
					channel_1_prev_note.extent &&
					channel_1_note_duration >= channel_1_prev_note.delay
				) {
					pattern_data.push_back(0x81);
					pattern_data.push_back(0x08); // command
					pattern_data.push_back(0x08); // vibrato
					pattern_data.push_back((16 - channel_1_prev_note.rate * 2) << 4 | (channel_1_prev_note.extent));
				}
				channel_1_note_length -= 1;
				channel_1_note_duration += 1;
			}

			if (channel_2_note_length == 0 && channel_2_itr != song.channel_2_notes().end()) {
				channel_2_note_length = channel_2_itr->length * channel_2_itr->speed / 2 - 1;
				channel_2_note_duration = 0;
				if (channel_2_itr->tempo != channel_2_prev_note.tempo) {
					pattern_data.push_back(0x85);
					pattern_data.push_back(0x08); // command
					pattern_data.push_back(0x14); // tempo
					pattern_data.push_back(convert_tempo(channel_2_itr->tempo));
				}
				if (channel_2_itr->pitch != Pitch::REST) {
					pattern_data.push_back(0x82);
					pattern_data.push_back(0x07); // note + sample + volume
					pattern_data.push_back(channel_2_itr->octave * 12 + (uint32_t)channel_2_itr->pitch - 1);
					pattern_data.push_back(channel_2_itr->duty + 1); // sample
					pattern_data.push_back((channel_2_itr->volume + 1) * 4); // volume
				}
				else {
					pattern_data.push_back(0x82);
					pattern_data.push_back(0x01); // note
					pattern_data.push_back(0xfe); // cut
				}
				channel_2_prev_note = *channel_2_itr;
				++channel_2_itr;
			}
			else if (channel_2_note_length > 0) {
				if (channel_2_prev_note.fade && row % 4 == 1) {
					pattern_data.push_back(0x82);
					pattern_data.push_back(0x08); // command
					if (
						channel_2_prev_note.rate &&
						channel_2_prev_note.extent &&
						channel_2_note_duration > channel_2_prev_note.delay
					) {
						pattern_data.push_back(0x0b); // fade+vibrato
					}
					else {
						pattern_data.push_back(0x04); // fade
					}
					if (channel_2_prev_note.fade > 0) {
						pattern_data.push_back(0xf0 | (10 - channel_2_prev_note.fade));
					}
					else {
						pattern_data.push_back(((10 + channel_2_prev_note.fade) << 4) | 0x0f);
					}
				}
				else if (
					channel_2_prev_note.rate &&
					channel_2_prev_note.extent &&
					channel_2_note_duration >= channel_2_prev_note.delay
				) {
					pattern_data.push_back(0x82);
					pattern_data.push_back(0x08); // command
					pattern_data.push_back(0x08); // vibrato
					pattern_data.push_back((16 - channel_2_prev_note.rate * 2) << 4 | (channel_2_prev_note.extent));
				}
				channel_2_note_length -= 1;
				channel_2_note_duration += 1;
			}

			if (channel_3_note_length == 0 && channel_3_itr != song.channel_3_notes().end()) {
				channel_3_note_length = channel_3_itr->length * channel_3_itr->speed / 2 - 1;
				channel_3_note_duration = 0;
				if (channel_3_itr->tempo != channel_3_prev_note.tempo) {
					pattern_data.push_back(0x85);
					pattern_data.push_back(0x08); // command
					pattern_data.push_back(0x14); // tempo
					pattern_data.push_back(convert_tempo(channel_3_itr->tempo));
				}
				if (channel_3_itr->pitch != Pitch::REST) {
					pattern_data.push_back(0x83);
					pattern_data.push_back(0x07); // note + sample + volume
					pattern_data.push_back(channel_3_itr->octave * 12 + (uint32_t)channel_3_itr->pitch - 1);
					pattern_data.push_back(channel_3_itr->wave + 5); // sample
					pattern_data.push_back(channel_3_volume(channel_3_itr->volume)); // volume
				}
				else {
					pattern_data.push_back(0x83);
					pattern_data.push_back(0x01); // note
					pattern_data.push_back(0xfe); // cut
				}
				channel_3_prev_note = *channel_3_itr;
				++channel_3_itr;
			}
			else if (channel_3_note_length > 0) {
				if (
					channel_3_prev_note.rate &&
					channel_3_prev_note.extent &&
					channel_3_note_duration >= channel_3_prev_note.delay
				) {
					pattern_data.push_back(0x83);
					pattern_data.push_back(0x08); // command
					pattern_data.push_back(0x08); // vibrato
					pattern_data.push_back((16 - channel_3_prev_note.rate * 2) << 4 | (channel_3_prev_note.extent));
				}
				channel_3_note_length -= 1;
				channel_3_note_duration += 1;
			}

			if (channel_4_note_length == 0 && channel_4_itr != song.channel_4_notes().end()) {
				channel_4_note_length = channel_4_itr->length * channel_4_itr->speed / 2 - 1;
				if (channel_4_itr->tempo != channel_4_prev_note.tempo) {
					pattern_data.push_back(0x85);
					pattern_data.push_back(0x08); // command
					pattern_data.push_back(0x14); // tempo
					pattern_data.push_back(convert_tempo(channel_4_itr->tempo));
				}
				channel_4_prev_note = *channel_4_itr;
				++channel_4_itr;
			}
			else if (channel_4_note_length > 0) {
				channel_4_note_length -= 1;
			}

			if (song_finished() && loop_tick != -1) {
				uint32_t pattern_number = (uint32_t)(loop_tick / 2) / ROWS_PER_PATTERN;
				uint32_t row_number = (uint32_t)(loop_tick / 2) % ROWS_PER_PATTERN;

				pattern_data.push_back(0x86);
				pattern_data.push_back(0x08); // command
				pattern_data.push_back(0x02); // pattern jump
				pattern_data.push_back(pattern_number);

				pattern_data.push_back(0x87);
				pattern_data.push_back(0x08); // command
				pattern_data.push_back(0x03); // row jump
				pattern_data.push_back(row_number);
			}

			pattern_data.push_back(0);
			row += 1;
		}

		put_short(pattern, pattern_data.size());
		put_short(pattern, row);
		put_short(pattern, 0); // unused
		put_short(pattern, 0); // unused
		pattern.insert(pattern.end(), pattern_data.begin(), pattern_data.end());

		patterns.push_back(std::move(pattern));
	} while (!song_finished());

	return patterns;
}

static std::size_t get_total_size(const std::vector<std::vector<uint8_t>> &data) {
	std::size_t size = 0;
	for (const auto &v : data) {
		size += v.size();
	}
	return size;
}

void IT_Module::generate_it_module(const Piano_Roll &song, const std::vector<Wave> &waves, int32_t loop_tick) {
	const uint32_t song_name_length = 26;
	const uint32_t pattern_row_highlight = 0x1004; // ???
	const uint32_t tracker_version = 0x5130;
	const uint32_t compatible_version = 0x0214;
	const uint32_t flags = 0b01001001;
	const uint32_t special = 0b00000110;

	const uint32_t global_volume = 128;
	const uint32_t mix_volume = 48;
	const uint32_t initial_speed = 1;
	const uint32_t initial_tempo = 128;
	const uint32_t panning_separation = 128;
	const uint32_t pitch_wheel_depth = 0;
	const uint32_t default_channel_panning = 32;
	const uint32_t channel_disabled = 0b10000000;
	const uint32_t max_num_channels = 64;
	const uint32_t default_channel_volume = 64;

	const uint32_t sample_header_size = 80;

	std::vector<std::vector<uint8_t>> instruments = get_instruments();
	std::vector<std::vector<uint8_t>> samples = get_samples(waves);
	std::vector<std::vector<uint8_t>> patterns = get_patterns(song, loop_tick);

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
