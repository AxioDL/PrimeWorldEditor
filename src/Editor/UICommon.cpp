#include "UICommon.h"
#include <QDesktopServices>
#include <QProcess>

namespace UICommon
{

void OpenContainingFolder(const QString& rkPath)
{
#if WIN32
    QStringList Args;
    Args << "/select," << QDir::toNativeSeparators(rkPath);
    QProcess::startDetached("explorer", Args);
#else
#error OpenContainingFolder() not implemented!
#endif
}

bool OpenInExternalApplication(const QString& rkPath)
{
    return QDesktopServices::openUrl( QString("file:///") + QDir::toNativeSeparators(rkPath) );
}

}
