#pragma warning(push, 0)
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Spinner.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Menu_.H>
#include <FL/fl_draw.H>
#pragma warning(pop)

#include "themes.h"
#include "utils.h"
#include "widgets.h"

void DnD_Receiver::deferred_callback(DnD_Receiver *dndr) {
	dndr->do_callback();
}

DnD_Receiver::DnD_Receiver(int x, int y, int w, int h, const char *l) : Fl_Box(x, y, w, h, l), _text() {
	labeltype(FL_NO_LABEL);
	box(FL_NO_BOX);
	clear_visible_focus();
}

int DnD_Receiver::handle(int event) {
	if (event == FL_PASTE && Fl::event_length()) {
		_text = Fl::event_text();
		// Callback deferral technique taken from <http://www.fltk.org/articles.php?L770>
		if (callback() && ((when() & FL_WHEN_RELEASE) || (when() & FL_WHEN_CHANGED))) {
			Fl::add_timeout(0.0, (Fl_Timeout_Handler)DnD_Receiver::deferred_callback, this);
		}
		return 1;
	}
	return 0;
}

Label::Label(int x, int y, int w, int h, const char *l) : Fl_Box(x, y, w, h, l) {
	labelfont(OS_FONT);
	labelsize(OS_FONT_SIZE);
	align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
}

Label_Button::Label_Button(int x, int y, int w, int h, const char *l) : Fl_Button(x, y, w, h, l) {
	labelfont(OS_FONT);
	labelsize(OS_FONT_SIZE);
	box(FL_NO_BOX);
	down_box(OS_SPACER_THIN_DOWN_BOX);
	align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
}

int Label_Button::handle(int event) {
	if (event == FL_ENTER && _enabled) {
		fl_cursor(FL_CURSOR_HAND);
		return 1;
	}
	if (event == FL_LEAVE) {
		fl_cursor(FL_CURSOR_DEFAULT);
		return 1;
	}
	if (event == FL_PUSH && !_enabled) return 0;
	return Fl_Button::handle(event);
}

Spacer::Spacer(int x, int y, int w, int h, const char *l) : Fl_Box(x, y, w, h, l) {
	labeltype(FL_NO_LABEL);
	box(OS_SPACER_THIN_DOWN_FRAME);
	color(FL_DARK1);
}

OS_Input::OS_Input(int x, int y, int w, int h, const char *l) : Fl_Input(x, y, w, h, l) {
	labelfont(OS_FONT);
	labelsize(OS_FONT_SIZE);
	textfont(OS_FONT);
	textsize(OS_FONT_SIZE);
	box(OS_INPUT_THIN_DOWN_BOX);
	align(FL_ALIGN_LEFT | FL_ALIGN_CLIP);
}

OS_Int_Input::OS_Int_Input(int x, int y, int w, int h, const char *l) : Fl_Int_Input(x, y, w, h, l) {
	labelfont(OS_FONT);
	labelsize(OS_FONT_SIZE);
	textfont(OS_FONT);
	textsize(OS_FONT_SIZE);
	box(OS_INPUT_THIN_DOWN_BOX);
	align(FL_ALIGN_LEFT | FL_ALIGN_CLIP);
}

OS_Hex_Input::OS_Hex_Input(int x, int y, int w, int h, const char *l) : Hex_Input(x, y, w, h, l) {
	labelfont(OS_FONT);
	labelsize(OS_FONT_SIZE);
	textfont(OS_FONT);
	textsize(OS_FONT_SIZE);
	box(OS_INPUT_THIN_DOWN_BOX);
	align(FL_ALIGN_LEFT | FL_ALIGN_CLIP);
}

OS_Button::OS_Button(int x, int y, int w, int h, const char *l) : Fl_Button(x, y, w, h, l) {
	labelfont(OS_FONT);
	labelsize(OS_FONT_SIZE);
	box(OS_BUTTON_UP_BOX);
	down_box(OS_DEPRESSED_DOWN_BOX);
}

int OS_Button::handle(int event) {
	if (OS::current_theme() != OS::Theme::AQUA) {
		switch (event) {
		case FL_ENTER:
			if (active_r()) {
				box(OS_HOVERED_UP_BOX);
				redraw();
				return 1;
			}
			return 0;
		case FL_LEAVE:
		case FL_HIDE:
		case FL_DEACTIVATE:
			box(OS_BUTTON_UP_BOX);
			redraw();
			return 1;
		}
	}
	if (event == FL_PUSH) {
		Fl::focus(this);
	}
	if (event == FL_SHORTCUT && !Fl::event_state(FL_ALT)) return 0;
	return Fl_Button::handle(event);
}

