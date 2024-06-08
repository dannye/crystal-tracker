#include "option-dialogs.h"

#include "themes.h"
#include "main-window.h"

Option_Dialog::Option_Dialog(int w, const char *t) : _width(w), _title(t), _has_reset(false), _canceled(false),
	_dialog(NULL), _content(NULL), _ok_button(NULL), _cancel_button(NULL), _reset_button(NULL) {}

Option_Dialog::~Option_Dialog() {
	delete _dialog;
	delete _content;
	delete _ok_button;
	delete _cancel_button;
	delete _reset_button;
}

void Option_Dialog::initialize() {
	if (_dialog) { return; }
	Fl_Group *prev_current = Fl_Group::current();
	Fl_Group::current(NULL);
	// Populate dialog
	_dialog = new Fl_Double_Window(0, 0, 0, 0, _title);
	_content = new Fl_Group(0, 0, 0, 0);
	_content->begin();
	initialize_content();
	_content->end();
	_dialog->begin();
	_ok_button = new Default_Button(0, 0, 0, 0, "OK");
	_cancel_button = new OS_Button(0, 0, 0, 0, "Cancel");
	_reset_button = new OS_Button(0, 0, 0, 0, "Reset");
	_dialog->end();
	// Initialize dialog
	_dialog->box(OS_BG_BOX);
	_dialog->resizable(NULL);
	_dialog->callback((Fl_Callback *)cancel_cb, this);
	_dialog->set_modal();
	// Initialize dialog's children
	_ok_button->tooltip("OK (Enter)");
	_ok_button->callback((Fl_Callback *)close_cb, this);
	_cancel_button->shortcut(FL_Escape);
	_cancel_button->tooltip("Cancel (Esc)");
	_cancel_button->callback((Fl_Callback *)cancel_cb, this);
	if (_has_reset) set_reset_cb();
	Fl_Group::current(prev_current);
}

void Option_Dialog::refresh(bool reset) {
	_canceled = false;
	_dialog->copy_label(_title);
	// Refresh widget positions and sizes
	fl_font(OS_FONT, OS_FONT_SIZE);
	int dy = 10;
	dy += refresh_content(_width - 20, dy, reset) + 16;
	if (_has_reset) {
		_reset_button->resize(10, dy, 80, 22);
		_reset_button->activate();
		_reset_button->show();
	}
	else {
		_reset_button->hide();
		_reset_button->deactivate();
	}
#ifdef _WIN32
	_ok_button->resize(_width - 184, dy, 80, 22);
	_cancel_button->resize(_width - 90, dy, 80, 22);
#else
	_cancel_button->resize(_width - 184, dy, 80, 22);
	_ok_button->resize(_width - 90, dy, 80, 22);
#endif
	dy += _cancel_button->h() + 10;
	_dialog->size_range(_width, dy, _width, dy);
	_dialog->size(_width, dy);
	_dialog->redraw();
}

void Option_Dialog::show(Fl_Widget *p, bool reset) {
	initialize();
	refresh(reset);
	Fl_Window *prev_grab = Fl::grab();
	Fl::grab(NULL);
	int x = p->x() + (p->w() - _dialog->w()) / 2;
	int y = p->y() + (p->h() - _dialog->h()) / 2;
	_dialog->position(x, y);
	_dialog->user_data(p);
	_ok_button->take_focus();
	_dialog->show();
	while (_dialog->shown()) { Fl::wait(); }
	Fl::grab(prev_grab);
}

void Option_Dialog::close_cb(Fl_Widget *, Option_Dialog *od) {
	od->_dialog->hide();
}

void Option_Dialog::cancel_cb(Fl_Widget *, Option_Dialog *od) {
	od->_canceled = true;
	od->_dialog->hide();
}

Song_Options_Dialog::Song_Options_Dialog(const char *t) : Option_Dialog(400, t) {}

Song_Options_Dialog::~Song_Options_Dialog() {
	delete _song_name;
	delete _looping_checkbox;
	delete _channel_1_checkbox;
	delete _channel_2_checkbox;
	delete _channel_3_checkbox;
	delete _channel_4_checkbox;
	delete _channel_1_loop_tick;
	delete _channel_2_loop_tick;
	delete _channel_3_loop_tick;
	delete _channel_4_loop_tick;
	delete _channel_1_end_tick;
	delete _channel_2_end_tick;
	delete _channel_3_end_tick;
	delete _channel_4_end_tick;
	delete _synchronize_checkbox;
	delete _beats_radio;
	delete _ticks_radio;
}

