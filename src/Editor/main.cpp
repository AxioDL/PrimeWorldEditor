#include "CStartWindow.h"
#include <Common/Log.h>
#include <Core/Resource/Factory/CTemplateLoader.h>

#include <QApplication>
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

int main(int argc, char *argv[])
{
    // Create application
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication App(argc, argv);
    App.setWindowIcon(QIcon(":/icons/AppIcon.ico"));

    // Init log
    bool Initialized = Log::InitLog("primeworldeditor.log");
    if (!Initialized) QMessageBox::warning(0, "Error", "Couldn't open log file. Logging will not work for this session.");
    qInstallMessageHandler(QtLogRedirect);

    // Load templates
    CTemplateLoader::LoadGameList();

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

    // Execute application
    CStartWindow StartWindow;
    StartWindow.show();

    return App.exec();
}
