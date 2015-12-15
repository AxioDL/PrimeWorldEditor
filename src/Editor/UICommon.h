#ifndef UICOMMON
#define UICOMMON

#include <QMap>
#include <QString>
#include <Common/TString.h>

#define TO_QSTRING(str) UICommon::ToQString(str)
#define TO_TSTRING(str) UICommon::ToTString(str)
#define TO_TWIDESTRING(str) UICommon::ToTWideSTring(str)

namespace UICommon
{
extern QMap<QString,QString> FilterMap;
QString ExtensionFilterString(const QString& extension);

// TString/TWideString <-> QString
inline QString ToQString(const TString& str)
{
    return QString::fromStdString(str.ToStdString());
}

inline QString ToQString(const TWideString& str)
{
    return QString::fromStdWString(str.ToStdString());
}

inline TString ToTString(const QString& str)
{
    return TString(str.toStdString());
}

inline TWideString ToTWideString(const QString& str)
{
    return TWideString(str.toStdWString());
}
}

#endif // UICOMMON

