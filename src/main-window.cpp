#pragma warning(push, 0)
#include <FL/Fl.H>
#include <FL/Fl_Multi_Label.H>
#pragma warning(pop)

#include "version.h"
#include "utils.h"
#include "themes.h"
#include "preferences.h"
#include "config.h"
#include "main-window.h"
#include "icons.h"

#ifdef _WIN32
#include "resource.h"
#elif defined(__APPLE__)
#include "cocoa.h"
#elif defined(__X11__)
#include <unistd.h>
#include <X11/xpm.h>
#include "app-icon.xpm"
#endif

#ifdef __APPLE__
	constexpr int MENU_BAR_HEIGHT = 0;
	constexpr int TOOLBAR_HEIGHT = 38;
#else
	constexpr int MENU_BAR_HEIGHT = 21;
	constexpr int TOOLBAR_HEIGHT = 26;
#endif
constexpr int TOOLBAR_BUTTON_HEIGHT = 24;
constexpr int STATUS_BAR_HEIGHT = 23;

Main_Window::Main_Window(int x, int y, int w, int h, const char *) : Fl_Double_Window(x, y, w, h, PROGRAM_NAME),
	_wx(x), _wy(y), _ww(w), _wh(h) {

	// Get global configs
	int loop_config = Preferences::get("loop", 1);
	int zoom_config = Preferences::get("zoom", 0);
	int key_labels_config = Preferences::get("key_labels", 1);

	for (int i = 0; i < NUM_RECENT; i++) {
		_recent[i] = Preferences::get_string(Fl_Preferences::Name("recent%d", i));
	}

	int fullscreen = Preferences::get("fullscreen", 0);

	// Populate window

	int wx = 0, wy = 0, ww = w, wh = h;

	// Initialize menu bar
	_menu_bar = new Fl_Sys_Menu_Bar(wx, wy, ww, MENU_BAR_HEIGHT);
	wy += _menu_bar->h();
	wh -= _menu_bar->h();

	// Toolbar
	_toolbar = new Toolbar(wx, wy, ww, TOOLBAR_HEIGHT);
#ifdef __APPLE__
#define SEPARATE_TOOLBAR_BUTTONS new Fl_Box(0, 0, 12, TOOLBAR_HEIGHT - 2);
	new Fl_Box(0, 0, 6, TOOLBAR_HEIGHT - 2);
#else
#define SEPARATE_TOOLBAR_BUTTONS new Fl_Box(0, 0, 2, TOOLBAR_BUTTON_HEIGHT); new Spacer(0, 0, 2, TOOLBAR_BUTTON_HEIGHT); new Fl_Box(0, 0, 2, TOOLBAR_BUTTON_HEIGHT)
#endif
	wy += _toolbar->h();
	wh -= _toolbar->h();
	_new_tb = new Toolbar_Button(0, 0, TOOLBAR_BUTTON_HEIGHT, TOOLBAR_BUTTON_HEIGHT);
	_open_tb = new Toolbar_Button(0, 0, TOOLBAR_BUTTON_HEIGHT, TOOLBAR_BUTTON_HEIGHT);
	_save_tb = new Toolbar_Button(0, 0, TOOLBAR_BUTTON_HEIGHT, TOOLBAR_BUTTON_HEIGHT);
	_save_as_tb = new Toolbar_Button(0, 0, TOOLBAR_BUTTON_HEIGHT, TOOLBAR_BUTTON_HEIGHT);
	SEPARATE_TOOLBAR_BUTTONS;
	_play_pause_tb = new Toolbar_Button(0, 0, TOOLBAR_BUTTON_HEIGHT, TOOLBAR_BUTTON_HEIGHT);
	_stop_tb = new Toolbar_Button(0, 0, TOOLBAR_BUTTON_HEIGHT, TOOLBAR_BUTTON_HEIGHT);
	_loop_tb = new Toolbar_Toggle_Button(0, 0, TOOLBAR_BUTTON_HEIGHT, TOOLBAR_BUTTON_HEIGHT);
	_continuous_tb = new Toolbar_Toggle_Button(0, 0, TOOLBAR_BUTTON_HEIGHT, TOOLBAR_BUTTON_HEIGHT);
	SEPARATE_TOOLBAR_BUTTONS;
	_undo_tb = new Toolbar_Button(0, 0, TOOLBAR_BUTTON_HEIGHT, TOOLBAR_BUTTON_HEIGHT);
	_redo_tb = new Toolbar_Button(0, 0, TOOLBAR_BUTTON_HEIGHT, TOOLBAR_BUTTON_HEIGHT);
	SEPARATE_TOOLBAR_BUTTONS;
	_channel_1_tb = new Toolbar_Radio_Button(0, 0, TOOLBAR_BUTTON_HEIGHT, TOOLBAR_BUTTON_HEIGHT);
	_channel_1_tb->when(FL_WHEN_RELEASE_ALWAYS);
	_channel_2_tb = new Toolbar_Radio_Button(0, 0, TOOLBAR_BUTTON_HEIGHT, TOOLBAR_BUTTON_HEIGHT);
	_channel_2_tb->when(FL_WHEN_RELEASE_ALWAYS);
	_channel_3_tb = new Toolbar_Radio_Button(0, 0, TOOLBAR_BUTTON_HEIGHT, TOOLBAR_BUTTON_HEIGHT);
	_channel_3_tb->when(FL_WHEN_RELEASE_ALWAYS);
	_channel_4_tb = new Toolbar_Radio_Button(0, 0, TOOLBAR_BUTTON_HEIGHT, TOOLBAR_BUTTON_HEIGHT);
	_channel_4_tb->when(FL_WHEN_RELEASE_ALWAYS);
	SEPARATE_TOOLBAR_BUTTONS;
	_zoom_tb = new Toolbar_Toggle_Button(0, 0, TOOLBAR_BUTTON_HEIGHT, TOOLBAR_BUTTON_HEIGHT);
	_decrease_spacing_tb = new Toolbar_Button(0, 0, TOOLBAR_BUTTON_HEIGHT, TOOLBAR_BUTTON_HEIGHT);
	_increase_spacing_tb = new Toolbar_Button(0, 0, TOOLBAR_BUTTON_HEIGHT, TOOLBAR_BUTTON_HEIGHT);
	_toolbar->end();
	begin();

	// Status bar
	_status_bar = new Toolbar(wx, h - STATUS_BAR_HEIGHT, ww, STATUS_BAR_HEIGHT);
	wh -= _status_bar->h();
	_status_label = new Label(0, 0, ww, STATUS_BAR_HEIGHT - 2, _status_message.c_str());
	_status_bar->end();
	begin();

	// Piano Roll
	_piano_roll = new Piano_Roll(wx, wy, ww, wh);

	// Dialogs
	_new_dir_chooser = new Directory_Chooser(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
	_asm_open_chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_FILE);
	_asm_save_chooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
	_error_dialog = new Modal_Dialog(this, "Error", Modal_Dialog::Icon::ERROR_ICON);
	_warning_dialog = new Modal_Dialog(this, "Warning", Modal_Dialog::Icon::WARNING_ICON);
	_success_dialog = new Modal_Dialog(this, "Success", Modal_Dialog::Icon::SUCCESS_ICON);
	_unsaved_dialog = new Modal_Dialog(this, "Warning", Modal_Dialog::Icon::WARNING_ICON, true);
	_about_dialog = new Modal_Dialog(this, "About " PROGRAM_NAME, Modal_Dialog::Icon::APP_ICON);
	_help_window = new Help_Window(48, 48, 700, 500, PROGRAM_NAME " Help");

	// Configure window
	box(OS_BG_BOX);
	callback((Fl_Callback *)exit_cb, this);
	xclass(PROGRAM_NAME);

	// Configure window icon
#ifdef _WIN32
	icon((const void *)LoadIcon(fl_display, MAKEINTRESOURCE(IDI_ICON1)));
#elif defined(__X11__)
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
		OS_MENU_ITEM("&New...", FL_COMMAND + 'n', (Fl_Callback *)new_cb, this, 0),
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
		OS_MENU_ITEM("&Save", FL_COMMAND + 's', (Fl_Callback *)save_cb, this, 0),
#ifdef __APPLE__
		OS_MENU_ITEM("Save &As...", FL_COMMAND + 'S', (Fl_Callback *)save_as_cb, this, 0),
#else
		OS_MENU_ITEM("Save &As...", FL_COMMAND + 'S', (Fl_Callback *)save_as_cb, this, FL_MENU_DIVIDER),
		OS_MENU_ITEM("E&xit", FL_ALT + FL_F + 4, (Fl_Callback *)exit_cb, this, 0),
#endif
		{},
		OS_SUBMENU("&Play"),
		OS_MENU_ITEM("&Play/Pause", ' ', (Fl_Callback *)play_pause_cb, this, 0),
		OS_MENU_ITEM("&Stop", ESCAPE_KEY, (Fl_Callback *)stop_cb, this, FL_MENU_DIVIDER),
		OS_MENU_ITEM("&Loop", FL_COMMAND + 'l', (Fl_Callback *)loop_cb, this,
			FL_MENU_TOGGLE | (loop_config ? FL_MENU_VALUE : 0)),
		OS_MENU_ITEM("&Continuous Scroll", '\\', (Fl_Callback *)continuous_cb, this, FL_MENU_TOGGLE | FL_MENU_VALUE | FL_MENU_DIVIDER),
		OS_MENU_ITEM("Mute Channel &1", FL_F + 5, (Fl_Callback *)channel_1_mute_cb, this, FL_MENU_TOGGLE),
		OS_MENU_ITEM("Mute Channel &2", FL_F + 6, (Fl_Callback *)channel_2_mute_cb, this, FL_MENU_TOGGLE),
		OS_MENU_ITEM("Mute Channel &3", FL_F + 7, (Fl_Callback *)channel_3_mute_cb, this, FL_MENU_TOGGLE),
		OS_MENU_ITEM("Mute Channel &4", FL_F + 8, (Fl_Callback *)channel_4_mute_cb, this, FL_MENU_TOGGLE | FL_MENU_DIVIDER),
		OS_MENU_ITEM("Step Backward", '[', (Fl_Callback *)step_backward_cb, this, 0),
		OS_MENU_ITEM("Step Forward", ']', (Fl_Callback *)step_forward_cb, this, FL_MENU_DIVIDER),
		OS_MENU_ITEM("Skip Backward", FL_COMMAND + '[', (Fl_Callback *)skip_backward_cb, this, 0),
		OS_MENU_ITEM("Skip Forward", FL_COMMAND + ']', (Fl_Callback *)skip_forward_cb, this, 0),
		{},
		OS_SUBMENU("&Edit"),
		OS_MENU_ITEM("&Undo", FL_COMMAND + 'z', (Fl_Callback *)undo_cb, this, 0),
		OS_MENU_ITEM("&Redo", FL_COMMAND + 'y', (Fl_Callback *)redo_cb, this, FL_MENU_DIVIDER),
		OS_MENU_ITEM("Select &All", FL_COMMAND + 'a', (Fl_Callback *)select_all_cb, this, 0),
		OS_MENU_ITEM("Select N&one", FL_COMMAND + 'A', (Fl_Callback *)select_none_cb, this, FL_MENU_DIVIDER),
		OS_MENU_ITEM("&Selection...", 0, NULL, NULL, FL_SUBMENU | FL_MENU_DIVIDER),
		OS_MENU_ITEM("Pitch Up", FL_COMMAND + UP_KEY, (Fl_Callback *)pitch_up_cb, this, 0),
		OS_MENU_ITEM("Pitch Down", FL_COMMAND + DOWN_KEY, (Fl_Callback *)pitch_down_cb, this, 0),
		OS_MENU_ITEM("Octave Up", FL_COMMAND + FL_SHIFT + UP_KEY, (Fl_Callback *)octave_up_cb, this, 0),
		OS_MENU_ITEM("Octave Down", FL_COMMAND + FL_SHIFT + DOWN_KEY, (Fl_Callback *)octave_down_cb, this, FL_MENU_DIVIDER),
		OS_MENU_ITEM("Move Left", FL_COMMAND + LEFT_KEY, (Fl_Callback *)move_left_cb, this, 0),
		OS_MENU_ITEM("Move Right", FL_COMMAND + RIGHT_KEY, (Fl_Callback *)move_right_cb, this, 0),
		OS_MENU_ITEM("Shorten", FL_COMMAND + FL_SHIFT + LEFT_KEY, (Fl_Callback *)shorten_cb, this, 0),
		OS_MENU_ITEM("Lengthen", FL_COMMAND + FL_SHIFT + RIGHT_KEY, (Fl_Callback *)lengthen_cb, this, FL_MENU_DIVIDER),
		OS_MENU_ITEM("&Delete Selection", DELETE_KEY, (Fl_Callback *)delete_cb, this, 0),
		OS_MENU_ITEM("&Snip Selection", FL_SHIFT + DELETE_KEY, (Fl_Callback *)snip_cb, this, 0),
		{},
		OS_MENU_ITEM("Spli&t Note", '/', (Fl_Callback *)split_note_cb, this, 0),
		OS_MENU_ITEM("&Glue Note", GLUE_KEY, (Fl_Callback *)glue_note_cb, this, FL_MENU_DIVIDER),
		OS_MENU_ITEM("Channel &1", '1', (Fl_Callback *)channel_1_cb, this,
			FL_MENU_RADIO | (selected_channel() == 1 ? FL_MENU_VALUE : 0)),
		OS_MENU_ITEM("Channel &2", '2', (Fl_Callback *)channel_2_cb, this,
			FL_MENU_RADIO | (selected_channel() == 2 ? FL_MENU_VALUE : 0)),
		OS_MENU_ITEM("Channel &3", '3', (Fl_Callback *)channel_3_cb, this,
			FL_MENU_RADIO | (selected_channel() == 3 ? FL_MENU_VALUE : 0)),
		OS_MENU_ITEM("Channel &4", '4', (Fl_Callback *)channel_4_cb, this,
			FL_MENU_RADIO | (selected_channel() == 4 ? FL_MENU_VALUE : 0) | FL_MENU_DIVIDER),
		OS_MENU_ITEM("&Next Channel", TAB_KEY, (Fl_Callback *)next_channel_cb, this, 0),
		OS_MENU_ITEM("&Previous Channel", FL_SHIFT + TAB_KEY, (Fl_Callback *)previous_channel_cb, this, 0),
		{},
		OS_SUBMENU("&View"),
		OS_MENU_ITEM("&Theme", 0, NULL, NULL, FL_SUBMENU | FL_MENU_DIVIDER),
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
		OS_MENU_ITEM("&Zoom", FL_COMMAND + '=', (Fl_Callback *)zoom_cb, this,
			FL_MENU_DIVIDER | FL_MENU_TOGGLE | (zoom_config ? FL_MENU_VALUE : 0)),
		OS_MENU_ITEM("Decrease Spacing", FL_SHIFT + '-', (Fl_Callback *)decrease_spacing_cb, this, 0),
		OS_MENU_ITEM("Increase Spacing", FL_SHIFT + '=', (Fl_Callback *)increase_spacing_cb, this, FL_MENU_DIVIDER),
		OS_MENU_ITEM("Key &Labels", FL_COMMAND + '0', (Fl_Callback *)key_labels_cb, this,
			FL_MENU_DIVIDER | FL_MENU_TOGGLE | (key_labels_config ? FL_MENU_VALUE : 0)),
		OS_MENU_ITEM("Full &Screen", FULLSCREEN_KEY, (Fl_Callback *)full_screen_cb, this,
			FL_MENU_TOGGLE | (fullscreen ? FL_MENU_VALUE : 0)),
		{},
		OS_SUBMENU("&Help"),
#ifdef __APPLE__
		OS_MENU_ITEM("&Help", FL_F + 1, (Fl_Callback *)help_cb, this, 0),
#else
		OS_MENU_ITEM("&Help", FL_F + 1, (Fl_Callback *)help_cb, this, FL_MENU_DIVIDER),
		OS_MENU_ITEM("&About", FL_COMMAND + '/', (Fl_Callback *)about_cb, this, 0),
#endif
		{},
		{}
	};
	_menu_bar->copy(menu_items);

	// Initialize macOS application menu
