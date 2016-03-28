#ifndef UICOMMON
#define UICOMMON

#include <Common/TString.h>
#include <QMap>
#include <QString>

#define TO_QSTRING(Str) UICommon::ToQString(Str)
#define TO_TSTRING(Str) UICommon::ToTString(Str)
#define TO_TWIDESTRING(Str) UICommon::ToTWideString(Str)
#define TO_CCOLOR(Clr) CColor::Integral(Clr.red(), Clr.green(), Clr.blue(), Clr.alpha())
#define TO_QCOLOR(Clr) QColor(Clr.R * 255, Clr.G * 255, Clr.B * 255, Clr.A * 255)

namespace UICommon
{
extern QMap<QString,QString> FilterMap;
QString ExtensionFilterString(const QString& rkExtension);

// TString/TWideString <-> QString
inline QString ToQString(const TString& rkStr)
{
    return QString::fromStdString(rkStr.ToStdString());
}

inline QString ToQString(const TWideString& rkStr)
{
    return QString::fromStdWString(rkStr.ToStdString());
}

inline TString ToTString(const QString& rkStr)
{
    return TString(rkStr.toStdString());
}

inline TWideString ToTWideString(const QString& rkStr)
{
    return TWideString(rkStr.toStdWString());
}
}

#endif // UICOMMON

