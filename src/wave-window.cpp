#include <algorithm>
#include <cassert>
#include <iomanip>

#include "themes.h"
#include "utils.h"
#include "wave-window.h"

const Fl_Color WAVE_COLOR = fl_rgb_color(0, 165, 0);

Wave_Graph::Wave_Graph(int x, int y, int w, int h, const char *l) : Fl_Box(x, y, w, h, l) {}

void Wave_Graph::draw() {
	draw_box();

	Wave_Window *ww = (Wave_Window *)user_data();
	Wave *wave = ww->wave();

	int x_step = w() / NUM_WAVE_SAMPLES;
	int y_step = h() / 16;

	const auto draw_grid_lines = [&](Fl_Color c) {
		fl_color(c);
		fl_yxline(x()+2 + x_step *  8, y()+2, y()-2 + h()-1);
		fl_yxline(x()+2 + x_step * 16, y()+2, y()-2 + h()-1);
		fl_yxline(x()+2 + x_step * 24, y()+2, y()-2 + h()-1);
		fl_xyline(x()+2, y()-2 + h() - y_step *  4, x()-2 + w()-1);
		fl_xyline(x()+2, y()-2 + h() - y_step * 12, x()-2 + w()-1);
	};

	Fl_Color grid_color = fl_darker(FL_BACKGROUND2_COLOR);
	draw_grid_lines(grid_color);

	if (wave) for (size_t i = 0; i < NUM_WAVE_SAMPLES; ++i) {
		int x_pos = x()+2 + x_step * (int)i;
		int y_pos = y()-2 + h() - y_step * (wave->at(i) + 1);
		fl_rectf(x_pos, y_pos, x_step, y_step, WAVE_COLOR);
		if (wave->at(i) < 7 || wave->at(i) > 8) {
			y_pos = y()-2 + h() - y_step * (wave->at(i) < 7 ? 8 : wave->at(i));
			int bar_height = y_step * (wave->at(i) < 7 ? (7 - wave->at(i)) : (wave->at(i) - 8));
			fl_rectf(x_pos, y_pos, x_step, bar_height, fl_color_average(WAVE_COLOR, FL_BACKGROUND2_COLOR, 0.33f));
			fl_push_clip(x_pos, y_pos, x_step, bar_height);
			draw_grid_lines(fl_color_average(grid_color, fl_color(), 0.33f));
			fl_pop_clip();
		}
	}

	fl_color(grid_color);
	fl_xyline(x()+2, y()-2 + h() - y_step * 8, x()-2 + w()-1);
}

int Wave_Graph::handle(int event) {
	switch (event) {
	case FL_ENTER:
	case FL_LEAVE:
		return 1;
	case FL_PUSH:
	case FL_DRAG: {
		Wave_Window *ww = (Wave_Window *)user_data();
		Wave *wave = ww->wave();
		if (wave) {
			int x_step = w() / NUM_WAVE_SAMPLES;
			int y_step = h() / 16;
			int x_pos = (Fl::event_x() - x() - 2) / x_step;
			int y_pos = 15 - (Fl::event_y() - y() - 2) / y_step;
			if (x_pos >= 0 && x_pos < NUM_WAVE_SAMPLES && y_pos >= 0 && y_pos < 16 && wave->at(x_pos) != y_pos) {
				wave->at(x_pos) = y_pos;
				ww->redraw_wave();
				ww->regenerate_mod();
			}
		}
		return 1;
	}
	case FL_RELEASE:
		return 1;
	}
	return 0;
}

Wave_Display::Wave_Display(int x, int y, int w, int h, const char *l) : Fl_Box(x, y, w, h, l) {}

