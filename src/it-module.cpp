#include <cmath>

#include "it-module.h"

#include "utils.h"

IT_Module::IT_Module() {
	generate_it_module();

	_mod = new openmpt::module_ext(_data);
	_mod->set_repeat_count(-1);

	_is_interleaved = false;
	if (!try_open()) {
		_is_interleaved = true;
		try_open();
	}
}

IT_Module::IT_Module(
	const std::vector<Note_View> &channel_1_notes,
	const std::vector<Note_View> &channel_2_notes,
	const std::vector<Note_View> &channel_3_notes,
	const std::vector<Note_View> &channel_4_notes,
	const std::vector<Wave> &waves,
	const std::vector<Drumkit> &drumkits,
	const std::vector<std::vector<uint8_t>> &drums,
	int32_t loop_tick,
	bool stereo
) {
	generate_it_module(channel_1_notes, channel_2_notes, channel_3_notes, channel_4_notes, waves, drumkits, drums, loop_tick, stereo);

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

bool IT_Module::export_file(const char *f) {
	std::ofstream ofs;
	open_ofstream(ofs, f);
	if (!ofs.good()) return false;

	ofs.write((char *)&_data[0], _data.size());
	ofs.close();

	return true;
}

bool IT_Module::play() {
	if (!ready() || !playing()) return true;

	std::size_t count = _is_interleaved ?
		_mod->read_interleaved_stereo(SAMPLE_RATE, BUFFER_SIZE, _buffer.data()) :
		_mod->read(SAMPLE_RATE, BUFFER_SIZE, _buffer.data(), _buffer.data() + BUFFER_SIZE);
	_current_pattern = _mod->get_current_pattern();
	_current_row = _mod->get_current_row();

	if (count == 0) {
		stop();
		return true;
	}
	try {
		if (_is_interleaved) {
			_stream.write(_buffer.data(), static_cast<unsigned long>(count));
		}
		else {
			const float * const buffers[2] = { _buffer.data(), _buffer.data() + BUFFER_SIZE };
			_stream.write(buffers, static_cast<unsigned long>(count));
		}
		return true;
	}
	catch (...) {}
	return false;
}

void IT_Module::mute_channel(int32_t channel, bool mute) {
	if (channel - 1 < _mod->get_num_channels()) {
		openmpt::ext::interactive *interactive = static_cast<openmpt::ext::interactive *>(_mod->get_interface(openmpt::ext::interactive_id));
		interactive->set_channel_mute_status(channel - 1, mute);
	}
}

int32_t IT_Module::play_note(Pitch pitch, int32_t octave) {
	openmpt::ext::interactive *interactive = static_cast<openmpt::ext::interactive *>(_mod->get_interface(openmpt::ext::interactive_id));
	return interactive->play_note(2, octave * NUM_PITCHES + (int32_t)pitch - 1, 1.0, 0.0);
}

void IT_Module::stop_note(int32_t channel) {
	openmpt::ext::interactive *interactive = static_cast<openmpt::ext::interactive *>(_mod->get_interface(openmpt::ext::interactive_id));
	interactive->stop_note(channel);
}

void IT_Module::set_tick(int32_t tick) {
	_mod->set_position_order_row(tick / ROWS_PER_PATTERN, tick % ROWS_PER_PATTERN);
}

double IT_Module::get_position_seconds() {
	return _mod->get_position_seconds();
}

double IT_Module::get_duration_seconds() {
	return _mod->get_duration_seconds();
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

std::vector<std::vector<uint8_t>> IT_Module::get_instruments() {
	std::vector<std::vector<uint8_t>> instruments;
	return instruments;
}

std::vector<std::vector<uint8_t>> IT_Module::get_samples(const std::vector<Wave> &waves, const std::vector<std::vector<uint8_t>> &drums) {
	const uint32_t sample_filename_length = 12;
	const uint32_t sample_global_volume = 64;
	const uint32_t sample_wave_flags = 0b00010001;
	const uint32_t sample_noise_flags = 0b00000001;
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
	auto sample_header = [&](std::vector<uint8_t> &sample, uint32_t sample_size, bool noise = false) {
		sample.push_back('I');
		sample.push_back('M');
		sample.push_back('P');
		sample.push_back('S');

		for (uint32_t i = 0; i < sample_filename_length; ++i) {
			sample.push_back('\0');
		}

		sample.push_back(0); // unused
		sample.push_back(sample_global_volume);
		sample.push_back(noise ? sample_noise_flags : sample_wave_flags);
		sample.push_back(sample_default_volume);

		for (uint32_t i = 0; i < sample_name_length; ++i) {
			sample.push_back('\0');
		}

		sample.push_back(1); // ???
		sample.push_back(sample_default_panning);

		put_int(sample, sample_size);
		put_int(sample, sample_loop_begin);
		put_int(sample, noise ? 0 : sample_loop_end);
		put_int(sample, noise ? sample_speed * NOISE_SAMPLE_SPEED_FACTOR : sample_speed);
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

		sample_header(sample, sample_length);

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

		sample_header(sample, sample_length);

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

		sample_header(sample, sample_length);

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

		sample_header(sample, sample_length);

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

		sample_header(sample, sample_length);

		for (uint32_t i = 0; i < NUM_WAVE_SAMPLES; ++i) {
			sample.push_back(wave[i] * 255 / 15 / 2);
			sample.push_back(wave[i] * 255 / 15 / 2);
		}

		samples.push_back(std::move(sample));
	}
	for (const std::vector<uint8_t> &drum : drums) {
		std::vector<uint8_t> sample;

		sample_header(sample, (uint32_t)drum.size(), true);

		for (uint32_t i = 0; i < drum.size(); ++i) {
			sample.push_back(drum[i]);
		}

		samples.push_back(std::move(sample));
	}

	return samples;
}

std::vector<std::vector<uint8_t>> IT_Module::get_patterns(
	const std::vector<Note_View> &channel_1_notes,
	const std::vector<Note_View> &channel_2_notes,
	const std::vector<Note_View> &channel_3_notes,
	const std::vector<Note_View> &channel_4_notes,
	const std::vector<Drumkit> &drumkits,
	int32_t loop_tick,
	bool stereo,
	int32_t num_inline_waves
) {
	const uint8_t CHANNEL = 0x80;
	const uint8_t CH1 = 1;
	const uint8_t CH2 = 2;
	const uint8_t CH3 = 3;
	const uint8_t CH4 = 4;
	const uint8_t CH5 = 5;
	const uint8_t CH6 = 6;
	const uint8_t CH7 = 7;
	const uint8_t CH8 = 8;
	const uint8_t CH9 = 9;
	const uint8_t CH10 = 10;

	const uint8_t NOTE    = 1;
	const uint8_t SAMPLE  = 2;
	const uint8_t VOLUME  = 4;
	const uint8_t COMMAND = 8;

	const uint8_t PATTERN_JUMP     = 0x02;
	const uint8_t ROW_JUMP         = 0x03;
	const uint8_t FADE             = 0x04;
	const uint8_t PITCH_SLIDE      = 0x07;
	const uint8_t VIBRATO          = 0x08;
	const uint8_t FADE_VIBRATO     = 0x0b;
	const uint8_t FADE_PITCH_SLIDE = 0x0c;
	const uint8_t TEMPO            = 0x14;
	const uint8_t STEREO_PANNING   = 0x18;
	const uint8_t EXTENSION        = 0x1b;

	const uint8_t CUT = 0xfe;

	const uint8_t FINELY = 0xf;

	std::vector<std::vector<uint8_t>> patterns;

	auto channel_1_itr = channel_1_notes.begin();
	auto channel_2_itr = channel_2_notes.begin();
	auto channel_3_itr = channel_3_notes.begin();
	auto channel_4_itr = channel_4_notes.begin();
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

	int32_t global_tempo    = 256;
	int32_t channel_1_tempo = 0;
	int32_t channel_2_tempo = 0;
	int32_t channel_3_tempo = 0;
	int32_t channel_4_tempo = 0;

	int32_t wave = 0;

	int32_t first_channel =
		channel_1_notes.size() > 0 ? 1 :
		channel_2_notes.size() > 0 ? 2 :
		channel_3_notes.size() > 0 ? 3 :
		4;

	auto song_finished = [&]() {
		return
			channel_1_itr == channel_1_notes.end() &&
			channel_2_itr == channel_2_notes.end() &&
			channel_3_itr == channel_3_notes.end() &&
			channel_4_itr == channel_4_notes.end() &&
			channel_1_note_length == 0 &&
			channel_2_note_length == 0 &&
			channel_3_note_length == 0 &&
			channel_4_note_length == 0;
	};

	auto note = [](const Note_View &view) {
		return (uint8_t)(
			((view.octave - view.transpose_octaves + view.transpose_pitches / 12) * 12 +
			((uint32_t)view.pitch - 1 + (uint32_t)view.transpose_pitches % 12)) % (9 * 12)
		);
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
		int32_t bpm = (int32_t)(2.0f * UNITS_PER_MINUTE / 48.0f / tempo + 0.5f);
		return std::max(bpm, 32);
	};

	auto convert_fade_period = [](int32_t tempo, int32_t fade) {
		return std::abs(fade) + std::max(192 - tempo, 0) / 32;
	};

	auto convert_vibrato_delay = [](int32_t tempo, int32_t speed, int32_t delay) {
		return (int32_t)(delay * speed / std::pow(tempo, 0.35));
	};

	auto convert_vibrato_rate = [](int32_t tempo, int32_t rate) {
		const int32_t v = 6 + tempo / 60;
		return std::min(v - std::min(rate, v - 1), 15);
	};

	auto get_stereo_panning = [](bool left, bool right) {
		if (left && right)  return 0x80;
		if (left && !right) return 0x00;
		if (!left && right) return 0xFF;
		return 0x80;
	};

	do {
		std::vector<uint8_t> pattern;

		std::vector<uint8_t> pattern_data;
		uint32_t row = 0;
		do {
			if (channel_1_tempo != 0) {
				pattern_data.push_back(CHANNEL + CH5);
				pattern_data.push_back(COMMAND);
				pattern_data.push_back(EXTENSION);
				pattern_data.push_back(channel_1_tempo % 256);
				channel_1_tempo = 0;
			}
			if (channel_2_tempo != 0) {
				pattern_data.push_back(CHANNEL + CH6);
				pattern_data.push_back(COMMAND);
				pattern_data.push_back(EXTENSION);
				pattern_data.push_back(channel_2_tempo % 256);
				channel_2_tempo = 0;
			}
			if (channel_3_tempo != 0) {
				pattern_data.push_back(CHANNEL + CH7);
				pattern_data.push_back(COMMAND);
				pattern_data.push_back(EXTENSION);
				pattern_data.push_back(channel_3_tempo % 256);
				channel_3_tempo = 0;
			}
			if (channel_4_tempo != 0) {
				pattern_data.push_back(CHANNEL + CH8);
				pattern_data.push_back(COMMAND);
				pattern_data.push_back(EXTENSION);
				pattern_data.push_back(channel_4_tempo % 256);
				channel_4_tempo = 0;
			}
			if (channel_1_note_length == 0 && channel_1_itr != channel_1_notes.end()) {
				channel_1_note_length = channel_1_itr->length * channel_1_itr->speed - 1;
				channel_1_note_duration = 0;
				if (channel_1_itr->tempo != channel_1_prev_note.tempo) {
					global_tempo = channel_1_itr->tempo;
					channel_1_tempo = convert_tempo(channel_1_itr->tempo);
					pattern_data.push_back(CHANNEL + CH5);
					pattern_data.push_back(COMMAND);
					pattern_data.push_back(TEMPO);
					pattern_data.push_back(channel_1_tempo / 256);

					if (_tempo_change_mid_note == -1) {
						if (channel_2_note_length > 0) {
							_tempo_change_mid_note = 2;
						}
						else if (channel_3_note_length > 0) {
							_tempo_change_mid_note = 3;
						}
						else if (channel_4_note_length > 0) {
							_tempo_change_mid_note = 4;
						}
					}
				}
				if (channel_1_itr->pitch != Pitch::REST) {
					pattern_data.push_back(CHANNEL + CH1);
					pattern_data.push_back(NOTE + SAMPLE + VOLUME);
					pattern_data.push_back(note(*channel_1_itr)); // note
					pattern_data.push_back(channel_1_itr->duty + 1); // sample
					pattern_data.push_back((channel_1_itr->volume + 1) * 4); // volume
				}
				else {
					pattern_data.push_back(CHANNEL + CH1);
					pattern_data.push_back(NOTE);
					pattern_data.push_back(CUT);
				}
				if (
					stereo &&
					((channel_1_itr->panning_left != channel_1_prev_note.panning_left) ||
					(channel_1_itr->panning_right != channel_1_prev_note.panning_right))
				) {
					pattern_data.push_back(CHANNEL + CH1);
					pattern_data.push_back(COMMAND);
					pattern_data.push_back(STEREO_PANNING);
					pattern_data.push_back(get_stereo_panning(channel_1_itr->panning_left, channel_1_itr->panning_right));
				}
				channel_1_prev_note = *channel_1_itr;
				++channel_1_itr;
			}
			else if (channel_1_note_length > 0) {
				uint32_t volume_fade_period = convert_fade_period(global_tempo, channel_1_prev_note.fade);
				if (channel_1_prev_note.fade && row % volume_fade_period == 0) {
					pattern_data.push_back(CHANNEL + CH1);
					pattern_data.push_back(COMMAND);
					if (channel_1_prev_note.slide_pitch != Pitch::REST) {
						pattern_data.push_back(FADE_PITCH_SLIDE);
					}
					else if (
						channel_1_prev_note.vibrato_rate &&
						channel_1_prev_note.vibrato_extent &&
						channel_1_note_duration > convert_vibrato_delay(global_tempo, channel_1_prev_note.speed, channel_1_prev_note.vibrato_delay)
					) {
						pattern_data.push_back(FADE_VIBRATO);
					}
					else {
						pattern_data.push_back(FADE);
					}
					if (channel_1_prev_note.fade > 0) {
						pattern_data.push_back((FINELY << 4) | 4);
					}
					else {
						pattern_data.push_back((4 << 4) | FINELY);
					}
				}
				else if (channel_1_prev_note.slide_pitch != Pitch::REST) {
					pattern_data.push_back(CHANNEL + CH1);
					pattern_data.push_back(NOTE + COMMAND);
					pattern_data.push_back(channel_1_prev_note.slide_octave * 12 + (uint32_t)channel_1_prev_note.slide_pitch - 1);
					pattern_data.push_back(PITCH_SLIDE);
					pattern_data.push_back(channel_1_prev_note.slide_duration * 6);
				}
				else if (
					channel_1_prev_note.vibrato_rate &&
					channel_1_prev_note.vibrato_extent &&
					channel_1_note_duration >= convert_vibrato_delay(global_tempo, channel_1_prev_note.speed, channel_1_prev_note.vibrato_delay)
				) {
					pattern_data.push_back(CHANNEL + CH1);
					pattern_data.push_back(COMMAND);
					pattern_data.push_back(VIBRATO);
					pattern_data.push_back(convert_vibrato_rate(global_tempo, channel_1_prev_note.vibrato_rate) << 4 | (channel_1_prev_note.vibrato_extent));
				}
				channel_1_note_length -= 1;
				channel_1_note_duration += 1;
			}
			else {
				pattern_data.push_back(CHANNEL + CH1);
				pattern_data.push_back(NOTE);
				pattern_data.push_back(CUT);
			}

			if (channel_2_note_length == 0 && channel_2_itr != channel_2_notes.end()) {
				channel_2_note_length = channel_2_itr->length * channel_2_itr->speed - 1;
				channel_2_note_duration = 0;
				if (channel_2_itr->tempo != channel_2_prev_note.tempo) {
					global_tempo = channel_2_itr->tempo;
					channel_2_tempo = convert_tempo(channel_2_itr->tempo);
					pattern_data.push_back(CHANNEL + CH6);
					pattern_data.push_back(COMMAND);
					pattern_data.push_back(TEMPO);
					pattern_data.push_back(channel_2_tempo / 256);

					if (first_channel != 2) {
						_tempo_change_wrong_channel = 2;
					}
					if (_tempo_change_mid_note == -1) {
						if (channel_1_note_length > 0) {
							_tempo_change_mid_note = 1;
						}
						else if (channel_3_note_length > 0) {
							_tempo_change_mid_note = 3;
						}
						else if (channel_4_note_length > 0) {
							_tempo_change_mid_note = 4;
						}
					}
				}
				if (channel_2_itr->pitch != Pitch::REST) {
					pattern_data.push_back(CHANNEL + CH2);
					pattern_data.push_back(NOTE + SAMPLE + VOLUME);
					pattern_data.push_back(note(*channel_2_itr)); // note
					pattern_data.push_back(channel_2_itr->duty + 1); // sample
					pattern_data.push_back((channel_2_itr->volume + 1) * 4); // volume
				}
				else {
					pattern_data.push_back(CHANNEL + CH2);
					pattern_data.push_back(NOTE);
					pattern_data.push_back(CUT);
				}
				if (
					stereo &&
					((channel_2_itr->panning_left != channel_2_prev_note.panning_left) ||
					(channel_2_itr->panning_right != channel_2_prev_note.panning_right))
				) {
					pattern_data.push_back(CHANNEL + CH2);
					pattern_data.push_back(COMMAND);
					pattern_data.push_back(STEREO_PANNING);
					pattern_data.push_back(get_stereo_panning(channel_2_itr->panning_left, channel_2_itr->panning_right));
				}
				channel_2_prev_note = *channel_2_itr;
				++channel_2_itr;
			}
			else if (channel_2_note_length > 0) {
				uint32_t volume_fade_period = convert_fade_period(global_tempo, channel_2_prev_note.fade);
				if (channel_2_prev_note.fade && row % volume_fade_period == 0) {
					pattern_data.push_back(CHANNEL + CH2);
					pattern_data.push_back(COMMAND);
					if (channel_2_prev_note.slide_pitch != Pitch::REST) {
						pattern_data.push_back(FADE_PITCH_SLIDE);
					}
					else if (
						channel_2_prev_note.vibrato_rate &&
						channel_2_prev_note.vibrato_extent &&
						channel_2_note_duration > convert_vibrato_delay(global_tempo, channel_2_prev_note.speed, channel_2_prev_note.vibrato_delay)
					) {
						pattern_data.push_back(FADE_VIBRATO);
					}
					else {
						pattern_data.push_back(FADE);
					}
					if (channel_2_prev_note.fade > 0) {
						pattern_data.push_back((FINELY << 4) | 4);
					}
					else {
						pattern_data.push_back((4 << 4) | FINELY);
					}
				}
				else if (channel_2_prev_note.slide_pitch != Pitch::REST) {
					pattern_data.push_back(CHANNEL + CH2);
					pattern_data.push_back(NOTE + COMMAND);
					pattern_data.push_back(channel_2_prev_note.slide_octave * 12 + (uint32_t)channel_2_prev_note.slide_pitch - 1);
					pattern_data.push_back(PITCH_SLIDE);
					pattern_data.push_back(channel_2_prev_note.slide_duration * 6);
				}
				else if (
					channel_2_prev_note.vibrato_rate &&
					channel_2_prev_note.vibrato_extent &&
					channel_2_note_duration >= convert_vibrato_delay(global_tempo, channel_2_prev_note.speed, channel_2_prev_note.vibrato_delay)
				) {
					pattern_data.push_back(CHANNEL + CH2);
					pattern_data.push_back(COMMAND);
					pattern_data.push_back(VIBRATO);
					pattern_data.push_back(convert_vibrato_rate(global_tempo, channel_2_prev_note.vibrato_rate) << 4 | (channel_2_prev_note.vibrato_extent));
				}
				channel_2_note_length -= 1;
				channel_2_note_duration += 1;
			}
			else {
				pattern_data.push_back(CHANNEL + CH2);
				pattern_data.push_back(NOTE);
				pattern_data.push_back(CUT);
			}

			if (channel_3_note_length == 0 && channel_3_itr != channel_3_notes.end()) {
				channel_3_note_length = channel_3_itr->length * channel_3_itr->speed - 1;
				channel_3_note_duration = 0;
				if (channel_3_itr->wave != 0x0f || !num_inline_waves) {
					wave = channel_3_itr->wave;
				}
				if (channel_3_itr->tempo != channel_3_prev_note.tempo) {
					global_tempo = channel_3_itr->tempo;
					channel_3_tempo = convert_tempo(channel_3_itr->tempo);
					pattern_data.push_back(CHANNEL + CH7);
					pattern_data.push_back(COMMAND);
					pattern_data.push_back(TEMPO);
					pattern_data.push_back(channel_3_tempo / 256);

					if (first_channel != 3) {
						_tempo_change_wrong_channel = 3;
					}
					if (_tempo_change_mid_note == -1) {
						if (channel_1_note_length > 0) {
							_tempo_change_mid_note = 1;
						}
						else if (channel_2_note_length > 0) {
							_tempo_change_mid_note = 2;
						}
						else if (channel_4_note_length > 0) {
							_tempo_change_mid_note = 4;
						}
					}
				}
				if (channel_3_itr->pitch != Pitch::REST && wave < 16 + 15) {
					pattern_data.push_back(CHANNEL + CH3);
					pattern_data.push_back(NOTE + SAMPLE + VOLUME);
					pattern_data.push_back(note(*channel_3_itr)); // note
					pattern_data.push_back(wave + 1 + 4); // sample
					pattern_data.push_back(channel_3_volume(channel_3_itr->volume)); // volume
				}
				else {
					pattern_data.push_back(CHANNEL + CH3);
					pattern_data.push_back(NOTE);
					pattern_data.push_back(CUT);
				}
				if (
					stereo &&
					((channel_3_itr->panning_left != channel_3_prev_note.panning_left) ||
					(channel_3_itr->panning_right != channel_3_prev_note.panning_right))
				) {
					pattern_data.push_back(CHANNEL + CH3);
					pattern_data.push_back(COMMAND);
					pattern_data.push_back(STEREO_PANNING);
					pattern_data.push_back(get_stereo_panning(channel_3_itr->panning_left, channel_3_itr->panning_right));
				}
				channel_3_prev_note = *channel_3_itr;
				++channel_3_itr;
			}
			else if (channel_3_note_length > 0) {
				if (channel_3_prev_note.slide_pitch != Pitch::REST) {
					pattern_data.push_back(CHANNEL + CH3);
					pattern_data.push_back(NOTE + COMMAND);
					pattern_data.push_back(channel_3_prev_note.slide_octave * 12 + (uint32_t)channel_3_prev_note.slide_pitch - 1);
					pattern_data.push_back(PITCH_SLIDE);
					pattern_data.push_back(channel_3_prev_note.slide_duration * 6);
				}
				else if (
					channel_3_prev_note.vibrato_rate &&
					channel_3_prev_note.vibrato_extent &&
					channel_3_note_duration >= convert_vibrato_delay(global_tempo, channel_3_prev_note.speed, channel_3_prev_note.vibrato_delay)
				) {
					pattern_data.push_back(CHANNEL + CH3);
					pattern_data.push_back(COMMAND);
					pattern_data.push_back(VIBRATO);
					pattern_data.push_back(convert_vibrato_rate(global_tempo, channel_3_prev_note.vibrato_rate) << 4 | (channel_3_prev_note.vibrato_extent));
				}
				channel_3_note_length -= 1;
				channel_3_note_duration += 1;
			}
			else {
				pattern_data.push_back(CHANNEL + CH3);
				pattern_data.push_back(NOTE);
				pattern_data.push_back(CUT);
			}

			if (channel_4_note_length == 0 && channel_4_itr != channel_4_notes.end()) {
				channel_4_note_length = channel_4_itr->length * channel_4_itr->speed - 1;
				if (channel_4_itr->tempo != channel_4_prev_note.tempo) {
					global_tempo = channel_4_itr->tempo;
					channel_4_tempo = convert_tempo(channel_4_itr->tempo);
					pattern_data.push_back(CHANNEL + CH8);
					pattern_data.push_back(COMMAND);
					pattern_data.push_back(TEMPO);
					pattern_data.push_back(channel_4_tempo / 256);

					if (first_channel != 4) {
						_tempo_change_wrong_channel = 4;
					}
					if (_tempo_change_mid_note == -1) {
						if (channel_1_note_length > 0) {
							_tempo_change_mid_note = 1;
						}
						else if (channel_2_note_length > 0) {
							_tempo_change_mid_note = 2;
						}
						else if (channel_3_note_length > 0) {
							_tempo_change_mid_note = 3;
						}
					}
				}
				if (channel_4_itr->pitch != Pitch::REST && channel_4_itr->drumkit != -1 && drumkits[channel_4_itr->drumkit].drums[(int32_t)channel_4_itr->pitch] < 64) {
					pattern_data.push_back(CHANNEL + CH4);
					pattern_data.push_back(NOTE + SAMPLE + VOLUME);
					pattern_data.push_back(60); // note
					pattern_data.push_back(drumkits[channel_4_itr->drumkit].drums[(int32_t)channel_4_itr->pitch] + 1 + 4 + 16 + num_inline_waves); // sample
					pattern_data.push_back(64); // volume
				}
				if (
					stereo &&
					((channel_4_itr->panning_left != channel_4_prev_note.panning_left) ||
					(channel_4_itr->panning_right != channel_4_prev_note.panning_right))
				) {
					pattern_data.push_back(CHANNEL + CH4);
					pattern_data.push_back(COMMAND);
					pattern_data.push_back(STEREO_PANNING);
					pattern_data.push_back(get_stereo_panning(channel_4_itr->panning_left, channel_4_itr->panning_right));
				}
				channel_4_prev_note = *channel_4_itr;
				++channel_4_itr;
			}
			else if (channel_4_note_length > 0) {
				channel_4_note_length -= 1;
			}

			if (song_finished() && loop_tick != -1) {
				uint32_t pattern_number = (uint32_t)(loop_tick) / ROWS_PER_PATTERN;
				uint32_t row_number = (uint32_t)(loop_tick) % ROWS_PER_PATTERN;

				pattern_data.push_back(CHANNEL + CH9);
				pattern_data.push_back(COMMAND);
				pattern_data.push_back(PATTERN_JUMP);
				pattern_data.push_back(pattern_number);

				pattern_data.push_back(CHANNEL + CH10);
				pattern_data.push_back(COMMAND);
				pattern_data.push_back(ROW_JUMP);
				pattern_data.push_back(row_number);
			}

			pattern_data.push_back(0);
			row += 1;
		} while (row < ROWS_PER_PATTERN && !song_finished());

		put_short(pattern, (uint32_t)pattern_data.size());
		put_short(pattern, row);
		put_short(pattern, 0); // unused
		put_short(pattern, 0); // unused
		pattern.insert(pattern.end(), pattern_data.begin(), pattern_data.end());

		patterns.push_back(std::move(pattern));
	} while (!song_finished());

	return patterns;
}

static uint32_t get_total_size(const std::vector<std::vector<uint8_t>> &data) {
	std::size_t size = 0;
	for (const auto &v : data) {
		size += v.size();
	}
	return (uint32_t)size;
}

void IT_Module::generate_it_module(
	const std::vector<Note_View> &channel_1_notes,
	const std::vector<Note_View> &channel_2_notes,
	const std::vector<Note_View> &channel_3_notes,
	const std::vector<Note_View> &channel_4_notes,
	const std::vector<Wave> &waves,
	const std::vector<Drumkit> &drumkits,
	const std::vector<std::vector<uint8_t>> &drums,
	int32_t loop_tick,
	bool stereo
) {
	const uint32_t song_name_length = 26;
	const uint32_t pattern_row_highlight = 0x1004; // ???
	const uint32_t tracker_version = 0x5130;
	const uint32_t compatible_version = 0x0214;
	const uint32_t flags = 0b01001001;
	const uint32_t special = 0b00000110;

	const uint32_t global_volume = 128;
	const uint32_t mix_volume = 48;
	const uint32_t initial_speed = 1;
	const uint32_t initial_tempo = 150;
	const uint32_t panning_separation = 128;
	const uint32_t pitch_wheel_depth = 0;
	const uint32_t default_channel_panning = 32;
	const uint32_t channel_disabled = 0b10000000;
	const uint32_t max_num_channels = 64;
	const uint32_t default_channel_volume = 64;

	const uint32_t sample_header_size = 80;

	std::vector<std::vector<uint8_t>> instruments = get_instruments();
	std::vector<std::vector<uint8_t>> samples = get_samples(waves, drums);
	std::vector<std::vector<uint8_t>> patterns = get_patterns(channel_1_notes, channel_2_notes, channel_3_notes, channel_4_notes, drumkits, loop_tick, stereo, (int32_t)waves.size() - 0x10);

	const uint32_t number_of_orders = (uint32_t)patterns.size() + 1;
	const uint32_t number_of_instruments = (uint32_t)instruments.size();
	const uint32_t number_of_samples = (uint32_t)samples.size();
	const uint32_t number_of_patterns = (uint32_t)patterns.size();

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
		instrument_offset += (uint32_t)instrument.size();
	}
	// sample offsets
	uint32_t sample_offset = samples_start;
	for (auto &sample : samples) {
		put_int(_data, sample_offset);

		// fix inner sample offset
		patch_int(sample, 72, sample_offset + sample_header_size);

		sample_offset += (uint32_t)sample.size();
	}
	// pattern offsets
	uint32_t pattern_offset = patterns_start;
	for (const auto &pattern : patterns) {
		put_int(_data, pattern_offset);
		pattern_offset += (uint32_t)pattern.size();
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

	// extensions
	{
		_data.push_back('S');
		_data.push_back('T');
		_data.push_back('P');
		_data.push_back('M');

		// compatibility flags, defaults unless noted
		{
			_data.push_back('.');
			_data.push_back('F');
			_data.push_back('S');
			_data.push_back('M');

			put_short(_data, 15);

			_data.push_back(0xc1); // bit 6: fine tone portamento
			_data.push_back(0xfe); // bit 0: tempo clamp off
			_data.push_back(0xff);
			_data.push_back(0xff);
			_data.push_back(0xff);
			_data.push_back(0xff);
			_data.push_back(0x05);
			_data.push_back(0x00);
			_data.push_back(0x00);
			_data.push_back(0x00);
			_data.push_back(0x80);
			_data.push_back(0x01);
			_data.push_back(0xd0);
			_data.push_back(0x01);
			_data.push_back(0x08);
		}
	}
}

std::vector<std::vector<uint8_t>> generate_noise_samples(const std::vector<Drum> &drums) {
	std::vector<std::vector<uint8_t>> samples;
	for (const Drum &drum : drums) {
		std::vector<uint8_t> sample;
		for (uint32_t i = 0; i < drum.noise_notes.size(); ++i) {
			const Noise_Note &note = drum.noise_notes[i];
			bool last = i == drum.noise_notes.size() - 1;

			uint32_t sample_len = note.length * 48 * NOISE_SAMPLE_SPEED_FACTOR;
			uint32_t lfsr_period = std::max(
				(uint32_t)(((note.clock_divider == 0 ? 0.5f : note.clock_divider) * (1 << note.clock_shift)) * (32768.0f * NOISE_SAMPLE_SPEED_FACTOR / 262144.0f)),
				(uint32_t)1
			);
			uint32_t envelope_period = note.sweep_pace * 512 * NOISE_SAMPLE_SPEED_FACTOR;

			uint16_t lfsr = 0;
			int32_t volume = note.volume;

			uint32_t j = 0;
			while (
				j < sample_len ||
				(last && volume != 0 && j < 255 * 48 * NOISE_SAMPLE_SPEED_FACTOR * 8)
			) {
				if (j % lfsr_period == 0) {
					uint16_t xnor = (~(((lfsr >> 1) & 1) ^ (lfsr & 1))) & 1;
					if (note.lfsr_width) {
						lfsr = ((lfsr & 0b0111111101111111) | (xnor << 15) | (xnor << 7)) >> 1;
					}
					else {
						lfsr = ((lfsr & 0b0111111111111111) | (xnor << 15)) >> 1;
					}
				}
				if (envelope_period && j % envelope_period == 0) {
					if (note.envelope_direction == 0 && volume != 0) {
						volume -= 1;
					}
					else if (note.envelope_direction == 1 && volume != 15) {
						volume += 1;
					}
				}
				sample.push_back((lfsr & 1) ? (volume * 255 / 15 / 2) : 0);
				j += 1;
			}
		}
		samples.push_back(std::move(sample));
	}
	return samples;
}
