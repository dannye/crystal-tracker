#ifndef COMMAND_H
#define COMMAND_H

#include <set>
#include <string>

enum class Pitch {
	REST,
	C_NAT,
	C_SHARP,
	D_NAT,
	D_SHARP,
	E_NAT,
	F_NAT,
	F_SHARP,
	G_NAT,
	G_SHARP,
	A_NAT,
	A_SHARP,
	B_NAT,
};

static const char * const PITCH_NAMES[] = {
	"",
	"C_",
	"C#",
	"D_",
	"D#",
	"E_",
	"F_",
	"F#",
	"G_",
	"G#",
	"A_",
	"A#",
	"B_",
};

static const size_t NUM_PITCHES = sizeof(PITCH_NAMES) / sizeof(char *) - 1;

enum class Command_Type {
	NOTE,
	DRUM_NOTE,
	REST,
	OCTAVE,
	NOTE_TYPE,
	DRUM_SPEED,
	TRANSPOSE,
	TEMPO,
	DUTY_CYCLE,
	VOLUME_ENVELOPE,
	PITCH_SWEEP,
	DUTY_CYCLE_PATTERN,
	PITCH_SLIDE,
	VIBRATO,
	TOGGLE_NOISE,
	FORCE_STEREO_PANNING,
	VOLUME,
	PITCH_OFFSET,
	STEREO_PANNING,
	SOUND_JUMP,
	SOUND_LOOP,
	SOUND_CALL,
	SOUND_RET,
};

static const char * const COMMAND_NAMES[] = {
	"note",
	"drum_note",
	"rest",
	"octave",
	"note_type",
	"drum_speed",
	"transpose",
	"tempo",
	"duty_cycle",
	"volume_envelope",
	"pitch_sweep",
	"duty_cycle_pattern",
	"pitch_slide",
	"vibrato",
	"toggle_noise",
	"force_stereo_panning",
	"volume",
	"pitch_offset",
	"stereo_panning",
	"sound_jump",
	"sound_loop",
	"sound_call",
	"sound_ret",
};

// excludes speed changes until that is handled specially
static inline bool is_note_setting_command(Command_Type type) {
	return (
		type == Command_Type::OCTAVE ||
		// type == Command_Type::NOTE_TYPE ||
		// type == Command_Type::DRUM_SPEED ||
		type == Command_Type::TRANSPOSE ||
		type == Command_Type::DUTY_CYCLE ||
		type == Command_Type::VOLUME_ENVELOPE ||
		type == Command_Type::PITCH_SWEEP ||
		type == Command_Type::DUTY_CYCLE_PATTERN ||
		type == Command_Type::PITCH_SLIDE ||
		type == Command_Type::VIBRATO ||
		type == Command_Type::TOGGLE_NOISE ||
		type == Command_Type::FORCE_STEREO_PANNING ||
		type == Command_Type::PITCH_OFFSET ||
		type == Command_Type::STEREO_PANNING
	);
}

struct Command {

	struct Note {
		int32_t length;
		Pitch pitch;
	};

	struct Drum_Note {
		int32_t length;
		int32_t instrument;
	};

	struct Rest {
		int32_t length;
	};

	struct Octave {
		int32_t octave;
	};

	struct Note_Type {
		int32_t speed;
		int32_t volume;
		union {
			int32_t fade;
			int32_t wave;
		};
	};

	struct Drum_Speed {
		int32_t speed;
	};

	struct Transpose {
		int32_t num_octaves;
		int32_t num_pitches;
	};

	struct Tempo {
		int32_t tempo;
	};

	struct Duty_Cycle {
		int32_t duty;
	};

	struct Volume_Envelope {
		int32_t volume;
		union {
			int32_t fade;
			int32_t wave;
		};
	};

	struct Pitch_Sweep {
		int32_t duration;
		int32_t pitch_change;
	};

	struct Duty_Cycle_Pattern {
		int32_t duty1;
		int32_t duty2;
		int32_t duty3;
		int32_t duty4;
	};

	struct Pitch_Slide {
		int32_t duration;
		int32_t octave;
		Pitch pitch;
	};

	struct Vibrato {
		int32_t delay;
		int32_t extent;
		int32_t rate;
	};

	struct Toggle_Noise {
		int32_t drumkit;
	};

	struct Force_Stereo_Panning {
		int32_t left;
		int32_t right;
	};

	struct Volume {
		int32_t left;
		int32_t right;
	};

	struct Pitch_Offset {
		int32_t offset;
	};

	struct Stereo_Panning {
		int32_t left;
		int32_t right;
	};

	struct Sound_Jump {};

	struct Sound_Loop {
		int32_t loop_count;
	};

	struct Sound_Call {};

	struct Sound_Ret {};

	Command_Type type;
	std::set<std::string> labels;
	std::string target;
	union {
		Note note;
		Drum_Note drum_note;
		Rest rest;
		Octave octave;
		Note_Type note_type;
		Drum_Speed drum_speed;
		Transpose transpose;
		Tempo tempo;
		Duty_Cycle duty_cycle;
		Volume_Envelope volume_envelope;
		Pitch_Sweep pitch_sweep;
		Duty_Cycle_Pattern duty_cycle_pattern;
		Pitch_Slide pitch_slide;
		Vibrato vibrato;
		Toggle_Noise toggle_noise;
		Force_Stereo_Panning force_stereo_panning;
		Volume volume;
		Pitch_Offset pitch_offset;
		Stereo_Panning stereo_panning;
		Sound_Jump sound_jump;
		Sound_Loop sound_loop;
		Sound_Call sound_call;
		Sound_Ret sound_ret;
	};

	Command() {}
	Command(Command_Type t) {
		type = t;
	}
	Command(Command_Type t, const std::string& label) {
		type = t;
		labels.insert(label);
	}
};

struct Note_View {
	int32_t length = 0;
	Pitch pitch = Pitch::REST;
	int32_t octave = 0;
	int32_t speed = 0;
	int32_t volume = 0;
	union {
		int32_t fade = 0;
		int32_t wave;
	};
	int32_t tempo  = 0;
	int32_t duty   = 0;
	int32_t delay  = 0; //
	int32_t extent = 0; // vibrato
	int32_t rate   = 0; //

	int32_t index = 0;
	bool ghost = false;
};

#endif