#ifdef __APPLE__
	Fl_Mac_App_Menu::about = "About " PROGRAM_NAME;
	Fl_Mac_App_Menu::hide = "Hide " PROGRAM_NAME;
	Fl_Mac_App_Menu::quit = "Quit " PROGRAM_NAME;
	fl_mac_set_about((Fl_Callback *)about_cb, this);
	fl_open_display();
	_menu_bar->update();
#endif

	// Initialize menu bar items
	int first_recent_i = _menu_bar->find_index((Fl_Callback *)open_recent_cb);
	for (int i = 0; i < NUM_RECENT; i++) {
		_recent_mis[i] = const_cast<Fl_Menu_Item *>(&_menu_bar->menu()[first_recent_i + i]);
	}
#define CT_FIND_MENU_ITEM_CB(c) (const_cast<Fl_Menu_Item *>(_menu_bar->find_item((Fl_Callback *)(c))))
	_classic_theme_mi = CT_FIND_MENU_ITEM_CB(classic_theme_cb);
	_aero_theme_mi = CT_FIND_MENU_ITEM_CB(aero_theme_cb);
	_metro_theme_mi = CT_FIND_MENU_ITEM_CB(metro_theme_cb);
	_aqua_theme_mi = CT_FIND_MENU_ITEM_CB(aqua_theme_cb);
	_greybird_theme_mi = CT_FIND_MENU_ITEM_CB(greybird_theme_cb);
	_ocean_theme_mi = CT_FIND_MENU_ITEM_CB(ocean_theme_cb);
	_blue_theme_mi = CT_FIND_MENU_ITEM_CB(blue_theme_cb);
	_olive_theme_mi = CT_FIND_MENU_ITEM_CB(olive_theme_cb);
	_rose_gold_theme_mi = CT_FIND_MENU_ITEM_CB(rose_gold_theme_cb);
	_dark_theme_mi = CT_FIND_MENU_ITEM_CB(dark_theme_cb);
	_brushed_metal_theme_mi = CT_FIND_MENU_ITEM_CB(brushed_metal_theme_cb);
	_high_contrast_theme_mi = CT_FIND_MENU_ITEM_CB(high_contrast_theme_cb);
	_continuous_mi = CT_FIND_MENU_ITEM_CB(continuous_cb);
	_channel_1_mute_mi = CT_FIND_MENU_ITEM_CB(channel_1_mute_cb);
	_channel_2_mute_mi = CT_FIND_MENU_ITEM_CB(channel_2_mute_cb);
	_channel_3_mute_mi = CT_FIND_MENU_ITEM_CB(channel_3_mute_cb);
	_channel_4_mute_mi = CT_FIND_MENU_ITEM_CB(channel_4_mute_cb);
	_zoom_mi = CT_FIND_MENU_ITEM_CB(zoom_cb);
	_key_labels_mi = CT_FIND_MENU_ITEM_CB(key_labels_cb);
	_full_screen_mi = CT_FIND_MENU_ITEM_CB(full_screen_cb);
	// Conditional menu items
	_close_mi = CT_FIND_MENU_ITEM_CB(close_cb);
	_save_mi = CT_FIND_MENU_ITEM_CB(save_cb);
	_save_as_mi = CT_FIND_MENU_ITEM_CB(save_as_cb);
	_play_pause_mi = CT_FIND_MENU_ITEM_CB(play_pause_cb);
	_stop_mi = CT_FIND_MENU_ITEM_CB(stop_cb);
	_loop_mi = CT_FIND_MENU_ITEM_CB(loop_cb);
	_step_backward_mi = CT_FIND_MENU_ITEM_CB(step_backward_cb);
	_step_forward_mi = CT_FIND_MENU_ITEM_CB(step_forward_cb);
	_skip_backward_mi = CT_FIND_MENU_ITEM_CB(skip_backward_cb);
	_skip_forward_mi = CT_FIND_MENU_ITEM_CB(skip_forward_cb);
	_undo_mi = CT_FIND_MENU_ITEM_CB(undo_cb);
	_redo_mi = CT_FIND_MENU_ITEM_CB(redo_cb);
	_select_all_mi = CT_FIND_MENU_ITEM_CB(select_all_cb);
	_select_none_mi = CT_FIND_MENU_ITEM_CB(select_none_cb);
	_pitch_up_mi = CT_FIND_MENU_ITEM_CB(pitch_up_cb);
	_pitch_down_mi = CT_FIND_MENU_ITEM_CB(pitch_down_cb);
	_octave_up_mi = CT_FIND_MENU_ITEM_CB(octave_up_cb);
	_octave_down_mi = CT_FIND_MENU_ITEM_CB(octave_down_cb);
	_move_left_mi = CT_FIND_MENU_ITEM_CB(move_left_cb);
	_move_right_mi = CT_FIND_MENU_ITEM_CB(move_right_cb);
	_shorten_mi = CT_FIND_MENU_ITEM_CB(shorten_cb);
	_lengthen_mi = CT_FIND_MENU_ITEM_CB(lengthen_cb);
	_delete_mi = CT_FIND_MENU_ITEM_CB(delete_cb);
	_snip_mi = CT_FIND_MENU_ITEM_CB(snip_cb);
	_split_note_mi = CT_FIND_MENU_ITEM_CB(split_note_cb);
	_glue_note_mi = CT_FIND_MENU_ITEM_CB(glue_note_cb);
	_channel_1_mi = CT_FIND_MENU_ITEM_CB(channel_1_cb);
	_channel_2_mi = CT_FIND_MENU_ITEM_CB(channel_2_cb);
	_channel_3_mi = CT_FIND_MENU_ITEM_CB(channel_3_cb);
	_channel_4_mi = CT_FIND_MENU_ITEM_CB(channel_4_cb);
	_next_channel_mi = CT_FIND_MENU_ITEM_CB(next_channel_cb);
	_previous_channel_mi = CT_FIND_MENU_ITEM_CB(previous_channel_cb);
	_decrease_spacing_mi = CT_FIND_MENU_ITEM_CB(decrease_spacing_cb);
	_increase_spacing_mi = CT_FIND_MENU_ITEM_CB(increase_spacing_cb);
