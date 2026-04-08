#include <cassert>

#include "themes.h"
#include "wave-window.h"

Wave_Graph::Wave_Graph(int x, int y, int w, int h, const char *l) : Fl_Box(x, y, w, h, l) {}

void Wave_Graph::draw() {
	draw_box();

	Wave_Window *ww = (Wave_Window *)user_data();
	Wave *wave = ww->wave();
	if (!wave) return;

	int x_step = w() / NUM_WAVE_SAMPLES;
	int y_step = h() / 16;

	fl_color(FL_SELECTION_COLOR);
	for (size_t i = 0; i < NUM_WAVE_SAMPLES; ++i) {
		int x_pos = x() + x_step * i;
		int y_pos = y() + h() - y_step * (wave->at(i) + 1);
		fl_rectf(x_pos, y_pos, x_step, y_step);
	}
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
	_window = new Wave_Double_Window(_dx, _dy, 514, 308, "Wave Editor");
	_wave_browser = new OS_Browser(10, 10, 100, 256);
	_wave_graph = new Wave_Graph(120, 10, 384, 256);
	_ok_button = new Default_Button(428, 276, 80, 22, "OK");
	_window->end();
	// Initialize window
	_window->box(OS_BG_BOX);
	_window->callback((Fl_Callback *)cancel_cb, this);
	_window->set_modal();
	// Initialize window's children
	_wave_browser->callback((Fl_Callback *)select_wave_cb, this);
	_wave_graph->box(FL_BORDER_BOX);
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
