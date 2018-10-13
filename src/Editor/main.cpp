#include "CEditorApplication.h"
#include "CUIRelay.h"
#include "UICommon.h"
#include <Common/Log.h>

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
    case QtDebugMsg:    Log::Write(TString("Qt Debug: ") + TO_TSTRING(rkMessage)); break;
    case QtWarningMsg:  Log::Write(TString("Qt Warning: ") + TO_TSTRING(rkMessage)); break;
    case QtCriticalMsg: Log::Write(TString("Qt Critical: ") + TO_TSTRING(rkMessage)); break;
    case QtFatalMsg:    Log::Write(TString("Qt Fatal: ") + TO_TSTRING(rkMessage)); break;
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
        QPalette DarkPalette;
        DarkPalette.setColor(QPalette::Window, QColor(53,53,53));
        DarkPalette.setColor(QPalette::WindowText, Qt::white);
        DarkPalette.setColor(QPalette::Base, QColor(25,25,25));
        DarkPalette.setColor(QPalette::AlternateBase, QColor(35,35,35));
        DarkPalette.setColor(QPalette::ToolTipBase, Qt::white);
        DarkPalette.setColor(QPalette::ToolTipText, Qt::white);
        DarkPalette.setColor(QPalette::Text, Qt::white);
        DarkPalette.setColor(QPalette::Button, QColor(53,53,53));
        DarkPalette.setColor(QPalette::ButtonText, Qt::white);
        DarkPalette.setColor(QPalette::BrightText, Qt::red);
        DarkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
        DarkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        DarkPalette.setColor(QPalette::HighlightedText, Qt::white);
        qApp->setPalette(DarkPalette);

        // Init log
        bool Initialized = Log::InitLog("primeworldeditor.log");
        if (!Initialized) QMessageBox::warning(0, "Error", "Couldn't open log file. Logging will not work for this session.");
        qInstallMessageHandler(QtLogRedirect);

        // Create editor resource store
        gpEditorStore = new CResourceStore("../resources/");

        if (!gpEditorStore->AreAllEntriesValid())
        {
            Log::Write("Editor store has invalid entries. Rebuilding database...");
            gpEditorStore->RebuildFromDirectory();
            gpEditorStore->ConditionalSaveStore();
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
};

int main(int argc, char *argv[])
{
    CMain Main;
    return Main.Main(argc, argv);
}