#undef CT_FIND_MENU_ITEM_CB

#ifndef __APPLE__
	// Create a multi-label to offset menu entries that don't have a checkbox or radio button
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
#endif

	// Configure toolbar buttons

	_new_tb->tooltip("New... (" COMMAND_KEY_PLUS "N)");
	_new_tb->callback((Fl_Callback *)new_cb, this);
	_new_tb->image(NEW_ICON);
	_new_tb->take_focus();

	_open_tb->tooltip("Open... (" COMMAND_KEY_PLUS "O)");
	_open_tb->callback((Fl_Callback *)open_cb, this);
	_open_tb->image(OPEN_ICON);

	_save_tb->tooltip("Save (" COMMAND_KEY_PLUS "S)");
	_save_tb->callback((Fl_Callback *)save_cb, this);
	_save_tb->image(SAVE_ICON);

	_save_as_tb->tooltip("Save As... (" COMMAND_SHIFT_KEYS_PLUS "S)");
	_save_as_tb->callback((Fl_Callback *)save_as_cb, this);
	_save_as_tb->image(SAVE_AS_ICON);

	_play_pause_tb->tooltip("Play/Pause (Spacebar)");
	_play_pause_tb->callback((Fl_Callback *)play_pause_cb, this);
	_play_pause_tb->image(PLAY_ICON);

	_stop_tb->tooltip("Stop (Esc)");
	_stop_tb->callback((Fl_Callback *)stop_cb, this);
	_stop_tb->image(STOP_ICON);

	_loop_tb->tooltip("Loop (" COMMAND_KEY_PLUS "L)");
	_loop_tb->callback((Fl_Callback *)loop_tb_cb, this);
	_loop_tb->image(LOOP_ICON);
	_loop_tb->value(loop());

	_continuous_tb->tooltip("Continuous Scroll (\\)");
	_continuous_tb->callback((Fl_Callback *)continuous_tb_cb, this);
	_continuous_tb->image(SCROLL_LIGHT_ICON);
	_continuous_tb->value(continuous_scroll());

	_undo_tb->tooltip("Undo (" COMMAND_KEY_PLUS "Z)");
	_undo_tb->callback((Fl_Callback *)undo_cb, this);
	_undo_tb->image(UNDO_ICON);

	_redo_tb->tooltip("Redo (" COMMAND_KEY_PLUS "Y)");
	_redo_tb->callback((Fl_Callback *)redo_cb, this);
	_redo_tb->image(REDO_ICON);

	_channel_1_tb->tooltip("Channel 1 (1)");
	_channel_1_tb->callback((Fl_Callback *)channel_1_tb_cb, this);
	_channel_1_tb->image(ONE_ICON);
	_channel_1_tb->value(selected_channel() == 1);

	_channel_2_tb->tooltip("Channel 2 (2)");
	_channel_2_tb->callback((Fl_Callback *)channel_2_tb_cb, this);
	_channel_2_tb->image(TWO_ICON);
	_channel_2_tb->value(selected_channel() == 2);

	_channel_3_tb->tooltip("Channel 3 (3)");
	_channel_3_tb->callback((Fl_Callback *)channel_3_tb_cb, this);
	_channel_3_tb->image(THREE_ICON);
	_channel_3_tb->value(selected_channel() == 3);

	_channel_4_tb->tooltip("Channel 4 (4)");
	_channel_4_tb->callback((Fl_Callback *)channel_4_tb_cb, this);
	_channel_4_tb->image(FOUR_ICON);
	_channel_4_tb->value(selected_channel() == 4);

	_zoom_tb->tooltip("Zoom (" COMMAND_KEY_PLUS "=)");
	_zoom_tb->callback((Fl_Callback *)zoom_tb_cb, this);
	_zoom_tb->image(ZOOM_ICON);
	_zoom_tb->value(zoom());

	_decrease_spacing_tb->tooltip("Decrease Spacing (" SHIFT_KEY_PLUS "-)");
	_decrease_spacing_tb->callback((Fl_Callback *)decrease_spacing_cb, this);
	_decrease_spacing_tb->image(DECREASE_SPACING_ICON);

	_increase_spacing_tb->tooltip("Increase Spacing (" SHIFT_KEY_PLUS "=)");
	_increase_spacing_tb->callback((Fl_Callback *)increase_spacing_cb, this);
	_increase_spacing_tb->image(INCREASE_SPACING_ICON);

	// Configure dialogs

	_new_dir_chooser->title("Choose Project Directory");

	_asm_open_chooser->title("Open Song");
	_asm_open_chooser->filter("ASM Files\t*.asm\n");

	_asm_save_chooser->title("Save Song");
	_asm_save_chooser->filter("ASM Files\t*.asm\n");
	_asm_save_chooser->options(Fl_Native_File_Chooser::Option::SAVEAS_CONFIRM);
	_asm_save_chooser->preset_file("NewSong.asm");

	_error_dialog->width_range(280, 700);
	_warning_dialog->width_range(280, 700);
	_success_dialog->width_range(280, 700);
	_unsaved_dialog->width_range(280, 700);

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

	update_icons();
	update_recent_songs();
	update_active_controls();
	update_zoom();

	_piano_roll->key_labels(key_labels());

	_piano_roll->scroll_to_y_max();

	start_interactive_thread();
}

Main_Window::~Main_Window() {
	stop_audio_thread();
	stop_interactive_thread();

	delete _menu_bar; // includes menu items
	delete _toolbar; // includes toolbar buttons
	delete _status_bar; // includes status bar fields
	delete _piano_roll;
	delete _new_dir_chooser;
	delete _asm_open_chooser;
	delete _asm_save_chooser;
	delete _error_dialog;
	delete _warning_dialog;
	delete _success_dialog;
	delete _unsaved_dialog;
	delete _about_dialog;
	delete _help_window;
	if (_it_module) {
		delete _it_module;
	}
}

void Main_Window::show() {
	Fl_Double_Window::show();
}

void Main_Window::resize(int X, int Y, int W, int H) {
	Fl_Double_Window::resize(X, Y, W, H);
	_menu_bar->resize(0, 0, W, _menu_bar->h());
	_toolbar->resize(0, _menu_bar->h(), W, _toolbar->h());
	_piano_roll->set_size(W, H - MENU_BAR_HEIGHT - TOOLBAR_HEIGHT - STATUS_BAR_HEIGHT);
	_status_bar->resize(0, H - STATUS_BAR_HEIGHT, W, _status_bar->h());
}

bool Main_Window::maximized() const {
#ifdef _WIN32
	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);
	if (!GetWindowPlacement(fl_xid(this), &wp)) { return false; }
	return wp.showCmd == SW_MAXIMIZE;
#elif defined(__APPLE__)
	return cocoa_is_maximized(this);
#elif defined(__X11__)
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
#elif defined(__APPLE__)
	cocoa_maximize(this);
