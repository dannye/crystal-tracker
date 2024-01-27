#include "edit-context-menu.h"

#include "main-window.h"

bool Edit_Context_Menu::prepare(int X, int Y) {
	Main_Window *mw = (Main_Window *)user_data();
	mw->set_context_menu(X, Y);
	return menu() && menu()->text;
}
