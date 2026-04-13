#ifndef ICONS_H
#define ICONS_H

#pragma warning(push, 0)
#include <FL/Fl_Pixmap.H>
#include <FL/fl_draw.H>
#pragma warning(pop)

#include "blank.xpm"
#include "brush.xpm"
#include "brush-cmy.xpm"
#include "decrease-spacing.xpm"
#include "delete.xpm"
#include "down.xpm"
#include "down-down.xpm"
#include "drumkit.xpm"
#include "four.xpm"
#include "glue-dark.xpm"
#include "glue-light.xpm"
#include "increase-spacing.xpm"
#include "keys.xpm"
#include "left.xpm"
#include "loop.xpm"
#include "minus.xpm"
#include "more-dark.xpm"
#include "more-light.xpm"
#include "new.xpm"
#include "notes.xpm"
#include "one.xpm"
#include "open.xpm"
#include "pause.xpm"
#include "pencil.xpm"
#include "pencil-blue.xpm"
#include "pencil-brown.xpm"
#include "pencil-green.xpm"
#include "pencil-red.xpm"
#include "play.xpm"
#include "plus.xpm"
#include "redo.xpm"
#include "right.xpm"
#include "ruler.xpm"
#include "save.xpm"
#include "save-as.xpm"
#include "scroll-dark.xpm"
#include "scroll-light.xpm"
#include "snip.xpm"
#include "split-dark.xpm"
#include "split-light.xpm"
#include "stop.xpm"
#include "three.xpm"
#include "two.xpm"
#include "undo.xpm"
#include "up.xpm"
#include "up-up.xpm"
#include "verify.xpm"
#include "wave.xpm"
#include "zoom-in.xpm"
#include "zoom-out.xpm"

struct Scalable_Pixmap {
	Fl_Pixmap pixmap;
	Fl_Image *image = nullptr;

	Scalable_Pixmap(const char * const * data) : pixmap(data) {}
	~Scalable_Pixmap() { if (image) delete image; }

	Fl_Image *get(float scale) {
#if defined(_WIN32) || defined(__APPLE__)
		return &pixmap;
#else
		if (scale == 1.0f) return &pixmap;
		if (!image) {
			int W = pixmap.w(), H = pixmap.h();
			Fl_Image *temp = new Fl_RGB_Image(&pixmap);
			Fl_Image::RGB_scaling(FL_RGB_SCALING_BILINEAR);
			image = temp->copy(2 * W, 2 * H);
			Fl_Image::RGB_scaling(FL_RGB_SCALING_NEAREST);
			delete temp;
			image->scale(W, H);
		}
		return image;
#endif
	}
};

static Fl_Pixmap BLANK_ICON(BLANK_XPM);

static Scalable_Pixmap BRUSH_ICON(BRUSH_XPM);
static Scalable_Pixmap BRUSH_CMY_ICON(BRUSH_CMY_XPM);
static Scalable_Pixmap DECREASE_SPACING_ICON(DECREASE_SPACING_XPM);
static Scalable_Pixmap DELETE_ICON(DELETE_XPM);
static Scalable_Pixmap DOWN_ICON(DOWN_XPM);
static Scalable_Pixmap DOWN_DOWN_ICON(DOWN_DOWN_XPM);
static Scalable_Pixmap DRUMKIT_ICON(DRUMKIT_XPM);
static Scalable_Pixmap FOUR_ICON(FOUR_XPM);
static Scalable_Pixmap GLUE_DARK_ICON(GLUE_DARK_XPM);
static Scalable_Pixmap GLUE_LIGHT_ICON(GLUE_LIGHT_XPM);
static Scalable_Pixmap INCREASE_SPACING_ICON(INCREASE_SPACING_XPM);
static Scalable_Pixmap KEYS_ICON(KEYS_XPM);
static Scalable_Pixmap LEFT_ICON(LEFT_XPM);
static Scalable_Pixmap LOOP_ICON(LOOP_XPM);
static Scalable_Pixmap MINUS_ICON(MINUS_XPM);
static Scalable_Pixmap MORE_DARK_ICON(MORE_DARK_XPM);
static Scalable_Pixmap MORE_LIGHT_ICON(MORE_LIGHT_XPM);
static Scalable_Pixmap NEW_ICON(NEW_XPM);
static Scalable_Pixmap NOTES_ICON(NOTES_XPM);
static Scalable_Pixmap ONE_ICON(ONE_XPM);
static Scalable_Pixmap OPEN_ICON(OPEN_XPM);
static Scalable_Pixmap PAUSE_ICON(PAUSE_XPM);
static Scalable_Pixmap PENCIL_ICON(PENCIL_XPM);
static Scalable_Pixmap PENCIL_BLUE_ICON(PENCIL_BLUE_XPM);
static Scalable_Pixmap PENCIL_BROWN_ICON(PENCIL_BROWN_XPM);
static Scalable_Pixmap PENCIL_GREEN_ICON(PENCIL_GREEN_XPM);
static Scalable_Pixmap PENCIL_RED_ICON(PENCIL_RED_XPM);
static Scalable_Pixmap PLAY_ICON(PLAY_XPM);
static Scalable_Pixmap PLUS_ICON(PLUS_XPM);
static Scalable_Pixmap REDO_ICON(REDO_XPM);
static Scalable_Pixmap RIGHT_ICON(RIGHT_XPM);
static Scalable_Pixmap RULER_ICON(RULER_XPM);
static Scalable_Pixmap SAVE_ICON(SAVE_XPM);
static Scalable_Pixmap SAVE_AS_ICON(SAVE_AS_XPM);
static Scalable_Pixmap SCROLL_DARK_ICON(SCROLL_DARK_XPM);
static Scalable_Pixmap SCROLL_LIGHT_ICON(SCROLL_LIGHT_XPM);
static Scalable_Pixmap SNIP_ICON(SNIP_XPM);
static Scalable_Pixmap SPLIT_DARK_ICON(SPLIT_DARK_XPM);
static Scalable_Pixmap SPLIT_LIGHT_ICON(SPLIT_LIGHT_XPM);
static Scalable_Pixmap STOP_ICON(STOP_XPM);
static Scalable_Pixmap THREE_ICON(THREE_XPM);
static Scalable_Pixmap TWO_ICON(TWO_XPM);
static Scalable_Pixmap UNDO_ICON(UNDO_XPM);
static Scalable_Pixmap UP_ICON(UP_XPM);
static Scalable_Pixmap UP_UP_ICON(UP_UP_XPM);
static Scalable_Pixmap VERIFY_ICON(VERIFY_XPM);
static Scalable_Pixmap WAVE_ICON(WAVE_XPM);
static Scalable_Pixmap ZOOM_IN_ICON(ZOOM_IN_XPM);
static Scalable_Pixmap ZOOM_OUT_ICON(ZOOM_OUT_XPM);