#elif defined(__X11__)
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

bool Main_Window::unsaved() const {
	return _song.loaded() && _song.modified();
}

const char *Main_Window::modified_filename() {
	if (_asm_file.empty()) { return NEW_SONG_NAME; }
	return fl_filename_name(_asm_file.c_str());
}

int Main_Window::handle(int event) {
	switch (event) {
	case FL_FOCUS:
	case FL_UNFOCUS:
		return 1;
	case FL_KEYBOARD:
#ifdef __APPLE__
		if (Fl::event_shift() && Fl::event_key() == FL_Tab && _previous_channel_mi->active()) {
			previous_channel_cb(nullptr, this);
			return 1;
		}
		if (Fl::event_shift() && Fl::event_key() == '_' && _decrease_spacing_mi->active()) {
			decrease_spacing_cb(nullptr, this);
			return 1;
		}
		if (Fl::event_shift() && Fl::event_key() == '+' && _increase_spacing_mi->active()) {
			increase_spacing_cb(nullptr, this);
			return 1;
		}
#endif
		if (!Fl::event_state(FL_SHIFT | FL_COMMAND | FL_ALT) && (!_it_module || _it_module->stopped())) {
			switch (Fl::event_key()) {
			case 'r':
				put_note(Pitch::REST);
				return 1;
			case 'z':
				put_note(Pitch::C_NAT);
				return 1;
			case 's':
				put_note(Pitch::C_SHARP);
				return 1;
			case 'x':
				put_note(Pitch::D_NAT);
				return 1;
			case 'd':
				put_note(Pitch::D_SHARP);
				return 1;
			case 'c':
				put_note(Pitch::E_NAT);
				return 1;
			case 'v':
				put_note(Pitch::F_NAT);
				return 1;
			case 'g':
				put_note(Pitch::F_SHARP);
				return 1;
			case 'b':
				put_note(Pitch::G_NAT);
				return 1;
			case 'h':
				put_note(Pitch::G_SHARP);
				return 1;
			case 'n':
				put_note(Pitch::A_NAT);
				return 1;
			case 'j':
				put_note(Pitch::A_SHARP);
				return 1;
			case 'm':
				put_note(Pitch::B_NAT);
				return 1;
			}
		}
		break;
	}
	return Fl_Double_Window::handle(event);
}

void Main_Window::set_song_position(int32_t tick) {
	_audio_mutex.lock();
	if (_it_module) {
		_it_module->set_tick(tick);
	}
	_audio_mutex.unlock();
}

bool Main_Window::play_note(Pitch pitch, int32_t octave) {
	if (pitch != _playing_pitch || octave != _playing_octave) {
		_interactive_mutex.lock();
		_playing_pitch = pitch;
		_playing_octave = octave;
		_interactive_mutex.unlock();
		return true;
	}
	return false;
}

bool Main_Window::stop_note() {
	if (_playing_pitch != Pitch::REST) {
		_interactive_mutex.lock();
		_playing_pitch = Pitch::REST;
		_playing_octave = 0;
		_interactive_mutex.unlock();
		return true;
	}
	return false;
}

void Main_Window::update_active_controls() {
	if (_song.loaded()) {
		bool playing = _it_module && _it_module->playing();
		bool stopped = !_it_module || _it_module->stopped();
		_close_mi->activate();
		_save_mi->activate();
		_save_tb->activate();
		_save_as_mi->activate();
		_save_as_tb->activate();
		_play_pause_mi->activate();
		_play_pause_tb->activate();
		if (playing) {
			_play_pause_tb->image(PAUSE_ICON);
		}
		else {
			_play_pause_tb->image(PLAY_ICON);
		}
		_play_pause_tb->redraw();
		if (!stopped) {
			_stop_mi->activate();
			_stop_tb->activate();
			_loop_mi->deactivate();
			_loop_tb->deactivate();
		}
		else {
			_stop_mi->deactivate();
			_stop_tb->deactivate();
			_loop_mi->activate();
			_loop_tb->activate();
		}
		if (playing) {
			_step_backward_mi->deactivate();
			_step_forward_mi->deactivate();
			_skip_backward_mi->deactivate();
			_skip_forward_mi->deactivate();
		}
		else {
			_step_backward_mi->activate();
			_step_forward_mi->activate();
			_skip_backward_mi->activate();
			_skip_forward_mi->activate();
		}
		if (_song.can_undo() && stopped) {
			_undo_mi->activate();
			_undo_tb->activate();
		}
		else {
			_undo_mi->deactivate();
			_undo_tb->deactivate();
		}
		if (_song.can_redo() && stopped) {
			_redo_mi->activate();
			_redo_tb->activate();
		}
		else {
			_redo_mi->deactivate();
			_redo_tb->deactivate();
		}
		if (stopped) {
			_select_all_mi->activate();
			_select_none_mi->activate();
			_pitch_up_mi->activate();
			_pitch_down_mi->activate();
			_octave_up_mi->activate();
			_octave_down_mi->activate();
			_move_left_mi->activate();
			_move_right_mi->activate();
			_shorten_mi->activate();
			_lengthen_mi->activate();
			_delete_mi->activate();
			_snip_mi->activate();
			_split_note_mi->activate();
			_glue_note_mi->activate();
		}
		else {
			_select_all_mi->deactivate();
			_select_none_mi->deactivate();
			_pitch_up_mi->deactivate();
			_pitch_down_mi->deactivate();
			_octave_up_mi->deactivate();
			_octave_down_mi->deactivate();
			_move_left_mi->deactivate();
			_move_right_mi->deactivate();
			_shorten_mi->deactivate();
			_lengthen_mi->deactivate();
			_delete_mi->deactivate();
			_snip_mi->deactivate();
			_split_note_mi->deactivate();
			_glue_note_mi->deactivate();
		}
		_channel_1_mi->activate();
		_channel_1_tb->activate();
		_channel_2_mi->activate();
		_channel_2_tb->activate();
		_channel_3_mi->activate();
		_channel_3_tb->activate();
		_channel_4_mi->activate();
		_channel_4_tb->activate();
		_next_channel_mi->activate();
		_previous_channel_mi->activate();
	}
	else {
		_close_mi->deactivate();
		_save_mi->deactivate();
		_save_tb->deactivate();
		_save_as_mi->deactivate();
		_save_as_tb->deactivate();
		_play_pause_mi->deactivate();
		_play_pause_tb->deactivate();
		_play_pause_tb->image(PLAY_ICON);
		_play_pause_tb->redraw();
		_stop_mi->deactivate();
		_stop_tb->deactivate();
		_loop_mi->activate();
		_loop_tb->activate();
		_step_backward_mi->deactivate();
		_step_forward_mi->deactivate();
		_skip_backward_mi->deactivate();
		_skip_forward_mi->deactivate();
		_undo_mi->deactivate();
		_undo_tb->deactivate();
		_redo_mi->deactivate();
		_redo_tb->deactivate();
		_select_all_mi->deactivate();
		_select_none_mi->deactivate();
		_pitch_up_mi->deactivate();
		_pitch_down_mi->deactivate();
		_octave_up_mi->deactivate();
		_octave_down_mi->deactivate();
		_move_left_mi->deactivate();
		_move_right_mi->deactivate();
		_shorten_mi->deactivate();
		_lengthen_mi->deactivate();
		_delete_mi->deactivate();
		_snip_mi->deactivate();
		_split_note_mi->deactivate();
		_glue_note_mi->deactivate();
		_channel_1_mi->deactivate();
		_channel_1_tb->deactivate();
		_channel_2_mi->deactivate();
		_channel_2_tb->deactivate();
		_channel_3_mi->deactivate();
		_channel_3_tb->deactivate();
		_channel_4_mi->deactivate();
		_channel_4_tb->deactivate();
		_next_channel_mi->deactivate();
		_previous_channel_mi->deactivate();
	}

	_menu_bar->update();
}

void Main_Window::update_channel_detail() {
	_piano_roll->set_channel_1_detailed(selected_channel() == 0 || selected_channel() == 1);
	_piano_roll->set_channel_2_detailed(selected_channel() == 0 || selected_channel() == 2);
	_piano_roll->set_channel_3_detailed(selected_channel() == 0 || selected_channel() == 3);
	_piano_roll->set_channel_4_detailed(selected_channel() == 0 || selected_channel() == 4);
	_piano_roll->align_cursor();
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
#ifndef __APPLE__
		Fl_Multi_Label *ml = (Fl_Multi_Label *)_recent_mis[i]->label();
		if (ml->labelb[0]) {
			delete [] ml->labelb;
			ml->labelb = "";
		}
#endif
		if (_recent[i].empty()) {
			_recent_mis[i]->hide();
		}
		else {
			const char *basename = fl_filename_name(_recent[i].c_str());
#ifndef __APPLE__
			char *label = new char[FL_PATH_MAX]();
			strcpy(label, OS_MENU_ITEM_PREFIX);
			strcat(label, basename);
			strcat(label, OS_MENU_ITEM_SUFFIX);
			ml->labelb = label;
#else
			_recent_mis[i]->label(basename);
#endif
			_recent_mis[i]->show();
			last = i;
		}
		_recent_mis[i]->flags &= ~FL_MENU_DIVIDER;
	}
	if (last > -1) {
		_recent_mis[last]->flags |= FL_MENU_DIVIDER;
	}
	_menu_bar->update();
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
	_song.modified(false);
	close_cb(NULL, this);

	_directory = directory;
	_new_dir_chooser->directory(directory);
	_asm_open_chooser->directory(directory);
	_asm_save_chooser->directory(directory);
	if (filename) {
		_asm_file = filename;
	}
	else {
		_asm_file = "";
	}

	Parsed_Waves parsed_waves(directory);
	if (parsed_waves.result() != Parsed_Waves::Result::WAVES_OK) {
		_directory.clear();
		std::string msg = "Error reading wave definitions!";
		_error_dialog->message(msg);
		_error_dialog->show(this);
		return;
	}
	_waves = parsed_waves.waves();

	const char *basename;

	if (filename) {
		basename = fl_filename_name(_asm_file.c_str());
		Parsed_Song::Result r = _song.read_song(filename);
		if (r != Parsed_Song::Result::SONG_OK) {
			_song.clear();
			std::string msg = "Error reading ";
			msg = msg + basename + "!\n\n" + _song.error_message();
			_error_dialog->message(msg);
			_error_dialog->show(this);
			return;
		}
		_waves.insert(_waves.end(), RANGE(_song.waves()));
		_piano_roll->set_timeline(_song);
	}
	else {
		basename = NEW_SONG_NAME;
		_song.modified(true);
		_song.new_song();
	}

	// set filenames
	char buffer[FL_PATH_MAX] = {};
	sprintf(buffer, PROGRAM_NAME " - %s", basename);
	copy_label(buffer);

	_status_message = "Opened ";
	_status_message += basename;
	_status_label->label(_status_message.c_str());

	update_active_controls();

	if (filename) {
		store_recent_song();
	}

	redraw();
}