Default_Button::Default_Button(int x, int y, int w, int h, const char *l) : Fl_Button(x, y, w, h, l) {
	labelfont(OS_FONT);
	labelsize(OS_FONT_SIZE);
	box(OS_DEFAULT_BUTTON_UP_BOX);
	down_box(OS_DEFAULT_DEPRESSED_DOWN_BOX);
	shortcut(FL_Enter);
}

int Default_Button::handle(int event) {
	if (OS::current_theme() == OS::Theme::AERO || OS::current_theme() == OS::Theme::METRO ||
		OS::current_theme() == OS::Theme::BLUE || OS::current_theme() == OS::Theme::OLIVE ||
		OS::current_theme() == OS::Theme::ROSE_GOLD || OS::current_theme() == OS::Theme::HIGH_CONTRAST) {
		switch (event) {
		case FL_ENTER:
			if (active_r()) {
				box(OS_DEFAULT_HOVERED_UP_BOX);
				redraw();
				return 1;
			}
			return 0;
		case FL_LEAVE:
		case FL_HIDE:
		case FL_DEACTIVATE:
			box(OS_DEFAULT_BUTTON_UP_BOX);
			redraw();
			return 1;
		}
	}
	return Fl_Button::handle(event);
}

OS_Check_Button::OS_Check_Button(int x, int y, int w, int h, const char *l) : Fl_Check_Button(x, y, w, h, l) {
	labelfont(OS_FONT);
	labelsize(OS_FONT_SIZE);
	box(OS_BG_BOX);
	align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
	selection_color(FL_SELECTION_COLOR);
}

int OS_Check_Button::handle(int event) {
	if (event == FL_PUSH) {
		Fl::focus(this);
	}
	if (event == FL_SHORTCUT && !Fl::event_state(FL_ALT)) return 0;
	return Fl_Check_Button::handle(event);
}

void OS_Check_Button::draw() {
	// Based on Fl_Light_Button::draw()
	draw_box(this == Fl::pushed() ? fl_down(box()) : box(), active_r() ? color() : fl_inactive(color()));
	int W = labelsize();
#ifdef _WIN32
	W++;
#endif
	int dx = Fl::box_dx(box()) + 2;
	int dy = (h() - W) / 2;
	draw_box(down_box(), x()+dx, y()+dy, W, W, FL_BACKGROUND2_COLOR);
	if (value()) {
		Fl_Color sc = Fl::scheme() && Fl::is_scheme("gtk+") ? selection_color() : FL_FOREGROUND_COLOR;
		fl_color(active_r() ? sc : fl_inactive(sc));
		int tw = W - 6;
		int d1 = tw / 3;
		int d2 = tw - d1;
		int tx = x() + dx + 3;
		int ty = y() + dy + (W + d2) / 2 - d1 - 2;
		for (int n = 0; n < 3; n++, ty++) {
			fl_line(tx, ty, tx+d1, ty+d1);
			fl_line(tx+d1, ty+d1, tx+tw-1, ty+d1-d2+1);
		}
	}
	draw_label(x()+W+2*dx, y(), w()-W-2*dx, h());
	if (Fl::focus() == this) { draw_focus(); }
}

OS_Radio_Button::OS_Radio_Button(int x, int y, int w, int h, const char *l) : Fl_Radio_Round_Button(x, y, w, h, l) {
	labelfont(OS_FONT);
	labelsize(OS_FONT_SIZE);
	box(OS_BG_BOX);
	align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
	selection_color(FL_SELECTION_COLOR);
}

int OS_Radio_Button::handle(int event) {
	if (event == FL_PUSH) {
		Fl::focus(this);
	}
	return Fl_Radio_Round_Button::handle(event);
}