Song_Options_Dialog::Song_Options Song_Options_Dialog::get_options() {
	Song_Options options;
	options.song_name = _song_name->value();
	options.looping = _looping_checkbox->value();
	options.channel_1 = _channel_1_checkbox->value();
	options.channel_2 = _channel_2_checkbox->value();
	options.channel_3 = _channel_3_checkbox->value();
	options.channel_4 = _channel_4_checkbox->value();
	options.channel_1_loop_tick = std::atoi(_channel_1_loop_tick->value()) * (_beats_radio->value() ? 48 : 1);
	options.channel_2_loop_tick = std::atoi(_channel_2_loop_tick->value()) * (_beats_radio->value() ? 48 : 1);
	options.channel_3_loop_tick = std::atoi(_channel_3_loop_tick->value()) * (_beats_radio->value() ? 48 : 1);
	options.channel_4_loop_tick = std::atoi(_channel_4_loop_tick->value()) * (_beats_radio->value() ? 48 : 1);
	options.channel_1_end_tick = std::atoi(_channel_1_end_tick->value()) * (_beats_radio->value() ? 48 : 1);
	options.channel_2_end_tick = std::atoi(_channel_2_end_tick->value()) * (_beats_radio->value() ? 48 : 1);
	options.channel_3_end_tick = std::atoi(_channel_3_end_tick->value()) * (_beats_radio->value() ? 48 : 1);
	options.channel_4_end_tick = std::atoi(_channel_4_end_tick->value()) * (_beats_radio->value() ? 48 : 1);
	options.result = Result::RESULT_OK;

	if (
		_song_name->active() &&
		(options.song_name.size() == 0 ||
		(options.song_name[0] >= '0' && options.song_name[0] <= '9') ||
		options.song_name.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_") != std::string::npos)
	) {
		options.result = Result::RESULT_BAD_TITLE;
	}
	else if (
		(options.channel_1 && options.channel_1_end_tick == 0) ||
		(options.channel_2 && options.channel_2_end_tick == 0) ||
		(options.channel_3 && options.channel_3_end_tick == 0) ||
		(options.channel_4 && options.channel_4_end_tick == 0)
	) {
		options.result = Result::RESULT_BAD_END;
	}
	else if (
		(options.channel_1 && options.channel_1_loop_tick >= options.channel_1_end_tick) ||
		(options.channel_2 && options.channel_2_loop_tick >= options.channel_2_end_tick) ||
		(options.channel_3 && options.channel_3_loop_tick >= options.channel_3_end_tick) ||
		(options.channel_4 && options.channel_4_loop_tick >= options.channel_4_end_tick)
	) {
		options.result = Result::RESULT_BAD_LOOP;
	}

	return options;
}

void Song_Options_Dialog::set_options(const Song_Options &options) {
	initialize();

	_synchronize_checkbox->clear();
	_ticks_radio->setonly();

	_channel_1_checkbox->set();
	_channel_2_checkbox->set();
	_channel_3_checkbox->set();
	_channel_4_checkbox->set();

	_song_name->value(options.song_name.c_str());
	_song_name->deactivate();

	std::string channel_1_loop_tick_str = std::to_string(options.channel_1_loop_tick);
	std::string channel_2_loop_tick_str = std::to_string(options.channel_2_loop_tick);
	std::string channel_3_loop_tick_str = std::to_string(options.channel_3_loop_tick);
	std::string channel_4_loop_tick_str = std::to_string(options.channel_4_loop_tick);

	_channel_1_loop_tick->value(channel_1_loop_tick_str.c_str());
	_channel_1_loop_tick->activate();
	_channel_2_loop_tick->value(channel_2_loop_tick_str.c_str());
	_channel_2_loop_tick->activate();
	_channel_3_loop_tick->value(channel_3_loop_tick_str.c_str());
	_channel_3_loop_tick->activate();
	_channel_4_loop_tick->value(channel_4_loop_tick_str.c_str());
	_channel_4_loop_tick->activate();

	std::string channel_1_end_tick_str = std::to_string(options.channel_1_end_tick);
	std::string channel_2_end_tick_str = std::to_string(options.channel_2_end_tick);
	std::string channel_3_end_tick_str = std::to_string(options.channel_3_end_tick);
	std::string channel_4_end_tick_str = std::to_string(options.channel_4_end_tick);

	_channel_1_end_tick->value(channel_1_end_tick_str.c_str());
	_channel_1_end_tick->activate();
	_channel_2_end_tick->value(channel_2_end_tick_str.c_str());
	_channel_2_end_tick->activate();
	_channel_3_end_tick->value(channel_3_end_tick_str.c_str());
	_channel_3_end_tick->activate();
	_channel_4_end_tick->value(channel_4_end_tick_str.c_str());
	_channel_4_end_tick->activate();

	_channel_1_checkbox->value(options.channel_1);
	_channel_1_checkbox->deactivate();
	if (!options.channel_1) channel_checkbox_cb(_channel_1_checkbox, this);
	_channel_2_checkbox->value(options.channel_2);
	_channel_2_checkbox->deactivate();
	if (!options.channel_2) channel_checkbox_cb(_channel_2_checkbox, this);
	_channel_3_checkbox->value(options.channel_3);
	_channel_3_checkbox->deactivate();
	if (!options.channel_3) channel_checkbox_cb(_channel_3_checkbox, this);
	_channel_4_checkbox->value(options.channel_4);
	_channel_4_checkbox->deactivate();
	if (!options.channel_4) channel_checkbox_cb(_channel_4_checkbox, this);

	_looping_checkbox->value(options.looping);
	_looping_checkbox->deactivate();
	if (!options.looping) looping_checkbox_cb(nullptr, this);

	int32_t max_loop_tick = std::max({
		options.channel_1_loop_tick,
		options.channel_2_loop_tick,
		options.channel_3_loop_tick,
		options.channel_4_loop_tick
	});
	bool loop_synced =
		(!options.channel_1 || options.channel_1_loop_tick == max_loop_tick) &&
		(!options.channel_2 || options.channel_2_loop_tick == max_loop_tick) &&
		(!options.channel_3 || options.channel_3_loop_tick == max_loop_tick) &&
		(!options.channel_4 || options.channel_4_loop_tick == max_loop_tick);

	int32_t max_end_tick = std::max({
		options.channel_1_end_tick,
		options.channel_2_end_tick,
		options.channel_3_end_tick,
		options.channel_4_end_tick
	});
	bool end_synced =
		(!options.channel_1 || options.channel_1_end_tick == max_end_tick) &&
		(!options.channel_2 || options.channel_2_end_tick == max_end_tick) &&
		(!options.channel_3 || options.channel_3_end_tick == max_end_tick) &&
		(!options.channel_4 || options.channel_4_end_tick == max_end_tick);

	_synchronize_checkbox->value((!options.looping || loop_synced) && end_synced);

	if (
		(!_channel_1_loop_tick->active() || std::atoi(_channel_1_loop_tick->value()) % 48 == 0) &&
		(!_channel_2_loop_tick->active() || std::atoi(_channel_2_loop_tick->value()) % 48 == 0) &&
		(!_channel_3_loop_tick->active() || std::atoi(_channel_3_loop_tick->value()) % 48 == 0) &&
		(!_channel_4_loop_tick->active() || std::atoi(_channel_4_loop_tick->value()) % 48 == 0) &&
		(!_channel_1_end_tick->active() || std::atoi(_channel_1_end_tick->value()) % 48 == 0) &&
		(!_channel_2_end_tick->active() || std::atoi(_channel_2_end_tick->value()) % 48 == 0) &&
		(!_channel_3_end_tick->active() || std::atoi(_channel_3_end_tick->value()) % 48 == 0) &&
		(!_channel_4_end_tick->active() || std::atoi(_channel_4_end_tick->value()) % 48 == 0)
	) {
		_beats_radio->setonly();
		beats_ticks_radio_cb(nullptr, this);
	}
}

const char *Song_Options_Dialog::get_error_message(Result r) {
	switch (r) {
	case Result::RESULT_OK:
		return "OK.";
	case Result::RESULT_BAD_TITLE:
		return "Title is invalid! Title can only contain letters, numbers, and underscores, and must not start with a number.";
	case Result::RESULT_BAD_END:
		return "Channel length must not be zero!";
	case Result::RESULT_BAD_LOOP:
		return "Loop tick must be less than end tick!";
	default:
		return "Unspecified error.";
	}
}

void Song_Options_Dialog::initialize_content() {
	// Populate content group
	_song_name = new OS_Input(0, 0, 0, 0, "Title:");
	_looping_checkbox = new OS_Check_Button(0, 0, 0, 0, "&Looping");
	_channel_1_checkbox = new OS_Check_Button(0, 0, 0, 0, "Channel &1");
	_channel_2_checkbox = new OS_Check_Button(0, 0, 0, 0, "Channel &2");
	_channel_3_checkbox = new OS_Check_Button(0, 0, 0, 0, "Channel &3");
	_channel_4_checkbox = new OS_Check_Button(0, 0, 0, 0, "Channel &4");
	_channel_1_loop_tick = new OS_Int_Input(0, 0, 0, 0, "Loop Point:");
	_channel_2_loop_tick = new OS_Int_Input(0, 0, 0, 0, "Loop Point:");
	_channel_3_loop_tick = new OS_Int_Input(0, 0, 0, 0, "Loop Point:");
	_channel_4_loop_tick = new OS_Int_Input(0, 0, 0, 0, "Loop Point:");
	_channel_1_end_tick = new OS_Int_Input(0, 0, 0, 0, "End Point:");
	_channel_2_end_tick = new OS_Int_Input(0, 0, 0, 0, "End Point:");
	_channel_3_end_tick = new OS_Int_Input(0, 0, 0, 0, "End Point:");
	_channel_4_end_tick = new OS_Int_Input(0, 0, 0, 0, "End Point:");
	_synchronize_checkbox = new OS_Check_Button(0, 0, 0, 0, "&Synchronize Channels");
	_beats_radio = new OS_Radio_Button(0, 0, 0, 0, "&Beats");
	_ticks_radio = new OS_Radio_Button(0, 0, 0, 0, "&Ticks");
	// Initialize content group's children
	_song_name->align(FL_ALIGN_LEFT);
	_looping_checkbox->callback((Fl_Callback *)looping_checkbox_cb, this);
	_channel_1_checkbox->callback((Fl_Callback *)channel_checkbox_cb, this);
	_channel_2_checkbox->callback((Fl_Callback *)channel_checkbox_cb, this);
	_channel_3_checkbox->callback((Fl_Callback *)channel_checkbox_cb, this);
	_channel_4_checkbox->callback((Fl_Callback *)channel_checkbox_cb, this);
	_channel_1_loop_tick->callback((Fl_Callback *)channel_loop_tick_cb, this);
	_channel_1_loop_tick->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE_ALWAYS);
	_channel_1_loop_tick->align(FL_ALIGN_LEFT);
	_channel_2_loop_tick->callback((Fl_Callback *)channel_loop_tick_cb, this);
	_channel_2_loop_tick->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE_ALWAYS);
	_channel_2_loop_tick->align(FL_ALIGN_LEFT);
	_channel_3_loop_tick->callback((Fl_Callback *)channel_loop_tick_cb, this);
	_channel_3_loop_tick->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE_ALWAYS);
	_channel_3_loop_tick->align(FL_ALIGN_LEFT);
	_channel_4_loop_tick->callback((Fl_Callback *)channel_loop_tick_cb, this);
	_channel_4_loop_tick->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE_ALWAYS);
	_channel_4_loop_tick->align(FL_ALIGN_LEFT);
	_channel_1_end_tick->callback((Fl_Callback *)channel_end_tick_cb, this);
	_channel_1_end_tick->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE_ALWAYS);
	_channel_1_end_tick->align(FL_ALIGN_LEFT);
	_channel_2_end_tick->callback((Fl_Callback *)channel_end_tick_cb, this);
	_channel_2_end_tick->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE_ALWAYS);
	_channel_2_end_tick->align(FL_ALIGN_LEFT);
	_channel_3_end_tick->callback((Fl_Callback *)channel_end_tick_cb, this);
	_channel_3_end_tick->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE_ALWAYS);
	_channel_3_end_tick->align(FL_ALIGN_LEFT);
	_channel_4_end_tick->callback((Fl_Callback *)channel_end_tick_cb, this);
	_channel_4_end_tick->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE_ALWAYS);
	_channel_4_end_tick->align(FL_ALIGN_LEFT);
	_synchronize_checkbox->callback((Fl_Callback *)synchronize_checkbox_cb, this);
	_beats_radio->callback((Fl_Callback *)beats_ticks_radio_cb, this);
	_ticks_radio->callback((Fl_Callback *)beats_ticks_radio_cb, this);
}

