#include "CResourceTableView.h"
#include <QDragEnterEvent>

CResourceTableView::CResourceTableView(QWidget *pParent /*= 0*/)
    : QTableView(pParent)
{}

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
