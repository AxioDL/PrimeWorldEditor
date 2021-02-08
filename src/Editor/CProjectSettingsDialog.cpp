#include "CProjectSettingsDialog.h"
#include "ui_CProjectSettingsDialog.h"
#include "CEditorApplication.h"
#include "CExportGameDialog.h"
#include "CProgressDialog.h"
#include "NDolphinIntegration.h"
#include "UICommon.h"
#include "Editor/ResourceBrowser/CResourceBrowser.h"
#include <Common/Macros.h>
#include <Core/GameProject/CGameExporter.h>
#include <Core/GameProject/COpeningBanner.h>

#include <nod/nod.hpp>

#include <QFileDialog>
#include <QFuture>
#include <QFutureWatcher>
#include <QMessageBox>
#include <QtConcurrent/QtConcurrentRun>

CProjectSettingsDialog::CProjectSettingsDialog(QWidget *pParent)
    : QDialog(pParent)
    , mpUI(std::make_unique<Ui::CProjectSettingsDialog>())
{
    mpUI->setupUi(this);

    connect(mpUI->GameNameLineEdit, &QLineEdit::editingFinished, this, &CProjectSettingsDialog::GameNameChanged);
    connect(mpUI->CookPackageButton, &QPushButton::clicked, this, &CProjectSettingsDialog::CookPackage);
    connect(mpUI->CookAllDirtyPackagesButton, &QPushButton::clicked, this, &CProjectSettingsDialog::CookAllDirtyPackages);
    connect(mpUI->BuildIsoButton, &QPushButton::clicked, this, &CProjectSettingsDialog::BuildISO);

    connect(gpEdApp, &CEditorApplication::ActiveProjectChanged, this, &CProjectSettingsDialog::ActiveProjectChanged);
    connect(gpEdApp, &CEditorApplication::AssetsModified, this, &CProjectSettingsDialog::SetupPackagesList);
    connect(gpEdApp, &CEditorApplication::PackagesCooked, this, &CProjectSettingsDialog::SetupPackagesList);

    // Set build ISO button color
    QPalette Palette = mpUI->BuildIsoButton->palette();
    QBrush ButtonBrush = Palette.button();
    ButtonBrush.setColor( UICommon::kImportantButtonColor );
    Palette.setBrush(QPalette::Button, ButtonBrush);
    mpUI->BuildIsoButton->setPalette(Palette);
}

CProjectSettingsDialog::~CProjectSettingsDialog() = default;

void CProjectSettingsDialog::ActiveProjectChanged(CGameProject *pProj)
{
    mpProject = pProj;

    if (mpProject)
    {
        // Set up project info
        mpUI->ProjectNameLineEdit->setText(TO_QSTRING(pProj->Name()));
        mpUI->GameLineEdit->setText(TO_QSTRING(GetGameName(pProj->Game())));
        mpUI->GameIdLineEdit->setText(TO_QSTRING(pProj->GameID()));

        const float BuildVer = pProj->BuildVersion();
        const ERegion Region = pProj->Region();
        const TString RegionName = TEnumReflection<ERegion>::ConvertValueToString(Region);
        const TString BuildName = pProj->GameInfo()->GetBuildName(BuildVer, Region);
        mpUI->BuildLineEdit->setText(tr("%1 (%2)").arg(BuildVer).arg(TO_QSTRING(BuildName)));
        mpUI->RegionLineEdit->setText(TO_QSTRING(RegionName));

        // Banner info
        const COpeningBanner Banner(pProj);
        mpUI->GameNameLineEdit->setText(TO_QSTRING(Banner.EnglishGameName()));
        mpUI->GameNameLineEdit->setMaxLength(Banner.MaxGameNameLength());
    }
    else
    {
        // Clear project info
        mpUI->ProjectNameLineEdit->clear();
        mpUI->GameLineEdit->clear();
        mpUI->GameIdLineEdit->clear();
        mpUI->BuildLineEdit->clear();
        mpUI->RegionLineEdit->clear();
        mpUI->GameNameLineEdit->clear();
        close();
    }

    SetupPackagesList();
}

void CProjectSettingsDialog::GameNameChanged()
{
    if (mpProject)
    {
        QString NewName = mpUI->GameNameLineEdit->text();

        COpeningBanner Banner(mpProject);
        Banner.SetEnglishGameName( TO_TSTRING(NewName) );
        Banner.Save();
    }
}

