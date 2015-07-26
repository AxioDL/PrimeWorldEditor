#include <QApplication>
#include <UI/CStartWindow.h>
#include <UI/CWorldEditorWindow.h>
#include <UI/CModelEditorWindow.h>
#include <UI/TestDialog.h>
#include <Common/CTimer.h>
#include <iostream>
#include <QStyleFactory>
#include <UI/CDarkStyle.h>
#include <time.h>
#include <Resource/factory/CTextureDecoder.h>
#include <Resource/cooker/CTextureEncoder.h>
#include <Common/CMatrix4f.h>
#include <Resource/factory/CTemplateLoader.h>
#include <Common/StringUtil.h>

#include <QDir>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication app(argc, argv);
    CStartWindow w;
    w.show();

    CTemplateLoader::LoadGameList();
    //CTexture *pTex = CTextureDecoder::LoadDDS(CFileInStream("E:/34f7c12211777ce8.dds", IOUtil::LittleEndian));
    //CTextureEncoder::EncodeTXTR(CFileOutStream("E:/Unpacked/Metroid Prime 3 Dolphin/Metroid4-pak/34f7c12211777ce8.TXTR", IOUtil::BigEndian), pTex);

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

    return app.exec();
}