void Wave_Display::draw() {
	draw_box();

	Wave_Window *ww = (Wave_Window *)user_data();
	Wave *wave = ww->wave();
	if (!wave) return;

	int x_step = w() / NUM_WAVE_SAMPLES;

	fl_font(FL_COURIER, 12);
	fl_color(FL_FOREGROUND_COLOR);
	for (size_t i = 0; i < NUM_WAVE_SAMPLES; ++i) {
		int x_pos = x()+2 + x_step * (int)i;
		char buffer[8];
		snprintf(buffer, 8, "%X", wave->at(i));
		fl_draw(buffer, x_pos, y(), x_step, h(), FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
	}
}

int Wave_Display::handle(int event) {
	switch (event) {
	case FL_ENTER:
	case FL_LEAVE:
		return 1;
	case FL_MOUSEWHEEL: {
		if (!Fl::event_inside(this)) return 0;
		Wave_Window *ww = (Wave_Window *)user_data();
		Wave *wave = ww->wave();
		if (wave) {
			int x_step = w() / NUM_WAVE_SAMPLES;
			int x_pos = (Fl::event_x() - x() - 2) / x_step;
			int dir = -Fl::event_dy();
			if (dir != 0 && x_pos >= 0 && x_pos < NUM_WAVE_SAMPLES && wave->at(x_pos)+dir >= 0 && wave->at(x_pos)+dir < 16) {
				wave->at(x_pos) += dir;
				ww->redraw_wave();
				ww->regenerate_mod();
			}
		}
		return 1;
	}
	}
	return 0;
}

Wave_Window::Wave_Window(int x, int y) : _dx(x), _dy(y) {}

Wave_Window::~Wave_Window() {
	stop_audio_thread();
	if (_mod) delete _mod;
	delete _confirm_dialog;
	delete _success_dialog;
	delete _error_dialog;
	delete _window;
}

void Wave_Window::initialize() {
	if (_window) { return; }
	Fl_Group *prev_current = Fl_Group::current();
	Fl_Group::current(NULL);
	// Populate window
	_window = new Fl_Double_Window(_dx, _dy, 608, 362, "Wave Editor");
	_add_button = new OS_Button(10, 10, 21, 21, "@+");
	_remove_button = new OS_Button(35, 10, 21, 21, "@1+");
	_up_button = new OS_Button(64, 10, 21, 21, "@8>");
	_down_button = new OS_Button(89, 10, 21, 21, "@2>");
	_wave_browser = new OS_Browser(10, 35, 100, 260);
	_wave_graph = new Wave_Graph(120, 35, 388, 260);
	_wave_display = new Wave_Display(120, 300, 388, 20);
	_play_button = new OS_Light_Button(343, 10, 65, 22, "Play");
	_pitch_input = new Dropdown(413, 10, 45, 22);
	_octave_input = new Dropdown(463, 10, 45, 22);
	_copy_button = new OS_Button(518, 35, 80, 22, "Copy");
	_paste_button = new OS_Button(518, 62, 80, 22, "Paste");
	_phase_left_button = new OS_Button(518, 94, 80, 22, "Phase @-2<<");
	_phase_right_button = new OS_Button(518, 121, 80, 22, "Phase @-2>>");
	_shift_up_button = new OS_Button(518, 153, 80, 22, "Shift @-1^_");
	_shift_down_button = new OS_Button(518, 180, 80, 22, "Shift @-1v_");
	_flip_button = new OS_Button(518, 212, 80, 22, "Flip @<->");
	_invert_button = new OS_Button(518, 239, 80, 22, "Invert @^|v");
	_save_button = new Default_Button(10, 330, 80, 22, "Save");
	_revert_button = new OS_Button(100, 330, 80, 22, "Revert");
	_close_button = new OS_Button(518, 330, 80, 22, "Close");
	_window->end();
	_error_dialog = new Modal_Dialog(_window, "Error", Modal_Dialog::Icon::ERROR_ICON);
	_success_dialog = new Modal_Dialog(_window, "Success", Modal_Dialog::Icon::SUCCESS_ICON);
	_confirm_dialog = new Modal_Dialog(_window, "Warning", Modal_Dialog::Icon::WARNING_ICON, true);
	// Initialize window
	_window->box(OS_BG_BOX);
	_window->callback((Fl_Callback *)cancel_cb, this);
	_window->set_modal();
	// Initialize window's children
	_add_button->tooltip("New wave");
	_add_button->callback((Fl_Callback *)add_wave_cb, this);
	_remove_button->tooltip("Delete wave");
	_remove_button->callback((Fl_Callback *)remove_wave_cb, this);
	_up_button->tooltip("Move up");
	_up_button->callback((Fl_Callback *)move_wave_up_cb, this);
	_down_button->tooltip("Move down");
	_down_button->callback((Fl_Callback *)move_wave_down_cb, this);
	_wave_browser->callback((Fl_Callback *)select_wave_cb, this);
	_wave_graph->box(OS_SWATCH_BOX);
	_wave_graph->color(FL_BACKGROUND2_COLOR);
	_wave_graph->user_data(this);
	_wave_display->box(OS_SWATCH_BOX);
	_wave_display->color(FL_BACKGROUND2_COLOR);
	_wave_display->user_data(this);
	_play_button->tooltip("Play (Spacebar)");
	_play_button->shortcut(' ');
	_play_button->callback((Fl_Callback *)play_cb, this);
	for (int i = 1; i <= NUM_PITCHES; ++i) {
		_pitch_input->add(PITCH_DISPLAY_NAMES[i]);
	}
	_pitch_input->tooltip("Pitch");
	_pitch_input->scroll_enabled(true);
	_pitch_input->callback((Fl_Callback *)pitch_cb, this);
	_octave_input->add("1|2|3|4|5|6|7|8");
	_octave_input->tooltip("Octave");
	_octave_input->scroll_enabled(true);
	_octave_input->callback((Fl_Callback *)octave_cb, this);
	_copy_button->callback((Fl_Callback *)copy_cb, this);
	_paste_button->callback((Fl_Callback *)paste_cb, this);
	_phase_left_button->callback((Fl_Callback *)phase_left_cb, this);
	_phase_right_button->callback((Fl_Callback *)phase_right_cb, this);
	_shift_up_button->callback((Fl_Callback *)shift_up_cb, this);
	_shift_down_button->callback((Fl_Callback *)shift_down_cb, this);
	_flip_button->callback((Fl_Callback *)flip_cb, this);
	_invert_button->callback((Fl_Callback *)invert_cb, this);
	_save_button->shortcut(0);
	_save_button->callback((Fl_Callback *)save_cb, this);
	_revert_button->callback((Fl_Callback *)revert_cb, this);
	_close_button->callback((Fl_Callback *)close_cb, this);
	_error_dialog->width_range(280, 500);
	_success_dialog->width_range(280, 500);
	_confirm_dialog->width_range(280, 500);
	Fl_Group::current(prev_current);
}

void Wave_Window::refresh() {
	_canceled = false;
	_wave_browser->select(1);
	_play_button->value(0);
	_pitch_input->value(0);
	_octave_input->value(3);
	std::fill(RANGE(_clipboard), (uint8_t)-1);
	select_wave_cb(nullptr, this);
}

void Wave_Window::start_audio_thread() {
	_audio_kill_signal = std::promise<void>();
	std::future<void> kill_future = _audio_kill_signal.get_future();
	_audio_thread = std::thread(&playback_thread, this, std::move(kill_future));
}

void Wave_Window::stop_audio_thread() {
	if (_audio_thread.joinable()) {
		_audio_mutex.lock();
		_audio_kill_signal.set_value();
		_audio_thread.join();
		_audio_mutex.unlock();
	}
}

bool Wave_Window::modified() {
	if (_waves.num_waves != _saved_waves.num_waves) return true;

	for (int32_t i = 0; i < _waves.num_waves; ++i) {
		for (size_t j = 0; j < NUM_WAVE_SAMPLES; ++j) {
			if (_waves.waves[i][j] != _saved_waves.waves[i][j]) return true;
		}
	}

	return false;
}

bool Wave_Window::write_waves(const char *f) {
	std::ofstream ofs;
	open_ofstream(ofs, f);
	if (!ofs.good()) { return false; }

	if (_waves.waves_label.size()) {
		ofs << _waves.waves_label << ":\n";
	}

	for (int32_t i = 0; i < _waves.num_waves; ++i) {
		if (_waves.uses_dn) {
			ofs << "\tdn ";
			for (size_t j = 0; j < NUM_WAVE_SAMPLES; ++j) {
				ofs << std::setw(2) << (int)_waves.waves[i][j];
				if (j < NUM_WAVE_SAMPLES - 1) {
					ofs << ", ";
				}
			}
			ofs << "\n";
		}
		else {
			ofs << "\tdb ";
			for (size_t j = 0; j < NUM_WAVE_SAMPLES; j += 2) {
				ofs << "$" << std::hex << (int)_waves.waves[i][j] << (int)_waves.waves[i][j+1];
				if (j < NUM_WAVE_SAMPLES - 2) {
					ofs << ", ";
				}
			}
			ofs << "\n";
		}
	}

	ofs.close();
	return true;
}

Wave *Wave_Window::wave() {
	if (_selected_wave == 0) return nullptr;
	return &_waves.waves[_selected_wave-1];
}

void Wave_Window::waves(const Waves &w) {
	initialize();

	_waves = w;

	_waves.waves.resize(16);
	assert(_waves.num_waves >= 1 && _waves.num_waves <= 16);

	_wave_browser->clear();
	char buffer[16];
	for (int32_t i = 0; i < _waves.num_waves; ++i) {
		snprintf(buffer, 16, "Wave %d", i);
		_wave_browser->add(buffer);
	}

	if (_waves.num_waves == 16) {
		_add_button->deactivate();
	}
	else {
		_add_button->activate();
	}

	_saved_waves = _waves;
}

void Wave_Window::show(const Fl_Widget *p) {
	initialize();
	refresh();
	Fl_Window *prev_grab = Fl::grab();
	Fl::grab(NULL);
	int x = p->x() + (p->w() - _window->w()) / 2;
	int y = p->y() + (p->h() - _window->h()) / 2;
	_window->position(x, y);
	_wave_browser->take_focus();
	_window->show();
	while (_window->shown()) { Fl::wait(); }
	Fl::grab(prev_grab);
}

void Wave_Window::redraw_wave() {
	_wave_graph->redraw();
	_wave_display->redraw();
}

void Wave_Window::regenerate_mod() {
	if (_mod && _audio_thread.joinable()) {
		_audio_mutex.lock();
		_mod_channel = -1;
		_mod->regenerate_it_module(_waves.waves, {}, {}, -1);
		_mod->start();
		_audio_mutex.unlock();
	}
}

void Wave_Window::close_cb(Fl_Widget *, Wave_Window *ww) {
	if (ww->modified()) {
		std::string msg = fl_filename_name(ww->_waves.waves_file.c_str());
		msg = msg + " has unsaved changes!\n\n"
			"Close anyway?";
		ww->_confirm_dialog->message(msg);
		ww->_confirm_dialog->show(ww->_window);
		if (ww->_confirm_dialog->canceled()) { return; }
	}

	ww->stop_audio_thread();
	if (ww->_mod) delete ww->_mod;
	ww->_mod = nullptr;
	ww->_window->hide();
}

void Wave_Window::cancel_cb(Fl_Widget *w, Wave_Window *ww) {
	ww->_canceled = true;
	close_cb(w, ww);
}

void Wave_Window::save_cb(Fl_Widget *, Wave_Window *ww) {
	const char *filename = ww->_waves.waves_file.c_str();
	const char *basename = fl_filename_name(filename);

	if (ww->modified() && !ww->write_waves(filename)) {
		std::string msg = "Could not write to ";
		msg = msg + basename + "!";
		ww->_error_dialog->message(msg);
		ww->_error_dialog->show(ww->_window);
		return;
	}

	std::string msg = "Saved ";
	msg = msg + basename + "!";
	ww->_success_dialog->message(msg);
	ww->_success_dialog->show(ww->_window);

	ww->_saved_waves = ww->_waves;
}

void Wave_Window::revert_cb(Fl_Widget *, Wave_Window *ww) {
	if (!ww->modified()) return;

	std::string msg = "Discard unsaved changes?";
	ww->_confirm_dialog->message(msg);
	ww->_confirm_dialog->show(ww->_window);
	if (ww->_confirm_dialog->canceled()) { return; }

	ww->waves(ww->_saved_waves);

	ww->_wave_browser->select(1);
	select_wave_cb(nullptr, ww);

	ww->regenerate_mod();
}

void Wave_Window::add_wave_cb(Fl_Widget *, Wave_Window *ww) {
	if (ww->_waves.num_waves == 16) return;

	std::fill(RANGE(ww->_waves.waves[ww->_waves.num_waves]), 8);
	char buffer[16];
	snprintf(buffer, 16, "Wave %d", ww->_waves.num_waves);
	ww->_wave_browser->add(buffer);
	ww->_wave_browser->select(ww->_waves.num_waves + 1);
	ww->_waves.num_waves += 1;
	if (ww->_waves.num_waves == 16) {
		ww->_add_button->deactivate();
	}
	select_wave_cb(nullptr, ww);

	ww->regenerate_mod();
}

void Wave_Window::remove_wave_cb(Fl_Widget *, Wave_Window *ww) {
	if (!ww->_selected_wave || ww->_waves.num_waves == 1) return;

	for (int i = ww->_selected_wave; i < ww->_waves.num_waves; ++i) {
		ww->_waves.waves[i-1] = ww->_waves.waves[i];
	}
	ww->_waves.waves[ww->_waves.num_waves - 1] = { 0 };
	ww->_wave_browser->deselect();
	ww->_wave_browser->remove(ww->_waves.num_waves);
	ww->_waves.num_waves -= 1;
	ww->_add_button->activate();
	select_wave_cb(nullptr, ww);

	ww->regenerate_mod();
}

void Wave_Window::move_wave_up_cb(Fl_Widget *, Wave_Window *ww) {
	if (!ww->_selected_wave || ww->_selected_wave == 1) return;

	std::swap(ww->_waves.waves[ww->_selected_wave - 1], ww->_waves.waves[ww->_selected_wave - 2]);
	ww->_wave_browser->select(ww->_selected_wave - 1);
	select_wave_cb(nullptr, ww);

	ww->regenerate_mod();
}

void Wave_Window::move_wave_down_cb(Fl_Widget *, Wave_Window *ww) {
	if (!ww->_selected_wave || ww->_selected_wave == ww->_waves.num_waves) return;

	std::swap(ww->_waves.waves[ww->_selected_wave - 1], ww->_waves.waves[ww->_selected_wave]);
	ww->_wave_browser->select(ww->_selected_wave + 1);
	select_wave_cb(nullptr, ww);

	ww->regenerate_mod();
}

void Wave_Window::select_wave_cb(Fl_Widget *, Wave_Window *ww) {
	ww->_selected_wave = ww->_wave_browser->value();
	if (!ww->_selected_wave || ww->_waves.num_waves == 1) {
		ww->_remove_button->deactivate();
	}
	else {
		ww->_remove_button->activate();
	}
	if (!ww->_selected_wave || ww->_selected_wave == 1) {
		ww->_up_button->deactivate();
	}
	else {
		ww->_up_button->activate();
	}
	if (!ww->_selected_wave || ww->_selected_wave == ww->_waves.num_waves) {
		ww->_down_button->deactivate();
	}
	else {
		ww->_down_button->activate();
	}
	if (!ww->_selected_wave) {
		ww->_copy_button->deactivate();
		ww->_paste_button->deactivate();
		ww->_phase_left_button->deactivate();
		ww->_phase_right_button->deactivate();
		ww->_shift_up_button->deactivate();
		ww->_shift_down_button->deactivate();
		ww->_flip_button->deactivate();
		ww->_invert_button->deactivate();
	}
	else {
		ww->_copy_button->activate();
		if (ww->_clipboard[0] == (uint8_t)-1) {
			ww->_paste_button->deactivate();
		}
		else {
			ww->_paste_button->activate();
		}
		ww->_phase_left_button->activate();
		ww->_phase_right_button->activate();
		ww->_shift_up_button->activate();
		ww->_shift_down_button->activate();
		ww->_flip_button->activate();
		ww->_invert_button->activate();
	}
	ww->redraw_wave();

	if (ww->_audio_thread.joinable()) {
		ww->_audio_mutex.lock();
		ww->_playing_instrument = ww->_selected_wave;
		ww->_audio_mutex.unlock();
	}
}

void Wave_Window::play_cb(Fl_Widget *, Wave_Window *ww) {
	ww->stop_audio_thread();
	if (ww->_mod) delete ww->_mod;
	ww->_mod = nullptr;

	if (ww->_play_button->value()) {
		ww->_playing_pitch = (Pitch)(ww->_pitch_input->value() + 1);
		ww->_playing_octave = ww->_octave_input->value() + 1;
		ww->_playing_instrument = ww->_selected_wave;
		ww->_mod_channel = -1;

		ww->_mod = new IT_Module(ww->_waves.waves, {}, {}, -1);
		ww->_mod->start();

		ww->start_audio_thread();
	}
}

void Wave_Window::pitch_cb(Fl_Widget *, Wave_Window *ww) {
	if (ww->_audio_thread.joinable()) {
		ww->_audio_mutex.lock();
		ww->_playing_pitch = (Pitch)(ww->_pitch_input->value() + 1);
		ww->_audio_mutex.unlock();
	}
}

void Wave_Window::octave_cb(Fl_Widget *, Wave_Window *ww) {
	if (ww->_audio_thread.joinable()) {
		ww->_audio_mutex.lock();
		ww->_playing_octave = ww->_octave_input->value() + 1;
		ww->_audio_mutex.unlock();
	}
}

void Wave_Window::copy_cb(Fl_Widget *, Wave_Window *ww) {
	if (!ww->_selected_wave) return;
	Wave *wave = ww->wave();
	ww->_clipboard = *wave;
	ww->_paste_button->activate();
}

void Wave_Window::paste_cb(Fl_Widget *, Wave_Window *ww) {
	if (!ww->_selected_wave || ww->_clipboard[0] == (uint8_t)-1) return;
	Wave *wave = ww->wave();
	*wave = ww->_clipboard;
	std::fill(RANGE(ww->_clipboard), (uint8_t)-1);
	ww->_paste_button->deactivate();
	ww->redraw_wave();

	ww->regenerate_mod();
}

void Wave_Window::phase_left_cb(Fl_Widget *, Wave_Window *ww) {
	if (!ww->_selected_wave) return;
	Wave *wave = ww->wave();
	uint8_t hold = wave->at(0);
	for (size_t i = 0; i < NUM_WAVE_SAMPLES - 1; ++i) {
		wave->at(i) = wave->at(i+1);
	}
	wave->at(NUM_WAVE_SAMPLES - 1) = hold;
	ww->redraw_wave();

	ww->regenerate_mod();
}

void Wave_Window::phase_right_cb(Fl_Widget *, Wave_Window *ww) {
	if (!ww->_selected_wave) return;
	Wave *wave = ww->wave();
	uint8_t hold = wave->at(NUM_WAVE_SAMPLES - 1);
	for (size_t i = NUM_WAVE_SAMPLES - 1; i > 0; --i) {
		wave->at(i) = wave->at(i-1);
	}
	wave->at(0) = hold;
	ww->redraw_wave();

	ww->regenerate_mod();
}

void Wave_Window::shift_up_cb(Fl_Widget *, Wave_Window *ww) {
	if (!ww->_selected_wave) return;
	Wave *wave = ww->wave();
	for (size_t i = 0; i < NUM_WAVE_SAMPLES; ++i) {
		if (wave->at(i) < 15) wave->at(i) += 1;
	}
	ww->redraw_wave();

	ww->regenerate_mod();
}

void Wave_Window::shift_down_cb(Fl_Widget *, Wave_Window *ww) {
	if (!ww->_selected_wave) return;
	Wave *wave = ww->wave();
	for (size_t i = 0; i < NUM_WAVE_SAMPLES; ++i) {
		if (wave->at(i) > 0) wave->at(i) -= 1;
	}
	ww->redraw_wave();

	ww->regenerate_mod();
}

void Wave_Window::flip_cb(Fl_Widget *, Wave_Window *ww) {
	if (!ww->_selected_wave) return;
	Wave *wave = ww->wave();
	for (size_t i = 0; i < NUM_WAVE_SAMPLES / 2; ++i) {
		std::swap(wave->at(i), wave->at(NUM_WAVE_SAMPLES - 1 - i));
	}
	ww->redraw_wave();

	ww->regenerate_mod();
}

void Wave_Window::invert_cb(Fl_Widget *, Wave_Window *ww) {
	if (!ww->_selected_wave) return;
	Wave *wave = ww->wave();
	for (size_t i = 0; i < NUM_WAVE_SAMPLES; ++i) {
		wave->at(i) = 15 - wave->at(i);
	}
	ww->redraw_wave();

	ww->regenerate_mod();
}

void Wave_Window::playback_thread(Wave_Window *ww, std::future<void> kill_signal) {
	Pitch pitch = Pitch::REST;
	int32_t octave = 0;
	int instrument = 0;

	while (kill_signal.wait_for(std::chrono::milliseconds(8)) == std::future_status::timeout) {
		if (ww->_audio_mutex.try_lock()) {
			IT_Module *mod = ww->_mod;
			if (mod && mod->playing()) {
				if (
					pitch != ww->_playing_pitch || octave != ww->_playing_octave || instrument != ww->_playing_instrument ||
					(ww->_mod_channel == -1 && pitch != Pitch::REST && octave != 0 && instrument != 0)
				) {
					if (ww->_mod_channel != -1) {
						mod->stop_note(ww->_mod_channel);
						ww->_mod_channel = -1;
					}
					pitch = ww->_playing_pitch;
					octave = ww->_playing_octave;
					instrument = ww->_playing_instrument;
					if (pitch != Pitch::REST && octave != 0 && instrument != 0) {
						ww->_mod_channel = mod->play_note(pitch, octave, 3, instrument - 1);
					}
				}

				bool success = mod->play();
				if (!success) mod->stop();
				ww->_audio_mutex.unlock();
			}
			else {
				ww->_audio_mutex.unlock();
				break;
			}
		}
	}
}