void OS_Radio_Button::draw() {
	// Based on Fl_Light_Button::draw()
	draw_box(this == Fl::pushed() ? fl_down(box()) : box(), active_r() ? color() : fl_inactive(color()));
	int W = labelsize();
#ifdef _WIN32
	W++;
#endif
	int dx = Fl::box_dx(box()) + 2;
	int dy = (h() - W) / 2;
	draw_box(down_box(), x()+dx, y()+dy, W, W, FL_BACKGROUND2_COLOR);
	if (value()) {
		Fl_Color sc = Fl::scheme() && Fl::is_scheme("gtk+") ? selection_color() : FL_FOREGROUND_COLOR;
		sc = active_r() ? sc : fl_inactive(sc);
		int tW = (W - Fl::box_dw(down_box())) / 2 + 1;
		if ((W - tW) & 1) tW++; // Make sure difference is even to center
		int tdx = dx + (W - tW) / 2;
		int tdy = dy + (W - tW) / 2;

		if (Fl::is_scheme("gtk+")) {
			fl_color(sc);
			tW--;
			fl_pie(x() + tdx - 1, y() + tdy - 1, tW + 3, tW + 3, 0.0, 360.0);
			fl_color(fl_color_average(FL_WHITE, sc, 0.2f));
		} else fl_color(sc);

		switch (tW) {
		// Larger circles draw fine...
		default:
			fl_pie(x() + tdx, y() + tdy, tW, tW, 0.0, 360.0);
			break;

		// Small circles don't draw well on many systems...
		case 6:
			fl_rectf(x() + tdx + 2, y() + tdy, tW - 4, tW);
			fl_rectf(x() + tdx + 1, y() + tdy + 1, tW - 2, tW - 2);
			fl_rectf(x() + tdx, y() + tdy + 2, tW, tW - 4);
			break;

		case 5:
		case 4:
		case 3:
			fl_rectf(x() + tdx + 1, y() + tdy, tW - 2, tW);
			fl_rectf(x() + tdx, y() + tdy + 1, tW, tW - 2);
			break;

		case 2:
		case 1:
			fl_rectf(x() + tdx, y() + tdy, tW, tW);
			break;
		}

		if (Fl::is_scheme("gtk+")) {
			fl_color(fl_color_average(FL_WHITE, sc, 0.5f));
			fl_arc(x() + tdx, y() + tdy, tW + 1, tW + 1, 60.0, 180.0);
		}
	}
	draw_label(x()+W+2*dx, y(), w()-W-2*dx, h());
	if (Fl::focus() == this) { draw_focus(); }
}

OS_Spinner::OS_Spinner(int x, int y, int w, int h, const char *l) : Fl_Spinner(x, y, w, h) {
	label(l);
	labelfont(OS_FONT);
	labelsize(OS_FONT_SIZE);
	textfont(OS_FONT);
	textsize(OS_FONT_SIZE);
	align(FL_ALIGN_LEFT | FL_ALIGN_CLIP);
	input_.box(OS_INPUT_THIN_DOWN_BOX);
	up_button_.labelfont(OS_FONT);
	up_button_.labelsize(OS_FONT_SIZE);
	up_button_.box(OS_MINI_BUTTON_UP_BOX);
	up_button_.down_box(OS_MINI_DEPRESSED_DOWN_BOX);
	down_button_.labelfont(OS_FONT);
	down_button_.labelsize(OS_FONT_SIZE);
	down_button_.box(OS_MINI_BUTTON_UP_BOX);
	down_button_.down_box(OS_MINI_DEPRESSED_DOWN_BOX);
}

int OS_Spinner::handle(int event) {
	switch (event) {
	case FL_FOCUS:
		if (input_.take_focus()) return 1;
		else return 0;
	case FL_SHORTCUT:
		if (Fl::event_state(FL_ALT) && input_.test_shortcut() && input_.take_focus()) return 1;
		else return 0;
	}
	return Fl_Group::handle(event);
}

OS_Hex_Spinner::OS_Hex_Spinner(int x, int y, int w, int h, const char *l) : Hex_Spinner(x, y, w, h, l) {
	labelfont(OS_FONT);
	labelsize(OS_FONT_SIZE);
	textfont(OS_FONT);
	textsize(OS_FONT_SIZE);
	align(FL_ALIGN_LEFT | FL_ALIGN_CLIP);
	_input.box(OS_INPUT_THIN_DOWN_BOX);
	_up_button.labelfont(OS_FONT);
	_up_button.labelsize(OS_FONT_SIZE);
	_up_button.box(OS_MINI_BUTTON_UP_BOX);
	_up_button.down_box(OS_MINI_DEPRESSED_DOWN_BOX);
	_down_button.labelfont(OS_FONT);
	_down_button.labelsize(OS_FONT_SIZE);
	_down_button.box(OS_MINI_BUTTON_UP_BOX);
	_down_button.down_box(OS_MINI_DEPRESSED_DOWN_BOX);
}

