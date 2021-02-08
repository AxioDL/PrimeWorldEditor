#include "CWorldInfoSidebar.h"
#include "ui_CWorldInfoSidebar.h"
#include "CWorldEditor.h"
#include "Editor/CEditorApplication.h"

CWorldInfoSidebar::CWorldInfoSidebar(CWorldEditor *pEditor)
    : CWorldEditorSidebar(pEditor)
    , mpUI(std::make_unique<Ui::CWorldInfoSidebar>())
    , mModel(pEditor)
{
    mpUI->setupUi(this);
    mProxyModel.setSourceModel(&mModel);
    mpUI->WorldTreeView->setModel(&mProxyModel);
    mpUI->WorldTreeView->header()->setSortIndicator(0, Qt::AscendingOrder);

    QHeaderView *pHeader = mpUI->WorldTreeView->header();
    pHeader->resizeSection(0, pHeader->width() * 2); // I really have no idea how this works, I just got this from trial & error

    connect(gpEdApp, &CEditorApplication::ActiveProjectChanged, this, &CWorldInfoSidebar::OnActiveProjectChanged);
    connect(mpUI->ProjectSettingsButton, &QPushButton::pressed, pEditor, &CWorldEditor::OpenProjectSettings);
    connect(mpUI->AreaSearchLineEdit, &CTimedLineEdit::StoppedTyping, this, &CWorldInfoSidebar::OnAreaFilterStringChanged);
    connect(mpUI->WorldTreeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &CWorldInfoSidebar::OnWorldTreeClicked);
    connect(mpUI->WorldTreeView, &QTreeView::doubleClicked, this, &CWorldInfoSidebar::OnWorldTreeDoubleClicked);

    // Set up UI for world/area info; disable editing for now
    mpUI->ProjectInfoWidget->setHidden(true);
    mpUI->WorldInfoWidget->setHidden(true);
    mpUI->AreaInfoWidget->setHidden(true);

    QSizePolicy SizePolicy = mpUI->WorldInfoWidget->sizePolicy();
    SizePolicy.setRetainSizeWhenHidden(true);
    mpUI->WorldInfoWidget->setSizePolicy(SizePolicy);

    SizePolicy = mpUI->AreaInfoWidget->sizePolicy();
    SizePolicy.setRetainSizeWhenHidden(true);
    mpUI->AreaInfoWidget->setSizePolicy(SizePolicy);

    mpUI->WorldSelector->SetEditable(false);
    mpUI->WorldNameSelector->SetEditable(false);
    mpUI->DarkWorldNameSelector->SetEditable(false);
    mpUI->SkySelector->SetEditable(false);

    mpUI->AreaNameLineEdit->setEnabled(false);
    mpUI->AreaSelector->SetEditable(false);
    mpUI->AreaNameSelector->SetEditable(false);
}

CWorldInfoSidebar::~CWorldInfoSidebar() = default;

// ************ SLOTS ************
void CWorldInfoSidebar::OnActiveProjectChanged(CGameProject *pProj)
{
    mpUI->ProjectInfoWidget->setHidden(pProj == nullptr);
    mpUI->WorldInfoWidget->setHidden(true);
    mpUI->AreaInfoWidget->setHidden(true);
    mpUI->AreaSearchLineEdit->clear();
    mProxyModel.SetFilterString({});

    mpUI->GameNameLabel->setText(pProj ? TO_QSTRING(pProj->Name()) : QString{});

    // Add/remove widgets from the form layout based on the game. This is needed because
    // simply hiding the widgets causes a minor spacing issue. The only fix seems to be
    // actually entirely removing the widgets from the layout when not in use.
    bool IsEchoes = pProj && (pProj->Game() == EGame::EchoesDemo || pProj->Game() == EGame::Echoes);
    mpUI->DarkWorldNameStringLabel->setHidden(!IsEchoes);
    mpUI->DarkWorldNameSelector->setHidden(!IsEchoes);

    if (IsEchoes)
    {
        mpUI->WorldInfoFormLayout->insertRow(2, mpUI->DarkWorldNameStringLabel, mpUI->DarkWorldNameSelector);
    }
    else
    {
        mpUI->WorldInfoFormLayout->removeWidget(mpUI->DarkWorldNameStringLabel);
        mpUI->WorldInfoFormLayout->removeWidget(mpUI->DarkWorldNameSelector);
    }

    ClearWorldInfo();
}

