#include "edit-context-menu.h"

#include "main-window.h"

bool Edit_Context_Menu::prepare() {
	Main_Window *mw = (Main_Window *)user_data();
	mw->set_context_menu();
	return menu() && menu()->text;
}
