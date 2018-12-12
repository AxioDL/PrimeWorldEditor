#include "TestDialog.h"
#include "ui_TestDialog.h"
#include "CEditorApplication.h"
#include <Core/GameProject/CGameProject.h>

TestDialog::TestDialog(QWidget *pParent)
    : QDialog(pParent)
    , ui(new Ui::TestDialog)
{
    ui->setupUi(this);
    connect(ui->spinBox, SIGNAL(valueChanged(int)), this, SLOT(OnSpinBoxChanged(int)));
    connect(ui->spinBox_2, SIGNAL(valueChanged(int)), this, SLOT(OnSpinBoxChanged(int)));
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(OnFind()));
}

TestDialog::~TestDialog()
{
    delete ui;
}

void TestDialog::OnSpinBoxChanged(int NewValue)
{
    if (sender() != ui->spinBox) ui->spinBox->setValue(NewValue);
    if (sender() != ui->spinBox_2) ui->spinBox_2->setValue(NewValue);
}

void TestDialog::OnFind()
{
    uint32 SoundID = ui->spinBox->value();
    CGameProject *pProj = gpEdApp->ActiveProject();

    if (pProj)
    {
        CAudioManager *pAudioMgr = pProj->AudioManager();
        pAudioMgr->LogSoundInfo(SoundID);
    }
}
