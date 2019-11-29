#ifndef MACOSEXTRAS_H
#define MACOSEXTRAS_H
#ifdef __APPLE__

#include <CoreGraphics/CGBase.h>
extern "C" Boolean AXIsProcessTrusted(void);

#include <QAbstractNativeEventFilter>
#include <QString>

void MacOSSetDarkAppearance();
QString MacOSPathToDolphinBinary();

class MouseDragCocoaEventFilter : public QAbstractNativeEventFilter
{
    CGFloat mX = 0.0, mY = 0.0;
public:
    bool nativeEventFilter(const QByteArray& eventType, void* message, long*) override;
    CGFloat claimX() { CGFloat ret = mX; mX = 0.0; return ret; }
    CGFloat claimY() { CGFloat ret = mY; mY = 0.0; return ret; }
};

extern MouseDragCocoaEventFilter* gpMouseDragCocoaEventFilter;

#endif // __APPLE__
#endif // MACOSEXTRAS_H