void Main_Window::open_recent(int n) {
	if (n < 0 || n >= NUM_RECENT || _recent[n].empty()) {
		return;
	}

	if (unsaved()) {
		std::string msg = modified_filename();
		msg = msg + " has unsaved changes!\n\n"
			"Open another song anyway?";
		_unsaved_dialog->message(msg);
		_unsaved_dialog->show(this);
		if (_unsaved_dialog->canceled()) { return; }
	}

	const char *filename = _recent[n].c_str();
	open_song(filename);
}

bool Main_Window::save_song(bool force) {
	const char *filename = _asm_file.c_str();
	const char *basename = fl_filename_name(filename);

	if (_song.modified() && _song.other_modified(filename)) {
		std::string msg = basename;
		msg = msg + " was modified by another program!\n\n"
			"Save the song and overwrite it anyway?";
		_unsaved_dialog->message(msg);
		_unsaved_dialog->show(this);
		if (_unsaved_dialog->canceled()) { return true; }
	}

	if (_song.modified() || force) {
		if (!_song.write_song(filename)) {
			std::string msg = "Could not write to ";
			msg = msg + basename + "!";
			_error_dialog->message(msg);
			_error_dialog->show(this);
			return false;
		}

		_song.modified(false);
	}

	if (force) {
		store_recent_song();
	}

	std::string msg = "Saved ";
	msg = msg + basename + "!";
	_success_dialog->message(msg);
	_success_dialog->show(this);

	_status_message = "Saved ";
	_status_message += basename;
	_status_label->label(_status_message.c_str());

	return true;
}

void Main_Window::toggle_playback() {
	stop_audio_thread();

	if (!_it_module || _it_module->stopped()) {
		if (_it_module) {
			delete _it_module;
		}
		int32_t loop_tick = loop() ? _piano_roll->get_loop_tick() : -1;
		_it_module = new IT_Module(
			_piano_roll->channel_1_notes(),
			_piano_roll->channel_2_notes(),
			_piano_roll->channel_3_notes(),
			_piano_roll->channel_4_notes(),
			_waves,
			loop_tick
		);
		_it_module->mute_channel(1, channel_1_muted());
		_it_module->mute_channel(2, channel_2_muted());
		_it_module->mute_channel(3, channel_3_muted());
		_it_module->mute_channel(4, channel_4_muted());
		if (_it_module->ready() && _it_module->start()) {
			_tick = _piano_roll->tick();
			if (_tick != -1) {
				_it_module->set_tick(_tick);
			}
			_piano_roll->start_following();
			stop_note();
			start_audio_thread();
			update_active_controls();
		}
		else {
			std::string msg = "There was a problem starting playback!";
			_error_dialog->message(msg);
			_error_dialog->show(this);
			return;
		}
	}
	else if (_it_module->paused()) {
		if (_it_module->ready() && _it_module->start()) {
			_piano_roll->unpause_following();
			stop_note();
			start_audio_thread();
			update_active_controls();
		}
		else {
			std::string msg = "There was a problem resuming playback!";
			_error_dialog->message(msg);
			_error_dialog->show(this);
			return;
		}
	}
	else { // if (_it_module->playing())
		_it_module->pause();
		_piano_roll->pause_following();
		update_active_controls();
	}
}

void Main_Window::stop_playback() {
	stop_audio_thread();

	if (_it_module && !_it_module->stopped()) {
		_it_module->stop();
		_tick = -1;
		_piano_roll->stop_following();
		update_active_controls();
	}
}

void Main_Window::start_audio_thread() {
	_audio_kill_signal = std::promise<void>();
	std::future<void> kill_future = _audio_kill_signal.get_future();
	_audio_thread = std::thread(&playback_thread, this, std::move(kill_future));
}

void Main_Window::stop_audio_thread() {
	if (_audio_thread.joinable()) {
		_audio_mutex.lock();
		_audio_kill_signal.set_value();
		_audio_thread.join();
		_audio_mutex.unlock();
	}
}

void Main_Window::start_interactive_thread() {
	_interactive_kill_signal = std::promise<void>();
	std::future<void> kill_future = _interactive_kill_signal.get_future();
	_interactive_thread = std::thread(&interactive_thread, this, std::move(kill_future));
}

void Main_Window::stop_interactive_thread() {
	if (_interactive_thread.joinable()) {
		_interactive_mutex.lock();
		_interactive_kill_signal.set_value();
		_interactive_thread.join();
		_interactive_mutex.unlock();
	}
}

void Main_Window::update_icons() {
	bool dark = OS::is_dark_theme(OS::current_theme());
	_continuous_tb->image(dark ? SCROLL_DARK_ICON : SCROLL_LIGHT_ICON);
	make_deimage(_new_tb);
	make_deimage(_open_tb);
	make_deimage(_save_tb);
	make_deimage(_save_as_tb);
	make_deimage(_play_pause_tb);
	make_deimage(_stop_tb);
	make_deimage(_loop_tb);
	make_deimage(_continuous_tb);
	make_deimage(_undo_tb);
	make_deimage(_redo_tb);
	make_deimage(_channel_1_tb);
	make_deimage(_channel_2_tb);
	make_deimage(_channel_3_tb);
	make_deimage(_channel_4_tb);
	make_deimage(_zoom_tb);
	make_deimage(_decrease_spacing_tb);
	make_deimage(_increase_spacing_tb);
}

void Main_Window::update_zoom() {
	_piano_roll->zoom(zoom());
	size_range(
		WHITE_KEY_WIDTH * 3 + 15,
		_piano_roll->octave_height() + MENU_BAR_HEIGHT + TOOLBAR_HEIGHT + STATUS_BAR_HEIGHT + 15,
		0,
		_piano_roll->octave_height() * NUM_OCTAVES + MENU_BAR_HEIGHT + TOOLBAR_HEIGHT + STATUS_BAR_HEIGHT + 15
	);
}

void Main_Window::new_cb(Fl_Widget *, Main_Window *mw) {
	if (mw->unsaved()) {
		std::string msg = mw->modified_filename();
		msg = msg + " has unsaved changes!\n\n"
			"Create a new song anyway?";
		mw->_unsaved_dialog->message(msg);
		mw->_unsaved_dialog->show(mw);
		if (mw->_unsaved_dialog->canceled()) { return; }
	}

	char directory[FL_PATH_MAX] = {};

	if (!mw->_directory.size()) {
		int status = mw->_new_dir_chooser->show();
		if (status == 1) { return; }
		if (status == -1) {
			std::string msg = "Could not get project directory!";
			mw->_error_dialog->message(msg);
			mw->_error_dialog->show(mw);
			return;
		}

		const char *project_dir = mw->_new_dir_chooser->filename();
		strcpy(directory, project_dir);
		strcat(directory, DIR_SEP);
	}
	else {
		strcpy(directory, mw->_directory.c_str());
	}

	Config::project_path_from_asm_path(directory, directory);
	mw->open_song(directory, NULL);
}

