#include "CProjectOverviewDialog.h"
#include "ui_CProjectOverviewDialog.h"
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
    mpWorldEditor = new CWorldEditor();

    connect(mpUI->OpenProjectButton, SIGNAL(clicked()), this, SLOT(OpenProject()));
    connect(mpUI->ExportGameButton, SIGNAL(clicked()), this, SLOT(ExportGame()));
    connect(mpUI->LoadWorldButton, SIGNAL(clicked()), this, SLOT(LoadWorld()));
    connect(mpUI->LaunchEditorButton, SIGNAL(clicked()), this, SLOT(LaunchEditor()));
    connect(mpUI->ViewResourcesButton, SIGNAL(clicked()), this, SLOT(LaunchResourceBrowser()));
    connect(mpUI->CookPackageButton, SIGNAL(clicked()), this, SLOT(CookPackage()));
}

CProjectOverviewDialog::~CProjectOverviewDialog()
{
    delete mpUI;
    delete mpWorldEditor;
}

void CProjectOverviewDialog::OpenProject()
{
    // Open project file
    QString ProjPath = QFileDialog::getOpenFileName(this, "Open Project", "", "Game Project (*.prj)");
    if (ProjPath.isEmpty()) return;

    // Load project
    TWideString Path = TO_TWIDESTRING(ProjPath);
    CGameProject *pNewProj = new CGameProject(Path.GetFileDirectory());

    if (pNewProj->Load(Path))
    {
        if (mpProject) delete mpProject;
        mpProject = pNewProj;
        mpProject->SetActive();
        SetupWorldsList();
        SetupPackagesList();
    }

    else
    {
        Log::Error("Failed to load project");
        delete pNewProj;
    }
}

void CProjectOverviewDialog::ExportGame()
{
    // TEMP - hardcoded names for convenience. will remove later!
#define USE_HARDCODED_GAME_ROOT 0
#define USE_HARDCODED_EXPORT_DIR 1

#if USE_HARDCODED_GAME_ROOT
    QString GameRoot = "E:/Unpacked/Metroid Prime";
#else
    QString GameRoot = QFileDialog::getExistingDirectory(this, "Select game root directory");
    if (GameRoot.isEmpty()) return;
#endif

#if USE_HARDCODED_EXPORT_DIR
    QString ExportDir = "E:/Unpacked/ExportTest";
#else
    QString ExportDir = QFileDialog::getExistingDirectory(this, "Select output export directory");
    if (ExportDir.isEmpty()) return;
#endif

    // Verify valid game root by checking if opening.bnr exists
    TString OpeningBNR = TO_TSTRING(GameRoot) + "/opening.bnr";
    if (!FileUtil::Exists(OpeningBNR.ToUTF16()))
    {
        QMessageBox::warning(this, "Error", "Error; this is not a valid game root directory!");
        return;
    }

    CGameExporter Exporter(TO_TSTRING(GameRoot), TO_TSTRING(ExportDir));
    Exporter.Export();
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
        mpUI->PackagesList->addItem(TO_QSTRING(pPackage->Name()));
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
        mpWorldEditor->SetArea(mpWorld, pArea);
        mpWorldEditor->showMaximized();
    }

    else
        Log::Error("Failed to load area");

    gpResourceStore->DestroyUnreferencedResources();
}

void CProjectOverviewDialog::LaunchResourceBrowser()
{
    CResourceBrowser *pBrowser = new CResourceBrowser(mpWorldEditor);
    pBrowser->show();
}

void CProjectOverviewDialog::CookPackage()
{
    u32 PackageIdx = mpUI->PackagesList->currentRow();
    CPackage *pPackage = mpProject->PackageByIndex(PackageIdx);
    pPackage->Cook();
}
