#include "Editor/CEditorApplication.h"
#include "Editor/CUIRelay.h"
#include "Editor/UICommon.h"
#include <Common/FileUtil.h>
#include <Common/Log.h>

#include <Core/NCoreTests.h>
#include <Core/Resource/Script/NGameList.h>

#include <QApplication>
#include <QCoreApplication>
#include <QIcon>
#include <QStyleFactory>
#include <QtGlobal>

#include <string>

#ifdef Q_OS_MACOS
#include "Editor/MacOSExtras.h"
#endif

#ifdef Q_OS_WIN
#include <windows.h>
#endif

// Redirect qDebug output to the log file
static void QtLogRedirect(QtMsgType Type, const QMessageLogContext& /*rkContext*/, const QString& rkMessage)
{
    switch (Type)
    {
    case QtDebugMsg:
        NLog::Debug("Qt Debug: {}", rkMessage.toStdString());
        break;
    case QtWarningMsg:
        NLog::Warn("Qt Warning: {}", rkMessage.toStdString());
        break;
    case QtCriticalMsg:
        NLog::Error("Qt Critical: {}", rkMessage.toStdString());
        break;
    case QtFatalMsg:
        NLog::Fatal("Qt Fatal: {}", rkMessage.toStdString());
        break;
    case QtInfoMsg:
        NLog::Debug("Qt Info: {}", rkMessage.toStdString());
        break;
    }
}

static TString LocateDataDirectory()
{
#if !defined(Q_OS_WIN) && !defined(Q_OS_MACOS)
#ifdef PWE_DATADIR
    {
        /* This is for build-configured root */
        TString dir = FileUtil::MakeAbsolute(PWE_DATADIR);
        NLog::Debug("Checking '{}' for resources", dir);
        if (FileUtil::IsDirectory(dir + "resources"))
            return dir;
    }
#endif
    {
        /* This is for locating appimage root */
        TString dir = FileUtil::MakeAbsolute(TString(QCoreApplication::applicationDirPath().toStdString()) + "/../share/PrimeWorldEditor");
        NLog::Debug("Checking '{}' for resources", dir);
        if (FileUtil::IsDirectory(dir + "resources"))
            return dir;
    }
#endif
#ifdef Q_OS_MACOS
    {
        /* This is for locating mac bundle root */
        TString dir = FileUtil::MakeAbsolute(TString(QCoreApplication::applicationDirPath().toStdString()) + "/../Resources");
        NLog::Debug("Checking '{}' for resources", dir);
        if (FileUtil::IsDirectory(dir + "resources"))
            return dir;
    }
#endif
    {
        /* This is for locating build directory root */
        TString dir = FileUtil::MakeAbsolute(TString(QCoreApplication::applicationDirPath().toStdString()) + "/..");
        NLog::Debug("Checking '{}' for resources", dir);
        if (FileUtil::IsDirectory(dir + "resources"))
            return dir;
    }
    TString dir = FileUtil::MakeAbsolute("..");
    NLog::Warn("Falling back to '{}' for resources", dir);
    return dir;
}

static std::string LocateLogPath()
{
#ifdef Q_OS_WIN
    return "primeworldeditor.log";
#else
    return std::string(getenv("HOME")) + "/.primeworldeditor.log";
#endif
}

static void SetUpLogging()
{
#ifdef Q_OS_WIN
    // Due to Win32 shenanigans, we need to attach to a parent process
    // if one exists. Otherwise, debug logs when launching the application via
    // the command line won't be received by spdlog.
    ::AttachConsole(ATTACH_PARENT_PROCESS);
#endif

    const bool Initialized = NLog::InitLog(LocateLogPath());
    if (!Initialized)
        UICommon::ErrorMsg(nullptr, QCoreApplication::translate("Main", "Couldn't open log file. Logging will not work for this session."));

    qInstallMessageHandler(QtLogRedirect);
}

