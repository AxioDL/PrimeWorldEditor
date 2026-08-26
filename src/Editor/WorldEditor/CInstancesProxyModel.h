#ifndef CINSTANCESPROXYMODEL_H
#define CINSTANCESPROXYMODEL_H

#include <QSortFilterProxyModel>

class CInstancesProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit CInstancesProxyModel(QObject *pParent = nullptr)
        : QSortFilterProxyModel(pParent)
    {
        setSortCaseSensitivity(Qt::CaseInsensitive);
    }

    void sort(int column, Qt::SortOrder sortOrder = Qt::AscendingOrder) override
    {
        // Don't perform sorting via the Show column
        if (column == 3)
            return;

        QSortFilterProxyModel::sort(column, sortOrder);
    }

protected:
    bool lessThan(const QModelIndex& rkLeft, const QModelIndex& rkRight) const override
    {
        // Don't sort from the top two levels
        if (rkLeft.parent() == QModelIndex() || rkLeft.parent().parent() == QModelIndex())
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
