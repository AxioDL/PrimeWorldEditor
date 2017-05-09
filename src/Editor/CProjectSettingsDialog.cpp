#include "CProjectSettingsDialog.h"
#include "ui_CProjectSettingsDialog.h"
#include "CEditorApplication.h"
#include "CExportGameDialog.h"
#include "UICommon.h"
#include "Editor/ResourceBrowser/CResourceBrowser.h"
#include <Common/AssertMacro.h>
#include <Core/GameProject/CGameExporter.h>
#include <QFileDialog>
#include <QMessageBox>

CProjectSettingsDialog::CProjectSettingsDialog(QWidget *pParent)
    : QDialog(pParent)
    , mpUI(new Ui::CProjectSettingsDialog)
    , mpProject(nullptr)
{
    mpUI->setupUi(this);

    connect(mpUI->CookPackageButton, SIGNAL(clicked()), this, SLOT(CookPackage()));
    connect(mpUI->CookAllDirtyPackagesButton, SIGNAL(clicked(bool)), this, SLOT(CookAllDirtyPackages()));

    connect(gpEdApp, SIGNAL(ActiveProjectChanged(CGameProject*)), this, SLOT(ActiveProjectChanged(CGameProject*)));
    connect(gpEdApp, SIGNAL(AssetsModified()), this, SLOT(SetupPackagesList()));
}

CProjectSettingsDialog::~CProjectSettingsDialog()
{
    delete mpUI;
}

void CProjectSettingsDialog::ActiveProjectChanged(CGameProject *pProj)
{
    mpProject = pProj;

    if (mpProject)
    {
        // Set up project info
        mpUI->ProjectNameLineEdit->setText( TO_QSTRING(pProj->Name()) );
        mpUI->GameLineEdit->setText( TO_QSTRING(GetGameName(pProj->Game())) );
        mpUI->GameIdLineEdit->setText( TO_QSTRING(pProj->GameID()) );

        float BuildVer = pProj->BuildVersion();
        ERegion Region = pProj->Region();
        TString BuildName = pProj->GameInfo()->GetBuildName(BuildVer, Region);
        mpUI->BuildLineEdit->setText( QString("%1 (%2)").arg(BuildVer).arg( TO_QSTRING(BuildName) ) );
        mpUI->RegionLineEdit->setText( TO_QSTRING(GetRegionName(Region)) );
    }
    else
    {
        // Clear project info
        mpUI->ProjectNameLineEdit->clear();
        mpUI->GameLineEdit->clear();
        mpUI->GameIdLineEdit->clear();
        mpUI->BuildLineEdit->clear();
        mpUI->RegionLineEdit->clear();
        close();
    }

    SetupPackagesList();
}

void CProjectSettingsDialog::SetupPackagesList()
{
    mpUI->PackagesList->clear();
    if (!mpProject) return;

    for (u32 iPkg = 0; iPkg < mpProject->NumPackages(); iPkg++)
    {
        CPackage *pPackage = mpProject->PackageByIndex(iPkg);
        ASSERT(pPackage != nullptr);

        QString PackageName = TO_QSTRING(pPackage->Name());
        if (pPackage->NeedsRecook()) PackageName += '*';
        mpUI->PackagesList->addItem(PackageName);
    }
}

void CProjectSettingsDialog::CookPackage()
{
    u32 PackageIdx = mpUI->PackagesList->currentRow();

    if (PackageIdx != -1)
    {
        CPackage *pPackage = mpProject->PackageByIndex(PackageIdx);
        pPackage->Cook();
        SetupPackagesList();
    }
}

void CProjectSettingsDialog::CookAllDirtyPackages()
{
    gpEdApp->CookAllDirtyPackages();
    SetupPackagesList();
}
