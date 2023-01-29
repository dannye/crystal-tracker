#include "note-properties.h"

#include <cassert>

#include "themes.h"
#include "utils.h"

Note_Properties::Note_Properties(int X, int Y, int W, int H, const char *l) : Fl_Group(X, Y, W, H, l) {
	box(OS_TOOLBAR_FRAME);
	int wgt_w = text_width("99", 12), wgt_w2 = text_width("999", 12), wgt_h = 22, wgt_m = 10;
	int dx = wgt_m;

	_speed_input = new OS_Spinner(x() + dx + text_width("Speed:", 2), y() + wgt_m, wgt_w, wgt_h, "Speed:");
	dx += wgt_w + wgt_m + text_width("Speed:", 2);

	_volume_input = new OS_Spinner(x() + dx + text_width("Volume:", 2), y() + wgt_m, wgt_w, wgt_h, "Volume:");
	dx += wgt_w + wgt_m + text_width("Volume:", 2);

	_fade_input = new OS_Spinner(x() + dx + text_width("Fade:", 2), y() + wgt_m, wgt_w, wgt_h, "Fade:");
	dx += wgt_w + wgt_m + text_width("Fade:", 2);

	_vibrato_delay_input = new OS_Spinner(x() + dx + text_width("Vibrato delay:", 2), y() + wgt_m, wgt_w2, wgt_h, "Vibrato delay:");
	dx += wgt_w2 + wgt_m + text_width("Vibrato delay:", 2);

	_vibrato_depth_input = new OS_Spinner(x() + dx + text_width("Vibrato depth:", 2), y() + wgt_m, wgt_w, wgt_h, "Vibrato depth:");
	dx += wgt_w + wgt_m + text_width("Vibrato depth:", 2);

	_vibrato_rate_input = new OS_Spinner(x() + dx + text_width("Vibrato rate:", 2), y() + wgt_m, wgt_w, wgt_h, "Vibrato rate:");
	dx += wgt_w + wgt_m + text_width("Vibrato rate:", 2);

	_duty_wave_input = new OS_Spinner(x() + dx + text_width("Wave:", 2), y() + wgt_m, wgt_w, wgt_h, "Wave:");
	dx += wgt_w + wgt_m + text_width("Wave:", 2);

	_advanced_button = new OS_Button(x() + dx + wgt_m, y() + wgt_m, text_width("Advanced", 8), wgt_h, "Advanced");

	dx = wgt_m;

	_tempo_input = new OS_Spinner(x() + dx + text_width("Tempo:", 2), y() + wgt_m, wgt_w2, wgt_h, "Tempo:");
	dx += wgt_w2 + wgt_m + text_width("Tempo:", 2);

	_transpose_octaves_input = new OS_Spinner(x() + dx + text_width("Transpose octaves:", 2), y() + wgt_m, wgt_w, wgt_h, "Transpose octaves:");
	dx += wgt_w + wgt_m + text_width("Transpose octaves:", 2);

	_transpose_pitches_input = new OS_Spinner(x() + dx + text_width("Transpose pitches:", 2), y() + wgt_m, wgt_w, wgt_h, "Transpose pitches:");
	dx += wgt_w + wgt_m + text_width("Transpose pitches:", 2);

	_slide_duration_input = new OS_Spinner(x() + dx + text_width("Slide duration:", 2), y() + wgt_m, wgt_w2, wgt_h, "Slide duration:");
	dx += wgt_w2 + wgt_m + text_width("Slide duration:", 2);

	_slide_octave_input = new OS_Spinner(x() + dx + text_width("Slide octave:", 2), y() + wgt_m, wgt_w, wgt_h, "Slide octave:");
	dx += wgt_w + wgt_m + text_width("Slide octave:", 2);

	_slide_pitch_input = new Dropdown(x() + dx + text_width("Slide pitch:", 2), y() + wgt_m, wgt_w2, wgt_h, "Slide pitch:");
	dx += wgt_w2 + wgt_m + text_width("Slide pitch:", 2);
	for (int i = 0; i <= NUM_PITCHES; ++i) {
		_slide_pitch_input->add(PITCH_NAMES[i]);
	}

	_basic_button = new OS_Button(x() + dx + wgt_m, y() + wgt_m, text_width("Basic", 8), wgt_h, "Basic");

	_advanced_button->callback((Fl_Callback *)advanced_button_cb, this);
	_basic_button->callback((Fl_Callback *)basic_button_cb, this);

	basic_button_cb(nullptr, this);
}

