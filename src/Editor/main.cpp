#include "CEditorApplication.h"
#include "CUIRelay.h"
#include "UICommon.h"
#include <Common/Log.h>

#include <Core/NCoreTests.h>
#include <Core/Resource/Script/NGameList.h>

#include <QApplication>
#include <QIcon>
#include <QMessageBox>
#include <QStyleFactory>
#include <QtGlobal>

// Redirect qDebug output to the log file
void QtLogRedirect(QtMsgType Type, const QMessageLogContext& /*rkContext*/, const QString& rkMessage)
{
    switch (Type)
    {
    case QtDebugMsg:    debugf("Qt Debug: %s",    *TO_TSTRING(rkMessage)); break;
    case QtWarningMsg:  warnf ("Qt Warning: %s",  *TO_TSTRING(rkMessage)); break;
    case QtCriticalMsg: errorf("Qt Critical: %s", *TO_TSTRING(rkMessage)); break;
    case QtFatalMsg:    fatalf("Qt Fatal: %s",    *TO_TSTRING(rkMessage)); break;
    }
}

class CMain
{
public:
    /** Main function */
    int Main(int argc, char *argv[])
    {
        // Create application
        QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
        CEditorApplication App(argc, argv);
        App.setApplicationName( APP_NAME );
        App.setApplicationVersion( APP_VERSION );
        App.setOrganizationName("Aruki");
        App.setWindowIcon(QIcon(":/icons/AppIcon.ico"));

        // Create UI relay
        CUIRelay UIRelay(&App);
        gpUIRelay = &UIRelay;

        // Set up dark theme
        qApp->setStyle(QStyleFactory::create("Fusion"));
        SetupPalette();

        // Init log
        bool Initialized = NLog::InitLog("primeworldeditor.log");
        if (!Initialized) QMessageBox::warning(0, "Error", "Couldn't open log file. Logging will not work for this session.");
        qInstallMessageHandler(QtLogRedirect);

        // Create editor resource store
        gpEditorStore = new CResourceStore("../resources/");

        if (!gpEditorStore->AreAllEntriesValid())
        {
            debugf("Editor store has invalid entries. Rebuilding database...");
            gpEditorStore->RebuildFromDirectory();
            gpEditorStore->ConditionalSaveStore();
        }

        // Check for unit tests being run
        if ( NCoreTests::RunTests(argc, argv) )
        {
            return 0;
        }

        // Execute application
        App.InitEditor();
        return App.exec();
    }

    /** Clean up any resources at the end of application execution */
    ~CMain()
    {
        NGameList::Shutdown();
    }

    void SetupPalette()
    {
        QPalette DarkPalette;
        ConfigureColorGroup(DarkPalette, QPalette::Active, 100);
        ConfigureColorGroup(DarkPalette, QPalette::Inactive, 100);
        ConfigureColorGroup(DarkPalette, QPalette::Disabled, 200);
        qApp->setPalette(DarkPalette);
    }

    void ConfigureColorGroup(QPalette& Palette, QPalette::ColorGroup Group, int Factor)
    {
        Palette.setColor( Group, QPalette::Window,           QColor(53,53,53)  .darker(Factor) );
        Palette.setColor( Group, QPalette::WindowText,       QColor(Qt::white) .darker(Factor) );
        Palette.setColor( Group, QPalette::Base,             QColor(25,25,25)  .darker(Factor) );
        Palette.setColor( Group, QPalette::AlternateBase,    QColor(35,35,35)  .darker(Factor) );
        Palette.setColor( Group, QPalette::ToolTipBase,      QColor(Qt::white) .darker(Factor) );
        Palette.setColor( Group, QPalette::ToolTipText,      QColor(Qt::white) .darker(Factor) );
        Palette.setColor( Group, QPalette::Text,             QColor(Qt::white) .darker(Factor) );
        Palette.setColor( Group, QPalette::Button,           QColor(53,53,53)  .darker(Factor) );
        Palette.setColor( Group, QPalette::ButtonText,       QColor(Qt::white) .darker(Factor) );
        Palette.setColor( Group, QPalette::BrightText,       QColor(Qt::red)   .darker(Factor) );
        Palette.setColor( Group, QPalette::Link,             QColor(42,130,218).darker(Factor) );
        Palette.setColor( Group, QPalette::Highlight,        QColor(42,130,218).darker(Factor) );
        Palette.setColor( Group, QPalette::HighlightedText,  QColor(Qt::white) .darker(Factor) );
    }
};

int main(int argc, char *argv[])
{
    CMain Main;
    return Main.Main(argc, argv);
}
