#ifndef NOTE_PROPERTIES_H
#define NOTE_PROPERTIES_H

#pragma warning(push, 0)
#include <FL/Fl_Group.H>
#pragma warning(pop)

#include "command.h"
#include "widgets.h"

class Note_Properties : public Fl_Group {
private:
	OS_Spinner *_speed_input = nullptr;
	OS_Spinner *_volume_input = nullptr;
	OS_Spinner *_fade_input = nullptr;
	OS_Spinner *_vibrato_delay_input = nullptr;
	OS_Spinner *_vibrato_depth_input = nullptr;
	OS_Spinner *_vibrato_rate_input = nullptr;
	OS_Spinner *_duty_wave_input = nullptr;
	OS_Button *_advanced_button = nullptr;

	OS_Spinner *_tempo_input = nullptr;
	OS_Spinner *_transpose_octaves_input = nullptr;
	OS_Spinner *_transpose_pitches_input = nullptr;
	OS_Spinner *_slide_duration_input = nullptr;
	OS_Spinner *_slide_octave_input = nullptr;
	Dropdown *_slide_pitch_input = nullptr;
	OS_Button *_basic_button = nullptr;
public:
	Note_Properties(int X, int Y, int W, int H, const char *l = nullptr);
	void set_note_properties(Note_View view, int channel_number);
private:
	static void advanced_button_cb(OS_Button *b, Note_Properties *np);
	static void basic_button_cb(OS_Button *b, Note_Properties *np);
};

#endif
