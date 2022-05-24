#pragma warning(push, 0)
#include <FL/Fl.H>
#include <FL/Fl_Multi_Label.H>
#pragma warning(pop)

#include "version.h"
#include "utils.h"
#include "themes.h"
#include "widgets.h"
#include "preferences.h"
#include "config.h"
#include "main-window.h"
#include "icons.h"

#ifdef _WIN32
#include "resource.h"
#else
#include <unistd.h>
#include <X11/xpm.h>
#include "app-icon.xpm"
#endif

Main_Window::Main_Window(int x, int y, int w, int h, const char *) : Fl_Double_Window(x, y, w, h, PROGRAM_NAME),
	_directory(), _asm_file(), _recent(), _song(), _wx(x), _wy(y), _ww(w), _wh(h) {

	for (int i = 0; i < NUM_RECENT; i++) {
		_recent[i] = Preferences::get_string(Fl_Preferences::Name("recent%d", i));
	}

	// Populate window

	int wx = 0, wy = 0, ww = w, wh = h;

	// Initialize menu bar
	_menu_bar = new Fl_Menu_Bar(wx, wy, w, 21);
	wy += _menu_bar->h();
	wh -= _menu_bar->h();

	// Text display
	_channel_1_text_display = new Fl_Text_Display(wx, wy + 21, w/4, wh, "Channel 1");
	wx += _channel_1_text_display->w();
	ww -= _channel_1_text_display->w();
	_channel_1_text_buffer = new Fl_Text_Buffer();
	_channel_1_text_display->buffer(_channel_1_text_buffer);
	_channel_1_text_display->textfont(FL_COURIER);

	_channel_2_text_display = new Fl_Text_Display(wx, wy + 21, w/4, wh, "Channel 2");
	wx += _channel_2_text_display->w();
	ww -= _channel_2_text_display->w();
	_channel_2_text_buffer = new Fl_Text_Buffer();
	_channel_2_text_display->buffer(_channel_2_text_buffer);
	_channel_2_text_display->textfont(FL_COURIER);

	_channel_3_text_display = new Fl_Text_Display(wx, wy + 21, w/4, wh, "Channel 3");
	wx += _channel_3_text_display->w();
	ww -= _channel_3_text_display->w();
	_channel_3_text_buffer = new Fl_Text_Buffer();
	_channel_3_text_display->buffer(_channel_3_text_buffer);
	_channel_3_text_display->textfont(FL_COURIER);

	_channel_4_text_display = new Fl_Text_Display(wx, wy + 21, w/4, wh, "Channel 4");
	wx += _channel_4_text_display->w();
	ww -= _channel_4_text_display->w();
	_channel_4_text_buffer = new Fl_Text_Buffer();
	_channel_4_text_display->buffer(_channel_4_text_buffer);
	_channel_4_text_display->textfont(FL_COURIER);

	// Dialogs
	_asm_open_chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_FILE);
	_error_dialog = new Modal_Dialog(this, "Error", Modal_Dialog::Icon::ERROR_ICON);
	_warning_dialog = new Modal_Dialog(this, "Warning", Modal_Dialog::Icon::WARNING_ICON);
	_success_dialog = new Modal_Dialog(this, "Success", Modal_Dialog::Icon::SUCCESS_ICON);
	_about_dialog = new Modal_Dialog(this, "About " PROGRAM_NAME, Modal_Dialog::Icon::APP_ICON);
	_help_window = new Help_Window(48, 48, 700, 500, PROGRAM_NAME " Help");

	// Configure window
	box(OS_BG_BOX);
	size_range(335, 262);
	callback((Fl_Callback *)exit_cb, this);
	xclass(PROGRAM_NAME);

	// Configure window icon
#ifdef _WIN32
	icon((const void *)LoadIcon(fl_display, MAKEINTRESOURCE(IDI_ICON1)));
#else
	fl_open_display();
	XpmCreatePixmapFromData(fl_display, DefaultRootWindow(fl_display), (char **)&APP_ICON_XPM, &_icon_pixmap, &_icon_mask, NULL);
	icon((const void *)_icon_pixmap);
