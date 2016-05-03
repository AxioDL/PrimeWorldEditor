#ifndef UICOMMON
#define UICOMMON

#include <Common/TString.h>
#include <QMap>
#include <QString>

// App string variable handling - automatically fill in application name/version
#define UI_APPVAR_NAME          "%APP_NAME%"
#define UI_APPVAR_FULLNAME      "%APP_FULL_NAME%"
#define UI_APPVAR_VERSION       "%APP_VERSION%"

#define REPLACE_APPVARS(InQString) \
    InQString.replace(UI_APPVAR_NAME, APP_NAME); \
    InQString.replace(UI_APPVAR_FULLNAME, APP_FULL_NAME); \
    InQString.replace(UI_APPVAR_VERSION, APP_VERSION);

#define SET_WINDOWTITLE_APPVARS(InString) \
    { \
        QString APPVAR_MACRO_NEWTITLE = InString; \
        REPLACE_APPVARS(APPVAR_MACRO_NEWTITLE) \
        setWindowTitle(APPVAR_MACRO_NEWTITLE); \
    }

#define REPLACE_WINDOWTITLE_APPVARS \
    SET_WINDOWTITLE_APPVARS(windowTitle());

// Common conversion functions
#define TO_QSTRING(Str)     UICommon::ToQString(Str)
#define TO_TSTRING(Str)     UICommon::ToTString(Str)
#define TO_TWIDESTRING(Str) UICommon::ToTWideString(Str)
#define TO_CCOLOR(Clr)      CColor::Integral(Clr.red(), Clr.green(), Clr.blue(), Clr.alpha())
#define TO_QCOLOR(Clr)      QColor(Clr.R * 255, Clr.G * 255, Clr.B * 255, Clr.A * 255)

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