bool make_deimage(Fl_Widget *wgt, Fl_Image *image = nullptr) {
	if (!wgt || !wgt->image()) {
		return false;
	}
	if (!image) image = wgt->image();
	Fl_Image *deimg = image->copy();
	if (!deimg) {
		return false;
	}
	deimg->desaturate();
	deimg->color_average(FL_GRAY, 0.5f);
	if (wgt->deimage()) {
		delete wgt->deimage();
	}
	wgt->deimage(deimg);
	return true;
}

#define BP fl_begin_polygon()
#define EP fl_end_polygon()
#define BC fl_begin_loop()
#define EC fl_end_loop()
#define vv(x,y) fl_vertex(x,y)

static void set_outline_color(Fl_Color c) {
	fl_color(fl_color_average(c, FL_BLACK, .9f));
}

static void rectangle(double x, double y, double x2, double y2, Fl_Color col) {
	fl_color(col);
	BP; vv(x,y); vv(x2,y); vv(x2,y2); vv(x,y2); EP;
	set_outline_color(col);
	BC; vv(x,y); vv(x2,y); vv(x2,y2); vv(x,y2); EC;
}

static void draw_arrow2(Fl_Color col) {
	fl_color(col);
	BP; vv(-0.3,0.8); vv(0.50,0.0); vv(-0.3,-0.8); EP;
	set_outline_color(col);
	BC; vv(-0.3,0.8); vv(0.50,0.0); vv(-0.3,-0.8); EC;
}

static void draw_arrow3(Fl_Color col) {
	fl_color(col);
	BP; vv(0.1,0.8); vv(0.9,0.0); vv(0.1,-0.72); EP;
	BP; vv(-0.7,0.8); vv(0.1,0.0); vv(-0.7,-0.72); EP;
	set_outline_color(col);
	BC; vv(0.1,0.8); vv(0.9,0.0); vv(0.1,-0.72); EC;
	BC; vv(-0.7,0.8); vv(0.1,0.0); vv(-0.7,-0.72); EC;
}

static void draw_arrow03(Fl_Color col) {
	fl_color(col);
	BP; vv(-0.1,0.8); vv(-0.9,0.0); vv(-0.1,-0.72); EP;
	BP; vv(0.7,0.8); vv(-0.1,0.0); vv(0.7,-0.72); EP;
	set_outline_color(col);
	BC; vv(-0.1,0.8); vv(-0.9,0.0); vv(-0.1,-0.72); EC;
	BC; vv(0.7,0.8); vv(-0.1,0.0); vv(0.7,-0.72); EC;
}

