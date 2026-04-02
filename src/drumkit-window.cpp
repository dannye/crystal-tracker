#include "themes.h"
#include "drumkit-window.h"

Drumkit_Double_Window::Drumkit_Double_Window(int x, int y, int w, int h, const char *l) : Fl_Double_Window(x, y, w, h, l) {}

int Drumkit_Double_Window::handle(int event) {
	return Fl_Double_Window::handle(event);
}

Drumkit_Window::Drumkit_Window(int x, int y) : _dx(x), _dy(y), _canceled(false), _window(NULL), _ok_button(NULL) {}

Drumkit_Window::~Drumkit_Window() {
	delete _window;
}

void Drumkit_Window::initialize() {
	if (_window) { return; }
	Fl_Group *prev_current = Fl_Group::current();
	Fl_Group::current(NULL);
	// Populate window
	_window = new Drumkit_Double_Window(_dx, _dy, 500, 300, "Drumkit Editor");
	_ok_button = new Default_Button(410, 268, 80, 22, "OK");
	_window->end();
	// Initialize window
	_window->box(OS_BG_BOX);
	_window->callback((Fl_Callback *)cancel_cb, this);
	_window->set_modal();
	// Initialize window's children
	_ok_button->tooltip("OK (Enter)");
	_ok_button->callback((Fl_Callback *)close_cb, this);
	Fl_Group::current(prev_current);
}

void Drumkit_Window::refresh() {
	_canceled = false;
}

void Drumkit_Window::show(const Fl_Widget *p) {
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

void Drumkit_Window::close_cb(Fl_Widget *, Drumkit_Window *dw) {
	dw->_window->hide();
}

void Drumkit_Window::cancel_cb(Fl_Widget *w, Drumkit_Window *dw) {
	dw->_canceled = true;
	close_cb(w, dw);
}