Default_Spinner::Default_Spinner(int x, int y, int w, int h, const char *l) : OS_Spinner(x, y, w, h, l),
	_default_value(0.0) {}

int Default_Spinner::handle(int event) {
	switch (event) {
	case FL_PUSH:
		if (Fl::event_button() == FL_MIDDLE_MOUSE) {
			return 1;
		}
		break;
	case FL_RELEASE:
		if (Fl::event_button() == FL_MIDDLE_MOUSE) {
			value(_default_value);
			do_callback();
			return 1;
		}
		break;
	}
	return Fl_Spinner::handle(event);
}

Default_Hex_Spinner::Default_Hex_Spinner(int x, int y, int w, int h, const char *l) : OS_Hex_Spinner(x, y, w, h, l),
	_default_value(0) {}

int Default_Hex_Spinner::handle(int event) {
	switch (event) {
	case FL_PUSH:
		if (Fl::event_button() == FL_MIDDLE_MOUSE) {
			return 1;
		}
		break;
	case FL_RELEASE:
		if (Fl::event_button() == FL_MIDDLE_MOUSE) {
			value(_default_value);
			do_callback();
			return 1;
		}
		break;
	}
	return Hex_Spinner::handle(event);
}

OS_Slider::OS_Slider(int x, int y, int w, int h, const char *l) : Fl_Hor_Nice_Slider(x, y, w, h, l) {
	labelfont(OS_FONT);
	labelsize(OS_FONT_SIZE);
	box(OS_BG_BOX);
	color(FL_BACKGROUND_COLOR);
	slider(OS_BUTTON_UP_BOX);
	slider_size(0.0);
	align(FL_ALIGN_LEFT | FL_ALIGN_CLIP);
}

int OS_Slider::handle(int event) {
	if (event == FL_PUSH) {
		Fl::focus(this);
	}
	return Fl_Hor_Nice_Slider::handle(event);
}

void OS_Slider::draw() {
	// Based on Fl_Slider::draw()
	Fl_Boxtype b = box();
	if (damage() & FL_DAMAGE_ALL) { draw_box(b, active_r() ? color() : fl_inactive(color())); }
	draw(x()+Fl::box_dx(b), y()+Fl::box_dy(b), w()-Fl::box_dw(b), h()-Fl::box_dh(b));
}

void OS_Slider::draw(int x, int y, int w, int h) {
	// Based on Fl_Slider::draw(...)
	double v = 0.5;
	if (minimum() != maximum()) {
		v = std::clamp((value() - minimum()) / (maximum() - minimum()), 0.0, 1.0);
	}
	int s = std::max((int)(slider_size() * w + 0.5), h / 2 + 2);
	int ws = w - s;
	int lx = x + (int)(v * ws + 0.5);
	fl_push_clip(x, y, w, h);
	draw_box(box(), active_r() ? color() : fl_inactive(color()));
	fl_pop_clip();
	draw_box(OS::current_theme() == OS::Theme::OCEAN || OS::current_theme() == OS::Theme::HIGH_CONTRAST ?
		OS_BUTTON_UP_BOX : OS_SPACER_THIN_DOWN_BOX, x, y+h/2-2, w, 4, active_r() ? FL_DARK2 : fl_inactive(FL_DARK2));
	draw_box(slider(), lx, y, s, h, FL_GRAY);
	draw_label(lx, y, s, h);
	if (Fl::focus() == this) {
		draw_focus(slider(), lx, y, s, h);
	}
}

Default_Slider::Default_Slider(int x, int y, int w, int h, const char *l) : OS_Slider(x, y, w, h, l),
	_default_value(0.0) {}

int Default_Slider::handle(int event) {
	return handle(event, x(), y(), w(), h());
}