class CMain
{
public:
    /** Main function */
    int Main(int argc, char *argv[])
    {
        // Default OpenGL format
        QSurfaceFormat glFormat;
        glFormat.setVersion(3, 3);
        glFormat.setProfile(QSurfaceFormat::CoreProfile);
        QSurfaceFormat::setDefaultFormat(glFormat);

        // Create application
        QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
        CEditorApplication App(argc, argv);

        // Set application metadata
        QCoreApplication::setApplicationName(QStringLiteral("PrimeWorldEditor"));
        QCoreApplication::setApplicationVersion(QStringLiteral(APP_VERSION));
        QCoreApplication::setOrganizationName(QStringLiteral("AxioDL"));
        QGuiApplication::setWindowIcon(QIcon(QStringLiteral(":/icons/win/AppIcon.ico")));

        // Create UI relay
        CUIRelay UIRelay(&App);
        SetUIRelay(&UIRelay);

        // Set up dark theme
        QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
        QApplication::setPalette(MakePalette());
#ifdef Q_OS_MACOS
        MacOSSetDarkAppearance();
        MouseDragCocoaEventFilter mouseDragCocoaEventFilter;
        gpMouseDragCocoaEventFilter = &mouseDragCocoaEventFilter;
        App.installNativeEventFilter(gpMouseDragCocoaEventFilter);
#endif

        // Init log
        SetUpLogging();

        // Locate data directory and check write permissions
        gDataDir = LocateDataDirectory();
        gResourcesWritable = FileUtil::IsDirectoryWritable(gDataDir + "resources");
        gTemplatesWritable = FileUtil::IsDirectoryWritable(gDataDir + "templates");

        // Create editor resource store
        gpEditorStore = new CResourceStore(gDataDir + "resources/");

        if (!gpEditorStore->DatabasePathExists())
        {
            UICommon::ErrorMsg(nullptr, QCoreApplication::translate("Main", "Unable to locate PWE resources directory; "
                                                                    "PWE's executable must remain as deployed."));
            return 1;
        }

        if (!gpEditorStore->AreAllEntriesValid())
        {
            NLog::Debug("Editor store has invalid entries. Rebuilding database...");
            gpEditorStore->RebuildFromDirectory();
            gpEditorStore->ConditionalSaveStore();
        }

        // Check for unit tests being run
        if (NCoreTests::RunTests(argc, argv))
        {
            return 0;
        }

        // Execute application
        App.InitEditor();
        return QCoreApplication::exec();
    }

    /** Clean up any resources at the end of application execution */
    ~CMain()
    {
        NGameList::Shutdown();
    }

    static QPalette MakePalette()
    {
        QPalette DarkPalette;
        ConfigureColorGroup(DarkPalette, QPalette::Active, 100);
        ConfigureColorGroup(DarkPalette, QPalette::Inactive, 100);
        ConfigureColorGroup(DarkPalette, QPalette::Disabled, 200);
        return DarkPalette;
    }

    static void ConfigureColorGroup(QPalette& Palette, QPalette::ColorGroup Group, int Factor)
    {
        Palette.setColor(Group, QPalette::Window,           QColor(53,53,53)  .darker(Factor));
        Palette.setColor(Group, QPalette::WindowText,       QColor(Qt::white) .darker(Factor));
        Palette.setColor(Group, QPalette::Base,             QColor(25,25,25)  .darker(Factor));
        Palette.setColor(Group, QPalette::AlternateBase,    QColor(35,35,35)  .darker(Factor));
        Palette.setColor(Group, QPalette::ToolTipBase,      QColor(Qt::black) .darker(Factor));
        Palette.setColor(Group, QPalette::ToolTipText,      QColor(Qt::white) .darker(Factor));
        Palette.setColor(Group, QPalette::Text,             QColor(Qt::white) .darker(Factor));
        Palette.setColor(Group, QPalette::Button,           QColor(53,53,53)  .darker(Factor));
        Palette.setColor(Group, QPalette::ButtonText,       QColor(Qt::white) .darker(Factor));
        Palette.setColor(Group, QPalette::BrightText,       QColor(Qt::red)   .darker(Factor));
        Palette.setColor(Group, QPalette::Link,             QColor(42,130,218).darker(Factor));
        Palette.setColor(Group, QPalette::Highlight,        QColor(42,130,218).darker(Factor));
        Palette.setColor(Group, QPalette::HighlightedText,  QColor(Qt::white) .darker(Factor));
    }
};

int main(int argc, char *argv[])
{
    CMain Main;
    return Main.Main(argc, argv);
}
