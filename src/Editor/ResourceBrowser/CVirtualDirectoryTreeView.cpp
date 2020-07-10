#include "CVirtualDirectoryTreeView.h"
#include "CResourceBrowser.h"
#include "CVirtualDirectoryModel.h"
#include <QDragEnterEvent>

CVirtualDirectoryTreeView::CVirtualDirectoryTreeView(QWidget *pParent)
    : QTreeView(pParent)
{
    // slightly hacky cuz there's not really a good way to pass in CResourceBrowser as a parameter
    // due to the fact that we set this class in the .ui file
    CResourceBrowser *pBrowser = nullptr;
    QWidget *pWidget = pParent;

    while (pWidget && !pBrowser)
    {
        pBrowser = qobject_cast<CResourceBrowser*>(pWidget);
        pWidget = pWidget->parentWidget();
    }
    ASSERT(pBrowser);

    connect(pBrowser, &CResourceBrowser::DirectoryAboutToBeMoved, this, &CVirtualDirectoryTreeView::OnDirectoryAboutToBeMoved);
    connect(pBrowser, &CResourceBrowser::DirectoryMoved, this, &CVirtualDirectoryTreeView::OnDirectoryMoved);
}

void CVirtualDirectoryTreeView::dragEnterEvent(QDragEnterEvent *pEvent)
{
    // need to reimplement this to fix a bug in QAbstractItemView
    if (dragDropMode() == QAbstractItemView::InternalMove &&
            (pEvent->source() != this || ((pEvent->possibleActions() & Qt::MoveAction) == 0)) )
        return;

    if (pEvent->possibleActions() & model()->supportedDropActions())
    {
        pEvent->accept();
        setState(QAbstractItemView::DraggingState);
    }
}

void CVirtualDirectoryTreeView::setModel(QAbstractItemModel *pModel)
{
    CVirtualDirectoryModel *pDirModel = qobject_cast<CVirtualDirectoryModel*>(pModel);

    if (pDirModel)
    {
        mpModel = pDirModel;
        QTreeView::setModel(pModel);
    }
}

void CVirtualDirectoryTreeView::OnDirectoryAboutToBeMoved(const CVirtualDirectory *pDir)
{
    if (mpModel == nullptr)
        return;

    const QModelIndex Index = mpModel->GetIndexForDirectory(pDir);

    if (selectionModel()->currentIndex() == Index)
        mTransferSelectionPostMove = true;
}

void CVirtualDirectoryTreeView::OnDirectoryMoved(const CVirtualDirectory *pDir)
{
    if (!mTransferSelectionPostMove)
        return;

    // Make sure the model has updated first
    mpModel->FinishModelChanges();

    const QModelIndex Index = mpModel->GetIndexForDirectory(pDir);

    {
        [[maybe_unused]] const QSignalBlocker blocker{this};
        expand(Index.parent());
        selectionModel()->setCurrentIndex(Index, QItemSelectionModel::ClearAndSelect);
    }

    mTransferSelectionPostMove = false;
}
