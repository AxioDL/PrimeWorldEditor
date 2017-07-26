#include "CResourceTableView.h"
#include "CResourceBrowser.h"
#include "CResourceProxyModel.h"
#include <QAction>
#include <QDragEnterEvent>

CResourceTableView::CResourceTableView(QWidget *pParent /*= 0*/)
    : QTableView(pParent)
{
    // todo: removed delete shortcut because it conflicts with the World Editor delete shortcut
#if 0
    mpDeleteAction = new QAction(this);
    mpDeleteAction->setShortcut(QKeySequence::Delete);
    connect(mpDeleteAction, SIGNAL(triggered(bool)), this, SLOT(DeleteSelected()));
    addAction(mpDeleteAction);
#endif
}

void CResourceTableView::setModel(QAbstractItemModel *pModel)
{
    if (qobject_cast<CResourceProxyModel*>(pModel) != nullptr)
        QTableView::setModel(pModel);
}

void CResourceTableView::dragEnterEvent(QDragEnterEvent *pEvent)
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

// ************ SLOTS ************
void CResourceTableView::DeleteSelected()
{
    QModelIndexList List = selectionModel()->selectedIndexes();

    // Figure out which indices can actually be deleted
    CResourceProxyModel *pProxy = static_cast<CResourceProxyModel*>(model());
    CResourceTableModel *pModel = static_cast<CResourceTableModel*>(pProxy->sourceModel());
    QList<CVirtualDirectory*> DirsToDelete;
    bool HasNonEmptyDirSelected = false;

    foreach (QModelIndex Index, List)
    {
        QModelIndex SourceIndex = pProxy->mapToSource(Index);

        if (pModel->IsIndexDirectory(SourceIndex))
        {
            CVirtualDirectory *pDir = pModel->IndexDirectory(SourceIndex);

            if (pDir)
            {
                if (pDir->IsEmpty(true))
                    DirsToDelete << pDir;
                else
                    HasNonEmptyDirSelected = true;
            }
        }
    }

    // Let the user know if all selected directories are non empty
    if (HasNonEmptyDirSelected && DirsToDelete.isEmpty())
    {
        UICommon::ErrorMsg(parentWidget(), "Unable to delete; one or more of the selected directories is non-empty.");
        return;
    }

    // Delete
    gpEdApp->ResourceBrowser()->DeleteDirectories(DirsToDelete);
}