int Song_Options_Dialog::refresh_content(int ww, int dy, bool reset) {
	int wgt_h = 22, win_m = 10, wgt_m = 4;
	int dx = win_m;
	int ch = wgt_h * 6 + wgt_m * 7;
	_content->resize(dx, dy, ww, ch);

	dx += text_width(_song_name->label(), 2) + wgt_h;
	int wgt_w = 200;
	_song_name->resize(dx, dy, wgt_w, wgt_h);
	if (reset) _song_name->value("UntitledSong");
	if (reset) _song_name->activate();
	dx += wgt_w + wgt_h;
	wgt_w = text_width(_looping_checkbox->label(), 2) + wgt_h;
	_looping_checkbox->resize(dx, dy, wgt_w, wgt_h);
	if (reset) _looping_checkbox->set();
	if (reset) _looping_checkbox->activate();

	dx = win_m;
	dy += wgt_h + wgt_m + wgt_m;
	wgt_w = text_width(_channel_4_checkbox->label(), 2) + wgt_h;
	_channel_1_checkbox->resize(dx, dy, wgt_w, wgt_h);
	if (reset) _channel_1_checkbox->set();
	if (reset) _channel_1_checkbox->activate();
	dx += wgt_w + wgt_m + text_width(_channel_1_loop_tick->label(), 2);
	wgt_w = text_width("9999", 2) + wgt_h;
	_channel_1_loop_tick->resize(dx, dy, wgt_w, wgt_h);
	if (reset) _channel_1_loop_tick->value("0");
	if (reset) _channel_1_loop_tick->activate();
	dx += wgt_w + wgt_h + text_width(_channel_1_end_tick->label(), 2);
	wgt_w = text_width("9999", 2) + wgt_h;
	_channel_1_end_tick->resize(dx, dy, wgt_w, wgt_h);
	if (reset) _channel_1_end_tick->value("64");
	if (reset) _channel_1_end_tick->activate();

	dx = win_m;
	dy += wgt_h + wgt_m;
	wgt_w = text_width(_channel_4_checkbox->label(), 2) + wgt_h;
	_channel_2_checkbox->resize(dx, dy, wgt_w, wgt_h);
	if (reset) _channel_2_checkbox->set();
	if (reset) _channel_2_checkbox->activate();
	dx += wgt_w + wgt_m + text_width(_channel_2_loop_tick->label(), 2);
	wgt_w = text_width("9999", 2) + wgt_h;
	_channel_2_loop_tick->resize(dx, dy, wgt_w, wgt_h);
	if (reset) _channel_2_loop_tick->value("0");
	if (reset) _channel_2_loop_tick->activate();
	dx += wgt_w + wgt_h + text_width(_channel_2_end_tick->label(), 2);
	wgt_w = text_width("9999", 2) + wgt_h;
	_channel_2_end_tick->resize(dx, dy, wgt_w, wgt_h);
	if (reset) _channel_2_end_tick->value("64");
	if (reset) _channel_2_end_tick->activate();

	dx = win_m;
	dy += wgt_h + wgt_m;
	wgt_w = text_width(_channel_4_checkbox->label(), 2) + wgt_h;
	_channel_3_checkbox->resize(dx, dy, wgt_w, wgt_h);
	if (reset) _channel_3_checkbox->set();
	if (reset) _channel_3_checkbox->activate();
	dx += wgt_w + wgt_m + text_width(_channel_3_loop_tick->label(), 2);
	wgt_w = text_width("9999", 2) + wgt_h;
	_channel_3_loop_tick->resize(dx, dy, wgt_w, wgt_h);
	if (reset) _channel_3_loop_tick->value("0");
	if (reset) _channel_3_loop_tick->activate();
	dx += wgt_w + wgt_h + text_width(_channel_3_end_tick->label(), 2);
	wgt_w = text_width("9999", 2) + wgt_h;
	_channel_3_end_tick->resize(dx, dy, wgt_w, wgt_h);
	if (reset) _channel_3_end_tick->value("64");
	if (reset) _channel_3_end_tick->activate();

	dx = win_m;
	dy += wgt_h + wgt_m;
	wgt_w = text_width(_channel_4_checkbox->label(), 2) + wgt_h;
	_channel_4_checkbox->resize(dx, dy, wgt_w, wgt_h);
	if (reset) _channel_4_checkbox->set();
	if (reset) _channel_4_checkbox->activate();
	dx += wgt_w + wgt_m + text_width(_channel_4_loop_tick->label(), 2);
	wgt_w = text_width("9999", 2) + wgt_h;
	_channel_4_loop_tick->resize(dx, dy, wgt_w, wgt_h);
	if (reset) _channel_4_loop_tick->value("0");
	if (reset) _channel_4_loop_tick->activate();
	dx += wgt_w + wgt_h + text_width(_channel_4_end_tick->label(), 2);
	wgt_w = text_width("9999", 2) + wgt_h;
	_channel_4_end_tick->resize(dx, dy, wgt_w, wgt_h);
	if (reset) _channel_4_end_tick->value("64");
	if (reset) _channel_4_end_tick->activate();

	dx = win_m;
	dy += wgt_h + wgt_m + wgt_m;
	wgt_w = text_width(_synchronize_checkbox->label(), 2) + wgt_h;
	_synchronize_checkbox->resize(dx, dy, wgt_w, wgt_h);
	if (reset) _synchronize_checkbox->set();
	dx += wgt_w + wgt_h + wgt_h;
	wgt_w = text_width(_beats_radio->label(), 2) + wgt_h;
	_beats_radio->resize(dx, dy, wgt_w, wgt_h);
	if (reset) _beats_radio->set();
	dx += wgt_w;
	wgt_w = text_width(_ticks_radio->label(), 2) + wgt_h;
	_ticks_radio->resize(dx, dy, wgt_w, wgt_h);
	if (reset) _ticks_radio->clear();

	return ch;
}

