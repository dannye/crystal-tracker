#include <cassert>
#include <set>

#include "themes.h"
#include "utils.h"
#include "drumkit-window.h"

Drum_Note_Table::Drum_Note_Table(int x, int y, int w, int h, const char *l) : OS_Table(x, y, w, h, l) {
	col_header(1);
	col_header_height(25);
	cols(NUM_COLUMNS);
	col_width_all((w-2) / cols());
	scrollbar_size(12);
	end();
}

void Drum_Note_Table::clear() {
	table->clear();
	rows(0);
	label("");
}

void Drum_Note_Table::set(Drum *drum) {
	table->clear();
	rows(drum->noise_notes.size());
	label(drum->label.c_str());

	begin();
	for (int r = 0; r < drum->noise_notes.size(); ++r) {
		const Noise_Note &note = drum->noise_notes[r];
		for (int c = 0; c < NUM_COLUMNS; ++c) {
			int X, Y, W, H;
			find_cell(CONTEXT_TABLE, r, c, X, Y, W, H);
			OS_Int_Input *input = new OS_Int_Input(X, Y, W, H);
			if (c == LENGTH) {
				input->value(note.length);
			}
			else if (c == VOLUME) {
				input->value(note.volume);
			}
			else if (c == FADE) {
				input->value(note.sweep_pace * (note.envelope_direction ? -1 : 1));
			}
			else if (c == SHIFT) {
				input->value(note.clock_shift);
			}
			else if (c == WIDTH) {
				input->value(note.lfsr_width);
			}
			else if (c == DIVIDER) {
				input->value(note.clock_divider);
			}
			input->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE_ALWAYS);
			input->callback((Fl_Callback *)edit_note_cb, this);
		}
	}
	end();
}

void Drum_Note_Table::add_row() {
	rows(rows() + 1);

	begin();
	for (int c = 0; c < NUM_COLUMNS; ++c) {
		int X, Y, W, H;
		find_cell(CONTEXT_TABLE, rows()-1, c, X, Y, W, H);
		OS_Int_Input *input = new OS_Int_Input(X, Y, W, H);
		input->value(0);
		input->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE_ALWAYS);
		input->callback((Fl_Callback *)edit_note_cb, this);
	}
	end();
}

void Drum_Note_Table::remove_row() {
	if (rows() == 0) return;

	for (int c = 0; c < NUM_COLUMNS; ++c) {
		Fl_Widget *ch = find_child(rows()-1, c);
		if (ch) delete ch;
	}

	rows(rows() - 1);
}

void Drum_Note_Table::draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H) {
	switch (context) {
	case CONTEXT_RC_RESIZE: {
		col_width_all((w()-2 - (vscrollbar->visible() ? scrollbar_size() : 0)) / cols());
		int X, Y, W, H;
		int index = 0;
		for (int r = 0; r < rows(); ++r) {
			for (int c = 0; c < cols(); ++c) {
				if (index >= children()) break;
				find_cell(CONTEXT_TABLE, r, c, X, Y, W, H);
				child(index++)->resize(X, Y, W, H);
			}
		}
		init_sizes();
		return;
	}
	case CONTEXT_COL_HEADER:
		fl_push_clip(X, Y, W, H);
		fl_draw_box(OS_PANEL_THIN_UP_BOX, X, Y, W, H, col_header_color());
		fl_font(OS_FONT, OS_FONT_SIZE);
		fl_color(FL_FOREGROUND_COLOR);
		fl_draw(COLUMN_LABELS[C], X, Y, W, H, FL_ALIGN_CENTER);
		fl_pop_clip();
		return;
	default:
		return;
	}
}

void Drum_Note_Table::find_coord(int x, int y, int &R, int &C) {
	int X, Y, W, H;
	for (int r = 0; r < rows(); ++r) {
		for (int c = 0; c < cols(); ++c) {
			find_cell(CONTEXT_TABLE, r, c, X, Y, W, H);
			if (X == x && Y == y) {
				R = r;
				C = c;
				return;
			}
		}
	}
	R = -1;
	C = -1;
}

