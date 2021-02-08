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
    explicit CCheckableTreeWidget(QWidget* pParent = nullptr)
        : QTreeWidget(pParent) {}
};

#endif // CCHECKABLETREEWIDGET_H