void Main_Window::open_cb(Fl_Widget *, Main_Window *mw) {
	if (mw->unsaved()) {
		std::string msg = mw->modified_filename();
		msg = msg + " has unsaved changes!\n\n"
			"Open another song anyway?";
		mw->_unsaved_dialog->message(msg);
		mw->_unsaved_dialog->show(mw);
		if (mw->_unsaved_dialog->canceled()) { return; }
	}

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

void Main_Window::clear_recent_cb(Fl_Widget *, Main_Window *mw) {
	for (int i = 0; i < NUM_RECENT; i++) {
		mw->_recent[i].clear();
		mw->_recent_mis[i]->hide();
	}
	mw->_menu_bar->update();
}

void Main_Window::close_cb(Fl_Widget *, Main_Window *mw) {
	if (!mw->_song.loaded()) { return; }

	if (mw->unsaved()) {
		std::string msg = mw->modified_filename();
		msg = msg + " has unsaved changes!\n\n"
			"Close it anyway?";
		mw->_unsaved_dialog->message(msg);
		mw->_unsaved_dialog->show(mw);
		if (mw->_unsaved_dialog->canceled()) { return; }
	}

	mw->stop_audio_thread();

	const char *basename;
	if (mw->_asm_file.size()) {
		basename = fl_filename_name(mw->_asm_file.c_str());
	}
	else {
		basename = NEW_SONG_NAME;
	}
	mw->_status_message = "Closed ";
	mw->_status_message += basename;

	mw->label(PROGRAM_NAME);
	mw->_piano_roll->clear();
	mw->_song.clear();
	mw->_waves.clear();
	if (mw->_it_module) {
		delete mw->_it_module;
		mw->_it_module = nullptr;
	}
	mw->init_sizes();
	mw->_directory.clear();
	mw->_asm_file.clear();

	mw->selected_channel(0);
	mw->_channel_1_mi->clear();
	mw->_channel_1_tb->clear();
	mw->_channel_2_mi->clear();
	mw->_channel_2_tb->clear();
	mw->_channel_3_mi->clear();
	mw->_channel_3_tb->clear();
	mw->_channel_4_mi->clear();
	mw->_channel_4_tb->clear();

	mw->update_active_controls();
	mw->_status_label->label(mw->_status_message.c_str());
	mw->redraw();
}

void Main_Window::save_cb(Fl_Widget *w, Main_Window *mw) {
	if (!mw->_song.loaded()) { return; }

	if (mw->_asm_file.empty()) {
		save_as_cb(w, mw);
	}
	else {
		mw->save_song(false);
	}
}

void Main_Window::save_as_cb(Fl_Widget *, Main_Window *mw) {
	if (!mw->_song.loaded()) { return; }

	int status = mw->_asm_save_chooser->show();
	if (status == 1) { return; }

	char filename[FL_PATH_MAX] = {};
	add_dot_ext(mw->_asm_save_chooser->filename(), ".asm", filename);
	const char *basename = fl_filename_name(filename);

	if (status == -1) {
		std::string msg = "Could not open ";
		msg = msg + basename + "!\n\n" + mw->_asm_save_chooser->errmsg();
		mw->_error_dialog->message(msg);
		mw->_error_dialog->show(mw);
		return;
	}

	char directory[FL_PATH_MAX] = {};
	if (!Config::project_path_from_asm_path(filename, directory)) {
		std::string msg = "Could not get project directory for ";
		msg = msg + basename + "!";
		mw->_error_dialog->message(msg);
		mw->_error_dialog->show(mw);
		return;
	}

	mw->_directory.assign(directory);
	mw->_asm_file.assign(filename);

	char buffer[FL_PATH_MAX] = {};
	sprintf(buffer, PROGRAM_NAME " - %s", basename);
	mw->copy_label(buffer);

	mw->save_song(true);
}

void Main_Window::exit_cb(Fl_Widget *, Main_Window *mw) {
	// Override default behavior of Esc to close main window
	if (Fl::event() == FL_SHORTCUT && Fl::event_key() == FL_Escape) { return; }

	if (mw->unsaved()) {
		std::string msg = mw->modified_filename();
		msg = msg + " has unsaved changes!\n\n"
			"Exit anyway?";
		mw->_unsaved_dialog->message(msg);
		mw->_unsaved_dialog->show(mw);
		if (mw->_unsaved_dialog->canceled()) { return; }
	}

	// Save global config
	Preferences::set("theme", (int)OS::current_theme());
#ifdef __APPLE__
	if (cocoa_is_fullscreen(mw)) {
#else
	if (mw->full_screen()) {
#endif
		Preferences::set("x", mw->_wx);
		Preferences::set("y", mw->_wy);
		Preferences::set("w", mw->_ww);
		Preferences::set("h", mw->_wh);
		Preferences::set("fullscreen", 1);
	}
	else if (mw->maximized()) {
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
		Preferences::set("fullscreen", 0);
	}
	else {
		Preferences::set("x", mw->x());
		Preferences::set("y", mw->y());
		Preferences::set("w", mw->w());
		Preferences::set("h", mw->h());
		Preferences::set("fullscreen", 0);
	}
	Preferences::set("maximized", mw->maximized());
	Preferences::set("loop", mw->loop());
	Preferences::set("zoom", mw->zoom());
	Preferences::set("key_labels", mw->key_labels());
	for (int i = 0; i < NUM_RECENT; i++) {
		Preferences::set_string(Fl_Preferences::Name("recent%d", i), mw->_recent[i]);
	}

	Preferences::close();

	exit(EXIT_SUCCESS);
}

void Main_Window::play_pause_cb(Fl_Widget *, Main_Window *mw) {
	mw->toggle_playback();
}

void Main_Window::stop_cb(Fl_Widget *, Main_Window *mw) {
	mw->stop_playback();
}

#define SYNC_TB_WITH_M(tb, m) tb->value(m->mvalue()->value())

void Main_Window::loop_cb(Fl_Menu_ *m, Main_Window *mw) {
	SYNC_TB_WITH_M(mw->_loop_tb, m);
	mw->redraw();
}

void Main_Window::continuous_cb(Fl_Menu_ *m, Main_Window *mw) {
	SYNC_TB_WITH_M(mw->_continuous_tb, m);
	mw->_piano_roll->set_continuous_scroll(mw->continuous_scroll());
	mw->redraw();
}

void Main_Window::zoom_cb(Fl_Menu_ *m, Main_Window *mw) {
	SYNC_TB_WITH_M(mw->_zoom_tb, m);
	mw->update_zoom();
	mw->redraw();
}

#undef SYNC_TB_WITH_M

#define SYNC_MI_WITH_TB(tb, mi) if (tb->value()) mi->set(); else mi->clear()

void Main_Window::loop_tb_cb(Toolbar_Toggle_Button *, Main_Window *mw) {
	SYNC_MI_WITH_TB(mw->_loop_tb, mw->_loop_mi);
	mw->_menu_bar->update();
	mw->redraw();
}

void Main_Window::continuous_tb_cb(Toolbar_Toggle_Button *, Main_Window *mw) {
	SYNC_MI_WITH_TB(mw->_continuous_tb, mw->_continuous_mi);
	mw->_piano_roll->set_continuous_scroll(mw->continuous_scroll());
	mw->_menu_bar->update();
	mw->redraw();
}

void Main_Window::zoom_tb_cb(Toolbar_Toggle_Button *, Main_Window *mw) {
	SYNC_MI_WITH_TB(mw->_zoom_tb, mw->_zoom_mi);
	mw->update_zoom();
	mw->_menu_bar->update();
	mw->redraw();
}

#undef SYNC_MI_WITH_TB

void Main_Window::channel_1_mute_cb(Fl_Widget *, Main_Window *mw) {
	mw->_piano_roll->channel_1_muted(mw->channel_1_muted());
	if (mw->_it_module) {
		mw->_it_module->mute_channel(1, mw->channel_1_muted());
	}
}

void Main_Window::channel_2_mute_cb(Fl_Widget *, Main_Window *mw) {
	mw->_piano_roll->channel_2_muted(mw->channel_2_muted());
	if (mw->_it_module) {
		mw->_it_module->mute_channel(2, mw->channel_2_muted());
	}
}

void Main_Window::channel_3_mute_cb(Fl_Widget *, Main_Window *mw) {
	mw->_piano_roll->channel_3_muted(mw->channel_3_muted());
	if (mw->_it_module) {
		mw->_it_module->mute_channel(3, mw->channel_3_muted());
	}
}

void Main_Window::channel_4_mute_cb(Fl_Widget *, Main_Window *mw) {
	mw->_piano_roll->channel_4_muted(mw->channel_4_muted());
	if (mw->_it_module) {
		mw->_it_module->mute_channel(4, mw->channel_4_muted());
	}
}

void Main_Window::step_backward_cb(Fl_Widget *, Main_Window *mw) {
	mw->_piano_roll->step_backward();
	mw->_piano_roll->focus_cursor(true);
	mw->redraw();
}

void Main_Window::step_forward_cb(Fl_Widget *, Main_Window *mw) {
	mw->_piano_roll->step_forward();
	mw->_piano_roll->focus_cursor(true);
	mw->redraw();
}

void Main_Window::skip_backward_cb(Fl_Widget *, Main_Window *mw) {
	mw->_piano_roll->skip_backward();
	mw->_piano_roll->focus_cursor(true);
	mw->redraw();
}

void Main_Window::skip_forward_cb(Fl_Widget *, Main_Window *mw) {
	mw->_piano_roll->skip_forward();
	mw->_piano_roll->focus_cursor(true);
	mw->redraw();
}

void Main_Window::put_note(Pitch pitch) {
	if (!_song.loaded()) { return; }
	if (_piano_roll->put_note(_song, pitch)) {
		_status_message = _song.undo_action_message();
		_status_label->label(_status_message.c_str());

		update_active_controls();
		redraw();
	}
}

void Main_Window::undo_cb(Fl_Widget *, Main_Window *mw) {
	if (!mw->_song.loaded()) { return; }

	mw->_status_message = "Undo: ";
	mw->_status_message += mw->_song.undo_action_message();
	mw->_status_label->label(mw->_status_message.c_str());

	int tick = mw->_song.undo_tick();
	int channel_number = mw->_song.undo_channel_number();
	std::set<int32_t> selection = mw->_song.undo_selection();
	Song::Song_State::Action action = mw->_song.undo_action();

	mw->_song.undo();
	if (channel_number != mw->selected_channel()) {
		mw->selected_channel(channel_number);
		mw->sync_channel_buttons();
	}
	mw->_piano_roll->set_active_channel_timeline(mw->_song);
	if (
		action == Song::Song_State::Action::PUT_NOTE ||
		action == Song::Song_State::Action::SPLIT_NOTE ||
		action == Song::Song_State::Action::GLUE_NOTE
	) {
		mw->_piano_roll->tick(tick);
		mw->_piano_roll->select_note_at_tick();
		mw->_piano_roll->focus_cursor(true);
	}
	else {
		mw->_piano_roll->set_active_channel_selection(selection);
	}

	mw->update_active_controls();
	mw->redraw();
}

void Main_Window::redo_cb(Fl_Widget *, Main_Window *mw) {
	if (!mw->_song.loaded()) { return; }

	mw->_status_message = "Redo: ";
	mw->_status_message += mw->_song.redo_action_message();
	mw->_status_label->label(mw->_status_message.c_str());

	int tick = mw->_song.redo_tick();
	int channel_number = mw->_song.redo_channel_number();
	std::set<int32_t> selection = mw->_song.redo_selection();
	Song::Song_State::Action action = mw->_song.redo_action();

	mw->_song.redo();
	if (channel_number != mw->selected_channel()) {
		mw->selected_channel(channel_number);
		mw->sync_channel_buttons();
	}
	mw->_piano_roll->set_active_channel_timeline(mw->_song);
	if (action == Song::Song_State::Action::PUT_NOTE) {
		mw->_piano_roll->tick(tick);
		mw->_piano_roll->select_note_at_tick();
		mw->_piano_roll->step_forward();
		mw->_piano_roll->focus_cursor(true);
	}
	else if (
		action == Song::Song_State::Action::SPLIT_NOTE ||
		action == Song::Song_State::Action::GLUE_NOTE
	) {
		mw->_piano_roll->tick(tick);
		mw->_piano_roll->select_note_at_tick();
		mw->_piano_roll->focus_cursor(true);
	}
	else if (
		action != Song::Song_State::Action::DELETE_SELECTION &&
		action != Song::Song_State::Action::SNIP_SELECTION
	) {
		mw->_piano_roll->set_active_channel_selection(selection);
	}

	mw->update_active_controls();
	mw->redraw();
}

void Main_Window::select_all_cb(Fl_Widget *, Main_Window *mw) {
	if (!mw->_song.loaded()) { return; }
	mw->_piano_roll->select_all();
}

void Main_Window::select_none_cb(Fl_Widget *, Main_Window *mw) {
	if (!mw->_song.loaded()) { return; }
	mw->_piano_roll->select_none();
}

void Main_Window::pitch_up_cb(Fl_Widget *, Main_Window *mw) {
	if (!mw->_song.loaded()) { return; }
	if (mw->_piano_roll->pitch_up(mw->_song)) {
		mw->_status_message = mw->_song.undo_action_message();
		mw->_status_label->label(mw->_status_message.c_str());

		mw->update_active_controls();
		mw->redraw();
	}
}

void Main_Window::pitch_down_cb(Fl_Widget *, Main_Window *mw) {
	if (!mw->_song.loaded()) { return; }
	if (mw->_piano_roll->pitch_down(mw->_song)) {
		mw->_status_message = mw->_song.undo_action_message();
		mw->_status_label->label(mw->_status_message.c_str());

		mw->update_active_controls();
		mw->redraw();
	}
}

void Main_Window::octave_up_cb(Fl_Widget *, Main_Window *mw) {
	if (!mw->_song.loaded()) { return; }
	if (mw->_piano_roll->octave_up(mw->_song)) {
		mw->_status_message = mw->_song.undo_action_message();
		mw->_status_label->label(mw->_status_message.c_str());

		mw->update_active_controls();
		mw->redraw();
	}
}

void Main_Window::octave_down_cb(Fl_Widget *, Main_Window *mw) {
	if (!mw->_song.loaded()) { return; }
	if (mw->_piano_roll->octave_down(mw->_song)) {
		mw->_status_message = mw->_song.undo_action_message();
		mw->_status_label->label(mw->_status_message.c_str());

		mw->update_active_controls();
		mw->redraw();
	}
}

void Main_Window::move_left_cb(Fl_Widget *, Main_Window *mw) {
	if (!mw->_song.loaded()) { return; }
	if (mw->_piano_roll->move_left(mw->_song)) {
		mw->_status_message = mw->_song.undo_action_message();
		mw->_status_label->label(mw->_status_message.c_str());

		mw->update_active_controls();
		mw->redraw();
	}
}

void Main_Window::move_right_cb(Fl_Widget *, Main_Window *mw) {
	if (!mw->_song.loaded()) { return; }
	if (mw->_piano_roll->move_right(mw->_song)) {
		mw->_status_message = mw->_song.undo_action_message();
		mw->_status_label->label(mw->_status_message.c_str());

		mw->update_active_controls();
		mw->redraw();
	}
}

void Main_Window::shorten_cb(Fl_Widget *, Main_Window *mw) {
	if (!mw->_song.loaded()) { return; }
	if (mw->_piano_roll->shorten(mw->_song)) {
		mw->_status_message = mw->_song.undo_action_message();
		mw->_status_label->label(mw->_status_message.c_str());

		mw->update_active_controls();
		mw->redraw();
	}
}

void Main_Window::lengthen_cb(Fl_Widget *, Main_Window *mw) {
	if (!mw->_song.loaded()) { return; }
	if (mw->_piano_roll->lengthen(mw->_song)) {
		mw->_status_message = mw->_song.undo_action_message();
		mw->_status_label->label(mw->_status_message.c_str());

		mw->update_active_controls();
		mw->redraw();
	}
}

void Main_Window::delete_cb(Fl_Widget *, Main_Window *mw) {
	if (!mw->_song.loaded()) { return; }
	if (mw->_piano_roll->delete_selection(mw->_song)) {
		mw->_status_message = mw->_song.undo_action_message();
		mw->_status_label->label(mw->_status_message.c_str());

		mw->update_active_controls();
		mw->redraw();
	}
}

void Main_Window::snip_cb(Fl_Widget *, Main_Window *mw) {
	if (!mw->_song.loaded()) { return; }
	if (mw->_piano_roll->snip_selection(mw->_song)) {
		mw->_status_message = mw->_song.undo_action_message();
		mw->_status_label->label(mw->_status_message.c_str());

		mw->update_active_controls();
		mw->redraw();
	}
}

void Main_Window::split_note_cb(Fl_Widget *, Main_Window *mw) {
	if (!mw->_song.loaded()) { return; }
	if (mw->_piano_roll->split_note(mw->_song)) {
		mw->_status_message = mw->_song.undo_action_message();
		mw->_status_label->label(mw->_status_message.c_str());

		mw->update_active_controls();
		mw->redraw();
	}
}

void Main_Window::glue_note_cb(Fl_Widget *, Main_Window *mw) {
	if (!mw->_song.loaded()) { return; }
	if (mw->_piano_roll->glue_note(mw->_song)) {
		mw->_status_message = mw->_song.undo_action_message();
		mw->_status_label->label(mw->_status_message.c_str());

		mw->update_active_controls();
		mw->redraw();
	}
}

void Main_Window::channel_1_cb(Fl_Widget *, Main_Window *mw) {
	if (mw->selected_channel() == 1) {
		mw->_channel_1_mi->clear();
		mw->_channel_1_tb->clear();
		mw->selected_channel(0);
	}
	else {
		mw->_channel_1_tb->setonly();
		mw->selected_channel(1);
	}
	mw->update_channel_detail();
	mw->_menu_bar->update();
	mw->redraw();
}

void Main_Window::channel_2_cb(Fl_Widget *, Main_Window *mw) {
	if (mw->selected_channel() == 2) {
		mw->_channel_2_mi->clear();
		mw->_channel_2_tb->clear();
		mw->selected_channel(0);
	}
	else {
		mw->_channel_2_tb->setonly();
		mw->selected_channel(2);
	}
	mw->update_channel_detail();
	mw->_menu_bar->update();
	mw->redraw();
}

void Main_Window::channel_3_cb(Fl_Widget *, Main_Window *mw) {
	if (mw->selected_channel() == 3) {
		mw->_channel_3_mi->clear();
		mw->_channel_3_tb->clear();
		mw->selected_channel(0);
	}
	else {
		mw->_channel_3_tb->setonly();
		mw->selected_channel(3);
	}
	mw->update_channel_detail();
	mw->_menu_bar->update();
	mw->redraw();
}

void Main_Window::channel_4_cb(Fl_Widget *, Main_Window *mw) {
	if (mw->selected_channel() == 4) {
		mw->_channel_4_mi->clear();
		mw->_channel_4_tb->clear();
		mw->selected_channel(0);
	}
	else {
		mw->_channel_4_tb->setonly();
		mw->selected_channel(4);
	}
	mw->update_channel_detail();
	mw->_menu_bar->update();
	mw->redraw();
}

void Main_Window::next_channel_cb(Fl_Widget *, Main_Window *mw) {
	mw->selected_channel(mw->selected_channel() % 4 + 1);
	mw->sync_channel_buttons();
}

void Main_Window::previous_channel_cb(Fl_Widget *, Main_Window *mw) {
	if (mw->selected_channel() == 0) {
		mw->selected_channel(4);
	}
	else {
		mw->selected_channel((mw->selected_channel() + 2) % 4 + 1);
	}
	mw->sync_channel_buttons();
}

void Main_Window::sync_channel_buttons() {
	if (selected_channel() == 1) {
		_channel_1_mi->setonly();
		_channel_1_tb->setonly();
	}
	else if (selected_channel() == 2) {
		_channel_2_mi->setonly();
		_channel_2_tb->setonly();
	}
	else if (selected_channel() == 3) {
		_channel_3_mi->setonly();
		_channel_3_tb->setonly();
	}
	else if (selected_channel() == 4) {
		_channel_4_mi->setonly();
		_channel_4_tb->setonly();
	}
	update_channel_detail();
	_menu_bar->update();
	redraw();
}

void Main_Window::channel_1_tb_cb(Toolbar_Radio_Button *, Main_Window *mw) {
	if (mw->selected_channel() == 1) {
		mw->_channel_1_mi->clear();
		mw->_channel_1_tb->clear();
		mw->selected_channel(0);
	}
	else {
		mw->_channel_1_mi->setonly();
		mw->selected_channel(1);
	}
	mw->update_channel_detail();
	mw->_menu_bar->update();
	mw->redraw();
}

void Main_Window::channel_2_tb_cb(Toolbar_Radio_Button *, Main_Window *mw) {
	if (mw->selected_channel() == 2) {
		mw->_channel_2_mi->clear();
		mw->_channel_2_tb->clear();
		mw->selected_channel(0);
	}
	else {
		mw->_channel_2_mi->setonly();
		mw->selected_channel(2);
	}
	mw->update_channel_detail();
	mw->_menu_bar->update();
	mw->redraw();
}

void Main_Window::channel_3_tb_cb(Toolbar_Radio_Button *, Main_Window *mw) {
	if (mw->selected_channel() == 3) {
		mw->_channel_3_mi->clear();
		mw->_channel_3_tb->clear();
		mw->selected_channel(0);
	}
	else {
		mw->_channel_3_mi->setonly();
		mw->selected_channel(3);
	}
	mw->update_channel_detail();
	mw->_menu_bar->update();
	mw->redraw();
}

void Main_Window::channel_4_tb_cb(Toolbar_Radio_Button *, Main_Window *mw) {
	if (mw->selected_channel() == 4) {
		mw->_channel_4_mi->clear();
		mw->_channel_4_tb->clear();
		mw->selected_channel(0);
	}
	else {
		mw->_channel_4_mi->setonly();
		mw->selected_channel(4);
	}
	mw->update_channel_detail();
	mw->_menu_bar->update();
	mw->redraw();
}

void Main_Window::classic_theme_cb(Fl_Widget *, Main_Window *mw) {
	OS::use_classic_theme();
	OS::update_macos_appearance(mw);
	mw->_classic_theme_mi->setonly();
	mw->update_icons();
	mw->redraw();
}

void Main_Window::aero_theme_cb(Fl_Widget *, Main_Window *mw) {
	OS::use_aero_theme();
	OS::update_macos_appearance(mw);
	mw->_aero_theme_mi->setonly();
	mw->update_icons();
	mw->redraw();
}

void Main_Window::metro_theme_cb(Fl_Widget *, Main_Window *mw) {
	OS::use_metro_theme();
	OS::update_macos_appearance(mw);
	mw->_metro_theme_mi->setonly();
	mw->update_icons();
	mw->redraw();
}

void Main_Window::aqua_theme_cb(Fl_Widget *, Main_Window *mw) {
	OS::use_aqua_theme();
	OS::update_macos_appearance(mw);
	mw->_aqua_theme_mi->setonly();
	mw->update_icons();
	mw->redraw();
}

void Main_Window::greybird_theme_cb(Fl_Widget *, Main_Window *mw) {
	OS::use_greybird_theme();
	OS::update_macos_appearance(mw);
	mw->_greybird_theme_mi->setonly();
	mw->update_icons();
	mw->redraw();
}

void Main_Window::ocean_theme_cb(Fl_Widget *, Main_Window *mw) {
	OS::use_ocean_theme();
	OS::update_macos_appearance(mw);
	mw->_ocean_theme_mi->setonly();
	mw->update_icons();
	mw->redraw();
}

void Main_Window::blue_theme_cb(Fl_Widget *, Main_Window *mw) {
	OS::use_blue_theme();
	OS::update_macos_appearance(mw);
	mw->_blue_theme_mi->setonly();
	mw->update_icons();
	mw->redraw();
}

void Main_Window::olive_theme_cb(Fl_Widget *, Main_Window *mw) {
	OS::use_olive_theme();
	OS::update_macos_appearance(mw);
	mw->_olive_theme_mi->setonly();
	mw->update_icons();
	mw->redraw();
}

void Main_Window::rose_gold_theme_cb(Fl_Widget *, Main_Window *mw) {
	OS::use_rose_gold_theme();
	OS::update_macos_appearance(mw);
	mw->_rose_gold_theme_mi->setonly();
	mw->update_icons();
	mw->redraw();
}

void Main_Window::dark_theme_cb(Fl_Widget *, Main_Window *mw) {
	OS::use_dark_theme();
	OS::update_macos_appearance(mw);
	mw->_dark_theme_mi->setonly();
	mw->update_icons();
	mw->redraw();
}

void Main_Window::brushed_metal_theme_cb(Fl_Widget *, Main_Window *mw) {
	OS::use_brushed_metal_theme();
	OS::update_macos_appearance(mw);
	mw->_brushed_metal_theme_mi->setonly();
	mw->update_icons();
	mw->redraw();
}

void Main_Window::high_contrast_theme_cb(Fl_Widget *, Main_Window *mw) {
	OS::use_high_contrast_theme();
	OS::update_macos_appearance(mw);
	mw->_high_contrast_theme_mi->setonly();
	mw->update_icons();
	mw->redraw();
}

void Main_Window::decrease_spacing_cb(Fl_Widget *, Main_Window *mw) {
	mw->_piano_roll->ticks_per_step(mw->_piano_roll->ticks_per_step() - 1);
	if (mw->_piano_roll->ticks_per_step() == 4) {
		mw->_decrease_spacing_mi->deactivate();
		mw->_decrease_spacing_tb->deactivate();
	}
	mw->_increase_spacing_mi->activate();
	mw->_increase_spacing_tb->activate();
	if (mw->_piano_roll->following() && mw->continuous_scroll()) {
		mw->_piano_roll->focus_cursor();
	}
	mw->_menu_bar->update();
	mw->redraw();
}

void Main_Window::increase_spacing_cb(Fl_Widget *, Main_Window *mw) {
	mw->_piano_roll->ticks_per_step(mw->_piano_roll->ticks_per_step() + 1);
	if (mw->_piano_roll->ticks_per_step() == 16) {
		mw->_increase_spacing_mi->deactivate();
		mw->_increase_spacing_tb->deactivate();
	}
	mw->_decrease_spacing_mi->activate();
	mw->_decrease_spacing_tb->activate();
	if (mw->_piano_roll->following() && mw->continuous_scroll()) {
		mw->_piano_roll->focus_cursor();
	}
	mw->_menu_bar->update();
	mw->redraw();
}

void Main_Window::key_labels_cb(Fl_Widget *, Main_Window *mw) {
	mw->_piano_roll->key_labels(mw->key_labels());
	mw->redraw();
}

void Main_Window::full_screen_cb(Fl_Widget *, Main_Window *mw) {
#ifdef __APPLE__
	if (!cocoa_is_fullscreen(mw)) {
		if (!mw->maximized()) {
			mw->_wx = mw->x(); mw->_wy = mw->y();
			mw->_ww = mw->w(); mw->_wh = mw->h();
		}
		cocoa_fullscreen(mw, true);
		mw->_full_screen_mi->set();
	}
	else {
		cocoa_fullscreen(mw, false);
		mw->_full_screen_mi->clear();
	}
#else
	if (mw->full_screen()) {
		if (!mw->maximized()) {
			mw->_wx = mw->x(); mw->_wy = mw->y();
			mw->_ww = mw->w(); mw->_wh = mw->h();
		}
		mw->fullscreen();
	}
	else {
		mw->fullscreen_off(mw->_wx, mw->_wy, mw->_ww, mw->_wh);
	}
#endif
}

void Main_Window::help_cb(Fl_Widget *, Main_Window *mw) {
	mw->_help_window->show(mw);
}

void Main_Window::about_cb(Fl_Widget *, Main_Window *mw) {
	mw->_about_dialog->show(mw);
}

void Main_Window::playback_thread(Main_Window *mw, std::future<void> kill_signal) {
	int32_t tick = -1;
	while (kill_signal.wait_for(std::chrono::milliseconds(8)) == std::future_status::timeout) {
		if (mw->_audio_mutex.try_lock()) {
			IT_Module *mod = mw->_it_module;
			if (mod && mod->playing()) {
				mod->play();
				int32_t t = mod->current_tick();
				if (tick != t) {
					tick = t;
					mw->_tick = t;
					if (!mw->_sync_requested) {
						Fl::awake((Fl_Awake_Handler)sync_cb, mw);
						mw->_sync_requested = true;
					}
				}
				mw->_audio_mutex.unlock();
			}
			else {
				mw->_tick = -1;
				if (!mw->_sync_requested) {
					Fl::awake((Fl_Awake_Handler)sync_cb, mw);
					mw->_sync_requested = true;
				}
				mw->_audio_mutex.unlock();
				break;
			}
		}
	}
}

void Main_Window::sync_cb(Main_Window *mw) {
	mw->_audio_mutex.lock();
	IT_Module *mod = mw->_it_module;
	if (mod && mod->playing() && mw->_tick > 0) {
		mw->_piano_roll->highlight_tick(mw->_tick);
	}
	else if (!mod || mod->stopped()) {
		mw->_tick = -1;
		mw->_piano_roll->stop_following();
		mw->update_active_controls();
	}
	mw->_sync_requested = false;
	mw->_audio_mutex.unlock();
}

void Main_Window::interactive_thread(Main_Window *mw, std::future<void> kill_signal) {
	IT_Module mod;
	mod.start();

	Pitch pitch = Pitch::REST;
	int32_t octave = 0;
	int32_t channel = -1;

	while (kill_signal.wait_for(std::chrono::milliseconds(8)) == std::future_status::timeout) {
		if (mw->_interactive_mutex.try_lock()) {
			if (pitch != mw->_playing_pitch || octave != mw->_playing_octave) {
				if (channel != -1) {
					mod.stop_note(channel);
					channel = -1;
				}
				pitch = mw->_playing_pitch;
				octave = mw->_playing_octave;
				if (pitch != Pitch::REST) {
					channel = mod.play_note(pitch, octave);
				}
			}

			if (pitch != Pitch::REST) {
				mod.play();
			}

			mw->_interactive_mutex.unlock();
		}
	}
	if (channel != -1) {
		mod.stop_note(channel);
	}
}