int Default_Slider::handle(int event, int x, int y, int w, int h) {
	switch (event) {
	case FL_PUSH:
		Fl::focus(this);
		if (Fl::event_button() == FL_MIDDLE_MOUSE) {
			return 1;
		}
		break;
	case FL_DRAG:
		if (Fl::event_button() == FL_MIDDLE_MOUSE) {
			return 0;
		}
		break;
	case FL_RELEASE:
		if (Fl::event_button() == FL_MIDDLE_MOUSE) {
			value(_default_value);
			do_callback();
			return 1;
		}
		break;
	}
	return Fl_Hor_Nice_Slider::handle(event, x, y, w, h);
}

HTML_View::HTML_View(int x, int y, int w, int h, const char *l) : Fl_Help_View(x, y, w, h, l) {
	box(OS_INPUT_THIN_DOWN_BOX);
	// TODO: scrollbar_.slider(OS_MINI_BUTTON_UP_BOX);
	// TODO: hscrollbar_.slider(OS_MINI_BUTTON_UP_BOX);
	textsize(16);
}

Dropdown::Dropdown(int x, int y, int w, int h, const char *l) : Fl_Choice(x, y, w, h, l) {
	box(OS_PANEL_THIN_UP_BOX);
	down_box(FL_FLAT_BOX);
	labelfont(OS_FONT);
	labelsize(OS_FONT_SIZE);
	align(FL_ALIGN_LEFT | FL_ALIGN_CLIP);
	textfont(OS_FONT);
	textsize(OS_FONT_SIZE);
}

void Dropdown::draw() {
	// Based on Fl_Choice::draw()
	Fl_Boxtype btype = OS::current_theme() == OS::Theme::OCEAN || OS::current_theme() == OS::Theme::HIGH_CONTRAST ?
		OS_INPUT_THIN_DOWN_BOX : OS::current_theme() == OS::Theme::OLIVE ? OS_SWATCH_BOX : Fl::scheme() ? FL_UP_BOX : FL_DOWN_BOX;
	int dx = Fl::box_dx(btype);
	int dy = Fl::box_dy(btype);
	int H = h() - 2 * dy;
	int W = std::min(H, 20);
	int X = x() + w() - W - std::max(dx, dy);
	int Y = y() + dy;
	Fl_Rect ab(X, Y, W, H);
	int active = active_r();
	Fl_Color arrow_color = active ? labelcolor() : fl_inactive(labelcolor());
	Fl_Color box_color = color();
	if (!Fl::scheme()) {
		box_color = fl_contrast(textcolor(), FL_BACKGROUND2_COLOR) == textcolor() ? FL_BACKGROUND2_COLOR : fl_lighter(color());
	}
	draw_box(btype, box_color);
	if (Fl::scheme()) {
		int x1 = x() + w() - W - 1;
		int y1 = y() + 3;
		int y2 = y() + h() - 3;
		fl_color(active ? fl_darker(color()) : fl_inactive(fl_darker(color())));
		fl_yxline(x1, y1, y2);
		fl_color(active ? fl_lighter(color()) : fl_inactive(fl_lighter(color())));
		fl_yxline(x1 + 1, y1, y2);
	}
	else {
		draw_box(OS_MINI_BUTTON_UP_BOX, X, Y, W, H, color());
		ab.inset(OS_MINI_BUTTON_UP_BOX);
		ab.inset(OS_MINI_BUTTON_UP_BOX);
	}
	fl_draw_arrow(ab, FL_ARROW_CHOICE, FL_ORIENT_NONE, arrow_color);
	W += 2 * dx;
	if (mvalue()) {
		Fl_Menu_Item m = *mvalue();
		if (active) { m.activate(); }
		else { m.deactivate(); }
		int xx = x() + dx, yy = y() + dy + 1, ww = w() - W, hh = H - 2;
		fl_push_clip(xx, yy, ww, hh);
		if (Fl::scheme()) {
			Fl_Label l;
			l.value = m.text;
			l.image = 0;
			l.deimage = 0;
			l.type = m.labeltype_;
			l.font = m.labelsize_ || m.labelfont_ ? m.labelfont_ : textfont();
			l.size = m.labelsize_ ? m.labelsize_ : textsize();
			l.color= m.labelcolor_ ? m.labelcolor_ : textcolor();
			l.h_margin_ = l.v_margin_ = l.spacing = 0;
			if (!m.active()) l.color = fl_inactive((Fl_Color)l.color);
			fl_draw_shortcut = 2; // hack value to make '&' disappear
			l.draw(xx+3, yy, ww>6 ? ww-6 : 0, hh, FL_ALIGN_LEFT);
			fl_draw_shortcut = 0;
			if (Fl::focus() == this) { draw_focus(box(), xx, yy, ww, hh); }
		}
		else {
			fl_draw_shortcut = 2; // hack value to make '&' disappear
			m.draw(xx, yy, ww, hh, this, Fl::focus() == this);
			fl_draw_shortcut = 0;
		}
		fl_pop_clip();
	}
	draw_label();
}

