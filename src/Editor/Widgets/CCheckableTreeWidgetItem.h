#ifndef CCHECKABLETREEWIDGETITEM_H
#define CCHECKABLETREEWIDGETITEM_H

#include "CCheckableTreeWidget.h"
#include <QTreeWidgetItem>

/** QTreeWidgetItem subclass that emits a signal when checked/unchecked. */
class CCheckableTreeWidgetItem : public QTreeWidgetItem
{
public:
    /** Constructors */
    explicit CCheckableTreeWidgetItem(int type = Type)
        : QTreeWidgetItem(type) {}

    explicit CCheckableTreeWidgetItem(const QStringList& strings, int type = Type)
        : QTreeWidgetItem(strings, type) {}

    explicit CCheckableTreeWidgetItem(QTreeWidget* parent, int type = Type)
        : QTreeWidgetItem(parent, type) {}

    explicit CCheckableTreeWidgetItem(QTreeWidget* parent, const QStringList& strings, int type = Type)
        : QTreeWidgetItem(parent, strings, type) {}

    explicit CCheckableTreeWidgetItem(QTreeWidget* parent, QTreeWidgetItem* preceding, int type = Type)
        : QTreeWidgetItem(parent, preceding, type) {}

    explicit CCheckableTreeWidgetItem(QTreeWidgetItem* parent, int type = Type)
        : QTreeWidgetItem(parent, type) {}

    explicit CCheckableTreeWidgetItem(QTreeWidgetItem* parent, const QStringList& strings, int type = Type)
        : QTreeWidgetItem(parent, strings, type) {}

    explicit CCheckableTreeWidgetItem(QTreeWidgetItem* parent, QTreeWidgetItem* preceding, int type = Type)
        : QTreeWidgetItem(parent, preceding, type) {}

    explicit CCheckableTreeWidgetItem(const QTreeWidgetItem& other)
        : QTreeWidgetItem(other) {}

    /** setData override to catch check state changes */
    void setData(int Column, int Role, const QVariant& rkValue) override
    {
        Qt::CheckState OldState = checkState(0);
        QTreeWidgetItem::setData(Column, Role, rkValue);
        Qt::CheckState NewState = checkState(0);

        if (OldState != NewState)
        {
            CCheckableTreeWidget* pCheckableTree =
                    qobject_cast<CCheckableTreeWidget*>(treeWidget());
            
            if (pCheckableTree)
            {
                pCheckableTree->CheckStateChanged(this);
            }
        }
    }
};

#endif // CCHECKABLETREEWIDGETITEM_H
