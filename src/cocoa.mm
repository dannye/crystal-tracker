#import <Cocoa/Cocoa.h>

#pragma warning(push, 0)
#include <FL/platform.H>
#pragma warning(pop)

#include "cocoa.h"

void cocoa_set_appearance(const Fl_Window *w, enum cocoa_appearance appearance_id) {
	NSAppearance *appearance;
	switch (appearance_id) {
	default:
	case COCOA_APPEARANCE_AQUA:
		appearance = [NSAppearance appearanceNamed:NSAppearanceNameAqua];
		break;
#ifdef MAC_OS_X_VERSION_10_14
	case COCOA_APPEARANCE_DARK_AQUA:
		if (@available(macOS 10.14, *)) {
			appearance = [NSAppearance appearanceNamed:NSAppearanceNameDarkAqua];
		} else {
			appearance = [NSAppearance appearanceNamed:NSAppearanceNameAqua];
		}
		break;
#endif
	}
	[fl_xid(w) setAppearance: appearance];
}

bool cocoa_is_dark_mode() {
	NSString *dark = @"Dark";
	NSString *osxMode = [[NSUserDefaults standardUserDefaults] stringForKey:@"AppleInterfaceStyle"];
	return [osxMode isEqualToString:dark];
}
