#include <cassert>

#include "note-properties.h"

#include "main-window.h"
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

	_speed_input->callback((Fl_Callback *)speed_input_cb, this);
	_speed_input->clear_visible_focus();

	_volume_input->callback((Fl_Callback *)volume_input_cb, this);
	_volume_input->clear_visible_focus();

	_fade_input->callback((Fl_Callback *)fade_input_cb, this);
	_fade_input->clear_visible_focus();

	_vibrato_delay_input->callback((Fl_Callback *)vibrato_delay_input_cb, this);
	_vibrato_delay_input->clear_visible_focus();

	_vibrato_depth_input->callback((Fl_Callback *)vibrato_depth_input_cb, this);
	_vibrato_depth_input->clear_visible_focus();

	_vibrato_rate_input->callback((Fl_Callback *)vibrato_rate_input_cb, this);
	_vibrato_rate_input->clear_visible_focus();

	_duty_wave_input->callback((Fl_Callback *)duty_wave_input_cb, this);
	_duty_wave_input->clear_visible_focus();

	_advanced_button->callback((Fl_Callback *)advanced_button_cb, this);

	_tempo_input->callback((Fl_Callback *)tempo_input_cb, this);
	_tempo_input->clear_visible_focus();

	_transpose_octaves_input->callback((Fl_Callback *)transpose_octaves_input_cb, this);
	_transpose_octaves_input->clear_visible_focus();

	_transpose_pitches_input->callback((Fl_Callback *)transpose_pitches_input_cb, this);
	_transpose_pitches_input->clear_visible_focus();

	_slide_duration_input->callback((Fl_Callback *)slide_duration_input_cb, this);
	_slide_duration_input->clear_visible_focus();

	_slide_octave_input->callback((Fl_Callback *)slide_octave_input_cb, this);
	_slide_octave_input->clear_visible_focus();

	_slide_pitch_input->callback((Fl_Callback *)slide_pitch_input_cb, this);
	_slide_pitch_input->clear_visible_focus();

	_basic_button->callback((Fl_Callback *)basic_button_cb, this);

	basic_button_cb(nullptr, this);
}

Note_Properties::~Note_Properties() {
	delete _speed_input;
	delete _volume_input;
	delete _fade_input;
	delete _vibrato_delay_input;
	delete _vibrato_depth_input;
	delete _vibrato_rate_input;
	delete _duty_wave_input;
	delete _advanced_button;

	delete _tempo_input;
	delete _transpose_octaves_input;
	delete _transpose_pitches_input;
	delete _slide_duration_input;
	delete _slide_octave_input;
	delete _slide_pitch_input;
	delete _basic_button;
}