void Song_Options_Dialog::looping_checkbox_cb(OS_Check_Button *, Song_Options_Dialog *sod) {
	if (sod->_looping_checkbox->value()) {
		if (sod->_channel_1_checkbox->value()) {
			sod->_channel_1_loop_tick->activate();
			sod->_channel_1_loop_tick->value("0");
		}
		if (sod->_channel_2_checkbox->value()) {
			sod->_channel_2_loop_tick->activate();
			sod->_channel_2_loop_tick->value("0");
		}
		if (sod->_channel_3_checkbox->value()) {
			sod->_channel_3_loop_tick->activate();
			sod->_channel_3_loop_tick->value("0");
		}
		if (sod->_channel_4_checkbox->value()) {
			sod->_channel_4_loop_tick->activate();
			sod->_channel_4_loop_tick->value("0");
		}
	}
	else {
		sod->_channel_1_loop_tick->deactivate();
		sod->_channel_1_loop_tick->value("");
		sod->_channel_2_loop_tick->deactivate();
		sod->_channel_2_loop_tick->value("");
		sod->_channel_3_loop_tick->deactivate();
		sod->_channel_3_loop_tick->value("");
		sod->_channel_4_loop_tick->deactivate();
		sod->_channel_4_loop_tick->value("");
	}
}