Fl_Widget *Drum_Note_Table::find_child(int R, int C) {
	int X, Y, W, H;
	find_cell(CONTEXT_TABLE, R, C, X, Y, W, H);
	for (int i = 0; i < children(); ++i) {
		Fl_Widget *c = child(i);
		if (c->x() == X && c->y() == Y) return c;
	}
	return nullptr;
}

void Drum_Note_Table::edit_note_cb(OS_Int_Input *i, Drum_Note_Table *dt) {
	Drumkit_Window *dw = (Drumkit_Window *)dt->user_data();
	Drum *drum = dw->drum();
	if (!drum) return;

	int R, C;
	dt->find_coord(i->x(), i->y(), R, C);
	assert(R >= 0 && R < drum->noise_notes.size());
	assert(C >= 0 && C < NUM_COLUMNS);

	if (
		(strlen(i->value()) == 0 && Fl::focus() != i) ||
		(strlen(i->value()) > 0 && i->value()[0] == '-' && COLUMN_MIN[C] >= 0) ||
		(strlen(i->value()) > 1 && i->value()[0] == '0' && i->value()[1] == 'x')
	) {
		i->value("0");
	}

	int value = i->ivalue();
	if (value < COLUMN_MIN[C]) {
		value = COLUMN_MIN[C];
		i->value(value);
	}
	if (value > COLUMN_MAX[C]) {
		value = COLUMN_MAX[C];
		i->value(value);
	}

	Noise_Note &note = drum->noise_notes[R];
	bool changed = false;
	if (C == LENGTH) {
		changed = note.length != value;
		note.length = value;
	}
	else if (C == VOLUME) {
		changed = note.volume != value;
		note.volume = value;
	}
	else if (C == FADE) {
		changed = (note.sweep_pace * (note.envelope_direction ? -1 : 1)) != value;
		if (value < 0) {
			note.sweep_pace = -value;
			note.envelope_direction = 1;
		}
		else {
			note.sweep_pace = value;
			note.envelope_direction = 0;
		}
	}
	else if (C == SHIFT) {
		changed = note.clock_shift != value;
		note.clock_shift = value;
	}
	else if (C == WIDTH) {
		changed = note.lfsr_width != value;
		note.lfsr_width = value;
	}
	else if (C == DIVIDER) {
		changed = note.clock_divider != value;
		note.clock_divider = value;
	}

	if (changed) dw->regenerate_mod();
}

Drumkit_Window::Drumkit_Window(int x, int y) : _dx(x), _dy(y) {}