void Note_Properties::set_note_properties(const std::vector<const Note_View *> &notes, int channel_number) {
	assert(notes.size() > 0);
	const Note_View *first = notes.front();
	_speed_input->value(first->speed);
	_volume_input->value(first->volume);
	if (channel_number == 1 || channel_number == 2) {
		_fade_input->value(first->fade);
	}
	else {
		_fade_input->value(0);
	}
	_vibrato_delay_input->value(first->vibrato_delay);
	_vibrato_depth_input->value(first->vibrato_extent);
	_vibrato_rate_input->value(first->vibrato_rate);
	if (channel_number == 3) {
		_duty_wave_input->value(first->wave);
	}
	else {
		_duty_wave_input->value(first->duty);
	}

	_tempo_input->value(first->tempo);
	_transpose_octaves_input->value(first->transpose_octaves);
	_transpose_pitches_input->value(first->transpose_pitches);
	_slide_duration_input->value(first->slide_duration);
	_slide_octave_input->value(first->slide_octave);
	_slide_pitch_input->value((int)first->slide_pitch);

	_speed_input->label("Speed:");
	_volume_input->label("Volume:");
	_fade_input->label("Fade:");
	_vibrato_delay_input->label("Vibrato delay:");
	_vibrato_depth_input->label("Vibrato depth:");
	_vibrato_rate_input->label("Vibrato rate:");
	if (channel_number == 3) {
		_duty_wave_input->label("Wave:");
	}
	else {
		_duty_wave_input->label("Duty:");
	}

	_tempo_input->label("Tempo:");
	_transpose_octaves_input->label("Transpose octaves:");
	_transpose_pitches_input->label("Transpose pitches:");
	_slide_duration_input->label("Slide duration:");
	_slide_octave_input->label("Slide octave:");
	_slide_pitch_input->label("Slide pitch:");

	for (const Note_View *note : notes) {
		if (note->speed != (int32_t)_speed_input->value()) {
			_speed_input->label("*Speed:");
		}
		if (note->volume != (int32_t)_volume_input->value()) {
			_volume_input->label("*Volume:");
		}
		if (
			(channel_number == 1 || channel_number == 2) &&
			note->fade != (int32_t)_fade_input->value()
		) {
			_fade_input->label("*Fade:");
		}
		if (note->vibrato_delay != (int32_t)_vibrato_delay_input->value()) {
			_vibrato_delay_input->label("*Vibrato delay:");
		}
		if (note->vibrato_extent != (int32_t)_vibrato_depth_input->value()) {
			_vibrato_depth_input->label("*Vibrato depth:");
		}
		if (note->vibrato_rate != (int32_t)_vibrato_rate_input->value()) {
			_vibrato_rate_input->label("*Vibrato rate:");
		}
		if (
			channel_number == 3 &&
			note->wave != (int32_t)_duty_wave_input->value()
		) {
			_duty_wave_input->label("*Wave:");
		}
		else if (
			channel_number != 3 &&
			note->duty != (int32_t)_duty_wave_input->value()
		) {
			_duty_wave_input->label("*Duty:");
		}

		if (note->tempo != (int32_t)_tempo_input->value()) {
			_tempo_input->label("*Tempo:");
		}
		if (note->transpose_octaves != (int32_t)_transpose_octaves_input->value()) {
			_transpose_octaves_input->label("*Transpose octaves:");
		}
		if (note->transpose_pitches != (int32_t)_transpose_pitches_input->value()) {
			_transpose_pitches_input->label("*Transpose pitches:");
		}
		if (note->slide_duration != (int32_t)_slide_duration_input->value()) {
			_slide_duration_input->label("*Slide duration:");
		}
		if (note->slide_octave != (int32_t)_slide_octave_input->value()) {
			_slide_octave_input->label("*Slide octave:");
		}
		if ((int)note->slide_pitch != _slide_pitch_input->value()) {
			_slide_pitch_input->label("*Slide pitch:");
		}
	}

	_speed_input->minimum(1);
	_speed_input->maximum(15);
	if (channel_number == 3) {
		_volume_input->minimum(0);
		_volume_input->maximum(3);
	}
	else {
		_volume_input->minimum(0);
		_volume_input->maximum(15);
	}
	_fade_input->minimum(-7);
	_fade_input->maximum(7);
	_vibrato_delay_input->minimum(0);
	_vibrato_delay_input->maximum(255);
	_vibrato_depth_input->minimum(0);
	_vibrato_depth_input->maximum(15);
	_vibrato_rate_input->minimum(0);
	_vibrato_rate_input->maximum(15);
	if (channel_number == 3) {
		_duty_wave_input->minimum(0);
		_duty_wave_input->maximum(15);
	}
	else {
		_duty_wave_input->minimum(0);
		_duty_wave_input->maximum(3);
	}

	_tempo_input->minimum(0);
	_tempo_input->maximum(1024);
	_transpose_octaves_input->minimum(0);
	_transpose_octaves_input->maximum(7);
	_transpose_pitches_input->minimum(0);
	_transpose_pitches_input->maximum(15);
	_slide_duration_input->minimum(0);
	_slide_duration_input->maximum(256);
	_slide_octave_input->minimum(0);
	_slide_octave_input->maximum(8);

	_speed_input->activate();
	_volume_input->activate();
	_fade_input->activate();
	_vibrato_delay_input->activate();
	_vibrato_depth_input->activate();
	_vibrato_rate_input->activate();
	_duty_wave_input->activate();

	_tempo_input->activate();
	_transpose_octaves_input->activate();
	_transpose_pitches_input->activate();
	_slide_duration_input->activate();
	_slide_octave_input->activate();
	_slide_pitch_input->activate();

	if (channel_number == 3) {
		_fade_input->deactivate();
	}
	if (channel_number == 4) {
		_volume_input->deactivate();
		_fade_input->deactivate();
		_vibrato_delay_input->deactivate();
		_vibrato_depth_input->deactivate();
		_vibrato_rate_input->deactivate();
		_duty_wave_input->deactivate();

		_transpose_octaves_input->deactivate();
		_transpose_pitches_input->deactivate();
		_slide_duration_input->deactivate();
		_slide_octave_input->deactivate();
		_slide_pitch_input->deactivate();
	}

	redraw();
}

