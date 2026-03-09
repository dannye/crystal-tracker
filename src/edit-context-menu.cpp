#include "edit-context-menu.h"

#include "main-window.h"

bool Edit_Context_Menu::prepare(int event, int X, int Y) {
	Main_Window *mw = (Main_Window *)user_data();
	bool set = mw->set_context_menu(event, X, Y);
	if (!set) return false;
	popup_x(X);
	popup_y(Y);
	return menu() && menu()->text;
}
