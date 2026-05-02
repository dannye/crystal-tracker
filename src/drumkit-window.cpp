#include <cassert>

#include "themes.h"
#include "drumkit-window.h"

Drumkit_Window::Drumkit_Window(int x, int y) : _dx(x), _dy(y) {}

Drumkit_Window::~Drumkit_Window() {
	delete _confirm_dialog;
	delete _success_dialog;
	delete _error_dialog;
	delete _window;
}

void Drumkit_Window::initialize() {
	if (_window) { return; }
	Fl_Group *prev_current = Fl_Group::current();
	Fl_Group::current(NULL);
	// Populate window
	_window = new Fl_Double_Window(_dx, _dy, 608, 362, "Drumkit Editor");
	_tabs = new OS_Tabs(10, 10, 588, 310);
	_drumkit_tab = new OS_Tab(10, 35, 588, 285, "Drumkits");
	_add_drumkit_button = new OS_Button(20, 45, 21, 21, "@+");
	_remove_drumkit_button = new OS_Button(45, 45, 21, 21, "@1+");
	_drumkit_up_button = new OS_Button(99, 45, 21, 21, "@8>");
	_drumkit_down_button = new OS_Button(124, 45, 21, 21, "@2>");
	_drumkit_browser = new OS_Browser(20, 70, 125, 240);
	for (int i = 0; i < NUM_DRUMS_PER_DRUMKIT; ++i) {
		_drumkit_drums[i] = new Dropdown(DRUM_DROPDOWNS[i].x, DRUM_DROPDOWNS[i].y, 125, 22, DRUM_DROPDOWNS[i].label);
	}
	_drumkit_tab->end();
	_drum_tab = new OS_Tab(10, 35, 588, 285, "Drums");
	_add_drum_button = new OS_Button(20, 45, 21, 21, "@+");
	_remove_drum_button = new OS_Button(45, 45, 21, 21, "@1+");
	_drum_up_button = new OS_Button(99, 45, 21, 21, "@8>");
	_drum_down_button = new OS_Button(124, 45, 21, 21, "@2>");
	_drum_browser = new OS_Browser(20, 70, 125, 240);
	_tabs->end();
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
	_add_drumkit_button->tooltip("New drumkit");
	_add_drumkit_button->callback((Fl_Callback *)add_drumkit_cb, this);
	_remove_drumkit_button->tooltip("Delete drumkit");
	_remove_drumkit_button->callback((Fl_Callback *)remove_drumkit_cb, this);
	_drumkit_up_button->tooltip("Move up");
	_drumkit_up_button->callback((Fl_Callback *)move_drumkit_up_cb, this);
	_drumkit_down_button->tooltip("Move down");
	_drumkit_down_button->callback((Fl_Callback *)move_drumkit_down_cb, this);
	_drumkit_browser->callback((Fl_Callback *)select_drumkit_cb, this);
	for (int i = 0; i < NUM_DRUMS_PER_DRUMKIT; ++i) {
		_drumkit_drums[i]->center_menu(true);
		_drumkit_drums[i]->callback((Fl_Callback *)edit_drumkit_cb, this);
	}
	_add_drum_button->tooltip("New drum");
	_add_drum_button->callback((Fl_Callback *)add_drum_cb, this);
	_remove_drum_button->tooltip("Delete drum");
	_remove_drum_button->callback((Fl_Callback *)remove_drum_cb, this);
	_drum_up_button->tooltip("Move up");
	_drum_up_button->callback((Fl_Callback *)move_drum_up_cb, this);
	_drum_down_button->tooltip("Move down");
	_drum_down_button->callback((Fl_Callback *)move_drum_down_cb, this);
	_drum_browser->callback((Fl_Callback *)select_drum_cb, this);
	_save_button->shortcut(0);
	_save_button->callback((Fl_Callback *)save_cb, this);
	_revert_button->callback((Fl_Callback *)revert_cb, this);
	_close_button->callback((Fl_Callback *)close_cb, this);
	_error_dialog->width_range(280, 500);
	_success_dialog->width_range(280, 500);
	_confirm_dialog->width_range(280, 500);
	Fl_Group::current(prev_current);
}

void Drumkit_Window::refresh() {
	_canceled = false;
	_tabs->value(_drumkit_tab);
	_drumkit_browser->select(1);
	select_drumkit_cb(nullptr, this);
	_drum_browser->select(1);
	select_drum_cb(nullptr, this);
}

Drumkit *Drumkit_Window::drumkit() {
	if (_selected_drumkit == 0) return nullptr;
	return &_drumkits.drumkits[_selected_drumkit-1];
}

Drum *Drumkit_Window::drum() {
	if (_selected_drum == 0) return nullptr;
	return &_drumkits.drums[_selected_drum-1];
}

