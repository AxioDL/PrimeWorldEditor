#include "CResourceTableView.h"
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
