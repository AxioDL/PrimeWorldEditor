#include "CWorldInfoSidebar.h"
#include "ui_CWorldInfoSidebar.h"
#include "CWorldEditor.h"
#include "Editor/CEditorApplication.h"

CWorldInfoSidebar::CWorldInfoSidebar(CWorldEditor *pEditor)
    : QWidget(pEditor)
    , mpUI(new Ui::CWorldInfoSidebar)
    , mModel(pEditor)
{
    mpUI->setupUi(this);
    mProxyModel.setSourceModel(&mModel);
    mpUI->WorldTreeView->setModel(&mProxyModel);
    mpUI->WorldTreeView->header()->setSortIndicator(0, Qt::AscendingOrder);

    QHeaderView *pHeader = mpUI->WorldTreeView->header();
    pHeader->resizeSection(0, pHeader->width() * 2); // I really have no idea how this works, I just got this from trial & error

    connect(gpEdApp, SIGNAL(ActiveProjectChanged(CGameProject*)), this, SLOT(OnActiveProjectChanged(CGameProject*)));
    connect(mpUI->AreaSearchLineEdit, SIGNAL(StoppedTyping(QString)), this, SLOT(OnAreaFilterStringChanged(QString)));
    connect(mpUI->WorldTreeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(OnWorldTreeClicked(QModelIndex)));
    connect(mpUI->WorldTreeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(OnWorldTreeDoubleClicked(QModelIndex)));

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
    mpUI->DarkWorldNameStringLabel->setHidden(true);
    mpUI->DarkWorldNameSelector->SetEditable(false);
    mpUI->DarkWorldNameSelector->setHidden(true);
    mpUI->SkySelector->SetEditable(false);

    mpUI->AreaNameLineEdit->setEnabled(false);
    mpUI->AreaSelector->SetEditable(false);
    mpUI->AreaNameSelector->SetEditable(false);
}

CWorldInfoSidebar::~CWorldInfoSidebar()
{
    delete mpUI;
}

// ************ SLOTS ************
void CWorldInfoSidebar::OnActiveProjectChanged(CGameProject *pProj)
{
    mpUI->ProjectInfoWidget->setHidden( pProj == nullptr );
    mpUI->WorldInfoWidget->setHidden(true);
    mpUI->AreaInfoWidget->setHidden(true);
    mpUI->AreaSearchLineEdit->clear();
    mProxyModel.SetFilterString("");

    bool IsEchoes = pProj && (pProj->Game() == eEchoesDemo || pProj->Game() == eEchoes);
    mpUI->GameNameLabel->setText( pProj ? TO_QSTRING(pProj->Name()) : "" );
    mpUI->DarkWorldNameStringLabel->setHidden(!IsEchoes);
    mpUI->DarkWorldNameSelector->setHidden(!IsEchoes);
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
    mpUI->WorldInfoWidget->setHidden(false);

    CWorld *pWorld = mModel.WorldForIndex(RealIndex);
    mpUI->WorldNameLabel->setText( TO_QSTRING(pWorld->InGameName()) );
    mpUI->WorldSelector->SetResource(pWorld);
    mpUI->WorldNameSelector->SetResource(pWorld->WorldName());
    mpUI->DarkWorldNameSelector->SetResource(pWorld->DarkWorldName());
    mpUI->SkySelector->SetResource(pWorld->DefaultSkybox());

    // Fill in area info
    bool IsArea = !mModel.IndexIsWorld(RealIndex);
    mpUI->AreaInfoWidget->setHidden(!IsArea);

    if (IsArea)
    {
        int AreaIndex = mModel.AreaIndexForIndex(RealIndex);
        mpUI->AreaNameLabel->setText( TO_QSTRING(pWorld->AreaInGameName(AreaIndex)) );
        mpUI->AreaSelector->SetResource( pWorld->AreaResourceID(AreaIndex) );
        mpUI->AreaNameLineEdit->setText( TO_QSTRING(pWorld->AreaInternalName(AreaIndex)) );
        mpUI->AreaNameSelector->SetResource( pWorld->AreaName(AreaIndex) );

        mpUI->AttachedAreasList->clear();

        for (u32 iAtt = 0; iAtt < pWorld->AreaAttachedCount(AreaIndex); iAtt++)
        {
            u32 AttachedIdx = pWorld->AreaAttachedID(AreaIndex, iAtt);
            QString Name = TO_QSTRING( pWorld->AreaInGameName(AttachedIdx) );
            mpUI->AttachedAreasList->addItem(Name);
        }
    }
}

void CWorldInfoSidebar::OnWorldTreeDoubleClicked(QModelIndex Index)
{
    QModelIndex RealIndex = mProxyModel.mapToSource(Index);

    if (!mModel.IndexIsWorld(RealIndex))
    {
        CWorld *pWorld = mModel.WorldForIndex(RealIndex);
        int AreaIndex = mModel.AreaIndexForIndex(RealIndex);
        gpEdApp->WorldEditor()->SetArea(pWorld, AreaIndex);
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
