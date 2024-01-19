#ifndef EDIT_CONTEXT_MENU_H
#define EDIT_CONTEXT_MENU_H

#include "widgets.h"

class Edit_Context_Menu : public Context_Menu {
public:
	using Context_Menu::Context_Menu;

	bool prepare() override;
};

#endif