void Song_Options_Dialog::channel_checkbox_cb(OS_Check_Button *c, Song_Options_Dialog *sod) {
	if (
		!sod->_channel_1_checkbox->value() &&
		!sod->_channel_2_checkbox->value() &&
		!sod->_channel_3_checkbox->value() &&
		!sod->_channel_4_checkbox->value()
	) {
		c->set();
		return;
	}

	if (c->value()) {
		if (c == sod->_channel_1_checkbox) {
			if (sod->_looping_checkbox->value()) {
				sod->_channel_1_loop_tick->activate();
				if (!sod->_synchronize_checkbox->value()) {
					sod->_channel_1_loop_tick->value("0");
				}
			}
			sod->_channel_1_end_tick->activate();
			if (!sod->_synchronize_checkbox->value()) {
				sod->_channel_1_end_tick->value(sod->_beats_radio->value() ? "64" : "3072");
			}
		}
		if (c == sod->_channel_2_checkbox) {
			if (sod->_looping_checkbox->value()) {
				sod->_channel_2_loop_tick->activate();
				if (!sod->_synchronize_checkbox->value()) {
					sod->_channel_2_loop_tick->value("0");
				}
			}
			sod->_channel_2_end_tick->activate();
			if (!sod->_synchronize_checkbox->value()) {
				sod->_channel_2_end_tick->value(sod->_beats_radio->value() ? "64" : "3072");
			}
		}
		if (c == sod->_channel_3_checkbox) {
			if (sod->_looping_checkbox->value()) {
				sod->_channel_3_loop_tick->activate();
				if (!sod->_synchronize_checkbox->value()) {
					sod->_channel_3_loop_tick->value("0");
				}
			}
			sod->_channel_3_end_tick->activate();
			if (!sod->_synchronize_checkbox->value()) {
				sod->_channel_3_end_tick->value(sod->_beats_radio->value() ? "64" : "3072");
			}
		}
		if (c == sod->_channel_4_checkbox) {
			if (sod->_looping_checkbox->value()) {
				sod->_channel_4_loop_tick->activate();
				if (!sod->_synchronize_checkbox->value()) {
					sod->_channel_4_loop_tick->value("0");
				}
			}
			sod->_channel_4_end_tick->activate();
			if (!sod->_synchronize_checkbox->value()) {
				sod->_channel_4_end_tick->value(sod->_beats_radio->value() ? "64" : "3072");
			}
		}
		synchronize_checkbox_cb(nullptr, sod);
	}
	else {
		if (c == sod->_channel_1_checkbox) {
			sod->_channel_1_loop_tick->deactivate();
			sod->_channel_1_loop_tick->value("");
			sod->_channel_1_end_tick->deactivate();
			sod->_channel_1_end_tick->value("");
		}
		if (c == sod->_channel_2_checkbox) {
			sod->_channel_2_loop_tick->deactivate();
			sod->_channel_2_loop_tick->value("");
			sod->_channel_2_end_tick->deactivate();
			sod->_channel_2_end_tick->value("");
		}
		if (c == sod->_channel_3_checkbox) {
			sod->_channel_3_loop_tick->deactivate();
			sod->_channel_3_loop_tick->value("");
			sod->_channel_3_end_tick->deactivate();
			sod->_channel_3_end_tick->value("");
		}
		if (c == sod->_channel_4_checkbox) {
			sod->_channel_4_loop_tick->deactivate();
			sod->_channel_4_loop_tick->value("");
			sod->_channel_4_end_tick->deactivate();
			sod->_channel_4_end_tick->value("");
		}
	}
}

