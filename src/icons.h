#ifndef ICONS_H
#define ICONS_H

#pragma warning(push, 0)
#include <FL/Fl_Pixmap.H>
#pragma warning(pop)

#include "blank.xpm"
#include "decrease-spacing.xpm"
#include "down.xpm"
#include "down-down.xpm"
#include "four.xpm"
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
#include "play.xpm"
#include "plus.xpm"
#include "redo.xpm"
#include "right.xpm"
#include "ruler.xpm"
#include "save.xpm"
#include "save-as.xpm"
#include "scroll-dark.xpm"
#include "scroll-light.xpm"
#include "stop.xpm"
#include "three.xpm"
#include "two.xpm"
#include "undo.xpm"
#include "up.xpm"
#include "up-up.xpm"
#include "verify.xpm"
#include "zoom.xpm"

static Fl_Pixmap BLANK_ICON(BLANK_XPM);
static Fl_Pixmap DECREASE_SPACING_ICON(DECREASE_SPACING_XPM);
static Fl_Pixmap DOWN_ICON(DOWN_XPM);
static Fl_Pixmap DOWN_DOWN_ICON(DOWN_DOWN_XPM);
static Fl_Pixmap FOUR_ICON(FOUR_XPM);
static Fl_Pixmap INCREASE_SPACING_ICON(INCREASE_SPACING_XPM);
static Fl_Pixmap KEYS_ICON(KEYS_XPM);
static Fl_Pixmap LEFT_ICON(LEFT_XPM);
static Fl_Pixmap LOOP_ICON(LOOP_XPM);
static Fl_Pixmap MINUS_ICON(MINUS_XPM);
static Fl_Pixmap NEW_ICON(NEW_XPM);
static Fl_Pixmap NOTES_ICON(NOTES_XPM);
static Fl_Pixmap ONE_ICON(ONE_XPM);
static Fl_Pixmap OPEN_ICON(OPEN_XPM);
static Fl_Pixmap PAUSE_ICON(PAUSE_XPM);
static Fl_Pixmap PENCIL_ICON(PENCIL_XPM);
static Fl_Pixmap PLAY_ICON(PLAY_XPM);
static Fl_Pixmap PLUS_ICON(PLUS_XPM);
static Fl_Pixmap REDO_ICON(REDO_XPM);
static Fl_Pixmap RIGHT_ICON(RIGHT_XPM);
static Fl_Pixmap RULER_ICON(RULER_XPM);
static Fl_Pixmap SAVE_ICON(SAVE_XPM);
static Fl_Pixmap SAVE_AS_ICON(SAVE_AS_XPM);
static Fl_Pixmap SCROLL_DARK_ICON(SCROLL_DARK_XPM);
static Fl_Pixmap SCROLL_LIGHT_ICON(SCROLL_LIGHT_XPM);
static Fl_Pixmap STOP_ICON(STOP_XPM);
static Fl_Pixmap THREE_ICON(THREE_XPM);
static Fl_Pixmap TWO_ICON(TWO_XPM);
static Fl_Pixmap UNDO_ICON(UNDO_XPM);
static Fl_Pixmap UP_ICON(UP_XPM);
static Fl_Pixmap UP_UP_ICON(UP_UP_XPM);
static Fl_Pixmap VERIFY_ICON(VERIFY_XPM);
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
