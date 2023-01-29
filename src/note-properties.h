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

	Note_View _note;
	int _channel_number = 0;
public:
	Note_Properties(int X, int Y, int W, int H, const char *l = nullptr);
	~Note_Properties();
	void set_note_properties(const std::vector<const Note_View *> &notes, int channel_number);
private:
	static void speed_input_cb(OS_Spinner *s, Note_Properties *np);
	static void volume_input_cb(OS_Spinner *s, Note_Properties *np);
	static void fade_input_cb(OS_Spinner *s, Note_Properties *np);
	static void vibrato_delay_input_cb(OS_Spinner *s, Note_Properties *np);
	static void vibrato_depth_input_cb(OS_Spinner *s, Note_Properties *np);
	static void vibrato_rate_input_cb(OS_Spinner *s, Note_Properties *np);
	static void duty_wave_input_cb(OS_Spinner *s, Note_Properties *np);
	static void advanced_button_cb(OS_Button *b, Note_Properties *np);

	static void tempo_input_cb(OS_Spinner *s, Note_Properties *np);
	static void transpose_octaves_input_cb(OS_Spinner *s, Note_Properties *np);
	static void transpose_pitches_input_cb(OS_Spinner *s, Note_Properties *np);
	static void slide_duration_input_cb(OS_Spinner *s, Note_Properties *np);
	static void slide_octave_input_cb(OS_Spinner *s, Note_Properties *np);
	static void slide_pitch_input_cb(Dropdown *d, Note_Properties *np);
	static void basic_button_cb(OS_Button *b, Note_Properties *np);
};

#endif
