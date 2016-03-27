#ifndef UICOMMON
#define UICOMMON

#include <Common/TString.h>
#include <QMap>
#include <QString>

#define TO_QSTRING(str) UICommon::ToQString(str)
#define TO_TSTRING(str) UICommon::ToTString(str)
#define TO_TWIDESTRING(str) UICommon::ToTWideString(str)

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

