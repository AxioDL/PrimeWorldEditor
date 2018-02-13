#ifndef CCHECKABLETREEWIDGETITEM_H
#define CCHECKABLETREEWIDGETITEM_H

#include "CCheckableTreeWidget.h"
#include <QTreeWidgetItem>

/** QTreeWidgetItem subclass that emits a signal when checked/unchecked. */
class CCheckableTreeWidgetItem : public QTreeWidgetItem
{
public:
    /** Constructors */
    CCheckableTreeWidgetItem(int type = Type)
        : QTreeWidgetItem(type) {}

    CCheckableTreeWidgetItem(const QStringList& strings, int type = Type)
        : QTreeWidgetItem(strings, type) {}

    CCheckableTreeWidgetItem(QTreeWidget* parent, int type = Type)
        : QTreeWidgetItem(parent, type) {}

    CCheckableTreeWidgetItem(QTreeWidget* parent, const QStringList& strings, int type = Type)
        : QTreeWidgetItem(parent, strings, type) {}

    CCheckableTreeWidgetItem(QTreeWidget* parent, QTreeWidgetItem* preceding, int type = Type)
        : QTreeWidgetItem(parent, preceding, type) {}

    CCheckableTreeWidgetItem(QTreeWidgetItem* parent, int type = Type)
        : QTreeWidgetItem(parent, type) {}

    CCheckableTreeWidgetItem(QTreeWidgetItem* parent, const QStringList& strings, int type = Type)
        : QTreeWidgetItem(parent, strings, type) {}

    CCheckableTreeWidgetItem(QTreeWidgetItem* parent, QTreeWidgetItem* preceding, int type = Type)
        : QTreeWidgetItem(parent, preceding, type) {}

    CCheckableTreeWidgetItem(const QTreeWidgetItem& other)
        : QTreeWidgetItem(other) {}

    /** setData override to catch check state changes */
    virtual void setData(int Column, int Role, const QVariant& rkValue)
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
