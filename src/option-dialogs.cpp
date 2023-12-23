#include "option-dialogs.h"

#include "themes.h"

Option_Dialog::Option_Dialog(int w, const char *t) : _width(w), _title(t), _canceled(false),
	_dialog(NULL), _content(NULL), _ok_button(NULL), _cancel_button(NULL) {}

Option_Dialog::~Option_Dialog() {
	delete _dialog;
	delete _content;
	delete _ok_button;
	delete _cancel_button;
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
	Fl_Group::current(prev_current);
}

void Option_Dialog::refresh(bool reset) {
	_canceled = false;
	_dialog->copy_label(_title);
	// Refresh widget positions and sizes
	fl_font(OS_FONT, OS_FONT_SIZE);
	int dy = 10;
	dy += refresh_content(_width - 20, dy, reset) + 16;
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

void Option_Dialog::show(const Fl_Widget *p, bool reset) {
	initialize();
	refresh(reset);
	int x = p->x() + (p->w() - _dialog->w()) / 2;
	int y = p->y() + (p->h() - _dialog->h()) / 2;
	_dialog->position(x, y);
	_ok_button->take_focus();
	_dialog->show();
	while (_dialog->shown()) { Fl::wait(); }
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
}

Song_Options_Dialog::Song_Options Song_Options_Dialog::get_options() {
	Song_Options options;
	options.song_name = _song_name->value();
	options.looping = _looping_checkbox->value();
	options.channel_1 = _channel_1_checkbox->value();
	options.channel_2 = _channel_2_checkbox->value();
	options.channel_3 = _channel_3_checkbox->value();
	options.channel_4 = _channel_4_checkbox->value();
	options.channel_1_loop_tick = std::atoi(_channel_1_loop_tick->value());
	options.channel_2_loop_tick = std::atoi(_channel_2_loop_tick->value());
	options.channel_3_loop_tick = std::atoi(_channel_3_loop_tick->value());
	options.channel_4_loop_tick = std::atoi(_channel_4_loop_tick->value());
	options.channel_1_end_tick = std::atoi(_channel_1_end_tick->value());
	options.channel_2_end_tick = std::atoi(_channel_2_end_tick->value());
	options.channel_3_end_tick = std::atoi(_channel_3_end_tick->value());
	options.channel_4_end_tick = std::atoi(_channel_4_end_tick->value());
	options.result = Result::RESULT_OK;

	if (
		options.song_name.size() == 0 ||
		(options.song_name[0] >= '0' && options.song_name[0] <= '9') ||
		options.song_name.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_") != std::string::npos
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
	_channel_1_loop_tick = new OS_Int_Input(0, 0, 0, 0, "Loop Tick:");
	_channel_2_loop_tick = new OS_Int_Input(0, 0, 0, 0, "Loop Tick:");
	_channel_3_loop_tick = new OS_Int_Input(0, 0, 0, 0, "Loop Tick:");
	_channel_4_loop_tick = new OS_Int_Input(0, 0, 0, 0, "Loop Tick:");
	_channel_1_end_tick = new OS_Int_Input(0, 0, 0, 0, "End Tick:");
	_channel_2_end_tick = new OS_Int_Input(0, 0, 0, 0, "End Tick:");
	_channel_3_end_tick = new OS_Int_Input(0, 0, 0, 0, "End Tick:");
	_channel_4_end_tick = new OS_Int_Input(0, 0, 0, 0, "End Tick:");
	_synchronize_checkbox = new OS_Check_Button(0, 0, 0, 0, "&Synchronize Channels");
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
}

int Song_Options_Dialog::refresh_content(int ww, int dy, bool reset) {
	int wgt_h = 22, win_m = 10, wgt_m = 4;
	int dx = win_m;
	int ch = wgt_h * 6 + wgt_m * 5;
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
	dy += wgt_h + wgt_m;
	wgt_w = text_width(_channel_4_checkbox->label(), 2) + wgt_h;
	_channel_1_checkbox->resize(dx, dy, wgt_w, wgt_h);
	if (reset) _channel_1_checkbox->set();
	if (reset) _channel_1_checkbox->activate();
	dx += wgt_w + wgt_h + text_width(_channel_1_loop_tick->label(), 2);
	wgt_w = text_width("9999", 2) + wgt_h;
	_channel_1_loop_tick->resize(dx, dy, wgt_w, wgt_h);
	if (reset) _channel_1_loop_tick->value("0");
	if (reset) _channel_1_loop_tick->activate();
	dx += wgt_w + wgt_h + text_width(_channel_1_end_tick->label(), 2);
	wgt_w = text_width("9999", 2) + wgt_h;
	_channel_1_end_tick->resize(dx, dy, wgt_w, wgt_h);
	if (reset) _channel_1_end_tick->value("3072");
	if (reset) _channel_1_end_tick->activate();

	dx = win_m;
	dy += wgt_h + wgt_m;
	wgt_w = text_width(_channel_4_checkbox->label(), 2) + wgt_h;
	_channel_2_checkbox->resize(dx, dy, wgt_w, wgt_h);
	if (reset) _channel_2_checkbox->set();
	if (reset) _channel_2_checkbox->activate();
	dx += wgt_w + wgt_h + text_width(_channel_2_loop_tick->label(), 2);
	wgt_w = text_width("9999", 2) + wgt_h;
	_channel_2_loop_tick->resize(dx, dy, wgt_w, wgt_h);
	if (reset) _channel_2_loop_tick->value("0");
	if (reset) _channel_2_loop_tick->activate();
	dx += wgt_w + wgt_h + text_width(_channel_2_end_tick->label(), 2);
	wgt_w = text_width("9999", 2) + wgt_h;
	_channel_2_end_tick->resize(dx, dy, wgt_w, wgt_h);
	if (reset) _channel_2_end_tick->value("3072");
	if (reset) _channel_2_end_tick->activate();

	dx = win_m;
	dy += wgt_h + wgt_m;
	wgt_w = text_width(_channel_4_checkbox->label(), 2) + wgt_h;
	_channel_3_checkbox->resize(dx, dy, wgt_w, wgt_h);
	if (reset) _channel_3_checkbox->set();
	if (reset) _channel_3_checkbox->activate();
	dx += wgt_w + wgt_h + text_width(_channel_3_loop_tick->label(), 2);
	wgt_w = text_width("9999", 2) + wgt_h;
	_channel_3_loop_tick->resize(dx, dy, wgt_w, wgt_h);
	if (reset) _channel_3_loop_tick->value("0");
	if (reset) _channel_3_loop_tick->activate();
	dx += wgt_w + wgt_h + text_width(_channel_3_end_tick->label(), 2);
	wgt_w = text_width("9999", 2) + wgt_h;
	_channel_3_end_tick->resize(dx, dy, wgt_w, wgt_h);
	if (reset) _channel_3_end_tick->value("3072");
	if (reset) _channel_3_end_tick->activate();

	dx = win_m;
	dy += wgt_h + wgt_m;
	wgt_w = text_width(_channel_4_checkbox->label(), 2) + wgt_h;
	_channel_4_checkbox->resize(dx, dy, wgt_w, wgt_h);
	if (reset) _channel_4_checkbox->set();
	if (reset) _channel_4_checkbox->activate();
	dx += wgt_w + wgt_h + text_width(_channel_4_loop_tick->label(), 2);
	wgt_w = text_width("9999", 2) + wgt_h;
	_channel_4_loop_tick->resize(dx, dy, wgt_w, wgt_h);
	if (reset) _channel_4_loop_tick->value("0");
	if (reset) _channel_4_loop_tick->activate();
	dx += wgt_w + wgt_h + text_width(_channel_4_end_tick->label(), 2);
	wgt_w = text_width("9999", 2) + wgt_h;
	_channel_4_end_tick->resize(dx, dy, wgt_w, wgt_h);
	if (reset) _channel_4_end_tick->value("3072");
	if (reset) _channel_4_end_tick->activate();

	dx = win_m;
	dy += wgt_h + wgt_m;
	wgt_w = text_width(_synchronize_checkbox->label(), 2) + wgt_h;
	_synchronize_checkbox->resize(dx, dy, wgt_w, wgt_h);
	if (reset) _synchronize_checkbox->set();

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
				sod->_channel_1_end_tick->value("3072");
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
				sod->_channel_2_end_tick->value("3072");
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
				sod->_channel_3_end_tick->value("3072");
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
				sod->_channel_4_end_tick->value("3072");
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
		return "3072";
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
