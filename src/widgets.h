#ifndef WIDGETS_H
#define WIDGETS_H

#pragma warning(push, 0)
#include <FL/Fl_Box.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Radio_Round_Button.H>
#include <FL/Fl_Spinner.H>
#include <FL/Fl_Hor_Nice_Slider.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Menu_.H>
#include <FL/Fl_Help_View.H>
#pragma warning(pop)

#define OS_MENU_ITEM_PREFIX " "
#define OS_MENU_ITEM_SUFFIX "         "

#ifdef __APPLE__
#define SYS_MENU_ITEM_PREFIX ""
#define SYS_MENU_ITEM_SUFFIX ""
#else
#define SYS_MENU_ITEM_PREFIX OS_MENU_ITEM_PREFIX
#define SYS_MENU_ITEM_SUFFIX OS_MENU_ITEM_SUFFIX
#endif

#define OS_SUBMENU(l) {l, 0, NULL, NULL, FL_SUBMENU, FL_NORMAL_LABEL, OS_FONT, OS_FONT_SIZE, FL_FOREGROUND_COLOR}
#define OS_NULL_MENU_ITEM(s, c, d, f) {"", s, c, d, f, FL_NORMAL_LABEL, OS_FONT, OS_FONT_SIZE, FL_FOREGROUND_COLOR}
#define OS_MENU_ITEM(l, s, c, d, f) {OS_MENU_ITEM_PREFIX l OS_MENU_ITEM_SUFFIX, s, c, d, f, FL_NORMAL_LABEL, OS_FONT, OS_FONT_SIZE, FL_FOREGROUND_COLOR}
#define SYS_MENU_ITEM(l, s, c, d, f) {SYS_MENU_ITEM_PREFIX l SYS_MENU_ITEM_SUFFIX, s, c, d, f, FL_NORMAL_LABEL, OS_FONT, OS_FONT_SIZE, FL_FOREGROUND_COLOR}

class Label : public Fl_Box {
public:
	Label(int x, int y, int w, int h, const char *l = NULL);
};

class Label_Button : public Fl_Button {
private:
	bool _enabled = true;
public:
	Label_Button(int x, int y, int w, int h, const char *l = NULL);
	inline void enable(void) { _enabled = true; }
	inline void disable(void) { _enabled = false; }
	int handle(int event);
};

class Spacer : public Fl_Box {
public:
	Spacer(int x, int y, int w, int h, const char *l = NULL);
};

class OS_Input : public Fl_Input {
public:
	OS_Input(int x, int y, int w, int h, const char *l = NULL);
};

class OS_Int_Input : public Fl_Int_Input {
public:
	OS_Int_Input(int x, int y, int w, int h, const char *l = NULL);
};

class OS_Button : public Fl_Button {
public:
	OS_Button(int x, int y, int w, int h, const char *l = NULL);
protected:
	int handle(int event);
};

class Default_Button : public Fl_Button {
public:
	Default_Button(int x, int y, int w, int h, const char *l = NULL);
protected:
	int handle(int event);
};

class OS_Light_Button : public Fl_Light_Button {
public:
	OS_Light_Button(int x, int y, int w, int h, const char *l = NULL);
	void draw(void);
};

class OS_Check_Button : public Fl_Check_Button {
public:
	OS_Check_Button(int x, int y, int w, int h, const char *l = NULL);
	void draw(void);
protected:
	int handle(int event);
};

class OS_Radio_Button : public Fl_Radio_Round_Button {
public:
	OS_Radio_Button(int x, int y, int w, int h, const char *l = NULL);
	void draw(void);
protected:
	int handle(int event);
};

class OS_Spinner : public Fl_Spinner {
private:
	Fl_Callback *_focus_cb = nullptr;
public:
	OS_Spinner(int x, int y, int w, int h, const char *l = NULL);
	void label(const char *text) { input_.label(text); }
	void labelfont(Fl_Font f) { input_.labelfont(f); }
	void labelsize(Fl_Fontsize pix) { input_.labelsize(pix); }
	Fl_Callback *focus_cb() const { return _focus_cb; }
	void focus_cb(Fl_Callback *c) { _focus_cb = c; }
protected:
	int handle(int event);
};

class OS_Slider : public Fl_Hor_Nice_Slider {
public:
	OS_Slider(int x, int y, int w, int h, const char *l = NULL);
	void draw(void);
	void draw(int x, int y, int w, int h);
protected:
	int handle(int event);
};

class HTML_View : public Fl_Help_View {
public:
	HTML_View(int x, int y, int w, int h, const char *l = NULL);
};

class Dropdown : public Fl_Choice {
private:
	Fl_Callback *_before_open_cb = nullptr;
	bool _scroll_enabled = false;
public:
	Dropdown(int x, int y, int w, int h, const char *l = NULL);
	Fl_Callback *before_open_cb() const { return _before_open_cb; }
	void before_open_cb(Fl_Callback *c) { _before_open_cb = c; }
	bool scroll_enabled() const { return _scroll_enabled; }
	void scroll_enabled(bool s) { _scroll_enabled = s; }
	void draw(void);
protected:
	int handle(int event);
};

class OS_Browser : public Fl_Browser {
public:
	OS_Browser(int x, int y, int w, int h, const char *l = NULL);
protected:
	int handle(int event);
};

class OS_Tabs : public Fl_Tabs {
public:
	OS_Tabs(int x, int y, int w, int h, const char *l = NULL);
};

class OS_Tab : public Fl_Group {
public:
	OS_Tab(int x, int y, int w, int h, const char *l = NULL);
};

class OS_Scroll : public Fl_Scroll {
public:
	OS_Scroll(int x, int y, int w, int h, const char *l = NULL);
};

class Toolbar : public Fl_Group {
public:
	Toolbar(int x, int y, int w, int h, const char *l = NULL);
};

class Scrollable_Toolbar : public OS_Scroll {
private:
	Fl_Box _bg;
	bool _show_scrollbar;
public:
	Scrollable_Toolbar(bool s, int x, int y, int w, int h, const char *l = NULL);
	int scroll_x_max();
	void resize(int X, int Y, int W, int H);
	int handle(int event);
	void scroll_to(int X, int Y);
private:
	static void hscrollbar_cb(Fl_Scrollbar *sb, void *);
};

class Toolbar_Button : public Fl_Button {
public:
	Toolbar_Button(int x, int y, int w, int h, const char *l = NULL);
	void simulate_key_action() { Fl_Button::simulate_key_action(); }
protected:
	void draw(void);
	int handle(int event);
};

class Toolbar_Toggle_Button : public Toolbar_Button {
public:
	Toolbar_Toggle_Button(int x, int y, int w, int h, const char *l = NULL);
};

class Toolbar_Radio_Button : public Toolbar_Button {
public:
	Toolbar_Radio_Button(int x, int y, int w, int h, const char *l = NULL);
};

class Context_Menu : public Fl_Menu_ {
private:
	int _shortcut;
	int _popup_x, _popup_y;
public:
	Context_Menu(int x, int y, int w, int h, const char *l = NULL);
	int shortcut() const { return _shortcut; }
	void shortcut(int s) { _shortcut = s; }
	int popup_x() const { return _popup_x; }
	void popup_x(int x) { _popup_x = x; }
	int popup_y() const { return _popup_y; }
	void popup_y(int y) { _popup_y = y; }
	int handle(int event);
	virtual bool prepare(int event, int X = 0, int Y = 0);
protected:
	void draw(void);
};

#endif