int Dropdown::handle(int event) {
	if (event == FL_SHORTCUT && !Fl::event_state(FL_ALT)) return 0;
	return Fl_Choice::handle(event);
}

OS_Scroll::OS_Scroll(int x, int y, int w, int h, const char *l) : Fl_Scroll(x, y, w, h, l) {
	scrollbar.slider(OS_MINI_BUTTON_UP_BOX);
	hscrollbar.slider(OS_MINI_BUTTON_UP_BOX);
}

Workspace::Workspace(int x, int y, int w, int h, const char *l) : OS_Scroll(x, y, w, h, l),
	_content_w(0), _content_h(0), _ox(0), _oy(0), _cx(0), _cy(0), _dnd_receiver(NULL), _correlates() {
	labeltype(FL_NO_LABEL);
	box(FL_NO_BOX);
	color(FL_INACTIVE_COLOR);
	hscrollbar.callback((Fl_Callback *)hscrollbar_cb);
	scrollbar.callback((Fl_Callback *)scrollbar_cb);
}

int Workspace::handle(int event) {
	if (_dnd_receiver) {
		switch (event) {
		case FL_DND_ENTER:
		case FL_DND_LEAVE:
		case FL_DND_DRAG:
		case FL_DND_RELEASE:
			return 1;
		case FL_PASTE:
			return _dnd_receiver->handle(event);
		}
	}
	switch (event) {
	case FL_PUSH:
		if (Fl::event_button() != FL_MIDDLE_MOUSE) { break; }
		Fl::belowmouse(this);
		_ox = xposition();
		_oy = yposition();
		_cx = Fl::event_x();
		_cy = Fl::event_y();
		fl_cursor(FL_CURSOR_MOVE);
		return 1;
	case FL_RELEASE:
		fl_cursor(FL_CURSOR_DEFAULT);
		return 1;
	case FL_DRAG:
		int dx = Fl::event_x(), dy = Fl::event_y();
		int nx = _ox + (_cx - dx), ny = _oy + (_cy - dy);
		int max_x = std::max(_content_w - w() + (scrollbar.visible() ? Fl::scrollbar_size() : 0), 0);
		int max_y = std::max(_content_h - h() + (hscrollbar.visible() ? Fl::scrollbar_size() : 0), 0);
		scroll_to(std::clamp(nx, 0, max_x), std::clamp(ny, 0, max_y));
		return 1;
	}
	return Fl_Scroll::handle(event);
}

void Workspace::scroll_to(int x, int y) {
	Fl_Scroll::scroll_to(x, y);
	for (Fl_Widget *wgt : _correlates) {
		wgt->damage(1);
	}
}

void Workspace::hscrollbar_cb(Fl_Scrollbar *sb, void *) {
	Workspace *ws = (Workspace *)(sb->parent());
	ws->scroll_to(sb->value(), ws->yposition());
}

void Workspace::scrollbar_cb(Fl_Scrollbar *sb, void *) {
	Workspace *ws = (Workspace *)(sb->parent());
	ws->scroll_to(ws->xposition(), sb->value());
}

Toolbar::Toolbar(int x, int y, int w, int h, const char *l) : Fl_Group(x, y, w, h, l) {
	labeltype(FL_NO_LABEL);
	box(OS_TOOLBAR_FRAME);
	resizable(nullptr);
	clip_children(1);
	begin();
}

Toolbar_Button::Toolbar_Button(int x, int y, int w, int h, const char *l) : Fl_Button(x, y, w, h, l) {
	box(OS_BG_BOX);
	color(FL_BACKGROUND_COLOR);
	down_box(OS_MINI_DEPRESSED_DOWN_BOX);
	down_color(FL_SELECTION_COLOR);
	labelfont(OS_FONT);
	labelsize(OS_FONT_SIZE);
	align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
}

