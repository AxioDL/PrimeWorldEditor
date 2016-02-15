#include "CStartWindow.h"
#include "ui_CStartWindow.h"
#include "CErrorLogDialog.h"
#include "UICommon.h"

#include "Editor/ModelEditor/CModelEditorWindow.h"
#include "Editor/WorldEditor/CWorldEditor.h"
#include <Core/Resource/CResCache.h>

#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>

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

void CStartWindow::closeEvent(QCloseEvent *pEvent)
{
    if (mpWorldEditor->close())
        qApp->quit();
    else
        pEvent->ignore();
}

void CStartWindow::on_actionOpen_MLVL_triggered()
{
    QString WorldFile = QFileDialog::getOpenFileName(this, "Open MLVL", "", "Metroid Prime World (*.MLVL)");
    if (WorldFile.isEmpty()) return;

    if (mpWorldEditor->close())
    {
        gResCache.SetFolder(TString(WorldFile.toStdString()).GetFileDirectory());
        mpWorld = gResCache.GetResource(WorldFile.toStdString());
        FillWorldUI();
    }
}

void CStartWindow::FillWorldUI()
{
    CStringTable *pWorldName = mpWorld->GetWorldName();
    if (pWorldName)
    {
        TWideString WorldName = pWorldName->GetString("ENGL", 0);
        ui->WorldNameLabel->setText( QString("<font size=5><b>") + TO_QSTRING(WorldName) + QString("</b></font>") );
        ui->WorldNameSTRGLineEdit->setText(TO_QSTRING(pWorldName->Source()));

        // hack to get around lack of UTF16 -> UTF8 support
        TString WorldNameTStr = TO_TSTRING(QString::fromStdWString(WorldName.ToStdString()));
        Log::Write("Loaded " + mpWorld->Source() + " (" + WorldNameTStr + ")");
    }
    else
    {
        ui->WorldNameLabel->clear();
        ui->WorldNameSTRGLineEdit->clear();
        Log::Write("Loaded " + mpWorld->Source() + " (unnamed world)");
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
            AttachedStr = QString("!") + TO_QSTRING(mpWorld->GetAreaInternalName(AttachedAreaIndex));

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
    if (mpWorldEditor->CheckUnsavedChanges())
    {
        Log::ClearErrorLog();

        u64 AreaID = mpWorld->GetAreaResourceID(mSelectedAreaIndex);
        TResPtr<CGameArea> pArea = gResCache.GetResource(AreaID, "MREA");

        if (!pArea)
        {
            QMessageBox::warning(this, "Error", "Couldn't load area!");
        }

        else
        {
            mpWorld->SetAreaLayerInfo(pArea, mSelectedAreaIndex);
            mpWorldEditor->SetArea(mpWorld, pArea, mSelectedAreaIndex);
            mpWorldEditor->setWindowModality(Qt::WindowModal);
            mpWorldEditor->showMaximized();

            // Display errors
            CErrorLogDialog ErrorDialog(mpWorldEditor);
            bool HasErrors = ErrorDialog.GatherErrors();

            if (HasErrors)
                ErrorDialog.exec();

            gResCache.Clean();
        }
    }
}

void CStartWindow::on_actionLaunch_model_viewer_triggered()
{
    mpModelEditor->setWindowModality(Qt::ApplicationModal);
    mpModelEditor->show();
}

void CStartWindow::on_actionExtract_PAK_triggered()
{
    QString Pak = QFileDialog::getOpenFileName(this, "Select pak", "", "Package (*.pak)");

    if (!Pak.isEmpty())
        ExtractPackage(Pak, true);
}

void CStartWindow::ExtractPackage(const QString& rkPath, bool PopupOnComplete)
{
    // Not the ideal way to be handling this but it's the easiest and it'll be a good holdover until later
    QProcess PakTool(this);

    // Dump assets
    QStringList PakToolArgs;
    PakToolArgs << "-x" << rkPath;

    QDialog Dialog;
    QLayout *pLayout = new QVBoxLayout(&Dialog);
    pLayout->addWidget(new QLabel("Extracting...", &Dialog));
    Dialog.setWindowFlags(Qt::Window | Qt::WindowTitleHint);

    connect(&PakTool, SIGNAL(finished(int)), &Dialog, SLOT(done(int)));
    PakTool.start("PakTool.exe", PakToolArgs);
    Dialog.exec();
    int Result = PakTool.exitCode();

    // Dump list
    if (Result == 0)
    {
        PakToolArgs.clear();
        PakToolArgs << "-d" << rkPath;
        PakTool.start("PakTool.exe", PakToolArgs);
        PakTool.waitForFinished();
        Result = PakTool.exitCode();
    }

    // Results
    if (PopupOnComplete)
    {
        if (Result == 0)
            QMessageBox::information(this, "Success", "Extracted pak successfully!");
        else
            QMessageBox::warning(this, "Error", "Couldn't extract pak");
    }
}
