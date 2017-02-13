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

    connect(mpUI->CookPackageButton, SIGNAL(clicked()), this, SLOT(CookPackage()));
    connect(mpUI->CookAllDirtyPackagesButton, SIGNAL(clicked(bool)), this, SLOT(CookAllDirtyPackages()));

    connect(gpEdApp, SIGNAL(ActiveProjectChanged(CGameProject*)), this, SLOT(ActiveProjectChanged(CGameProject*)));
    connect(gpEdApp, SIGNAL(AssetsModified()), this, SLOT(SetupPackagesList()));
}

CProjectOverviewDialog::~CProjectOverviewDialog()
{
    delete mpUI;
}

void CProjectOverviewDialog::ActiveProjectChanged(CGameProject *pProj)
{
    mpProject = pProj;
    SetupPackagesList();
}

void CProjectOverviewDialog::SetupPackagesList()
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
