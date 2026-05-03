#include <cassert>
#include <set>

#include "themes.h"
#include "drumkit-window.h"

Drumkit_Window::Drumkit_Window(int x, int y) : _dx(x), _dy(y) {}

Drumkit_Window::~Drumkit_Window() {
	delete _drumkit_name_dialog;
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
	for (size_t i = 0; i < NUM_DRUMS_PER_DRUMKIT; ++i) {
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
	_drumkit_name_dialog = new Drumkit_Name_Dialog("New Drumkit");
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
	for (Dropdown *drumkit_drum : _drumkit_drums) {
		drumkit_drum->center_menu(true);
		drumkit_drum->callback((Fl_Callback *)edit_drumkit_cb, this);
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

bool Drumkit_Window::modified() {
	if (_drumkits.drumkits.size() != _saved_drumkits.drumkits.size()) return true;
	if (_drumkits.drums.size() != _saved_drumkits.drums.size()) return true;

	for (size_t i = 0; i < _drumkits.drumkits.size(); ++i) {
		if (_drumkits.drumkits[i].label != _saved_drumkits.drumkits[i].label) return true;
		for (size_t j = 0; j < NUM_DRUMS_PER_DRUMKIT; ++j) {
			if (_drumkits.drumkits[i].drums[j] != _saved_drumkits.drumkits[i].drums[j]) return true;
		}
	}

	for (size_t i = 0; i < _drumkits.drums.size(); ++i) {
		if (_drumkits.drums[i].label != _saved_drumkits.drums[i].label) return true;
		if (_drumkits.drums[i].noise_notes.size() != _saved_drumkits.drums[i].noise_notes.size()) return true;
		for (size_t j = 0; j < _drumkits.drums[i].noise_notes.size(); ++j) {
			const Noise_Note &note = _drumkits.drums[i].noise_notes[j];
			const Noise_Note &saved_note = _saved_drumkits.drums[i].noise_notes[j];
			if (
				note.length != saved_note.length ||
				note.volume != saved_note.volume ||
				note.envelope_direction != saved_note.envelope_direction ||
				note.sweep_pace != saved_note.sweep_pace ||
				note.clock_shift != saved_note.clock_shift ||
				note.lfsr_width != saved_note.lfsr_width ||
				note.clock_divider != saved_note.clock_divider
			) return true;
		}
	}

	return false;
}

bool Drumkit_Window::write_drumkits(const char *f) {
	std::ofstream ofs;
	open_ofstream(ofs, f);
	if (!ofs.good()) { return false; }

	ofs << _drumkits.drumkits_label << ":\n";
	for (const Drumkit &drumkit : _drumkits.drumkits) {
		ofs << (_drumkits.uses_dr ? "\tdr " : "\tdw ") << drumkit.label << "\n";
	}
	ofs << "\n";

	std::set<std::string> written_drumkits;
	for (const Drumkit &drumkit : _drumkits.drumkits) {
		if (!written_drumkits.count(drumkit.label)) {
			ofs << drumkit.label << ":\n";
			for (int32_t drum : drumkit.drums) {
				ofs << (_drumkits.uses_dr ? "\tdr " : "\tdw ") << _drumkits.drums[drum].label << "\n";
			}
			written_drumkits.insert(drumkit.label);
		}
	}

	for (const Drum &drum : _drumkits.drums) {
		ofs << "\n";
		ofs << drum.label << ":\n";
		for (const Noise_Note &note : drum.noise_notes) {
			int32_t fade = note.sweep_pace * (note.envelope_direction ? -1 : 1);
			if (fade == 0) fade = 8;
			int32_t frequency = (note.clock_shift << 4) | (note.lfsr_width << 3) | (note.clock_divider);
			ofs << "\tnoise_note " << note.length << ", " << note.volume << ", " << fade << ", " << frequency << "\n";
		}
		ofs << "\tsound_ret\n";
	}

	ofs.close();
	return true;
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
	for (const Drumkit &drumkit : _drumkits.drumkits) {
		_drumkit_browser->add(drumkit.label.c_str());
	}

	if (_drumkits.drumkits.size() == 256) {
		_add_drumkit_button->deactivate();
	}
	else {
		_add_drumkit_button->activate();
	}

	for (Dropdown *drumkit_drum : _drumkit_drums) {
		drumkit_drum->clear();
		for (const Drum &drum : _drumkits.drums) {
			drumkit_drum->add(drum.label.c_str());
		}
	}

	_drum_browser->clear();
	for (const Drum &drum : _drumkits.drums) {
		_drum_browser->add(drum.label.c_str());
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
	if (dw->modified()) {
		std::string msg = fl_filename_name(dw->_drumkits.drumkits_file.c_str());
		msg = msg + " has unsaved changes!\n\n"
			"Close anyway?";
		dw->_confirm_dialog->message(msg);
		dw->_confirm_dialog->show(dw->_window);
		if (dw->_confirm_dialog->canceled()) { return; }
	}

	dw->_window->hide();
}

void Drumkit_Window::cancel_cb(Fl_Widget *w, Drumkit_Window *dw) {
	dw->_canceled = true;
	close_cb(w, dw);
}

void Drumkit_Window::save_cb(Fl_Widget *, Drumkit_Window *dw) {
	const char *filename = dw->_drumkits.drumkits_file.c_str();
	const char *basename = fl_filename_name(filename);

	if (dw->modified() && !dw->write_drumkits(filename)) {
		std::string msg = "Could not write to ";
		msg = msg + basename + "!";
		dw->_error_dialog->message(msg);
		dw->_error_dialog->show(dw->_window);
		return;
	}

	std::string msg = "Saved ";
	msg = msg + basename + "!";
	dw->_success_dialog->message(msg);
	dw->_success_dialog->show(dw->_window);

	dw->_saved_drumkits = dw->_drumkits;
}

void Drumkit_Window::revert_cb(Fl_Widget *, Drumkit_Window *dw) {
	if (!dw->modified()) return;

	std::string msg = "Discard unsaved changes?";
	dw->_confirm_dialog->message(msg);
	dw->_confirm_dialog->show(dw->_window);
	if (dw->_confirm_dialog->canceled()) { return; }

	dw->drumkits(dw->_saved_drumkits);

	dw->_drumkit_browser->select(1);
	select_drumkit_cb(nullptr, dw);
	dw->_drum_browser->select(1);
	select_drum_cb(nullptr, dw);
}

void Drumkit_Window::add_drumkit_cb(Fl_Widget *, Drumkit_Window *dw) {
	if (dw->_drumkits.drumkits.size() == 256) return;

	const auto is_label_taken = [&](const std::string &label) {
		if (label == dw->_drumkits.drumkits_label) return true;
		for (const Drum &drum : dw->_drumkits.drums) {
			if (label == drum.label) {
				return true;
			}
		}
		return false;
	};
	const auto find_drumkit = [&](const std::string &label) {
		for (const Drumkit &drumkit : dw->_drumkits.drumkits) {
			if (label == drumkit.label) {
				return &drumkit;
			}
		}
		return (const Drumkit *)nullptr;
	};

	std::string drumkit_name;
	bool reset = true;
	while (true) {
		dw->_drumkit_name_dialog->show(dw->_window, reset);
		bool canceled = dw->_drumkit_name_dialog->canceled();
		if (canceled) { return; }

		drumkit_name = dw->_drumkit_name_dialog->get_drumkit_name();
		if (
			drumkit_name.size() == 0 ||
			(drumkit_name[0] >= '0' && drumkit_name[0] <= '9') ||
			drumkit_name.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_") != std::string::npos
		) {
			std::string msg = "Name is invalid! Name can only contain letters, numbers, and underscores, and must not start with a number.";
			dw->_error_dialog->message(msg);
			dw->_error_dialog->show(dw->_window);
			reset = false;
		}
		else if (is_label_taken(drumkit_name)) {
			std::string msg = "Duplicate label.";
			dw->_error_dialog->message(msg);
			dw->_error_dialog->show(dw->_window);
			reset = false;
		}
		else {
			break;
		}
	}

	const Drumkit *drumkit = find_drumkit(drumkit_name);
	Drumkit new_drumkit = {};
	new_drumkit.label = drumkit_name;
	if (drumkit) new_drumkit.drums = drumkit->drums;
	dw->_drumkits.drumkits.push_back(new_drumkit);

	dw->_drumkit_browser->add(drumkit_name.c_str());
	dw->_drumkit_browser->select((int)dw->_drumkits.drumkits.size());
	if (dw->_drumkits.drumkits.size() == 256) {
		dw->_add_drumkit_button->deactivate();
	}
	select_drumkit_cb(nullptr, dw);
}

void Drumkit_Window::remove_drumkit_cb(Fl_Widget *, Drumkit_Window *dw) {
	if (!dw->_selected_drumkit || dw->_drumkits.drumkits.size() == 1) return;

	dw->_drumkits.drumkits.erase(dw->_drumkits.drumkits.begin() + dw->_selected_drumkit-1);
	dw->_drumkit_browser->deselect();
	dw->_drumkit_browser->remove(dw->_selected_drumkit);
	dw->_add_drumkit_button->activate();
	select_drumkit_cb(nullptr, dw);
}

void Drumkit_Window::move_drumkit_up_cb(Fl_Widget *, Drumkit_Window *dw) {
	if (!dw->_selected_drumkit || dw->_selected_drumkit == 1) return;

	std::swap(dw->_drumkits.drumkits[dw->_selected_drumkit - 1], dw->_drumkits.drumkits[dw->_selected_drumkit - 2]);
	dw->_drumkit_browser->swap(dw->_selected_drumkit, dw->_selected_drumkit - 1);
	dw->_drumkit_browser->select(dw->_selected_drumkit - 1);
	select_drumkit_cb(nullptr, dw);
}

void Drumkit_Window::move_drumkit_down_cb(Fl_Widget *, Drumkit_Window *dw) {
	if (!dw->_selected_drumkit || dw->_selected_drumkit == dw->_drumkits.drumkits.size()) return;

	std::swap(dw->_drumkits.drumkits[dw->_selected_drumkit - 1], dw->_drumkits.drumkits[dw->_selected_drumkit]);
	dw->_drumkit_browser->swap(dw->_selected_drumkit, dw->_selected_drumkit + 1);
	dw->_drumkit_browser->select(dw->_selected_drumkit + 1);
	select_drumkit_cb(nullptr, dw);
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
		for (size_t i = 0; i < NUM_DRUMS_PER_DRUMKIT; ++i) {
			dw->_drumkit_drums[i]->value(nullptr);
			dw->_drumkit_drums[i]->deactivate();
		}
	}
	else {
		for (size_t i = 0; i < NUM_DRUMS_PER_DRUMKIT; ++i) {
			dw->_drumkit_drums[i]->value(dw->_drumkits.drumkits[dw->_selected_drumkit-1].drums[i]);
			dw->_drumkit_drums[i]->activate();
		}
	}
}

void Drumkit_Window::edit_drumkit_cb(Fl_Widget *w, Drumkit_Window *dw) {
	Drumkit *drumkit = dw->drumkit();
	if (!drumkit) return;
	for (size_t i = 0; i < NUM_DRUMS_PER_DRUMKIT; ++i) {
		if (dw->_drumkit_drums[i] == (Dropdown *)w) {
			drumkit->drums[i] = dw->_drumkit_drums[i]->value();
			for (Drumkit &other : dw->_drumkits.drumkits) {
				if (&other != drumkit && other.label == drumkit->label) {
					other.drums[i] = dw->_drumkit_drums[i]->value();
				}
			}
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
