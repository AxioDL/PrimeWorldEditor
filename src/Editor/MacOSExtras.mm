#import <AppKit/AppKit.h>
#include "MacOSExtras.h"

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

/* Filter to accumulate relative coordinates of middle and right mouse drags for camera control. */
bool MouseDragCocoaEventFilter::nativeEventFilter(const QByteArray& eventType, void* message, long*)
{
    if (eventType == "mac_generic_NSEvent")
    {
        NSEvent* event = static_cast<NSEvent*>(message);
        NSEventType evType = event.type;
        if (evType == NSEventTypeRightMouseDragged ||
            (evType == NSEventTypeOtherMouseDragged && event.buttonNumber == 2))
        {
            mX += event.deltaX;
            mY += event.deltaY;
        }
    }
    return false;
}

MouseDragCocoaEventFilter* gpMouseDragCocoaEventFilter = nullptr;
