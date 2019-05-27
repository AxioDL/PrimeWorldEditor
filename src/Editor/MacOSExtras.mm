#import <AppKit/AppKit.h>

void MacOSSetDarkAppearance()
{
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 101400
    if ([NSApp respondsToSelector:@selector(setAppearance:)])
        [NSApp setAppearance:[NSAppearance appearanceNamed:NSAppearanceNameDarkAqua]];
#endif
}
