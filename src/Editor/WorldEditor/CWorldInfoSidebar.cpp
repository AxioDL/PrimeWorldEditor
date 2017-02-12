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

    connect(mpUI->AreaSearchLineEdit, SIGNAL(StoppedTyping(QString)), this, SLOT(OnAreaFilterStringChanged(QString)));
    connect(mpUI->WorldTreeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(OnWorldTreeDoubleClicked(QModelIndex)));
}

CWorldInfoSidebar::~CWorldInfoSidebar()
{
    delete mpUI;
}

// ************ SLOTS ************
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