#endif

	// Configure menu bar
	_menu_bar->box(OS_PANEL_THIN_UP_BOX);
	_menu_bar->down_box(FL_FLAT_BOX);

	// Configure menu bar items
	Fl_Menu_Item menu_items[] = {
		// label, shortcut, callback, data, flags
		OS_SUBMENU("&File"),
		OS_MENU_ITEM("&Open...", FL_COMMAND + 'o', (Fl_Callback *)open_cb, this, 0),
		OS_MENU_ITEM("Open Recent", 0, NULL, NULL, FL_SUBMENU | FL_MENU_DIVIDER),
		// NUM_RECENT items with callback open_recent_cb
		OS_NULL_MENU_ITEM(FL_ALT + '1', (Fl_Callback *)open_recent_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + '2', (Fl_Callback *)open_recent_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + '3', (Fl_Callback *)open_recent_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + '4', (Fl_Callback *)open_recent_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + '5', (Fl_Callback *)open_recent_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + '6', (Fl_Callback *)open_recent_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + '7', (Fl_Callback *)open_recent_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + '8', (Fl_Callback *)open_recent_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + '9', (Fl_Callback *)open_recent_cb, this, 0),
		OS_NULL_MENU_ITEM(FL_ALT + '0', (Fl_Callback *)open_recent_cb, this, 0),
		OS_MENU_ITEM("Clear &Recent", 0, (Fl_Callback *)clear_recent_cb, this, 0),
		{},
		OS_MENU_ITEM("&Close", FL_COMMAND + 'w', (Fl_Callback *)close_cb, this, FL_MENU_DIVIDER),
		OS_MENU_ITEM("E&xit", FL_ALT + FL_F + 4, (Fl_Callback *)exit_cb, this, 0),
		{},
		OS_SUBMENU("&View"),
		OS_MENU_ITEM("&Theme", 0, NULL, NULL, FL_SUBMENU),
		OS_MENU_ITEM("&Classic", 0, (Fl_Callback *)classic_theme_cb, this,
			FL_MENU_RADIO | (OS::current_theme() == OS::Theme::CLASSIC ? FL_MENU_VALUE : 0)),
		OS_MENU_ITEM("&Aero", 0, (Fl_Callback *)aero_theme_cb, this,
			FL_MENU_RADIO | (OS::current_theme() == OS::Theme::AERO ? FL_MENU_VALUE : 0)),
		OS_MENU_ITEM("&Metro", 0, (Fl_Callback *)metro_theme_cb, this,
			FL_MENU_RADIO | (OS::current_theme() == OS::Theme::METRO ? FL_MENU_VALUE : 0)),
		OS_MENU_ITEM("A&qua", 0, (Fl_Callback *)aqua_theme_cb, this,
			FL_MENU_RADIO | (OS::current_theme() == OS::Theme::AQUA ? FL_MENU_VALUE : 0)),
		OS_MENU_ITEM("&Greybird", 0, (Fl_Callback *)greybird_theme_cb, this,
			FL_MENU_RADIO | (OS::current_theme() == OS::Theme::GREYBIRD ? FL_MENU_VALUE : 0)),
		OS_MENU_ITEM("&Ocean", 0, (Fl_Callback *)ocean_theme_cb, this,
			FL_MENU_RADIO | (OS::current_theme() == OS::Theme::OCEAN ? FL_MENU_VALUE : 0)),
		OS_MENU_ITEM("&Blue", 0, (Fl_Callback *)blue_theme_cb, this,
			FL_MENU_RADIO | (OS::current_theme() == OS::Theme::BLUE ? FL_MENU_VALUE : 0)),
		OS_MENU_ITEM("Oli&ve", 0, (Fl_Callback *)olive_theme_cb, this,
			FL_MENU_RADIO | (OS::current_theme() == OS::Theme::OLIVE ? FL_MENU_VALUE : 0)),
		OS_MENU_ITEM("&Rose Gold", 0, (Fl_Callback *)rose_gold_theme_cb, this,
			FL_MENU_RADIO | (OS::current_theme() == OS::Theme::ROSE_GOLD ? FL_MENU_VALUE : 0)),
		OS_MENU_ITEM("&Dark", 0, (Fl_Callback *)dark_theme_cb, this,
			FL_MENU_RADIO | (OS::current_theme() == OS::Theme::DARK ? FL_MENU_VALUE : 0)),
		OS_MENU_ITEM("Brushed Me&tal", 0, (Fl_Callback *)brushed_metal_theme_cb, this,
			FL_MENU_RADIO | (OS::current_theme() == OS::Theme::BRUSHED_METAL ? FL_MENU_VALUE : 0)),
		OS_MENU_ITEM("&High Contrast", 0, (Fl_Callback *)high_contrast_theme_cb, this,
			FL_MENU_RADIO | (OS::current_theme() == OS::Theme::HIGH_CONTRAST ? FL_MENU_VALUE : 0)),
		{},
		{},
		OS_SUBMENU("&Help"),
		OS_MENU_ITEM("&Help", FL_F + 1, (Fl_Callback *)help_cb, this, FL_MENU_DIVIDER),
		OS_MENU_ITEM("&About", FL_COMMAND + '/', (Fl_Callback *)about_cb, this, 0),
		{},
		{}
	};
	_menu_bar->copy(menu_items);

	// Initialize menu bar items
	int first_recent_i = _menu_bar->find_index((Fl_Callback *)open_recent_cb);
	for (int i = 0; i < NUM_RECENT; i++) {
		_recent_mis[i] = const_cast<Fl_Menu_Item *>(&_menu_bar->menu()[first_recent_i + i]);
	}
