#pragma warning(push, 0)
#include <FL/fl_draw.H>
#pragma warning(pop)

#include <cstdio>

#include "ruler.h"
#include "main-window.h"
#include "themes.h"

Ruler::Ruler(int x, int y, int w, int h, const char *l) : Fl_Box(x, y, w, h, l) {}

static inline void print_tick_label(char *s, int n) {
	sprintf(s, "%d", n);
}

void Ruler::draw() {
	int X = x(), Y = y(), W = w(), H = h();
	Main_Window *mw = (Main_Window *)user_data();
	int S = mw->song_ticks_per_step() * 16 * (mw->zoom() ? TICK_WIDTH_ZOOMED : TICK_WIDTH_UNZOOMED);
	int s = S / 4;
	// background
	fl_color(FL_DARK2);
	fl_rectf(X, Y, W, H);
	// edges
	fl_color(fl_color_average(FL_FOREGROUND_COLOR, FL_BACKGROUND_COLOR, 0.4f));
	fl_xyline(X, Y+H-1, X+W-1);
	// tick marks and labels
	int mx = mw->song_scroll_x();
	// tick marks
	int o = s * 4 - WHITE_KEY_WIDTH;
	int r = mx % s;
	int d = ((mx / s) % 2) ? 0 : H / 2;
	for (int i = s-r-1; i < W + o; i += s, d = d ? 0 : H / 2) {
		fl_yxline(X+i - o, Y+d, Y+H-1);
	}
	// labels
	char t[8] = {};
	fl_font(FL_COURIER, 12);
	fl_color(FL_FOREGROUND_COLOR);
	fl_push_clip(X, Y, W, H);
	int O = S - WHITE_KEY_WIDTH;
	int R = mx % S;
	int n = mx / S;
	for (int i = S-R-1; i < W+S + O; i += S, n++) {
		print_tick_label(t, n);
		fl_draw(t, X+i-S+1 - O, Y, S-2, H, FL_ALIGN_BOTTOM_RIGHT | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
	}
	fl_pop_clip();
}
