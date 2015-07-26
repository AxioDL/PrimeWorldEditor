#include "CStartWindow.h"
#include "ui_CStartWindow.h"

#include <QFileDialog>
#include <QMessageBox>

#include "CModelEditorWindow.h"
#include "CWorldEditor.h"
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

    gResCache.SetFolder(StringUtil::GetFileDirectory(WorldFile.toStdString()));
    mpWorld = (CWorld*) gResCache.GetResource(WorldFile.toStdString());
    mWorldToken = CToken(mpWorld);
    mpWorldEditor->close();

    FillWorldUI();
}

void CStartWindow::FillWorldUI()
{

    CStringTable *pWorldName = mpWorld->GetWorldName();
    if (pWorldName)
    {
        std::wstring WorldName = pWorldName->GetString("ENGL", 0);
        ui->WorldNameLabel->setText( QString("<font size=5><b>") + QString::fromStdWString(WorldName) + QString("</b></font>") );
        ui->WorldNameSTRGLineEdit->setText( QString::fromStdString(pWorldName->Source()) );
    }
    else
    {
        ui->WorldNameLabel->clear();
        ui->WorldNameSTRGLineEdit->clear();
    }

    CStringTable *pDarkWorldName = mpWorld->GetDarkWorldName();
    if (pDarkWorldName)
        ui->DarkWorldNameSTRGLineEdit->setText( QString::fromStdString(pDarkWorldName->Source()) );
    else
        ui->DarkWorldNameSTRGLineEdit->clear();

    CModel *pDefaultSkybox = mpWorld->GetDefaultSkybox();
    if (pDefaultSkybox)
        ui->DefaultSkyboxCMDLLineEdit->setText( QString::fromStdString(pDefaultSkybox->Source()) );
    else
        ui->DefaultSkyboxCMDLLineEdit->clear();

    CResource *pSaveWorld = mpWorld->GetSaveWorld();
    if (pSaveWorld)
        ui->WorldSAVWLineEdit->setText( QString::fromStdString(pSaveWorld->Source()) );
    else
        ui->WorldSAVWLineEdit->clear();

    CResource *pMapWorld = mpWorld->GetMapWorld();
    if (pMapWorld)
        ui->WorldMAPWLineEdit->setText( QString::fromStdString(pMapWorld->Source()) );
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
        QString AreaName = (pAreaName != nullptr) ? QString::fromStdWString(pAreaName->GetString("ENGL", 0)) : QString("!!") + QString::fromStdString(mpWorld->GetAreaInternalName(iArea));
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

    ui->AreaNameLineEdit->setText( QString::fromStdString(mpWorld->GetAreaInternalName(mSelectedAreaIndex)));

    CStringTable *pAreaName = mpWorld->GetAreaName(mSelectedAreaIndex);
    if (pAreaName)
        ui->AreaNameSTRGLineEdit->setText( QString::fromStdString( pAreaName->Source() ));
    else
        ui->AreaNameSTRGLineEdit->clear();

    u64 MREA = mpWorld->GetAreaResourceID(mSelectedAreaIndex);
    std::string MREAStr;
    if (MREA & 0xFFFFFFFF00000000)
        MREAStr = StringUtil::ResToStr(MREA);
    else
        MREAStr = StringUtil::ResToStr( (u32) MREA );

    ui->AreaMREALineEdit->setText(QString::fromStdString(MREAStr) + QString(".MREA") );

    u32 NumAttachedAreas = mpWorld->GetAreaAttachedCount(mSelectedAreaIndex);
    ui->AttachedAreasList->clear();

    for (u32 iArea = 0; iArea < NumAttachedAreas; iArea++)
    {
        u32 AttachedAreaIndex = mpWorld->GetAreaAttachedID(mSelectedAreaIndex, iArea);

        CStringTable *AttachedAreaSTRG = mpWorld->GetAreaName(AttachedAreaIndex);
        QString AttachedStr;

        if (AttachedAreaSTRG)
            AttachedStr = QString::fromStdWString(AttachedAreaSTRG->GetString("ENGL", 0) );
        else
            AttachedStr = QString("!!") + QString::fromStdString(mpWorld->GetAreaInternalName(AttachedAreaIndex));

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
    CGameArea *pArea = (CGameArea*) gResCache.GetResource(AreaID, "MREA");

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