#define PM_FIND_MENU_ITEM_CB(c) (const_cast<Fl_Menu_Item *>(_menu_bar->find_item((Fl_Callback *)(c))))
	_classic_theme_mi = PM_FIND_MENU_ITEM_CB(classic_theme_cb);
	_aero_theme_mi = PM_FIND_MENU_ITEM_CB(aero_theme_cb);
	_metro_theme_mi = PM_FIND_MENU_ITEM_CB(metro_theme_cb);
	_aqua_theme_mi = PM_FIND_MENU_ITEM_CB(aqua_theme_cb);
	_greybird_theme_mi = PM_FIND_MENU_ITEM_CB(greybird_theme_cb);
	_ocean_theme_mi = PM_FIND_MENU_ITEM_CB(ocean_theme_cb);
	_blue_theme_mi = PM_FIND_MENU_ITEM_CB(blue_theme_cb);
	_olive_theme_mi = PM_FIND_MENU_ITEM_CB(olive_theme_cb);
	_rose_gold_theme_mi = PM_FIND_MENU_ITEM_CB(rose_gold_theme_cb);
	_dark_theme_mi = PM_FIND_MENU_ITEM_CB(dark_theme_cb);
	_brushed_metal_theme_mi = PM_FIND_MENU_ITEM_CB(brushed_metal_theme_cb);
	_high_contrast_theme_mi = PM_FIND_MENU_ITEM_CB(high_contrast_theme_cb);
	// Conditional menu items
	_close_mi = PM_FIND_MENU_ITEM_CB(close_cb);
#undef PM_FIND_MENU_ITEM_CB

	for (int i = 0, md = 0; i < _menu_bar->size(); i++) {
		Fl_Menu_Item *mi = (Fl_Menu_Item *)&_menu_bar->menu()[i];
		if (!mi) { continue; }
		if (md > 0 && mi->label() && !mi->checkbox() && !mi->radio()) {
			Fl_Pixmap *icon = &BLANK_ICON;
			Fl_Multi_Label *ml = new Fl_Multi_Label();
			ml->typea = _FL_IMAGE_LABEL;
			ml->labela = (const char *)icon;
			ml->typeb = FL_NORMAL_LABEL;
			ml->labelb = mi->text;
			mi->image(icon);
			ml->label(mi);
		}
		if (mi->submenu()) { md++; }
		else if (!mi->label()) { md--; }
	}

	// Configure dialogs
	_asm_open_chooser->title("Open Song");
	_asm_open_chooser->filter("ASM Files\t*.asm\n");

	_error_dialog->width_range(280, 700);
	_warning_dialog->width_range(280, 700);
	_success_dialog->width_range(280, 700);

	std::string subject(PROGRAM_NAME " " PROGRAM_VERSION_STRING), message(
		"Copyright \xc2\xa9 " CURRENT_YEAR " " PROGRAM_AUTHOR ".\n"
		"\n"
		"Source code is available at:\n"
		"https://github.com/dannye/crystal-tracker"
	);
	_about_dialog->subject(subject);
	_about_dialog->message(message);
	_about_dialog->width_range(280, 700);

	_help_window->content(
#include "help.html" // a C++11 raw string literal
	);

	update_recent_songs();
	update_active_controls();
}

