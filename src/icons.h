#ifndef ICONS_H
#define ICONS_H

#pragma warning(push, 0)
#include <FL/Fl_Pixmap.H>
#pragma warning(pop)

#include "blank.xpm"
#include "four.xpm"
#include "loop.xpm"
#include "new.xpm"
#include "one.xpm"
#include "open.xpm"
#include "pause.xpm"
#include "play.xpm"
#include "redo.xpm"
#include "save.xpm"
#include "save-as.xpm"
#include "scroll-dark.xpm"
#include "scroll-light.xpm"
#include "stop.xpm"
#include "three.xpm"
#include "two.xpm"
#include "undo.xpm"
#include "zoom.xpm"

static Fl_Pixmap BLANK_ICON(BLANK_XPM);
static Fl_Pixmap FOUR_ICON(FOUR_XPM);
static Fl_Pixmap LOOP_ICON(LOOP_XPM);
static Fl_Pixmap NEW_ICON(NEW_XPM);
static Fl_Pixmap ONE_ICON(ONE_XPM);
static Fl_Pixmap OPEN_ICON(OPEN_XPM);
static Fl_Pixmap PAUSE_ICON(PAUSE_XPM);
static Fl_Pixmap PLAY_ICON(PLAY_XPM);
static Fl_Pixmap REDO_ICON(REDO_XPM);
static Fl_Pixmap SAVE_ICON(SAVE_XPM);
static Fl_Pixmap SAVE_AS_ICON(SAVE_AS_XPM);
static Fl_Pixmap SCROLL_DARK_ICON(SCROLL_DARK_XPM);
static Fl_Pixmap SCROLL_LIGHT_ICON(SCROLL_LIGHT_XPM);
static Fl_Pixmap STOP_ICON(STOP_XPM);
static Fl_Pixmap THREE_ICON(THREE_XPM);
static Fl_Pixmap TWO_ICON(TWO_XPM);
static Fl_Pixmap UNDO_ICON(UNDO_XPM);
static Fl_Pixmap ZOOM_ICON(ZOOM_XPM);

bool make_deimage(Fl_Widget *wgt) {
	if (!wgt || !wgt->image()) {
		return false;
	}
	Fl_Image *deimg = wgt->image()->copy();
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
