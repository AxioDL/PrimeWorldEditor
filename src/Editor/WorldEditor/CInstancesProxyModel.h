#ifndef CINSTANCESPROXYMODEL_H
#define CINSTANCESPROXYMODEL_H

#include <QSortFilterProxyModel>

class CInstancesProxyModel : public QSortFilterProxyModel
{
public:
    CInstancesProxyModel(QObject *pParent = 0)
        : QSortFilterProxyModel(pParent)
    {
        setSortCaseSensitivity(Qt::CaseInsensitive);
    }

    virtual bool lessThan(const QModelIndex& rkLeft, const QModelIndex& rkRight) const
    {
        // Don't sort from the top two levels and don't sort the Show column
        if (rkLeft.parent() == QModelIndex() || rkLeft.parent().parent() == QModelIndex() || rkLeft.column() == 2)
        {
            if (sortOrder() == Qt::AscendingOrder)
                return rkLeft.row() < rkRight.row();
            else
                return rkLeft.row() > rkRight.row();
        }

        else
        {
            QString Left = sourceModel()->data(rkLeft).toString();
            QString Right = sourceModel()->data(rkRight).toString();
            return Left < Right;
        }
    }
};

#endif // CINSTANCESPROXYMODEL_H
