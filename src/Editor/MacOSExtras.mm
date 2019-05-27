#import <AppKit/AppKit.h>
#include <QString>

void MacOSSetDarkAppearance()
{
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 101400
    if ([NSApp respondsToSelector:@selector(setAppearance:)])
        [NSApp setAppearance:[NSAppearance appearanceNamed:NSAppearanceNameDarkAqua]];
#endif
}

QString MacOSPathToDolphinBinary()
{
    if (id path = [[NSWorkspace sharedWorkspace]
                   absolutePathForAppBundleWithIdentifier:@"org.dolphin-emu.dolphin"])
        return QString::fromNSString(path) + QStringLiteral("/Contents/MacOS/Dolphin");
    return QString();
}