void Toolbar_Button::draw() {
	// Based on Fl_Button::draw()
	Fl_Color col = value() ? (OS::current_theme() == OS::Theme::CLASSIC ? fl_lighter(color()) : selection_color()) : color();
	draw_box(value() ? OS::current_theme() == OS::Theme::OLIVE ? OS_MINI_BUTTON_UP_BOX :
		down_box() ? down_box() : fl_down(box()) : box(), col);
	draw_backdrop();
#ifndef _WIN32
	draw_label();
#else
	Fl_Image *img = active_r() ? image() : deimage();
	fl_push_no_clip();
	float scale = fl_override_scale();
	int SX = (int)(x() * scale + (x() < 0 ? -0.001f : 0.001f));
	int SY = (int)(y() * scale + (y() < 0 ? -0.001f : 0.001f));
	int SW = (int)(w() * scale + (w() < 0 ? -0.001f : 0.001f));
	int SH = (int)(h() * scale + (h() < 0 ? -0.001f : 0.001f));
	int IW = img->w();
	int IH = img->h();
	if (SW < IW || SH < IH) {
		fl_restore_scale(scale);
		fl_pop_clip();
		draw_label();
		return;
	}
	int W = IW * (SW / IW);
	int H = IH * (SH / IH);
	int X = (int)(SX + SW/2.0f - W/2.0f + 0.5f);
	int Y = (int)(SY + SH/2.0f - H/2.0f + 0.5f);
	img->scale(W, H, 0, 1);
	img->draw(X, Y, W, H, 0, 0);
	img->scale(IW, IH, 0, 1);
	fl_restore_scale(scale);
	fl_pop_clip();
#endif
}

int Toolbar_Button::handle(int event) {
	switch (event) {
	case FL_ENTER:
		if (active_r()) {
			color(FL_LIGHT3);
			box(OS_TOOLBAR_BUTTON_HOVER_BOX);
			redraw();
			return 1;
		}
		return 0;
	case FL_LEAVE:
		color(FL_BACKGROUND_COLOR);
		box(OS_BG_BOX);
		redraw();
		return 1;
	case FL_KEYBOARD:
		// ignore spacebar
		if (Fl::event_key() == ' ') {
			return 0;
		}
		break;
	case FL_PUSH:
		Fl::focus(this);
	}
	return Fl_Button::handle(event);
}

Toolbar_Toggle_Button::Toolbar_Toggle_Button(int x, int y, int w, int h, const char *l) :
	Toolbar_Button(x, y, w, h, l) {
	type(FL_TOGGLE_BUTTON);
}

Toolbar_Radio_Button::Toolbar_Radio_Button(int x, int y, int w, int h, const char *l) :
	Toolbar_Button(x, y, w, h, l) {
	type(FL_RADIO_BUTTON);
}

Context_Menu::Context_Menu(int x, int y, int w, int h, const char *l) :
	Fl_Menu_(x, y, w, h, l) {
	box(OS_PANEL_THIN_UP_BOX);
	down_box(FL_FLAT_BOX);
	type(FL_RIGHT_MOUSE);
	shortcut(FL_SHIFT + FL_F + 10);
}

int Context_Menu::handle(int event) {
	switch (event) {
	case FL_PUSH:
		if (Fl::event_button() != type()) return 0;
		if (!prepare(Fl::event_x(), Fl::event_y())) return 0;
		picked(menu()->popup(Fl::event_x(), Fl::event_y(), nullptr, nullptr, this));
		return 1;
	case FL_SHORTCUT:
		int X, Y;
		Fl::get_mouse(X, Y);
		for (Fl_Window *w = window(); w; w = w->window()) {
			X -= w->x();
			Y -= w->y();
		}
		if (X >= x() && X < x() + w() && Y >= y() && Y < y() + h()) {
			if (Fl::test_shortcut(shortcut())) {
				if (!prepare(X, Y)) return 0;
				picked(menu()->popup(X, Y, nullptr, nullptr, this));
				return 1;
			}
			return test_shortcut() != 0;
		}
		return 0;
	default:
		return 0;
	}
}

bool Context_Menu::prepare(int X, int Y) {
	return menu() && menu()->text;
}

void Context_Menu::draw(void) {
	return;
}
