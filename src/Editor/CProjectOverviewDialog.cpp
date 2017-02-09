#include "CProjectOverviewDialog.h"
#include "ui_CProjectOverviewDialog.h"
#include "CEditorApplication.h"
#include "CExportGameDialog.h"
#include "UICommon.h"
#include "Editor/ResourceBrowser/CResourceBrowser.h"
#include <Common/AssertMacro.h>
#include <Core/GameProject/CGameExporter.h>
#include <QFileDialog>
#include <QMessageBox>

CProjectOverviewDialog::CProjectOverviewDialog(QWidget *pParent)
    : QDialog(pParent)
    , mpUI(new Ui::CProjectOverviewDialog)
    , mpProject(nullptr)
{
    mpUI->setupUi(this);

    connect(mpUI->OpenProjectButton, SIGNAL(clicked()), this, SLOT(OpenProject()));
    connect(mpUI->ExportGameButton, SIGNAL(clicked()), this, SLOT(ExportGame()));
    connect(mpUI->LoadWorldButton, SIGNAL(clicked()), this, SLOT(LoadWorld()));
    connect(mpUI->LaunchEditorButton, SIGNAL(clicked()), this, SLOT(LaunchEditor()));
    connect(mpUI->ViewResourcesButton, SIGNAL(clicked()), this, SLOT(LaunchResourceBrowser()));
    connect(mpUI->CookPackageButton, SIGNAL(clicked()), this, SLOT(CookPackage()));
    connect(mpUI->CookAllDirtyPackagesButton, SIGNAL(clicked(bool)), this, SLOT(CookAllDirtyPackages()));

    connect(gpEdApp, SIGNAL(AssetsModified()), this, SLOT(SetupPackagesList()));
}

CProjectOverviewDialog::~CProjectOverviewDialog()
{
    delete mpUI;
}

void CProjectOverviewDialog::InternalLoadProject(const QString& rkPath)
{
    // Load project
    TWideString Path = TO_TWIDESTRING(rkPath);
    CGameProject *pNewProj = CGameProject::LoadProject(Path);

    if (pNewProj)
    {
        if (mpProject) delete mpProject;
        mpProject = pNewProj;
        mpProject->SetActive();
        SetupWorldsList();
        SetupPackagesList();
        emit ActiveProjectChanged(mpProject);
    }

    else
        Log::Error("Failed to load project");
}

void CProjectOverviewDialog::OpenProject()
{
    // Open project file
    QString ProjPath = UICommon::OpenFileDialog(this, "Open Project", "Game Project (*.prj)");
    if (!ProjPath.isEmpty()) InternalLoadProject(ProjPath);
}

void CProjectOverviewDialog::ExportGame()
{
    QString IsoPath = UICommon::OpenFileDialog(this, "Select ISO", "*.iso *.gcm *.tgc *.wbfs");
    if (IsoPath.isEmpty()) return;

    QString ExportDir = UICommon::OpenDirDialog(this, "Select output export directory");
    if (ExportDir.isEmpty()) return;

    CExportGameDialog ExportDialog(IsoPath, ExportDir, this);
    if (ExportDialog.HasValidDisc()) ExportDialog.exec();

    if (ExportDialog.ExportSucceeded())
    {
        int OpenChoice = QMessageBox::information(this, "Export complete", "Export finished successfully! Open new project?", QMessageBox::Yes, QMessageBox::No);

        if (OpenChoice == QMessageBox::Yes)
            InternalLoadProject(ExportDialog.ProjectPath());
    }
}

void CProjectOverviewDialog::SetupWorldsList()
{
    ASSERT(mpProject != nullptr && mpProject->IsActive());

    std::list<CAssetID> WorldIDs;
    mpProject->GetWorldList(WorldIDs);
    mWorldEntries.clear();
    mpUI->WorldsList->clear();

    for (auto It = WorldIDs.begin(); It != WorldIDs.end(); It++)
    {
        CAssetID ID = *It;
        CResourceEntry *pEntry = gpResourceStore->FindEntry(ID);

        if (!pEntry)
        {
            Log::Error("Couldn't find entry for world: " + ID.ToString());
            continue;
        }

        mWorldEntries << pEntry;
        mpUI->WorldsList->addItem(TO_QSTRING(pEntry->Name()));
    }

    mpUI->AreasGroupBox->setEnabled(false);
    mpUI->LoadWorldButton->setEnabled(!mWorldEntries.isEmpty());
}

void CProjectOverviewDialog::SetupPackagesList()
{
    ASSERT(mpProject != nullptr && mpProject->IsActive());
    mpUI->PackagesList->clear();

    for (u32 iPkg = 0; iPkg < mpProject->NumPackages(); iPkg++)
    {
        CPackage *pPackage = mpProject->PackageByIndex(iPkg);
        ASSERT(pPackage != nullptr);

        QString PackageName = TO_QSTRING(pPackage->Name());
        if (pPackage->NeedsRecook()) PackageName += '*';
        mpUI->PackagesList->addItem(PackageName);
    }
}

void CProjectOverviewDialog::LoadWorld()
{
    // Find world
    u32 WorldIdx = mpUI->WorldsList->currentRow();
    CResourceEntry *pWorldEntry = mWorldEntries[WorldIdx];
    mpWorld = pWorldEntry->Load();

    mAreaEntries.clear();
    mpUI->AreaComboBox->clear();

    if (mpWorld)
    {
        for (u32 iArea = 0; iArea < mpWorld->NumAreas(); iArea++)
        {
            CResourceEntry *pAreaEntry = gpResourceStore->FindEntry( mpWorld->AreaResourceID(iArea) );

            if (pAreaEntry)
            {
                mAreaEntries << pAreaEntry;
                mpUI->AreaComboBox->addItem(TO_QSTRING(pAreaEntry->Name()));
            }
        }
    }

    mpUI->AreasGroupBox->setEnabled(true);
    mpUI->AreaComboBox->setEnabled(!mAreaEntries.isEmpty());
    mpUI->LaunchEditorButton->setEnabled(!mAreaEntries.isEmpty());
    gpResourceStore->DestroyUnreferencedResources();
}

void CProjectOverviewDialog::LaunchEditor()
{
    // Load area
    u32 AreaIdx = mpUI->AreaComboBox->currentIndex();
    CResourceEntry *pAreaEntry = mAreaEntries[AreaIdx];
    CGameArea *pArea = (CGameArea*) pAreaEntry->Load();

    if (pArea)
    {
        pArea->SetWorldIndex(AreaIdx);
        mpWorld->SetAreaLayerInfo(pArea);
        gpEdApp->WorldEditor()->SetArea(mpWorld, pArea);
        gpEdApp->WorldEditor()->showMaximized();
    }

    else
        Log::Error("Failed to load area");

    gpResourceStore->DestroyUnreferencedResources();
}

void CProjectOverviewDialog::LaunchResourceBrowser()
{
    gpEdApp->ResourceBrowser()->show();
    gpEdApp->ResourceBrowser()->raise();
}

void CProjectOverviewDialog::CookPackage()
{
    u32 PackageIdx = mpUI->PackagesList->currentRow();
    CPackage *pPackage = mpProject->PackageByIndex(PackageIdx);
    pPackage->Cook();
    SetupPackagesList();
}

void CProjectOverviewDialog::CookAllDirtyPackages()
{
    gpEdApp->CookAllDirtyPackages();
    SetupPackagesList();
}
