#include "TestDialog.h"
#include "ui_TestDialog.h"
#include <Core/CResCache.h>
#include <iostream>
#include "WResourceSelector.h"
#include "WTextureGLWidget.h"
#include <Resource/factory/CTextureDecoder.h>

TestDialog::TestDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TestDialog)
{
    ui->setupUi(this);

    CTexture *pTex = CTextureDecoder::LoadDDS(CFileInStream("E:/test2.dds", IOUtil::LittleEndian));
    ui->widget->SetTexture(pTex);
}

TestDialog::~TestDialog()
{
    delete ui;
}
