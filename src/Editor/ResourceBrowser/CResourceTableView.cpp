#include "CResourceTableView.h"
#include "CResourceBrowser.h"
#include "CResourceProxyModel.h"
#include <QAction>
#include <QDragEnterEvent>

CResourceTableView::CResourceTableView(QWidget *pParent /*= 0*/)
    : QTableView(pParent)
{
    // Create "rename" key shortcut
    // todo - there's no QKeySequence::Rename. Is there another standard "rename" shortcut on other platforms?
    mpRenameAction = new QAction(this);
    mpRenameAction->setShortcut( QKeySequence(Qt::Key_F2) );
    connect(mpRenameAction, SIGNAL(triggered(bool)), this, SLOT(RenameSelected()));
    addAction(mpRenameAction);

    mpDeleteAction = new QAction(this);
    mpDeleteAction->setShortcut(QKeySequence::Delete);
    connect(mpDeleteAction, SIGNAL(triggered(bool)), this, SLOT(DeleteSelected()));
    addAction(mpDeleteAction);
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

void CResourceTableView::focusInEvent(QFocusEvent*)
{
    mpRenameAction->setEnabled(true);
}

void CResourceTableView::focusOutEvent(QFocusEvent*)
{
    mpRenameAction->setEnabled(false);
}

// ************ SLOTS ************
void CResourceTableView::RenameSelected()
{
    QModelIndexList List = selectionModel()->selectedIndexes();

    if (List.size() == 1)
    {
        edit(List.front());
    }
}

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
