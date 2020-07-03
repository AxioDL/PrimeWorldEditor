#include "UICommon.h"
#include <QDesktopServices>
#include <QProcess>

namespace UICommon
{

QWindow* FindWidgetWindowHandle(QWidget *pWidget)
{
    while (pWidget && !pWidget->windowHandle())
        pWidget = pWidget->parentWidget();

    return pWidget ? pWidget->windowHandle() : nullptr;
}

void OpenContainingFolder(QWidget* parent, const QString& pathIn)
{
    const QFileInfo fileInfo(pathIn);
    // Mac, Windows support folder or file.
#if defined(Q_OS_WIN)
    const QString paths = QProcessEnvironment::systemEnvironment().value(QStringLiteral("Path"));
    QString explorer;
    for (const QString& path : paths.split(QLatin1Char(';')))
    {
        const QFileInfo finfo(QDir(path), QStringLiteral("explorer.exe"));
        if (finfo.exists())
        {
            explorer = finfo.filePath();
            break;
        }
    }
    if (explorer.isEmpty())
    {
        QMessageBox::warning(parent, "Launching Windows Explorer Failed",
                             "Could not find explorer.exe in path to launch Windows Explorer.");
        return;
    }
    QStringList param;
    if (!fileInfo.isDir())
        param += QLatin1String("/select,");
    param += QDir::toNativeSeparators(fileInfo.canonicalFilePath());
    QProcess::startDetached(explorer, param);
#elif defined(Q_OS_MAC)
    QStringList scriptArgs;
    scriptArgs << QLatin1String("-e")
               << QString::fromLatin1("tell application \"Finder\" to reveal POSIX file \"%1\"")
                      .arg(fileInfo.canonicalFilePath());
    QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
    scriptArgs.clear();
    scriptArgs << QLatin1String("-e") << QLatin1String("tell application \"Finder\" to activate");
    QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
#else
    // we cannot select a file here, because no file browser really supports it...
    const QString folder = fileInfo.isDir() ? fileInfo.absoluteFilePath() : fileInfo.filePath();
    QProcess browserProc;
    const QString browserArgs = QStringLiteral("xdg-open \"%1\"").arg(QFileInfo(folder).path());
    browserProc.startDetached(browserArgs);
#endif
}

bool OpenInExternalApplication(const QString& rkPath)
{
    return QDesktopServices::openUrl( QString("file:///") + QDir::toNativeSeparators(rkPath) );
}

}
