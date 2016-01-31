#include "TestDialog.h"
#include "ui_TestDialog.h"
#include "Editor/PropertyEdit/CPropertyDelegate.h"
#include "Editor/Widgets/WResourceSelector.h"
#include "Editor/Widgets/WTextureGLWidget.h"
#include <Core/Resource/Factory/CTextureDecoder.h>
#include <Core/Resource/CResCache.h>
#include <Core/Resource/Script/CMasterTemplate.h>
#include <Core/Resource/Script/CScriptTemplate.h>
#include <Core/Resource/Factory/CTemplateLoader.h>

#include <iostream>

TestDialog::TestDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TestDialog)
{
    ui->setupUi(this);

    /*CTemplateLoader::LoadGameTemplates(eCorruption);
    CMasterTemplate *pMaster = CMasterTemplate::GetMasterForGame(eCorruption);
    CScriptTemplate *pTemp = pMaster->TemplateByID("PCKP");

    CPropertyStruct *pBase = static_cast<CPropertyStruct*>(pTemp->BaseStruct()->InstantiateProperty(nullptr));
    ui->treeView->setItemDelegate(new CPropertyDelegate(ui->treeView));
    ui->treeView->SetObject(pBase);*/
}

TestDialog::~TestDialog()
{
    delete ui;
}