void Song_Options_Dialog::channel_loop_tick_cb(OS_Int_Input *i, Song_Options_Dialog *sod) {
	if (!i->active()) return;
	if (
		(strlen(i->value()) == 0 && Fl::focus() != i) ||
		(strlen(i->value()) > 0 && i->value()[0] == '-') ||
		(strlen(i->value()) > 1 && i->value()[0] == '0' && i->value()[1] == 'x')
	) {
		i->value("0");
	}
	if (sod->_synchronize_checkbox->value()) {
		if (sod->_channel_1_checkbox->value()) {
			sod->_channel_1_loop_tick->value(i->value());
		}
		if (sod->_channel_2_checkbox->value()) {
			sod->_channel_2_loop_tick->value(i->value());
		}
		if (sod->_channel_3_checkbox->value()) {
			sod->_channel_3_loop_tick->value(i->value());
		}
		if (sod->_channel_4_checkbox->value()) {
			sod->_channel_4_loop_tick->value(i->value());
		}
	}
}

void Song_Options_Dialog::channel_end_tick_cb(OS_Int_Input *i, Song_Options_Dialog *sod) {
	if (!i->active()) return;
	if (
		(strlen(i->value()) == 0 && Fl::focus() != i) ||
		(strlen(i->value()) > 0 && i->value()[0] == '-') ||
		(strlen(i->value()) > 1 && i->value()[0] == '0' && i->value()[1] == 'x')
	) {
		i->value("0");
	}
	if (sod->_synchronize_checkbox->value()) {
		if (sod->_channel_1_checkbox->value()) {
			sod->_channel_1_end_tick->value(i->value());
		}
		if (sod->_channel_2_checkbox->value()) {
			sod->_channel_2_end_tick->value(i->value());
		}
		if (sod->_channel_3_checkbox->value()) {
			sod->_channel_3_end_tick->value(i->value());
		}
		if (sod->_channel_4_checkbox->value()) {
			sod->_channel_4_end_tick->value(i->value());
		}
	}
}