void Note_Properties::set_note_properties(const std::vector<const Note_View *> &notes, int channel_number) {
	assert(notes.size() > 0);
	_note = *notes.front();
	_channel_number = channel_number;
	_speed_input->value(_note.speed);
	_volume_input->value(_note.volume);
	if (channel_number == 1 || channel_number == 2) {
		_fade_input->value(_note.fade);
	}
	else {
		_fade_input->value(0);
	}
	_vibrato_delay_input->value(_note.vibrato_delay);
	_vibrato_depth_input->value(_note.vibrato_extent);
	_vibrato_rate_input->value(_note.vibrato_rate);
	if (channel_number == 3) {
		_duty_wave_input->value(_note.wave);
	}
	else {
		_duty_wave_input->value(_note.duty);
	}

	_tempo_input->value(_note.tempo);
	_transpose_octaves_input->value(_note.transpose_octaves);
	_transpose_pitches_input->value(_note.transpose_pitches);
	_slide_duration_input->value(_note.slide_duration);
	_slide_octave_input->value(_note.slide_octave);
	_slide_pitch_input->value((int)_note.slide_pitch);

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

	_speed_input->range(1, 15);
	if (channel_number == 3) {
		_volume_input->range(0, 3);
	}
	else {
		_volume_input->range(0, 15);
	}
	_fade_input->range(-7, 7);
	_vibrato_delay_input->range(0, 255);
	_vibrato_depth_input->range(0, 15);
	_vibrato_rate_input->range(0, 15);
	if (channel_number == 3) {
		_duty_wave_input->range(0, 15);
	}
	else {
		_duty_wave_input->range(0, 3);
	}

	_tempo_input->range(0, 1024);
	_transpose_octaves_input->range(0, 7);
	_transpose_pitches_input->range(0, 15);
	_slide_duration_input->range(0, 256);
	_slide_octave_input->range(0, 8);

	_speed_input->deactivate(); // TODO: handle speed change
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

void Note_Properties::speed_input_cb(OS_Spinner *s, Note_Properties *np) {
	if (!s->active()) return;

	int32_t val = (int32_t)s->value();
	if (val != np->_note.speed) {
		Main_Window *mw = (Main_Window *)np->user_data();
		mw->set_speed(val);
	}
}

void Note_Properties::volume_input_cb(OS_Spinner *s, Note_Properties *np) {
	if (!s->active()) return;

	int32_t val = (int32_t)s->value();
	if (val != np->_note.volume) {
		Main_Window *mw = (Main_Window *)np->user_data();
		mw->set_volume(val);
	}
	else if (val == 0) s->value(0);
}

void Note_Properties::fade_input_cb(OS_Spinner *s, Note_Properties *np) {
	if (!s->active()) return;

	int32_t val = (int32_t)s->value();
	if (val != np->_note.fade) {
		Main_Window *mw = (Main_Window *)np->user_data();
		mw->set_fade(val);
	}
	else if (val == 0) s->value(0);
}

void Note_Properties::vibrato_delay_input_cb(OS_Spinner *s, Note_Properties *np) {
	if (!s->active()) return;

	int32_t val = (int32_t)s->value();
	if (val != np->_note.vibrato_delay) {
		Main_Window *mw = (Main_Window *)np->user_data();
		mw->set_vibrato_delay(val);
	}
	else if (val == 0) s->value(0);
}

void Note_Properties::vibrato_depth_input_cb(OS_Spinner *s, Note_Properties *np) {
	if (!s->active()) return;

	int32_t val = (int32_t)s->value();
	if (val != np->_note.vibrato_extent) {
		Main_Window *mw = (Main_Window *)np->user_data();
		mw->set_vibrato_extent(val);
	}
	else if (val == 0) s->value(0);
}

void Note_Properties::vibrato_rate_input_cb(OS_Spinner *s, Note_Properties *np) {
	if (!s->active()) return;

	int32_t val = (int32_t)s->value();
	if (val != np->_note.vibrato_rate) {
		Main_Window *mw = (Main_Window *)np->user_data();
		mw->set_vibrato_rate(val);
	}
	else if (val == 0) s->value(0);
}

void Note_Properties::duty_wave_input_cb(OS_Spinner *s, Note_Properties *np) {
	if (!s->active()) return;

	int32_t val = (int32_t)s->value();
	if (np->_channel_number == 3 && val != np->_note.wave) {
		Main_Window *mw = (Main_Window *)np->user_data();
		mw->set_wave(val);
	}
	else if (np->_channel_number != 3 && val != np->_note.duty) {
		Main_Window *mw = (Main_Window *)np->user_data();
		mw->set_duty(val);
	}
	else if (val == 0) s->value(0);
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

void Note_Properties::tempo_input_cb(OS_Spinner *s, Note_Properties *np) {
	if (!s->active()) return;

	int32_t val = (int32_t)s->value();
	if (val != np->_note.tempo) {
		Main_Window *mw = (Main_Window *)np->user_data();
		mw->set_tempo(val);
	}
	else if (val == 0) s->value(0);
}

void Note_Properties::transpose_octaves_input_cb(OS_Spinner *s, Note_Properties *np) {
	if (!s->active()) return;

	int32_t val = (int32_t)s->value();
	if (val != np->_note.transpose_octaves) {
		Main_Window *mw = (Main_Window *)np->user_data();
		mw->set_transpose_octaves(val);
	}
	else if (val == 0) s->value(0);
}

void Note_Properties::transpose_pitches_input_cb(OS_Spinner *s, Note_Properties *np) {
	if (!s->active()) return;

	int32_t val = (int32_t)s->value();
	if (val != np->_note.transpose_pitches) {
		Main_Window *mw = (Main_Window *)np->user_data();
		mw->set_transpose_pitches(val);
	}
	else if (val == 0) s->value(0);
}

void Note_Properties::slide_duration_input_cb(OS_Spinner *s, Note_Properties *np) {
	if (!s->active()) return;

	int32_t val = (int32_t)s->value();
	if (val != np->_note.slide_duration) {
		if (val != 0 && (np->_slide_octave_input->value() == 0 || np->_slide_pitch_input->value() == 0)) {
			if (np->_slide_octave_input->value() == 0) np->_slide_octave_input->value(1);
			if (np->_slide_pitch_input->value() == 0) np->_slide_pitch_input->value(1);
			Main_Window *mw = (Main_Window *)np->user_data();
			mw->set_slide(val, (int32_t)np->_slide_octave_input->value(), (Pitch)np->_slide_pitch_input->value());
		}
		else if (val == 0 && (np->_slide_octave_input->value() != 0 || np->_slide_pitch_input->value() != 0)) {
			np->_slide_octave_input->value(0);
			np->_slide_pitch_input->value(0);
			Main_Window *mw = (Main_Window *)np->user_data();
			mw->set_slide(val, (int32_t)np->_slide_octave_input->value(), (Pitch)np->_slide_pitch_input->value());
		}
		else {
			Main_Window *mw = (Main_Window *)np->user_data();
			mw->set_slide_duration(val);
		}
	}
	else if (val == 0) s->value(0);
}

void Note_Properties::slide_octave_input_cb(OS_Spinner *s, Note_Properties *np) {
	if (!s->active()) return;

	int32_t val = (int32_t)s->value();
	if (val != np->_note.slide_octave) {
		if (val != 0 && (np->_slide_duration_input->value() == 0 || np->_slide_pitch_input->value() == 0)) {
			if (np->_slide_duration_input->value() == 0) np->_slide_duration_input->value(1);
			if (np->_slide_pitch_input->value() == 0) np->_slide_pitch_input->value(1);
			Main_Window *mw = (Main_Window *)np->user_data();
			mw->set_slide((int32_t)np->_slide_duration_input->value(), val, (Pitch)np->_slide_pitch_input->value());
		}
		else if (val == 0 && (np->_slide_duration_input->value() != 0 || np->_slide_pitch_input->value() != 0)) {
			np->_slide_duration_input->value(0);
			np->_slide_pitch_input->value(0);
			Main_Window *mw = (Main_Window *)np->user_data();
			mw->set_slide((int32_t)np->_slide_duration_input->value(), val, (Pitch)np->_slide_pitch_input->value());
		}
		else {
			Main_Window *mw = (Main_Window *)np->user_data();
			mw->set_slide_octave(val);
		}
	}
	else if (val == 0) s->value(0);
}

void Note_Properties::slide_pitch_input_cb(Dropdown *s, Note_Properties *np) {
	if (!s->active()) return;

	Pitch val = (Pitch)s->value();
	if (val != np->_note.slide_pitch) {
		if (val != Pitch::REST && (np->_slide_duration_input->value() == 0 || np->_slide_octave_input->value() == 0)) {
			if (np->_slide_duration_input->value() == 0) np->_slide_duration_input->value(1);
			if (np->_slide_octave_input->value() == 0) np->_slide_octave_input->value(1);
			Main_Window *mw = (Main_Window *)np->user_data();
			mw->set_slide((int32_t)np->_slide_duration_input->value(), (int32_t)np->_slide_octave_input->value(), val);
		}
		else if (val == Pitch::REST && (np->_slide_duration_input->value() != 0 || np->_slide_octave_input->value() != 0)) {
			np->_slide_duration_input->value(0);
			np->_slide_octave_input->value(0);
			Main_Window *mw = (Main_Window *)np->user_data();
			mw->set_slide((int32_t)np->_slide_duration_input->value(), (int32_t)np->_slide_octave_input->value(), val);
		}
		else {
			Main_Window *mw = (Main_Window *)np->user_data();
			mw->set_slide_pitch(val);
		}
	}
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
