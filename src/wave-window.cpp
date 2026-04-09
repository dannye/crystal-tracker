#include <cassert>

#include "themes.h"
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
		int x_pos = x()+2 + x_step * i;
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
				redraw();
			}
		}
		return 1;
	}
	case FL_RELEASE:
		return 1;
	}
	return 0;
}

Wave_Double_Window::Wave_Double_Window(int x, int y, int w, int h, const char *l) : Fl_Double_Window(x, y, w, h, l) {}

int Wave_Double_Window::handle(int event) {
	return Fl_Double_Window::handle(event);
}

Wave_Window::Wave_Window(int x, int y) : _dx(x), _dy(y), _canceled(false), _window(NULL),
	_wave_browser(NULL), _wave_graph(NULL), _ok_button(NULL), _waves(), _num_waves(0), _selected_wave(0) {}

Wave_Window::~Wave_Window() {
	delete _window;
}

void Wave_Window::initialize() {
	if (_window) { return; }
	Fl_Group *prev_current = Fl_Group::current();
	Fl_Group::current(NULL);
	// Populate window
	_window = new Wave_Double_Window(_dx, _dy, 518, 312, "Wave Editor");
	_wave_browser = new OS_Browser(10, 10, 100, 260);
	_wave_graph = new Wave_Graph(120, 10, 388, 260);
	_ok_button = new Default_Button(428, 280, 80, 22, "OK");
	_window->end();
	// Initialize window
	_window->box(OS_BG_BOX);
	_window->callback((Fl_Callback *)cancel_cb, this);
	_window->set_modal();
	// Initialize window's children
	_wave_browser->callback((Fl_Callback *)select_wave_cb, this);
	_wave_graph->box(OS_SWATCH_BOX);
	_wave_graph->color(FL_BACKGROUND2_COLOR);
	_wave_graph->user_data(this);
	_ok_button->tooltip("OK (Enter)");
	_ok_button->callback((Fl_Callback *)close_cb, this);
	Fl_Group::current(prev_current);
}

void Wave_Window::refresh() {
	_canceled = false;
	_wave_browser->select(1, 1);
	_selected_wave = 1;
}

Wave *Wave_Window::wave() {
	if (_selected_wave == 0) return nullptr;
	return &_waves[_selected_wave-1];
}

void Wave_Window::waves(const std::vector<Wave> &w, int32_t n) {
	initialize();

	_waves = w;
	_num_waves = n;

	assert(_waves.size() == 16);
	assert(_num_waves >= 1 && _num_waves <= 16);

	_wave_browser->clear();
	char buffer[16];
	for (int32_t i = 0; i < _num_waves; ++i) {
		snprintf(buffer, 16, "Wave %d", i + 1);
		_wave_browser->add(buffer);
	}
}

void Wave_Window::show(const Fl_Widget *p) {
	initialize();
	refresh();
	Fl_Window *prev_grab = Fl::grab();
	Fl::grab(NULL);
	int x = p->x() + (p->w() - _window->w()) / 2;
	int y = p->y() + (p->h() - _window->h()) / 2;
	_window->position(x, y);
	_ok_button->take_focus();
	_window->show();
	while (_window->shown()) { Fl::wait(); }
	Fl::grab(prev_grab);
}

void Wave_Window::close_cb(Fl_Widget *, Wave_Window *ww) {
	ww->_window->hide();
}

void Wave_Window::cancel_cb(Fl_Widget *w, Wave_Window *ww) {
	ww->_canceled = true;
	close_cb(w, ww);
}

void Wave_Window::select_wave_cb(Fl_Widget *, Wave_Window *ww) {
	ww->_selected_wave = ww->_wave_browser->value();
	ww->_wave_graph->redraw();
}