void CWorldInfoSidebar::OnAreaFilterStringChanged(const QString& rkFilter)
{
    mProxyModel.SetFilterString(rkFilter);

    // Expand top-level items that contain matches for the new filter string
    int NumTopLevel = mModel.rowCount(QModelIndex());

    for (int iRow = 0; iRow < NumTopLevel; iRow++)
    {
        QModelIndex Index = mModel.index(iRow, 0, QModelIndex());
        QModelIndex ProxyIndex = mProxyModel.mapFromSource(Index);
        bool Matches = !rkFilter.isEmpty() && mProxyModel.rowCount(ProxyIndex) > 0;
        mpUI->WorldTreeView->setExpanded(ProxyIndex, Matches);
    }
}

void CWorldInfoSidebar::OnWorldTreeClicked(QModelIndex Index)
{
    QModelIndex RealIndex = mProxyModel.mapToSource(Index);

    // Fill in world info
    CWorld *pWorld = mModel.WorldForIndex(RealIndex);
    mpUI->WorldInfoWidget->setHidden( pWorld == nullptr );

    if (pWorld)
    {
        mpUI->WorldNameLabel->setText( TO_QSTRING(pWorld->InGameName()) );
        mpUI->WorldSelector->SetResource(pWorld);
        mpUI->WorldNameSelector->SetResource(pWorld->NameString());
        mpUI->DarkWorldNameSelector->SetResource(pWorld->DarkNameString());
        mpUI->SkySelector->SetResource(pWorld->DefaultSkybox());
    }

    // Fill in area info
    bool IsArea = !mModel.IndexIsWorld(RealIndex);
    mpUI->AreaInfoWidget->setHidden(!IsArea);

    if (IsArea)
    {
        int AreaIndex = Editor()->CurrentGame() == EGame::DKCReturns ? 0 : mModel.AreaIndexForIndex(RealIndex);
        mpUI->AreaNameLabel->setText( TO_QSTRING(pWorld->AreaInGameName(AreaIndex)) );
        mpUI->AreaSelector->SetResource( pWorld->AreaResourceID(AreaIndex) );
        mpUI->AreaNameLineEdit->setText( TO_QSTRING(pWorld->AreaInternalName(AreaIndex)) );
        mpUI->AreaNameSelector->SetResource( pWorld->AreaName(AreaIndex) );

        mpUI->AttachedAreasList->clear();

        for (size_t iAtt = 0; iAtt < pWorld->AreaAttachedCount(AreaIndex); iAtt++)
        {
            const uint32 AttachedIdx = pWorld->AreaAttachedID(AreaIndex, iAtt);
            const QString Name = TO_QSTRING(pWorld->AreaInGameName(AttachedIdx));
            mpUI->AttachedAreasList->addItem(Name);
        }
    }
}

void CWorldInfoSidebar::OnWorldTreeDoubleClicked(QModelIndex Index)
{
    QModelIndex RealIndex = mProxyModel.mapToSource(Index);

    if (!mModel.IndexIsWorld(RealIndex))
    {
        TResPtr<CWorld> pWorld = mModel.WorldForIndex(RealIndex);
        int AreaIndex = mModel.AreaIndexForIndex(RealIndex);

        // Validate area actually exists... DKCR has worlds that contain areas that don't exist
        CAssetID AreaAssetID = pWorld->AreaResourceID(AreaIndex);

        if (gpResourceStore->IsResourceRegistered(AreaAssetID))
        {
            gpEdApp->WorldEditor()->SetArea(pWorld, AreaIndex);
        }
        else
        {
            UICommon::ErrorMsg(Editor(), tr("The MREA asset associated with this area doesn't exist!"));
        }
    }
}

void CWorldInfoSidebar::ClearWorldInfo()
{
    mpUI->WorldNameLabel->clear();
    mpUI->WorldSelector->Clear();
    mpUI->WorldNameSelector->Clear();
    mpUI->DarkWorldNameSelector->Clear();
    mpUI->SkySelector->Clear();
    ClearAreaInfo();
}

void CWorldInfoSidebar::ClearAreaInfo()
{
    mpUI->AreaNameLabel->clear();
    mpUI->AreaSelector->Clear();
    mpUI->AreaNameLineEdit->clear();
    mpUI->AreaNameSelector->Clear();
    mpUI->AttachedAreasList->clear();
}