Main_Window::~Main_Window() {
	delete _menu_bar; // includes menu items
	delete _channel_1_text_display;
	delete _channel_2_text_display;
	delete _channel_3_text_display;
	delete _channel_4_text_display;
	delete _channel_1_text_buffer;
	delete _channel_2_text_buffer;
	delete _channel_3_text_buffer;
	delete _channel_4_text_buffer;
	delete _asm_open_chooser;
	delete _error_dialog;
	delete _warning_dialog;
	delete _success_dialog;
	delete _about_dialog;
	delete _help_window;
}

void Main_Window::show() {
	Fl_Double_Window::show();
}

bool Main_Window::maximized() const {
#ifdef _WIN32
	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);
	if (!GetWindowPlacement(fl_xid(this), &wp)) { return false; }
	return wp.showCmd == SW_MAXIMIZE;
#else
	Atom wmState = XInternAtom(fl_display, "_NET_WM_STATE", True);
	Atom actual;
	int format;
	unsigned long numItems, bytesAfter;
	unsigned char *properties = NULL;
	int result = XGetWindowProperty(fl_display, fl_xid(this), wmState, 0, 1024, False, AnyPropertyType, &actual, &format,
		&numItems, &bytesAfter, &properties);
	int numMax = 0;
	if (result == Success && format == 32 && properties) {
		Atom maxVert = XInternAtom(fl_display, "_NET_WM_STATE_MAXIMIZED_VERT", False);
		Atom maxHorz = XInternAtom(fl_display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
		for (unsigned long i = 0; i < numItems; i++) {
			Atom property = ((Atom *)properties)[i];
			if (property == maxVert || property == maxHorz) {
				numMax++;
			}
		}
		XFree(properties);
	}
	return numMax == 2;
#endif
}

