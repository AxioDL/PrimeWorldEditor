#include "CProjectSettingsDialog.h"
#include "ui_CProjectSettingsDialog.h"
#include "CEditorApplication.h"
#include "CExportGameDialog.h"
#include "CProgressDialog.h"
#include "UICommon.h"
#include "Editor/ResourceBrowser/CResourceBrowser.h"
#include <Common/AssertMacro.h>
#include <Core/GameProject/CGameExporter.h>
#include <QFileDialog>
#include <QFuture>
#include <QFutureWatcher>
#include <QMessageBox>
#include <QtConcurrent/QtConcurrentRun>

CProjectSettingsDialog::CProjectSettingsDialog(QWidget *pParent)
    : QDialog(pParent)
    , mpUI(new Ui::CProjectSettingsDialog)
    , mpProject(nullptr)
{
    mpUI->setupUi(this);

    connect(mpUI->CookPackageButton, SIGNAL(clicked()), this, SLOT(CookPackage()));
    connect(mpUI->CookAllDirtyPackagesButton, SIGNAL(clicked(bool)), this, SLOT(CookAllDirtyPackages()));
    connect(mpUI->BuildIsoButton, SIGNAL(clicked(bool)), this, SLOT(BuildISO()));

    connect(gpEdApp, SIGNAL(ActiveProjectChanged(CGameProject*)), this, SLOT(ActiveProjectChanged(CGameProject*)));
    connect(gpEdApp, SIGNAL(AssetsModified()), this, SLOT(SetupPackagesList()));
    connect(gpEdApp, SIGNAL(PackagesCooked()), this, SLOT(SetupPackagesList()));

    // Set build ISO button color
    QPalette Palette = mpUI->BuildIsoButton->palette();
    QBrush ButtonBrush = Palette.button();
    ButtonBrush.setColor( UICommon::kImportantButtonColor );
    Palette.setBrush(QPalette::Button, ButtonBrush);
    mpUI->BuildIsoButton->setPalette(Palette);
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

    mpUI->BuildIsoButton->setEnabled( pProj && !pProj->IsWiiBuild() );
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
        gpEdApp->CookPackage(pPackage);
    }
}

void CProjectSettingsDialog::CookAllDirtyPackages()
{
    gpEdApp->CookAllDirtyPackages();
}

void CProjectSettingsDialog::BuildISO()
{
    CGameProject *pProj = gpEdApp->ActiveProject();
    ASSERT(pProj && !pProj->IsWiiBuild());

    QString DefaultPath = TO_QSTRING( pProj->ProjectRoot() + FileUtil::SanitizeName(pProj->Name(), false) + ".gcm" );
    QString IsoPath = UICommon::SaveFileDialog(this, "Choose output ISO path", "*.gcm", DefaultPath);

    if (!IsoPath.isEmpty())
    {
        if (gpEdApp->CookAllDirtyPackages())
        {
            CProgressDialog Dialog("Building ISO", true, this);
            Dialog.DisallowCanceling();
            QFuture<void> Future = QtConcurrent::run(pProj, &CGameProject::BuildISO, TO_TSTRING(IsoPath), &Dialog);
            Dialog.WaitForResults(Future);
        }
    }
}