void Song_Options_Dialog::synchronize_checkbox_cb(OS_Check_Button *, Song_Options_Dialog *sod) {
	auto get_first_loop_tick = [&]() {
		if (sod->_channel_1_checkbox->value() && strlen(sod->_channel_1_loop_tick->value())) {
			return sod->_channel_1_loop_tick->value();
		}
		if (sod->_channel_2_checkbox->value() && strlen(sod->_channel_2_loop_tick->value())) {
			return sod->_channel_2_loop_tick->value();
		}
		if (sod->_channel_3_checkbox->value() && strlen(sod->_channel_3_loop_tick->value())) {
			return sod->_channel_3_loop_tick->value();
		}
		if (sod->_channel_4_checkbox->value() && strlen(sod->_channel_4_loop_tick->value())) {
			return sod->_channel_4_loop_tick->value();
		}
		return "0";
	};
	auto get_first_end_tick = [&]() {
		if (sod->_channel_1_checkbox->value() && strlen(sod->_channel_1_end_tick->value())) {
			return sod->_channel_1_end_tick->value();
		}
		if (sod->_channel_2_checkbox->value() && strlen(sod->_channel_2_end_tick->value())) {
			return sod->_channel_2_end_tick->value();
		}
		if (sod->_channel_3_checkbox->value() && strlen(sod->_channel_3_end_tick->value())) {
			return sod->_channel_3_end_tick->value();
		}
		if (sod->_channel_4_checkbox->value() && strlen(sod->_channel_4_end_tick->value())) {
			return sod->_channel_4_end_tick->value();
		}
		return sod->_beats_radio->value() ? "64" : "3072";
	};

	if (sod->_synchronize_checkbox->value()) {
		if (sod->_looping_checkbox->value()) {
			const char *loop_tick = get_first_loop_tick();
			if (sod->_channel_1_checkbox->value()) {
				sod->_channel_1_loop_tick->value(loop_tick);
			}
			if (sod->_channel_2_checkbox->value()) {
				sod->_channel_2_loop_tick->value(loop_tick);
			}
			if (sod->_channel_3_checkbox->value()) {
				sod->_channel_3_loop_tick->value(loop_tick);
			}
			if (sod->_channel_4_checkbox->value()) {
				sod->_channel_4_loop_tick->value(loop_tick);
			}
		}
		const char *end_tick = get_first_end_tick();
		if (sod->_channel_1_checkbox->value()) {
			sod->_channel_1_end_tick->value(end_tick);
		}
		if (sod->_channel_2_checkbox->value()) {
			sod->_channel_2_end_tick->value(end_tick);
		}
		if (sod->_channel_3_checkbox->value()) {
			sod->_channel_3_end_tick->value(end_tick);
		}
		if (sod->_channel_4_checkbox->value()) {
			sod->_channel_4_end_tick->value(end_tick);
		}
	}
}

void Song_Options_Dialog::beats_ticks_radio_cb(OS_Radio_Button *, Song_Options_Dialog *sod) {
	const auto ticks_to_beats = [](OS_Int_Input *i) {
		if (!i->active()) return;
		std::string val = std::to_string(std::atoi(i->value()) / 48);
		i->value(val.c_str());
	};
	const auto beats_to_ticks = [](OS_Int_Input *i) {
		if (!i->active()) return;
		std::string val = std::to_string(std::atoi(i->value()) * 48);
		i->value(val.c_str());
	};

	if (sod->_beats_radio->value()) {
		ticks_to_beats(sod->_channel_1_loop_tick);
		ticks_to_beats(sod->_channel_2_loop_tick);
		ticks_to_beats(sod->_channel_3_loop_tick);
		ticks_to_beats(sod->_channel_4_loop_tick);
		ticks_to_beats(sod->_channel_1_end_tick);
		ticks_to_beats(sod->_channel_2_end_tick);
		ticks_to_beats(sod->_channel_3_end_tick);
		ticks_to_beats(sod->_channel_4_end_tick);
	}
	else {
		beats_to_ticks(sod->_channel_1_loop_tick);
		beats_to_ticks(sod->_channel_2_loop_tick);
		beats_to_ticks(sod->_channel_3_loop_tick);
		beats_to_ticks(sod->_channel_4_loop_tick);
		beats_to_ticks(sod->_channel_1_end_tick);
		beats_to_ticks(sod->_channel_2_end_tick);
		beats_to_ticks(sod->_channel_3_end_tick);
		beats_to_ticks(sod->_channel_4_end_tick);
	}
}

