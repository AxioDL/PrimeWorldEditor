#ifndef CCHECKABLETREEWIDGET_H
#define CCHECKABLETREEWIDGET_H

#include <QTreeWidget>

/**
 * QTreeWidget subclass that emits a signal when an item is checked or unchecked.
 * Items must be instantiated as CCheckableTreeWidgetItem, not QTreeWidgetItem.
*/
class CCheckableTreeWidget : public QTreeWidget
{
    Q_OBJECT

signals:
    void CheckStateChanged(QTreeWidgetItem* pItem);
    
public:
    CCheckableTreeWidget(QWidget* pParent = 0)
        : QTreeWidget(pParent) {}
};

#endif // CCHECKABLETREEWIDGET_H
