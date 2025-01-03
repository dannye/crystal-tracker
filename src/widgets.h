#ifndef WIDGETS_H
#define WIDGETS_H

#include <string>
#include <vector>

#pragma warning(push, 0)
#include <FL/Fl_Box.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Spinner.H>
#include <FL/Fl_Hor_Nice_Slider.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Radio_Round_Button.H>
#include <FL/Fl_Menu_.H>
#include <FL/Fl_Help_View.H>
#pragma warning(pop)

#include "hex-spinner.h"

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

class DnD_Receiver : public Fl_Box {
public:
	static void deferred_callback(DnD_Receiver *dndr);
private:
	std::string _text;
public:
	DnD_Receiver(int x, int y, int w, int h, const char *l = NULL);
	inline const std::string &text(void) const { return _text; }
	int handle(int event);
};

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

class OS_Hex_Input : public Hex_Input {
public:
	OS_Hex_Input(int x, int y, int w, int h, const char *l = NULL);
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
public:
	OS_Spinner(int x, int y, int w, int h, const char *l = NULL);
	void label(const char *text) { input_.label(text); }
	void labelfont(Fl_Font f) { input_.labelfont(f); }
	void labelsize(Fl_Fontsize pix) { input_.labelsize(pix); }
protected:
	int handle(int event);
};

class OS_Hex_Spinner : public Hex_Spinner {
public:
	OS_Hex_Spinner(int x, int y, int w, int h, const char *l = NULL);
};

class Default_Spinner : public OS_Spinner {
private:
	double _default_value;
public:
	Default_Spinner(int x, int y, int w, int h, const char *l = NULL);
	inline double default_value(void) const { return _default_value; }
	inline void default_value(double v) { _default_value = v; value(_default_value); }
protected:
	int handle(int event);
};

class Default_Hex_Spinner : public OS_Hex_Spinner {
private:
	int _default_value;
public:
	Default_Hex_Spinner(int x, int y, int w, int h, const char *l = NULL);
	inline int default_value(void) const { return _default_value; }
	inline void default_value(int v) { _default_value = v; value(_default_value); }
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

class Default_Slider : public OS_Slider {
private:
	double _default_value;
public:
	Default_Slider(int x, int y, int w, int h, const char *l = NULL);
	inline double default_value(void) const { return _default_value; }
	inline void default_value(double v) { _default_value = v; value(_default_value); }
protected:
	int handle(int event);
	int handle(int event, int x, int y, int w, int h);
};

class HTML_View : public Fl_Help_View {
public:
	HTML_View(int x, int y, int w, int h, const char *l = NULL);
};

class Dropdown : public Fl_Choice {
public:
	Dropdown(int x, int y, int w, int h, const char *l = NULL);
	void draw(void);
protected:
	int handle(int event);
};

class OS_Scroll : public Fl_Scroll {
public:
	OS_Scroll(int x, int y, int w, int h, const char *l = NULL);
};

class Workspace : public OS_Scroll {
private:
	int _content_w, _content_h;
	int _ox, _oy, _cx, _cy;
	DnD_Receiver *_dnd_receiver;
	std::vector<Fl_Widget *> _correlates;
public:
	Workspace(int x, int y, int w, int h, const char *l = NULL);
	inline void contents(int w, int h) { _content_w = w; _content_h = h; }
	inline bool has_x_scroll(void) const { return !!hscrollbar.visible(); }
	inline bool has_y_scroll(void) const { return !!scrollbar.visible(); }
	inline void dnd_receiver(DnD_Receiver *dndr) { _dnd_receiver = dndr; }
	inline void add_correlate(Fl_Widget *wgt) { _correlates.push_back(wgt); }
	inline void clear_correlates(void) { _correlates.clear(); }
	int handle(int event);
	void scroll_to(int x, int y);
private:
	static void hscrollbar_cb(Fl_Scrollbar *sb, void *);
	static void scrollbar_cb(Fl_Scrollbar *sb, void *);
};

class Toolbar : public Fl_Group {
public:
	Toolbar(int x, int y, int w, int h, const char *l = NULL);
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
public:
	Context_Menu(int x, int y, int w, int h, const char *l = NULL);
	int shortcut() const { return _shortcut; }
	void shortcut(int s) { _shortcut = s; }
	int handle(int event);
	virtual bool prepare(int X, int Y);
protected:
	void draw(void);
};

#endif
