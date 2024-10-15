#ifndef ICONS_H
#define ICONS_H

#pragma warning(push, 0)
#include <FL/Fl_Pixmap.H>
#pragma warning(pop)

#include "blank.xpm"
#include "brush.xpm"
#include "brush-cmy.xpm"
#include "decrease-spacing.xpm"
#include "delete.xpm"
#include "down.xpm"
#include "down-down.xpm"
#include "four.xpm"
#include "glue-dark.xpm"
#include "glue-light.xpm"
#include "increase-spacing.xpm"
#include "keys.xpm"
#include "left.xpm"
#include "loop.xpm"
#include "minus.xpm"
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
#include "zoom-in.xpm"
#include "zoom-out.xpm"

struct Scalable_Pixmap {
	Fl_Pixmap pixmap;
	Fl_Image *image = nullptr;

	Scalable_Pixmap(const char * const * data) : pixmap(data) {}
	~Scalable_Pixmap() { if (image) delete image; }

	Fl_Image *get(float scale) {
#ifdef __APPLE__
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
static Scalable_Pixmap FOUR_ICON(FOUR_XPM);
static Scalable_Pixmap GLUE_DARK_ICON(GLUE_DARK_XPM);
static Scalable_Pixmap GLUE_LIGHT_ICON(GLUE_LIGHT_XPM);
static Scalable_Pixmap INCREASE_SPACING_ICON(INCREASE_SPACING_XPM);
static Scalable_Pixmap KEYS_ICON(KEYS_XPM);
static Scalable_Pixmap LEFT_ICON(LEFT_XPM);
static Scalable_Pixmap LOOP_ICON(LOOP_XPM);
static Scalable_Pixmap MINUS_ICON(MINUS_XPM);
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

#endif
