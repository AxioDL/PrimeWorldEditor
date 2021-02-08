#include "TestDialog.h"
#include "ui_TestDialog.h"
#include "CEditorApplication.h"
#include <Core/GameProject/CGameProject.h>

TestDialog::TestDialog(QWidget *pParent)
    : QDialog(pParent)
    , ui(std::make_unique<Ui::TestDialog>())
{
    ui->setupUi(this);
    connect(ui->spinBox, qOverload<int>(&QSpinBox::valueChanged), this, &TestDialog::OnSpinBoxChanged);
    connect(ui->spinBox_2, qOverload<int>(&QSpinBox::valueChanged), this, &TestDialog::OnSpinBoxChanged);
    connect(ui->pushButton, &QPushButton::clicked, this, &TestDialog::OnFind);
}

TestDialog::~TestDialog() = default;

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