Ruler_Config_Dialog::Ruler_Config_Dialog(const char *t) : Option_Dialog(325, t) {
	_has_reset = true;
}

Ruler_Config_Dialog::~Ruler_Config_Dialog() {
	delete _beats_per_measure;
	delete _steps_per_beat;
	delete _ticks_per_step;
	delete _pickup_offset;
}

Ruler_Config_Dialog::Ruler_Options Ruler_Config_Dialog::get_options() {
	Ruler_Options options;

	options.beats_per_measure = (int)_beats_per_measure->value();
	options.steps_per_beat = (int)_steps_per_beat->value();
	options.ticks_per_step = (int)_ticks_per_step->value();
	options.pickup_offset = (int)_pickup_offset->value();

	if (options.beats_per_measure < 1) options.beats_per_measure = 1;
	if (options.steps_per_beat < 1) options.steps_per_beat = 1;
	if (options.ticks_per_step < 1) options.ticks_per_step = 1;
	if (options.pickup_offset < 0) options.pickup_offset = 0;

	return options;
}

void Ruler_Config_Dialog::set_options(const Ruler_Options &options) {
	initialize();

	_beats_per_measure->value(options.beats_per_measure);
	_steps_per_beat->value(options.steps_per_beat);
	_ticks_per_step->value(options.ticks_per_step);
	_pickup_offset->value(options.pickup_offset);
}

void Ruler_Config_Dialog::initialize_content() {
	// Populate content group
	_beats_per_measure = new OS_Spinner(0, 0, 0, 0, "&Beats per Measure:");
	_steps_per_beat = new OS_Spinner(0, 0, 0, 0, "&Steps per Beat:");
	_ticks_per_step = new OS_Spinner(0, 0, 0, 0, "&Ticks per Step:");
	_pickup_offset = new OS_Spinner(0, 0, 0, 0, "&Pickup Offset:");
	// Initialize content group's children
	_beats_per_measure->range(1, 64);
	_beats_per_measure->callback((Fl_Callback *)ruler_config_cb, this);
	_steps_per_beat->range(1, 64);
	_steps_per_beat->callback((Fl_Callback *)ruler_config_cb, this);
	_ticks_per_step->range(4, 32);
	_ticks_per_step->callback((Fl_Callback *)ruler_config_cb, this);
	_pickup_offset->range(0, 64);
	_pickup_offset->callback((Fl_Callback *)ruler_config_cb, this);
}

int Ruler_Config_Dialog::refresh_content(int ww, int dy, bool reset) {
	int wgt_h = 22, win_m = 10, wgt_m = 4;
	int dx = win_m;
	int ch = wgt_h * 4 + wgt_m * 6;
	_content->resize(dx, dy, ww, ch);

	int wgt_w = 50;
	dx = ww / 2 + 36;
	_beats_per_measure->resize(dx, dy, wgt_w, wgt_h);
	if (reset) _beats_per_measure->value(4);

	dy += wgt_h + wgt_m + wgt_m;
	_steps_per_beat->resize(dx, dy, wgt_w, wgt_h);
	if (reset) _steps_per_beat->value(4);

	dy += wgt_h + wgt_m + wgt_m;
	_ticks_per_step->resize(dx, dy, wgt_w, wgt_h);
	if (reset) _ticks_per_step->value(12);

	dy += wgt_h + wgt_m + wgt_m;
	_pickup_offset->resize(dx, dy, wgt_w, wgt_h);
	if (reset) _pickup_offset->value(0);

	return ch;
}

void Ruler_Config_Dialog::set_reset_cb() {
	_reset_button->callback((Fl_Callback *)reset_button_cb, this);
}

void Ruler_Config_Dialog::ruler_config_cb(Fl_Widget *, Ruler_Config_Dialog *rcd) {
	Main_Window *mw = (Main_Window *)rcd->_dialog->user_data();
	mw->set_ruler_config(rcd->get_options());
	mw->redraw();
}

void Ruler_Config_Dialog::reset_button_cb(Fl_Widget *, Ruler_Config_Dialog *rcd) {
	rcd->_beats_per_measure->value(4);
	rcd->_steps_per_beat->value(4);
	rcd->_ticks_per_step->value(12);
	rcd->_pickup_offset->value(0);
	rcd->ruler_config_cb(nullptr, rcd);
}
