#ifndef UICOMMON
#define UICOMMON

#include <QMap>
#include <QString>

namespace UICommon
{
extern QMap<QString,QString> FilterMap;
QString ExtensionFilterString(const QString& extension);
}

#endif // UICOMMON