void Drumkit_Window::drumkits(const Drumkits &d) {
	initialize();

	_drumkits = d;
	assert(_drumkits.drumkits.size() >= 1 && _drumkits.drumkits.size() <= 256);
	assert(_drumkits.drums.size() > 0);

	_drumkit_browser->clear();
	for (size_t i = 0; i < _drumkits.drumkits.size(); ++i) {
		_drumkit_browser->add(_drumkits.drumkits[i].label.c_str());
	}

	if (_drumkits.drumkits.size() == 256) {
		_add_drumkit_button->deactivate();
	}
	else {
		_add_drumkit_button->activate();
	}

	for (int i = 0; i < NUM_DRUMS_PER_DRUMKIT; ++i) {
		_drumkit_drums[i]->clear();
		for (size_t j = 0; j < _drumkits.drums.size(); ++j) {
			_drumkit_drums[i]->add(_drumkits.drums[j].label.c_str());
		}
	}

	_drum_browser->clear();
	for (size_t i = 0; i < _drumkits.drums.size(); ++i) {
		_drum_browser->add(_drumkits.drums[i].label.c_str());
	}

	_saved_drumkits = _drumkits;
}

void Drumkit_Window::show(const Fl_Widget *p) {
	initialize();
	refresh();
	Fl_Window *prev_grab = Fl::grab();
	Fl::grab(NULL);
	int x = p->x() + (p->w() - _window->w()) / 2;
	int y = p->y() + (p->h() - _window->h()) / 2;
	_window->position(x, y);
	_drumkit_browser->take_focus();
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

void Drumkit_Window::save_cb(Fl_Widget *, Drumkit_Window *dw) {
	// TODO
}

void Drumkit_Window::revert_cb(Fl_Widget *, Drumkit_Window *dw) {
	// TODO
}

void Drumkit_Window::add_drumkit_cb(Fl_Widget *, Drumkit_Window *dw) {
	// TODO
}

void Drumkit_Window::remove_drumkit_cb(Fl_Widget *, Drumkit_Window *dw) {
	// TODO
}

void Drumkit_Window::move_drumkit_up_cb(Fl_Widget *, Drumkit_Window *dw) {
	// TODO
}

void Drumkit_Window::move_drumkit_down_cb(Fl_Widget *, Drumkit_Window *dw) {
	// TODO
}

void Drumkit_Window::select_drumkit_cb(Fl_Widget *, Drumkit_Window *dw) {
	dw->_selected_drumkit = dw->_drumkit_browser->value();
	if (!dw->_selected_drumkit || dw->_drumkits.drumkits.size() == 1) {
		dw->_remove_drumkit_button->deactivate();
	}
	else {
		dw->_remove_drumkit_button->activate();
	}
	if (!dw->_selected_drumkit || dw->_selected_drumkit == 1) {
		dw->_drumkit_up_button->deactivate();
	}
	else {
		dw->_drumkit_up_button->activate();
	}
	if (!dw->_selected_drumkit || dw->_selected_drumkit == dw->_drumkits.drumkits.size()) {
		dw->_drumkit_down_button->deactivate();
	}
	else {
		dw->_drumkit_down_button->activate();
	}
	if (!dw->_selected_drumkit) {
		for (int i = 0; i < NUM_DRUMS_PER_DRUMKIT; ++i) {
			dw->_drumkit_drums[i]->value(nullptr);
			dw->_drumkit_drums[i]->deactivate();
		}
	}
	else {
		for (int i = 0; i < NUM_DRUMS_PER_DRUMKIT; ++i) {
			dw->_drumkit_drums[i]->value(dw->_drumkits.drumkits[dw->_selected_drumkit-1].drums[i]);
			dw->_drumkit_drums[i]->activate();
		}
	}
}

void Drumkit_Window::edit_drumkit_cb(Fl_Widget *w, Drumkit_Window *dw) {
	if (!dw->_selected_drumkit) return;
	for (int i = 0; i < NUM_DRUMS_PER_DRUMKIT; ++i) {
		if (dw->_drumkit_drums[i] == (Dropdown *)w) {
			dw->_drumkits.drumkits[dw->_selected_drumkit-1].drums[i] = dw->_drumkit_drums[i]->value();
			return;
		}
	}
	assert(false);
}

void Drumkit_Window::add_drum_cb(Fl_Widget *, Drumkit_Window *dw) {
	// TODO
}

void Drumkit_Window::remove_drum_cb(Fl_Widget *, Drumkit_Window *dw) {
	// TODO
}

void Drumkit_Window::move_drum_up_cb(Fl_Widget *, Drumkit_Window *dw) {
	// TODO
}

void Drumkit_Window::move_drum_down_cb(Fl_Widget *, Drumkit_Window *dw) {
	// TODO
}

void Drumkit_Window::select_drum_cb(Fl_Widget *, Drumkit_Window *dw) {
	// TODO
}