Drumkit_Window::~Drumkit_Window() {
	stop_audio_thread();
	if (_mod) delete _mod;
	delete _new_name_dialog;
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
	_copy_drumkit_button = new OS_Button(72, 45, 21, 21, "@copy");
	_drumkit_up_button = new OS_Button(99, 45, 21, 21, "@8>");
	_drumkit_down_button = new OS_Button(124, 45, 21, 21, "@2>");
	_drumkit_browser = new OS_Browser(20, 70, 125, 240);
	for (size_t i = 0; i < NUM_DRUMS_PER_DRUMKIT; ++i) {
		_drumkit_drum_dropdowns[i] = new Dropdown(DRUM_DROPDOWNS[i].x, DRUM_DROPDOWNS[i].y, 125, 22, DRUM_DROPDOWNS[i].label);
	}
	for (size_t i = 0; i < NUM_DRUMS_PER_DRUMKIT - 1; ++i) {
		_drumkit_drum_buttons[i] = new OS_Button(DRUM_DROPDOWNS[i+1].x + 135, DRUM_DROPDOWNS[i+1].y, 22, 22, "@>");
	}
	_drumkit_tab->end();
	_drum_tab = new OS_Tab(10, 35, 588, 285, "Drums");
	_add_drum_button = new OS_Button(20, 45, 21, 21, "@+");
	_remove_drum_button = new OS_Button(45, 45, 21, 21, "@1+");
	_copy_drum_button = new OS_Button(72, 45, 21, 21, "@copy");
	_drum_up_button = new OS_Button(99, 45, 21, 21, "@8>");
	_drum_down_button = new OS_Button(124, 45, 21, 21, "@2>");
	_drum_browser = new OS_Browser(20, 70, 125, 240);
	_add_note_button = new OS_Button(160, 45, 21, 21, "@+");
	_remove_note_button = new OS_Button(185, 45, 21, 21, "@-");
	_play_button = new OS_Light_Button(523, 45, 65, 22, "Play");
	_drum_note_table = new Drum_Note_Table(160, 70, 428, 240);
	_tabs->end();
	_save_button = new Default_Button(10, 330, 80, 22, "Save");
	_revert_button = new OS_Button(100, 330, 80, 22, "Revert");
	_close_button = new OS_Button(518, 330, 80, 22, "Close");
	_window->end();
	_error_dialog = new Modal_Dialog(_window, "Error", Modal_Dialog::Icon::ERROR_ICON);
	_success_dialog = new Modal_Dialog(_window, "Success", Modal_Dialog::Icon::SUCCESS_ICON);
	_confirm_dialog = new Modal_Dialog(_window, "Warning", Modal_Dialog::Icon::WARNING_ICON, true);
	_new_name_dialog = new New_Name_Dialog("New Name");
	// Initialize window
	_window->box(OS_BG_BOX);
	_window->callback((Fl_Callback *)cancel_cb, this);
	_window->set_modal();
	// Initialize window's children
	_tabs->callback((Fl_Callback *)tabs_cb, this);
	_add_drumkit_button->tooltip("New drumkit");
	_add_drumkit_button->callback((Fl_Callback *)add_drumkit_cb, this);
	_remove_drumkit_button->tooltip("Delete drumkit");
	_remove_drumkit_button->callback((Fl_Callback *)remove_drumkit_cb, this);
	_copy_drumkit_button->tooltip("Duplicate drumkit");
	_copy_drumkit_button->callback((Fl_Callback *)add_drumkit_cb, this);
	_drumkit_up_button->tooltip("Move up");
	_drumkit_up_button->callback((Fl_Callback *)move_drumkit_up_cb, this);
	_drumkit_down_button->tooltip("Move down");
	_drumkit_down_button->callback((Fl_Callback *)move_drumkit_down_cb, this);
	_drumkit_browser->callback((Fl_Callback *)select_drumkit_cb, this);
	for (Dropdown *dropdown : _drumkit_drum_dropdowns) {
		dropdown->center_menu(true);
		dropdown->callback((Fl_Callback *)edit_drumkit_cb, this);
	}
	for (OS_Button *button : _drumkit_drum_buttons) {
		button->callback((Fl_Callback *)play_drumkit_drum_cb, this);
	}
	_add_drum_button->tooltip("New drum");
	_add_drum_button->callback((Fl_Callback *)add_drum_cb, this);
	_remove_drum_button->tooltip("Delete drum");
	_remove_drum_button->callback((Fl_Callback *)remove_drum_cb, this);
	_copy_drum_button->tooltip("Duplicate drum");
	_copy_drum_button->callback((Fl_Callback *)add_drum_cb, this);
	_drum_up_button->tooltip("Move up");
	_drum_up_button->callback((Fl_Callback *)move_drum_up_cb, this);
	_drum_down_button->tooltip("Move down");
	_drum_down_button->callback((Fl_Callback *)move_drum_down_cb, this);
	_drum_browser->callback((Fl_Callback *)select_drum_cb, this);
	_add_note_button->tooltip("Add note");
	_add_note_button->callback((Fl_Callback *)add_note_cb, this);
	_remove_note_button->tooltip("Remove note");
	_remove_note_button->callback((Fl_Callback *)remove_note_cb, this);
	_play_button->tooltip("Play (Spacebar)");
	_play_button->shortcut(' ');
	_play_button->callback((Fl_Callback *)play_drum_cb, this);
	_drum_note_table->user_data(this);
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
	_play_button->value(0);
	_drumkit_browser->select(1);
	select_drumkit_cb(nullptr, this);
	_drum_browser->select(1);
	select_drum_cb(nullptr, this);
}

void Drumkit_Window::start_audio_thread() {
	_audio_kill_signal = std::promise<void>();
	std::future<void> kill_future = _audio_kill_signal.get_future();
	_audio_thread = std::thread(&playback_thread, this, std::move(kill_future));
}