static void draw_uparrowbar(Fl_Color col) {
	fl_color(col);
	if (fl_graphics_driver->can_fill_non_convex_polygon()) {
		// draw the arrow as a 8-vertex filled polygon
		BP; vv(-0.2,0.8); vv(0.2,0.8); vv(0.2,-0.05); vv(0.6,-0.05); vv(0.01,-0.6);
		vv(-0.01,-0.6); vv(-0.6,-0.05); vv(-0.2,-0.05); EP;
	} else {
		// draw the arrow as a rectangle plus a triangle
		BP; vv(-0.2,0.8); vv(0.2,0.8); vv(0.2,-0.05); vv(-0.2,-0.05); EP;
		BP; vv(0.6,-0.05); vv(0.01,-0.6); vv(-0.01,-0.6); vv(-0.6,-0.05); vv(-0.2,-0.05); vv(0.2,-0.05); EP;
	}
	set_outline_color(col);
	BC; vv(-0.2,0.8); vv(0.2,0.8); vv(0.2,-0.05); vv(0.6,-0.05); vv(0.01,-0.6);
		vv(-0.01,-0.6); vv(-0.6,-0.05); vv(-0.2,-0.05); EC;
	rectangle(-.8,-.7,.8,-.9,col);
}

static void draw_dnarrowbar(Fl_Color col) {
	fl_color(col);
	if (fl_graphics_driver->can_fill_non_convex_polygon()) {
		// draw the arrow as a 8-vertex filled polygon
		BP; vv(-0.2,-0.8); vv(0.2,-0.8); vv(0.2,0.05); vv(0.6,0.05); vv(0.01,0.6);
		vv(-0.01,0.6); vv(-0.6,0.05); vv(-0.2,0.05); EP;
	} else {
		// draw the arrow as a rectangle plus a triangle
		BP; vv(-0.2,-0.8); vv(0.2,-0.8); vv(0.2,0.05); vv(-0.2,0.05); EP;
		BP; vv(0.6,0.05); vv(0.01,0.6); vv(-0.01,0.6); vv(-0.6,0.05); vv(-0.2,0.05); vv(0.2,0.05); EP;
	}
	set_outline_color(col);
	BC; vv(-0.2,-0.8); vv(0.2,-0.8); vv(0.2,0.05); vv(0.6,0.05); vv(0.01,0.6);
		vv(-0.01,0.6); vv(-0.6,0.05); vv(-0.2,0.05); EC;
	rectangle(-.8,.7,.8,.9,col);
}

static void draw_doublearrow(Fl_Color col) {
	fl_color(col);
	BP; vv(-0.35,-0.13); vv(-0.35,0.14); vv(0.35,0.14); vv(0.35,-0.13); EP;
	BP; vv(0.25,0.54); vv(0.75,0.0); vv(0.25,-0.53); EP;
	BP; vv(-0.25,0.54); vv(-0.75,0.0); vv(-0.25,-0.53); EP;
	set_outline_color(col);
	BC; vv(-0.25,0.14); vv(0.25,0.14); vv(0.25,0.54); vv(0.75,0.0);
		vv(0.25,-0.53); vv(0.25,-0.13); vv(-0.25,-0.13); vv(-0.25,-0.53);
		vv(-0.75,0.0); vv(-0.25,0.54); EC;
}

static void draw_doublearrow_vert(Fl_Color col) {
	fl_color(col);
	BP; vv(-0.13,-0.35); vv(0.14,-0.35); vv(0.14,0.35); vv(-0.13,0.35); EP;
	BP; vv(0.54,0.25); vv(0.0,0.75); vv(-0.53,0.25); EP;
	BP; vv(0.54,-0.25); vv(0.0,-0.75); vv(-0.53,-0.25); EP;
	set_outline_color(col);
	BC; vv(0.14,-0.25); vv(0.14,0.25); vv(0.54,0.25); vv(0.0,0.75);
		vv(-0.53,0.25); vv(-0.13,0.25); vv(-0.13,-0.25); vv(-0.53,-0.25);
		vv(0.0,-0.75); vv(0.54,-0.25); EC;
}

static void draw_plus(Fl_Color col) {
	fl_color(col);
	BP; vv(-0.9,-0.15); vv(-0.9,0.15); vv(0.9,0.15); vv(0.9,-0.15); EP;
	BP; vv(-0.15,-0.9); vv(-0.15,0.9); vv(0.15,0.9); vv(0.15,-0.9); EP;
	set_outline_color(col);
	BC;
	vv(-0.9,-0.15); vv(-0.9,0.15); vv(-0.15,0.15); vv(-0.15,0.9);
	vv(0.15,0.9); vv(0.15,0.15); vv(0.9,0.15); vv(0.9,-0.15);
	vv(0.15,-0.15); vv(0.15,-0.9); vv(-0.15,-0.9); vv(-0.15,-0.15);
	EC;
}

static void override_symbols() {
	fl_add_symbol(">",    draw_arrow2,           1);
	fl_add_symbol(">>",   draw_arrow3,           1);
	fl_add_symbol("<<",   draw_arrow03,          1);
	fl_add_symbol("^_",   draw_uparrowbar,       1);
	fl_add_symbol("v_",   draw_dnarrowbar,       1);
	fl_add_symbol("<->",  draw_doublearrow,      1);
	fl_add_symbol("^|v",  draw_doublearrow_vert, 1);
	fl_add_symbol("+",    draw_plus,             1);
}

#undef BP
#undef EP
#undef BC
#undef EC
#undef vv

#endif
