#include <iostream>

#include <FL/Fl.H>

#include "main-window.h"

int main(int argc, char** argv) {
	int x = 48, y = 48;
	int w = 800, h = 600;
	Main_Window window(x, y, w, h);
	window.show();

	return Fl::run();
}
