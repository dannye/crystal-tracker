#include "note-properties.h"

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

	_speed_input->value(0);
	_volume_input->value(0);
	_fade_input->value(0);
	_vibrato_delay_input->value(0);
	_vibrato_depth_input->value(0);
	_vibrato_rate_input->value(0);
	_duty_wave_input->value(0);

	_speed_input->deactivate();
	_volume_input->deactivate();
	_fade_input->deactivate();
	_vibrato_delay_input->deactivate();
	_vibrato_depth_input->deactivate();
	_vibrato_rate_input->deactivate();
	_duty_wave_input->deactivate();
}

void Note_Properties::set_note_properties(Note_View view, int channel_number) {
	_speed_input->value(view.speed);
	_volume_input->value(view.volume);
	if (channel_number == 1 || channel_number == 2) {
		_fade_input->value(view.fade);
	}
	else {
		_fade_input->value(0);
	}
	_vibrato_delay_input->value(view.vibrato_delay);
	_vibrato_depth_input->value(view.vibrato_extent);
	_vibrato_rate_input->value(view.vibrato_rate);
	if (channel_number == 3) {
		_duty_wave_input->label("Wave:");
		_duty_wave_input->value(view.wave);
	}
	else {
		_duty_wave_input->label("Duty:");
		_duty_wave_input->value(view.duty);
	}
	redraw();
}