void CProjectSettingsDialog::SetupPackagesList()
{
    mpUI->PackagesList->clear();
    if (!mpProject) return;

    for (size_t iPkg = 0; iPkg < mpProject->NumPackages(); iPkg++)
    {
        CPackage *pPackage = mpProject->PackageByIndex(iPkg);
        ASSERT(pPackage != nullptr);

        QString PackageName = TO_QSTRING(pPackage->Name());
        if (pPackage->NeedsRecook())
            PackageName += '*';
        mpUI->PackagesList->addItem(PackageName);
    }
}

void CProjectSettingsDialog::CookPackage()
{
    const auto PackageIdx = static_cast<uint32>(mpUI->PackagesList->currentRow());

    if (PackageIdx == UINT32_MAX)
        return;

    CPackage *pPackage = mpProject->PackageByIndex(PackageIdx);
    gpEdApp->CookPackage(pPackage);
}

void CProjectSettingsDialog::CookAllDirtyPackages()
{
    gpEdApp->CookAllDirtyPackages();
}

void CProjectSettingsDialog::BuildISO()
{
    CGameProject *pProj = gpEdApp->ActiveProject();
    ASSERT(pProj);

    QString DefaultExtension, FilterString;

    if (!pProj->IsWiiBuild())
    {
        DefaultExtension = QStringLiteral(".gcm");
        FilterString = QStringLiteral("*.gcm;*.iso");
    }
    else
    {
        DefaultExtension = QStringLiteral(".iso");
        FilterString = QStringLiteral("*.iso");
    }

    QString DefaultPath = TO_QSTRING( pProj->ProjectRoot() + FileUtil::SanitizeName(pProj->Name(), false) ) + DefaultExtension;
    QString IsoPath = UICommon::SaveFileDialog(this, tr("Choose output ISO path"), FilterString, DefaultPath);

    if (!IsoPath.isEmpty())
    {
        bool NeedsDiscMerge = pProj->IsWiiDeAsobu() || pProj->IsTrilogy();
        std::unique_ptr<nod::DiscBase> pBaseDisc = nullptr;

        if (NeedsDiscMerge)
        {
            FilterString += QStringLiteral(";*.wbfs;*.nfs");
            QString SourceIsoPath = UICommon::OpenFileDialog(this, tr("Select the original ISO"), FilterString, DefaultPath);

            if (SourceIsoPath.isEmpty())
                return;

            // Verify this ISO matches the original
            bool IsWii;
            pBaseDisc = nod::OpenDiscFromImage(QStringToNodString(SourceIsoPath), IsWii);

            if (!pBaseDisc || !IsWii)
            {
                UICommon::ErrorMsg(this, tr("The ISO provided is not a valid Wii ISO!"));
                return;
            }

            const nod::Header& rkHeader = pBaseDisc->getHeader();
            TString GameID = pProj->GameID();

            if (strncmp(*GameID, rkHeader.m_gameID, 6) != 0)
            {
                UICommon::ErrorMsg(this, tr("The ISO provided doesn't match the project!"));
                return;
            }
        }

        if (gpEdApp->CookAllDirtyPackages())
        {
            // Make sure there will be no leftover quickplay files in the built ISO
            NDolphinIntegration::CleanupQuickplayFiles(pProj);

            CProgressDialog Dialog(tr("Building ISO"), false, true, this);
            Dialog.DisallowCanceling();
            bool Success;

            if (!NeedsDiscMerge)
            {
                QFuture<bool> Future = QtConcurrent::run(pProj, &CGameProject::BuildISO, TO_TSTRING(IsoPath), &Dialog);
                Success = Dialog.WaitForResults(Future);
            }
            else
            {
                QFuture<bool> Future = QtConcurrent::run(pProj, &CGameProject::MergeISO, TO_TSTRING(IsoPath), (nod::DiscWii*) pBaseDisc.get(), &Dialog);
                Success = Dialog.WaitForResults(Future);
            }

            if (Success)
                UICommon::InfoMsg(this, tr("Success"), tr("ISO built successfully!"));
            else
                UICommon::ErrorMsg(this, tr("ISO build failed!"));
        }
    }
}