void Drumkit_Window::stop_audio_thread() {
	if (_audio_thread.joinable()) {
		_audio_mutex.lock();
		_audio_kill_signal.set_value();
		_audio_thread.join();
		_audio_mutex.unlock();
	}
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

	if (_drumkits.drumkits_label.size()) {
		ofs << _drumkits.drumkits_label << ":\n";
	}
	for (const Drumkit &drumkit : _drumkits.drumkits) {
		ofs << (_drumkits.uses_dr ? "\tdr " : "\tdw ") << (_drumkits.uses_local ? "." : "") << drumkit.label << "\n";
	}
	ofs << "\n";

	std::set<std::string> written_drumkits;
	for (const Drumkit &drumkit : _drumkits.drumkits) {
		if (!written_drumkits.count(drumkit.label)) {
			ofs << (_drumkits.uses_local ? "." : "") << drumkit.label << ":\n";
			for (int32_t drum : drumkit.drums) {
				ofs << (_drumkits.uses_dr ? "\tdr " : "\tdw ") << (_drumkits.uses_local ? "." : "") << _drumkits.drums[drum].label << "\n";
			}
			written_drumkits.insert(drumkit.label);
		}
	}

	for (const Drum &drum : _drumkits.drums) {
		ofs << "\n";
		ofs << (_drumkits.uses_local ? "." : "") << drum.label << ":\n";
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

	for (Dropdown *dropdown : _drumkit_drum_dropdowns) {
		dropdown->clear();
		for (const Drum &drum : _drumkits.drums) {
			dropdown->add(drum.label.c_str());
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

void Drumkit_Window::regenerate_mod() {
	if (_mod && _audio_thread.joinable()) {
		_audio_mutex.lock();
		_playing_drum = _selected_drum ? Pitch::C_NAT : Pitch::REST;
		_mod_channel = -1;

		Drumkit drumkit = {};
		drumkit.drums[(size_t)_playing_drum] = _selected_drum ? _selected_drum - 1 : 0;

		_drum_samples = generate_noise_samples(_drumkits.drums, drumkit.drums[(size_t)_playing_drum], true);
		_mod->regenerate_it_module({}, { drumkit }, _drum_samples, _playing_drumkit - 1, true);
		_mod->start();
		_audio_mutex.unlock();
	}
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

	dw->stop_audio_thread();
	if (dw->_mod) delete dw->_mod;
	dw->_mod = nullptr;
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

void Drumkit_Window::tabs_cb(Fl_Widget *, Drumkit_Window *dw) {
	dw->stop_audio_thread();
	if (dw->_mod) delete dw->_mod;
	dw->_mod = nullptr;
	dw->_play_button->value(0);

	if (dw->_tabs->value() == dw->_drumkit_tab) {
		for (Dropdown *dropdown : dw->_drumkit_drum_dropdowns) {
			dropdown->clear();
			for (const Drum &drum : dw->_drumkits.drums) {
				dropdown->add(drum.label.c_str());
			}
		}
		select_drumkit_cb(nullptr, dw);
		dw->_drumkit_browser->take_focus();
	}
	else {
		dw->_drum_browser->take_focus();
	}
}

void Drumkit_Window::add_drumkit_cb(Fl_Widget *w, Drumkit_Window *dw) {
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
	dw->_new_name_dialog->title("New Drumkit");
	dw->_new_name_dialog->label("Drumkit Name:");
	while (true) {
		dw->_new_name_dialog->show(dw->_window, reset);
		bool canceled = dw->_new_name_dialog->canceled();
		if (canceled) { return; }

		drumkit_name = dw->_new_name_dialog->get_name();
		if (!is_label_valid(drumkit_name)) {
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
	else if (w == dw->_copy_drumkit_button && dw->drumkit()) {
		new_drumkit.drums = dw->drumkit()->drums;
	}
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

void Drumkit_Window::select_drumkit_cb(Fl_Widget *w, Drumkit_Window *dw) {
	if (Fl::callback_reason() == FL_REASON_RESELECTED && Fl::event_clicks() > 0 && w) {
		assert(dw->_selected_drumkit == dw->_drumkit_browser->value());
		Drumkit *selected_drumkit = dw->drumkit();
		if (!selected_drumkit) return;

		const auto is_label_taken = [&](const std::string &label) {
			if (label == dw->_drumkits.drumkits_label) return true;
			for (const Drumkit &drumkit : dw->_drumkits.drumkits) {
				if (label == drumkit.label) {
					return true;
				}
			}
			for (const Drum &drum : dw->_drumkits.drums) {
				if (label == drum.label) {
					return true;
				}
			}
			return false;
		};

		std::string drumkit_name;
		std::string original_name = selected_drumkit->label;
		dw->_new_name_dialog->title("Rename Drumkit");
		dw->_new_name_dialog->label("Drumkit Name:");
		dw->_new_name_dialog->set_name(original_name.c_str());
		while (true) {
			int focus = Fl::visible_focus();
			Fl::visible_focus(0);
			dw->_new_name_dialog->show(dw->_window, false);
			Fl::visible_focus(focus);
			bool canceled = dw->_new_name_dialog->canceled();
			if (canceled) { return; }

			drumkit_name = dw->_new_name_dialog->get_name();
			if (drumkit_name == original_name) return;
			if (!is_label_valid(drumkit_name)) {
				std::string msg = "Name is invalid! Name can only contain letters, numbers, and underscores, and must not start with a number.";
				dw->_error_dialog->message(msg);
				Fl::visible_focus(0);
				dw->_error_dialog->show(dw->_window);
				Fl::visible_focus(focus);
			}
			else if (is_label_taken(drumkit_name)) {
				std::string msg = "Duplicate label.";
				dw->_error_dialog->message(msg);
				Fl::visible_focus(0);
				dw->_error_dialog->show(dw->_window);
				Fl::visible_focus(focus);
			}
			else {
				break;
			}
		}

		for (size_t i = 0; i < dw->_drumkits.drumkits.size(); ++i) {
			if (dw->_drumkits.drumkits[i].label == original_name) {
				dw->_drumkits.drumkits[i].label = drumkit_name;
				dw->_drumkit_browser->text((int)i+1, drumkit_name.c_str());
			}
		}

		return;
	}

	dw->_selected_drumkit = dw->_drumkit_browser->value();
	if (!dw->_selected_drumkit || dw->_drumkits.drumkits.size() == 1) {
		dw->_remove_drumkit_button->deactivate();
	}
	else {
		dw->_remove_drumkit_button->activate();
	}
	if (!dw->_selected_drumkit) {
		dw->_copy_drumkit_button->deactivate();
	}
	else {
		dw->_copy_drumkit_button->activate();
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
		for (Dropdown *dropdown : dw->_drumkit_drum_dropdowns) {
			dropdown->value(nullptr);
			dropdown->deactivate();
		}
		for (OS_Button *button : dw->_drumkit_drum_buttons) {
			button->deactivate();
		}
	}
	else {
		for (size_t i = 0; i < NUM_DRUMS_PER_DRUMKIT; ++i) {
			dw->_drumkit_drum_dropdowns[i]->value(dw->_drumkits.drumkits[dw->_selected_drumkit-1].drums[i]);
			dw->_drumkit_drum_dropdowns[i]->activate();
		}
		for (OS_Button *button : dw->_drumkit_drum_buttons) {
			button->activate();
		}
	}

	if (dw->_tabs->value() == dw->_drumkit_tab) {
		dw->stop_audio_thread();
		if (dw->_mod) delete dw->_mod;
		dw->_mod = nullptr;
	}
}

void Drumkit_Window::edit_drumkit_cb(Fl_Widget *w, Drumkit_Window *dw) {
	Drumkit *drumkit = dw->drumkit();
	if (!drumkit) return;

	dw->stop_audio_thread();
	if (dw->_mod) delete dw->_mod;
	dw->_mod = nullptr;

	for (size_t i = 0; i < NUM_DRUMS_PER_DRUMKIT; ++i) {
		if (dw->_drumkit_drum_dropdowns[i] == (Dropdown *)w) {
			drumkit->drums[i] = dw->_drumkit_drum_dropdowns[i]->value();
			for (Drumkit &other : dw->_drumkits.drumkits) {
				if (&other != drumkit && other.label == drumkit->label) {
					other.drums[i] = dw->_drumkit_drum_dropdowns[i]->value();
				}
			}
			return;
		}
	}
	assert(false);
}

void Drumkit_Window::play_drumkit_drum_cb(Fl_Widget *w, Drumkit_Window *dw) {
	if (!dw->_selected_drumkit) return;

	dw->stop_audio_thread();
	if (dw->_mod) delete dw->_mod;
	dw->_mod = nullptr;

	Pitch pitch = Pitch::REST;
	for (size_t i = 0; i < NUM_DRUMS_PER_DRUMKIT - 1; ++i) {
		if (dw->_drumkit_drum_buttons[i] == (OS_Button *)w) {
			pitch = (Pitch)(i + 1);
			break;
		}
	}
	assert(pitch != Pitch::REST);

	dw->_playing_drum = pitch;
	dw->_playing_drumkit = dw->_selected_drumkit;
	dw->_mod_channel = -1;

	dw->_drum_samples = generate_noise_samples(dw->_drumkits.drums, dw->_drumkits.drumkits[dw->_selected_drumkit-1].drums[(size_t)pitch]);
	dw->_mod = new IT_Module({}, dw->_drumkits.drumkits, dw->_drum_samples, dw->_selected_drumkit - 1);
	dw->_mod->start();

	dw->start_audio_thread();
}

void Drumkit_Window::add_drum_cb(Fl_Widget *w, Drumkit_Window *dw) {
	const auto is_label_taken = [&](const std::string &label) {
		if (label == dw->_drumkits.drumkits_label) return true;
		for (const Drumkit &drumkit : dw->_drumkits.drumkits) {
			if (label == drumkit.label) {
				return true;
			}
		}
		for (const Drum &drum : dw->_drumkits.drums) {
			if (label == drum.label) {
				return true;
			}
		}
		return false;
	};

	std::string drum_name;
	bool reset = true;
	dw->_new_name_dialog->title("New Drum");
	dw->_new_name_dialog->label("Drum Name:");
	while (true) {
		dw->_new_name_dialog->show(dw->_window, reset);
		bool canceled = dw->_new_name_dialog->canceled();
		if (canceled) { return; }

		drum_name = dw->_new_name_dialog->get_name();
		if (!is_label_valid(drum_name)) {
			std::string msg = "Name is invalid! Name can only contain letters, numbers, and underscores, and must not start with a number.";
			dw->_error_dialog->message(msg);
			dw->_error_dialog->show(dw->_window);
			reset = false;
		}
		else if (is_label_taken(drum_name)) {
			std::string msg = "Duplicate label.";
			dw->_error_dialog->message(msg);
			dw->_error_dialog->show(dw->_window);
			reset = false;
		}
		else {
			break;
		}
	}

	Drum new_drum;
	new_drum.label = drum_name;
	if (w == dw->_copy_drum_button && dw->drum()) {
		new_drum.noise_notes = dw->drum()->noise_notes;
	}
	dw->_drumkits.drums.push_back(new_drum);

	dw->_drum_browser->add(drum_name.c_str());
	dw->_drum_browser->select((int)dw->_drumkits.drums.size());
	select_drum_cb(nullptr, dw);
}

void Drumkit_Window::remove_drum_cb(Fl_Widget *, Drumkit_Window *dw) {
	if (!dw->_selected_drum || dw->_drumkits.drums.size() == 1) return;

	for (Drumkit &drumkit : dw->_drumkits.drumkits) {
		for (int32_t &drum : drumkit.drums) {
			if (drum == dw->_selected_drum - 1) {
				drum = 0;
			}
			else if (drum > dw->_selected_drum - 1) {
				drum -= 1;
			}
		}
	}

	dw->_drumkits.drums.erase(dw->_drumkits.drums.begin() + dw->_selected_drum-1);
	dw->_drum_browser->deselect();
	dw->_drum_browser->remove(dw->_selected_drum);
	select_drum_cb(nullptr, dw);
}

void Drumkit_Window::move_drum_up_cb(Fl_Widget *, Drumkit_Window *dw) {
	if (!dw->_selected_drum || dw->_selected_drum == 1) return;

	for (Drumkit &drumkit : dw->_drumkits.drumkits) {
		for (int32_t &drum : drumkit.drums) {
			if (drum == dw->_selected_drum - 1) {
				drum -= 1;
			}
			else if (drum == dw->_selected_drum - 2) {
				drum += 1;
			}
		}
	}

	std::swap(dw->_drumkits.drums[dw->_selected_drum - 1], dw->_drumkits.drums[dw->_selected_drum - 2]);
	dw->_drum_browser->swap(dw->_selected_drum, dw->_selected_drum - 1);
	dw->_drum_browser->select(dw->_selected_drum - 1);
	select_drum_cb(nullptr, dw);
}

void Drumkit_Window::move_drum_down_cb(Fl_Widget *, Drumkit_Window *dw) {
	if (!dw->_selected_drum || dw->_selected_drum == dw->_drumkits.drums.size()) return;

	for (Drumkit &drumkit : dw->_drumkits.drumkits) {
		for (int32_t &drum : drumkit.drums) {
			if (drum == dw->_selected_drum - 1) {
				drum += 1;
			}
			else if (drum == dw->_selected_drum) {
				drum -= 1;
			}
		}
	}

	std::swap(dw->_drumkits.drums[dw->_selected_drum - 1], dw->_drumkits.drums[dw->_selected_drum]);
	dw->_drum_browser->swap(dw->_selected_drum, dw->_selected_drum + 1);
	dw->_drum_browser->select(dw->_selected_drum + 1);
	select_drum_cb(nullptr, dw);
}

void Drumkit_Window::select_drum_cb(Fl_Widget *w, Drumkit_Window *dw) {
	if (Fl::callback_reason() == FL_REASON_RESELECTED && Fl::event_clicks() > 0 && w) {
		assert(dw->_selected_drum == dw->_drum_browser->value());
		Drum *selected_drum = dw->drum();
		if (!selected_drum) return;

		const auto is_label_taken = [&](const std::string &label) {
			if (label == dw->_drumkits.drumkits_label) return true;
			for (const Drumkit &drumkit : dw->_drumkits.drumkits) {
				if (label == drumkit.label) {
					return true;
				}
			}
			for (const Drum &drum : dw->_drumkits.drums) {
				if (label == drum.label) {
					return true;
				}
			}
			return false;
		};

		std::string drum_name;
		std::string original_name = selected_drum->label;
		dw->_new_name_dialog->title("Rename Drum");
		dw->_new_name_dialog->label("Drum Name:");
		dw->_new_name_dialog->set_name(original_name.c_str());
		while (true) {
			int focus = Fl::visible_focus();
			Fl::visible_focus(0);
			dw->_new_name_dialog->show(dw->_window, false);
			Fl::visible_focus(focus);
			bool canceled = dw->_new_name_dialog->canceled();
			if (canceled) { return; }

			drum_name = dw->_new_name_dialog->get_name();
			if (drum_name == original_name) return;
			if (!is_label_valid(drum_name)) {
				std::string msg = "Name is invalid! Name can only contain letters, numbers, and underscores, and must not start with a number.";
				dw->_error_dialog->message(msg);
				Fl::visible_focus(0);
				dw->_error_dialog->show(dw->_window);
				Fl::visible_focus(focus);
			}
			else if (is_label_taken(drum_name)) {
				std::string msg = "Duplicate label.";
				dw->_error_dialog->message(msg);
				Fl::visible_focus(0);
				dw->_error_dialog->show(dw->_window);
				Fl::visible_focus(focus);
			}
			else {
				break;
			}
		}

		selected_drum->label = drum_name;
		dw->_drum_browser->text(dw->_selected_drum, drum_name.c_str());

		return;
	}

	dw->_selected_drum = dw->_drum_browser->value();
	if (!dw->_selected_drum || dw->_drumkits.drums.size() == 1) {
		dw->_remove_drum_button->deactivate();
	}
	else {
		dw->_remove_drum_button->activate();
	}
	if (!dw->_selected_drum) {
		dw->_copy_drum_button->deactivate();
	}
	else {
		dw->_copy_drum_button->activate();
	}
	if (!dw->_selected_drum || dw->_selected_drum == 1) {
		dw->_drum_up_button->deactivate();
	}
	else {
		dw->_drum_up_button->activate();
	}
	if (!dw->_selected_drum || dw->_selected_drum == dw->_drumkits.drums.size()) {
		dw->_drum_down_button->deactivate();
	}
	else {
		dw->_drum_down_button->activate();
	}
	if (!dw->_selected_drum) {
		dw->_add_note_button->deactivate();
		dw->_remove_note_button->deactivate();
		dw->_drum_note_table->clear();
		dw->_drum_tab->redraw();
	}
	else {
		dw->_add_note_button->activate();
		if (dw->drum()->noise_notes.size() == 0) {
			dw->_remove_note_button->deactivate();
		}
		else {
			dw->_remove_note_button->activate();
		}
		dw->_drum_note_table->set(&dw->_drumkits.drums[dw->_selected_drum-1]);
		dw->_drum_tab->redraw();
	}

	if (dw->_tabs->value() == dw->_drum_tab) dw->regenerate_mod();
}

void Drumkit_Window::add_note_cb(Fl_Widget *, Drumkit_Window *dw) {
	Drum *drum = dw->drum();
	if (!drum) return;

	Noise_Note note = {};
	drum->noise_notes.push_back(note);
	dw->_drum_note_table->add_row();

	dw->_remove_note_button->activate();

	dw->regenerate_mod();
}

void Drumkit_Window::remove_note_cb(Fl_Widget *, Drumkit_Window *dw) {
	Drum *drum = dw->drum();
	if (!drum || drum->noise_notes.size() == 0) return;

	drum->noise_notes.pop_back();
	dw->_drum_note_table->remove_row();

	if (drum->noise_notes.size() == 0) {
		dw->_remove_note_button->deactivate();
	}

	dw->regenerate_mod();
}

void Drumkit_Window::play_drum_cb(Fl_Widget *, Drumkit_Window *dw) {
	dw->stop_audio_thread();
	if (dw->_mod) delete dw->_mod;
	dw->_mod = nullptr;

	if (dw->_play_button->value()) {
		dw->_playing_drum = dw->_selected_drum ? Pitch::C_NAT : Pitch::REST;
		dw->_playing_drumkit = 1;
		dw->_mod_channel = -1;

		Drumkit drumkit = {};
		drumkit.drums[(size_t)dw->_playing_drum] = dw->_selected_drum ? dw->_selected_drum - 1 : 0;

		dw->_drum_samples = generate_noise_samples(dw->_drumkits.drums, drumkit.drums[(size_t)dw->_playing_drum], true);
		dw->_mod = new IT_Module({}, { drumkit }, dw->_drum_samples, dw->_playing_drumkit - 1, true);
		dw->_mod->start();

		dw->start_audio_thread();
	}
}

void Drumkit_Window::playback_thread(Drumkit_Window *dw, std::future<void> kill_signal) {
	Pitch pitch = Pitch::REST;
	int instrument = 0;

	int error_count = 0;
	while (kill_signal.wait_for(std::chrono::milliseconds(8)) == std::future_status::timeout) {
		if (dw->_audio_mutex.try_lock()) {
			IT_Module *mod = dw->_mod;
			if (mod && mod->playing()) {
				if (
					pitch != dw->_playing_drum || instrument != dw->_playing_drumkit ||
					(dw->_mod_channel == -1 && pitch != Pitch::REST && instrument != 0)
				) {
					if (dw->_mod_channel != -1) {
						mod->stop_note(dw->_mod_channel);
						dw->_mod_channel = -1;
					}
					pitch = dw->_playing_drum;
					instrument = dw->_playing_drumkit;
					if (pitch != Pitch::REST && instrument != 0) {
						dw->_mod_channel = mod->play_note(pitch, 0, 4, 0);
					}
				}

				bool success = mod->play();
				if (success) {
					error_count = 0;
				}
				else {
					error_count += 1;
					if (error_count >= 10) mod->stop();
				}
				dw->_audio_mutex.unlock();
			}
			else {
				dw->_audio_mutex.unlock();
				break;
			}
		}
	}
}
