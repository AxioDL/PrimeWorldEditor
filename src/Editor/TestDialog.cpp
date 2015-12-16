#include "TestDialog.h"
#include "ui_TestDialog.h"
#include "Editor/Widgets/WResourceSelector.h"
#include "Editor/Widgets/WTextureGLWidget.h"
#include <Core/Resource/Factory/CTextureDecoder.h>
#include <Core/Resource/CResCache.h>

#include <iostream>

TestDialog::TestDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TestDialog)
{
    ui->setupUi(this);

    CTexture *pTex = CTextureDecoder::LoadDDS(CFileInStream("E:/test2.dds", IOUtil::eLittleEndian));
    ui->widget->SetTexture(pTex);
}

TestDialog::~TestDialog()
{
    delete ui;
}
