#include "CStartWindow.h"
#include "ui_CStartWindow.h"

#include <QFileDialog>
#include <QMessageBox>

#include "CModelEditorWindow.h"
#include "CWorldEditor.h"
#include "UICommon.h"
#include <Core/CResCache.h>

CStartWindow::CStartWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CStartWindow)
{
    ui->setupUi(this);
    mpWorld = nullptr;
    mpWorldEditor = new CWorldEditor(0);
    mpModelEditor = new CModelEditorWindow(this);
}

CStartWindow::~CStartWindow()
{
    delete ui;
    delete mpWorldEditor;
    delete mpModelEditor;
}

void CStartWindow::on_actionOpen_MLVL_triggered()
{
    QString WorldFile = QFileDialog::getOpenFileName(this, "Open MLVL", "", "Metroid Prime World (*.MLVL)");
    if (WorldFile.isEmpty()) return;

    gResCache.SetFolder(TString(WorldFile.toStdString()).GetFileDirectory());
    mpWorld = gResCache.GetResource(WorldFile.toStdString());
    mpWorldEditor->close();

    FillWorldUI();
}

void CStartWindow::FillWorldUI()
{

    CStringTable *pWorldName = mpWorld->GetWorldName();
    if (pWorldName)
    {
        TWideString WorldName = pWorldName->GetString("ENGL", 0);
        ui->WorldNameLabel->setText( QString("<font size=5><b>") + TO_QSTRING(WorldName) + QString("</b></font>") );
        ui->WorldNameSTRGLineEdit->setText(TO_QSTRING(pWorldName->Source()));
    }
    else
    {
        ui->WorldNameLabel->clear();
        ui->WorldNameSTRGLineEdit->clear();
    }

    CStringTable *pDarkWorldName = mpWorld->GetDarkWorldName();
    if (pDarkWorldName)
        ui->DarkWorldNameSTRGLineEdit->setText(TO_QSTRING(pDarkWorldName->Source()));
    else
        ui->DarkWorldNameSTRGLineEdit->clear();

    CModel *pDefaultSkybox = mpWorld->GetDefaultSkybox();
    if (pDefaultSkybox)
        ui->DefaultSkyboxCMDLLineEdit->setText(TO_QSTRING(pDefaultSkybox->Source()));
    else
        ui->DefaultSkyboxCMDLLineEdit->clear();

    CResource *pSaveWorld = mpWorld->GetSaveWorld();
    if (pSaveWorld)
        ui->WorldSAVWLineEdit->setText(TO_QSTRING(pSaveWorld->Source()));
    else
        ui->WorldSAVWLineEdit->clear();

    CResource *pMapWorld = mpWorld->GetMapWorld();
    if (pMapWorld)
        ui->WorldMAPWLineEdit->setText(TO_QSTRING(pMapWorld->Source()));
    else
        ui->WorldMAPWLineEdit->clear();

    u32 NumAreas = mpWorld->GetNumAreas();
    ui->AreaSelectComboBox->blockSignals(true);
    ui->AreaSelectComboBox->clear();
    ui->AreaSelectComboBox->blockSignals(false);
    ui->AreaSelectComboBox->setDisabled(false);
    for (u32 iArea = 0; iArea < NumAreas; iArea++)
    {
        CStringTable *pAreaName = mpWorld->GetAreaName(iArea);
        QString AreaName = (pAreaName != nullptr) ? TO_QSTRING(pAreaName->GetString("ENGL", 0)) : QString("!!") + TO_QSTRING(mpWorld->GetAreaInternalName(iArea));
        ui->AreaSelectComboBox->addItem(AreaName);
    }
}

void CStartWindow::FillAreaUI()
{
    ui->AreaNameLineEdit->setDisabled(false);
    ui->AreaNameSTRGLineEdit->setDisabled(false);
    ui->AreaMREALineEdit->setDisabled(false);
    ui->AttachedAreasList->setDisabled(false);

    ui->AreaSelectComboBox->blockSignals(true);
    ui->AreaSelectComboBox->setCurrentIndex(mSelectedAreaIndex);
    ui->AreaSelectComboBox->blockSignals(false);

    ui->AreaNameLineEdit->setText(TO_QSTRING(mpWorld->GetAreaInternalName(mSelectedAreaIndex)));

    CStringTable *pAreaName = mpWorld->GetAreaName(mSelectedAreaIndex);
    if (pAreaName)
        ui->AreaNameSTRGLineEdit->setText(TO_QSTRING(pAreaName->Source()));
    else
        ui->AreaNameSTRGLineEdit->clear();

    u64 MREA = mpWorld->GetAreaResourceID(mSelectedAreaIndex);
    TString MREAStr;
    if (MREA & 0xFFFFFFFF00000000)
        MREAStr = TString::FromInt64(MREA, 16);
    else
        MREAStr = TString::FromInt32(MREA, 8);

    ui->AreaMREALineEdit->setText(TO_QSTRING(MREAStr) + QString(".MREA"));

    u32 NumAttachedAreas = mpWorld->GetAreaAttachedCount(mSelectedAreaIndex);
    ui->AttachedAreasList->clear();

    for (u32 iArea = 0; iArea < NumAttachedAreas; iArea++)
    {
        u32 AttachedAreaIndex = mpWorld->GetAreaAttachedID(mSelectedAreaIndex, iArea);

        CStringTable *AttachedAreaSTRG = mpWorld->GetAreaName(AttachedAreaIndex);
        QString AttachedStr;

        if (AttachedAreaSTRG)
            AttachedStr = TO_QSTRING(AttachedAreaSTRG->GetString("ENGL", 0));
        else
            AttachedStr = QString("!!") + TO_QSTRING(mpWorld->GetAreaInternalName(AttachedAreaIndex));

        ui->AttachedAreasList->addItem(AttachedStr);
    }

    ui->LaunchWorldEditorButton->setDisabled(false);
}

void CStartWindow::on_AreaSelectComboBox_currentIndexChanged(int index)
{
    mSelectedAreaIndex = index;
    FillAreaUI();
}

void CStartWindow::on_AttachedAreasList_doubleClicked(const QModelIndex &index)
{
    mSelectedAreaIndex = mpWorld->GetAreaAttachedID(mSelectedAreaIndex, index.row());
    FillAreaUI();
}

void CStartWindow::on_LaunchWorldEditorButton_clicked()
{
    u64 AreaID = mpWorld->GetAreaResourceID(mSelectedAreaIndex);
    TResPtr<CGameArea> pArea = gResCache.GetResource(AreaID, "MREA");

    if (!pArea)
    {
        QMessageBox::warning(this, "Error", "Couldn't load area!");
        mpWorldEditor->close();
    }

    else
    {
        mpWorld->SetAreaLayerInfo(pArea, mSelectedAreaIndex);
        mpWorldEditor->SetArea(mpWorld, pArea);
        mpWorldEditor->setWindowModality(Qt::WindowModal);
        mpWorldEditor->showMaximized();
    }

    gResCache.Clean();
}

void CStartWindow::on_actionLaunch_model_viewer_triggered()
{
    mpModelEditor->setWindowModality(Qt::ApplicationModal);
    mpModelEditor->show();
}