void Main_Window::maximize() {
#ifdef _WIN32
	ShowWindow(fl_xid(this), SW_MAXIMIZE);
#else
	XEvent event;
	memset(&event, 0, sizeof(event));
	event.xclient.type = ClientMessage;
	event.xclient.window = fl_xid(this);
	event.xclient.message_type = XInternAtom(fl_display, "_NET_WM_STATE", False);
	event.xclient.format = 32;
	event.xclient.data.l[0] = 1;
	event.xclient.data.l[1] = XInternAtom(fl_display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
	event.xclient.data.l[2] = XInternAtom(fl_display, "_NET_WM_STATE_MAXIMIZED_VERT", False);
	event.xclient.data.l[3] = 1;
	XSendEvent(fl_display, DefaultRootWindow(fl_display), False, SubstructureNotifyMask | SubstructureNotifyMask, &event);
#endif
}

void Main_Window::update_active_controls() {
	if (_song.loaded()) {
		_close_mi->activate();
	}
	else {
		_close_mi->deactivate();
	}
}

void Main_Window::store_recent_song() {
	std::string last(_asm_file);
	for (int i = 0; i < NUM_RECENT; i++) {
		if (_recent[i] == _asm_file) {
			_recent[i] = last;
			break;
		}
		std::swap(last, _recent[i]);
	}
	update_recent_songs();
}

void Main_Window::update_recent_songs() {
	int last = -1;
	for (int i = 0; i < NUM_RECENT; i++) {
		Fl_Multi_Label *ml = (Fl_Multi_Label *)_recent_mis[i]->label();
		if (ml->labelb[0]) {
			delete ml->labelb;
			ml->labelb = "";
		}
		if (_recent[i].empty()) {
			_recent_mis[i]->hide();
		}
		else {
			const char *basename = fl_filename_name(_recent[i].c_str());
			char *label = new char[FL_PATH_MAX]();
			strcpy(label, OS_MENU_ITEM_PREFIX);
			strcat(label, basename);
			strcat(label, OS_MENU_ITEM_SUFFIX);
			ml->labelb = label;
			_recent_mis[i]->show();
			last = i;
		}
		_recent_mis[i]->flags &= ~FL_MENU_DIVIDER;
	}
	if (last > -1) {
		_recent_mis[last]->flags |= FL_MENU_DIVIDER;
	}
}

void Main_Window::open_song(const char *filename) {
	const char *basename = fl_filename_name(filename);

	char directory[FL_PATH_MAX] = {};
	if (!Config::project_path_from_asm_path(filename, directory)) {
		std::string msg = "Could not find the project directory for\n";
		msg = msg + basename + "!\nMake sure it contains a Makefile.";
		_error_dialog->message(msg);
		_error_dialog->show(this);
		return;
	}

	open_song(directory, filename);
}

void Main_Window::open_song(const char *directory, const char *filename) {
	close_cb(NULL, this);

	_directory = directory;
	if (filename) {
		_asm_file = filename;
	}
	else {
		_asm_file = "";
	}

	const char *basename;

	if (filename) {
		basename = fl_filename_name(_asm_file.c_str());
		Song::Result r = _song.read_song(filename);
		if (r != Song::Result::SONG_OK) {
			_song.clear();
			std::string msg = "Error reading ";
			msg = msg + basename + "!\n\n" + Song::error_message(r);
			_error_dialog->message(msg);
			_error_dialog->show(this);
			return;
		}
		_channel_1_text_buffer->append(_song.channel_1_commands_str().c_str());
		_channel_2_text_buffer->append(_song.channel_2_commands_str().c_str());
		_channel_3_text_buffer->append(_song.channel_3_commands_str().c_str());
		_channel_4_text_buffer->append(_song.channel_4_commands_str().c_str());
	}
	else {
		basename = NEW_SONG_NAME;
		// TODO: initialize new _song
	}

	// set filenames
	char buffer[FL_PATH_MAX] = {};
	sprintf(buffer, PROGRAM_NAME " - %s", basename);
	copy_label(buffer);

	update_active_controls();

	store_recent_song();

	redraw();
}

void Main_Window::open_recent(int n) {
	if (n < 0 || n >= NUM_RECENT || _recent[n].empty()) {
		return;
	}

	const char *filename = _recent[n].c_str();
	open_song(filename);
}

void Main_Window::open_cb(Fl_Widget *, Main_Window *mw) {
	int status = mw->_asm_open_chooser->show();
	if (status == 1) { return; }

	const char *filename = mw->_asm_open_chooser->filename();
	const char *basename = fl_filename_name(filename);
	if (status == -1) {
		std::string msg = "Could not open ";
		msg = msg + basename + "!\n\n" + mw->_asm_open_chooser->errmsg();
		mw->_error_dialog->message(msg);
		mw->_error_dialog->show(mw);
		return;
	}

	mw->open_song(filename);
}

void Main_Window::open_recent_cb(Fl_Menu_ *m, Main_Window *mw) {
	int first_recent_i = m->find_index((Fl_Callback *)open_recent_cb);
	int i = m->find_index(m->mvalue()) - first_recent_i;
	mw->open_recent(i);
}

void Main_Window::clear_recent_cb(Fl_Menu_ *, Main_Window *mw) {
	for (int i = 0; i < NUM_RECENT; i++) {
		mw->_recent[i].clear();
		mw->_recent_mis[i]->hide();
	}
}

void Main_Window::close_cb(Fl_Widget *, Main_Window *mw) {
	if (!mw->_song.loaded()) { return; }

	mw->label(PROGRAM_NAME);
	mw->_channel_1_text_buffer->remove(0, mw->_channel_1_text_buffer->length());
	mw->_channel_2_text_buffer->remove(0, mw->_channel_2_text_buffer->length());
	mw->_channel_3_text_buffer->remove(0, mw->_channel_3_text_buffer->length());
	mw->_channel_4_text_buffer->remove(0, mw->_channel_4_text_buffer->length());
	mw->_song.clear();
	mw->init_sizes();
	mw->_directory.clear();
	mw->_asm_file.clear();

	mw->update_active_controls();
	mw->redraw();
}

void Main_Window::exit_cb(Fl_Widget *, Main_Window *mw) {
	// Override default behavior of Esc to close main window
	if (Fl::event() == FL_SHORTCUT && Fl::event_key() == FL_Escape) { return; }

	// Save global config
	Preferences::set("theme", (int)OS::current_theme());
	if (mw->maximized()) {
#ifdef _WIN32
		HWND hwnd = fl_xid(mw);
		WINDOWPLACEMENT wp;
		wp.length = sizeof(wp);
		if (GetWindowPlacement(hwnd, &wp)) {
			// Get the window border size
			RECT br;
			SetRectEmpty(&br);
			DWORD styleEx = GetWindowLong(hwnd, GWL_EXSTYLE);
			AdjustWindowRectEx(&br, WS_OVERLAPPEDWINDOW, FALSE, styleEx);
			// Subtract the border size from the normal window position
			RECT wr = wp.rcNormalPosition;
			wr.left -= br.left;
			wr.right -= br.right;
			wr.top -= br.top;
			wr.bottom -= br.bottom;
			Preferences::set("x", wr.left);
			Preferences::set("y", wr.top);
			Preferences::set("w", wr.right - wr.left);
			Preferences::set("h", wr.bottom - wr.top);
		}
		else {
			Preferences::set("x", mw->x());
			Preferences::set("y", mw->y());
			Preferences::set("w", mw->w());
			Preferences::set("h", mw->h());
		}
#else
		Preferences::set("x", mw->_wx);
		Preferences::set("y", mw->_wy);
		Preferences::set("w", mw->_ww);
		Preferences::set("h", mw->_wh);
#endif
	}
	else {
		Preferences::set("x", mw->x());
		Preferences::set("y", mw->y());
		Preferences::set("w", mw->w());
		Preferences::set("h", mw->h());
	}
	Preferences::set("maximized", mw->maximized());
	for (int i = 0; i < NUM_RECENT; i++) {
		Preferences::set_string(Fl_Preferences::Name("recent%d", i), mw->_recent[i]);
	}

	Preferences::close();

	exit(EXIT_SUCCESS);
}

void Main_Window::classic_theme_cb(Fl_Menu_ *, Main_Window *mw) {
	OS::use_classic_theme();
	mw->_classic_theme_mi->setonly();
	mw->redraw();
}

void Main_Window::aero_theme_cb(Fl_Menu_ *, Main_Window *mw) {
	OS::use_aero_theme();
	mw->_aero_theme_mi->setonly();
	mw->redraw();
}

void Main_Window::metro_theme_cb(Fl_Menu_ *, Main_Window *mw) {
	OS::use_metro_theme();
	mw->_metro_theme_mi->setonly();
	mw->redraw();
}

void Main_Window::aqua_theme_cb(Fl_Menu_ *, Main_Window *mw) {
	OS::use_aqua_theme();
	mw->_aqua_theme_mi->setonly();
	mw->redraw();
}

void Main_Window::greybird_theme_cb(Fl_Menu_ *, Main_Window *mw) {
	OS::use_greybird_theme();
	mw->_greybird_theme_mi->setonly();
	mw->redraw();
}

void Main_Window::ocean_theme_cb(Fl_Menu_ *, Main_Window *mw) {
	OS::use_ocean_theme();
	mw->_ocean_theme_mi->setonly();
	mw->redraw();
}

void Main_Window::blue_theme_cb(Fl_Menu_ *, Main_Window *mw) {
	OS::use_blue_theme();
	mw->_blue_theme_mi->setonly();
	mw->redraw();
}

void Main_Window::olive_theme_cb(Fl_Menu_ *, Main_Window *mw) {
	OS::use_olive_theme();
	mw->_olive_theme_mi->setonly();
	mw->redraw();
}

void Main_Window::rose_gold_theme_cb(Fl_Menu_ *, Main_Window *mw) {
	OS::use_rose_gold_theme();
	mw->_rose_gold_theme_mi->setonly();
	mw->redraw();
}

void Main_Window::dark_theme_cb(Fl_Menu_ *, Main_Window *mw) {
	OS::use_dark_theme();
	mw->_dark_theme_mi->setonly();
	mw->redraw();
}

void Main_Window::brushed_metal_theme_cb(Fl_Menu_ *, Main_Window *mw) {
	OS::use_brushed_metal_theme();
	mw->_brushed_metal_theme_mi->setonly();
	mw->redraw();
}

void Main_Window::high_contrast_theme_cb(Fl_Menu_ *, Main_Window *mw) {
	OS::use_high_contrast_theme();
	mw->_high_contrast_theme_mi->setonly();
	mw->redraw();
}

void Main_Window::help_cb(Fl_Widget *, Main_Window *mw) {
	mw->_help_window->show(mw);
}

void Main_Window::about_cb(Fl_Widget *, Main_Window *mw) {
	mw->_about_dialog->show(mw);
}
