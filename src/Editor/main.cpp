#include "CStartWindow.h"
#include "CDarkStyle.h"
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

    std::cout << TO_TSTRING(rkMessage) << "\n";
}

int main(int argc, char *argv[])
{
    // Create application
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/icons/AppIcon.ico"));

    // Init log
    bool Initialized = Log::InitLog("primeworldeditor.log");
    if (!Initialized) QMessageBox::warning(0, "Error", "Couldn't open log file. Logging will not work for this session.");
    qInstallMessageHandler(QtLogRedirect);

    // Load templates
    CTemplateLoader::LoadGameList();

    // Set up dark style
    app.setStyle(new CDarkStyle);
    qApp->setStyle(QStyleFactory::create("Fusion"));

    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53,53,53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25,25,25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53,53,53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53,53,53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));

    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::white);

    qApp->setPalette(darkPalette);

    // Execute application
    CStartWindow w;
    w.show();

    return app.exec();
}