void Note_Properties::advanced_button_cb(OS_Button *, Note_Properties *np) {
	np->_speed_input->hide();
	np->_volume_input->hide();
	np->_fade_input->hide();
	np->_vibrato_delay_input->hide();
	np->_vibrato_depth_input->hide();
	np->_vibrato_rate_input->hide();
	np->_duty_wave_input->hide();
	np->_advanced_button->hide();

	np->_tempo_input->show();
	np->_transpose_octaves_input->show();
	np->_transpose_pitches_input->show();
	np->_slide_duration_input->show();
	np->_slide_octave_input->show();
	np->_slide_pitch_input->show();
	np->_basic_button->show();
}

void Note_Properties::basic_button_cb(OS_Button *, Note_Properties *np) {
	np->_speed_input->show();
	np->_volume_input->show();
	np->_fade_input->show();
	np->_vibrato_delay_input->show();
	np->_vibrato_depth_input->show();
	np->_vibrato_rate_input->show();
	np->_duty_wave_input->show();
	np->_advanced_button->show();

	np->_tempo_input->hide();
	np->_transpose_octaves_input->hide();
	np->_transpose_pitches_input->hide();
	np->_slide_duration_input->hide();
	np->_slide_octave_input->hide();
	np->_slide_pitch_input->hide();
	np->_basic_button->hide();
}
