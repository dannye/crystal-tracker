#pragma warning(push, 0)
#include <FL/fl_draw.H>
#pragma warning(pop)

#include <cstdio>

#include "ruler.h"
#include "main-window.h"
#include "themes.h"

Ruler::Ruler(int x, int y, int w, int h, const char *l) : Fl_Box(x, y, w, h, l) {}

int Ruler::handle(int event) {
	Main_Window *mw = (Main_Window *)user_data();
	if (event == FL_PUSH || (event == FL_DRAG && !mw->playing())) {
		mw->set_tick_from_x_pos(Fl::event_x());
		return 1;
	}
	if (event == FL_ENTER && mw->song_loaded()) {
		fl_cursor(FL_CURSOR_HAND);
		return 1;
	}
	if (event == FL_LEAVE) {
		fl_cursor(FL_CURSOR_DEFAULT);
		return 1;
	}
	return Fl_Box::handle(event);
}

static inline void print_tick_label(char *s, int n) {
	sprintf(s, "%d", n);
}

void Ruler::draw() {
	int X = x(), Y = y(), W = w(), H = h();
	Main_Window *mw = (Main_Window *)user_data();
	int px = mw->song_ticks_per_step() * TICK_WIDTHS[mw->zoom()+1];
	int s = _options.steps_per_beat * px;
	int S = _options.beats_per_measure * s;
	int p = _options.pickup_offset * px + WHITE_KEY_WIDTH;
	// background
	fl_color(FL_DARK2);
	fl_rectf(X, Y, W, H);
	// edges
	fl_color(fl_color_average(FL_FOREGROUND_COLOR, FL_BACKGROUND_COLOR, 0.4f));
	fl_xyline(X, Y+H-1, X+W-1);
	// tick marks and labels
	int mx = mw->song_scroll_x();
	// tick marks
	int o = (p / S + 1) * S - p;
	int r = mx % s;
	int n = mx / s + 1;
	for (int i = s-r-1; i < W + o; i += s, n++) {
		int d = (n % _options.beats_per_measure) ? H / 2 : 0;
		fl_yxline(X+i - o, Y+d, Y+H-1);
	}
	// labels
	char t[8] = {};
	fl_font(FL_COURIER, 12);
	fl_color(FL_FOREGROUND_COLOR);
	fl_push_clip(X, Y, W, H);
	int O = (p / S + 1) * S - p;
	int R = mx % S;
	int N = mx / S - p / S;
	for (int i = S-R-1; i < W+S + O; i += S, N++) {
		if (N >= 0) {
			print_tick_label(t, N);
			fl_draw(t, X+i-S+1 - O, Y, S-2, H, FL_ALIGN_BOTTOM_RIGHT | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
		}
	}
	fl_pop_clip();
}
