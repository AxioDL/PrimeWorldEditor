#ifndef CRESOURCEPROXYMODEL
#define CRESOURCEPROXYMODEL

#include "CResourceTableModel.h"
#include <QSortFilterProxyModel>

class CResourceProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    enum ESortMode
    {
        eSortByName, eSortBySize
    };

private:
    CResourceTableModel *mpModel;
    CVirtualDirectory *mpDirectory;
    TWideString mSearchString;
    ESortMode mSortMode;

public:
    explicit CResourceProxyModel(QObject *pParent = 0)
        : QSortFilterProxyModel(pParent)
        , mpModel(nullptr)
        , mpDirectory(nullptr)
    {
        SetSortMode(eSortByName);
    }

    void setSourceModel(QAbstractItemModel *pSourceModel)
    {
        CResourceTableModel *pResModel = qobject_cast<CResourceTableModel*>(pSourceModel);
        mpModel = pResModel;
        QSortFilterProxyModel::setSourceModel(pResModel);
        sort(0);
    }

    bool lessThan(const QModelIndex& rkLeft, const QModelIndex& rkRight) const
    {
        CResourceEntry *pLeft = mpModel->IndexEntry(rkLeft);
        CResourceEntry *pRight = mpModel->IndexEntry(rkRight);

        if (mSortMode == eSortByName)
            return pLeft->UppercaseName() < pRight->UppercaseName();
        else
            return pLeft->Size() < pRight->Size();
    }

    bool filterAcceptsRow(int SourceRow, const QModelIndex& rkSourceParent) const
    {
        QModelIndex Index = mpModel->index(SourceRow, 0, rkSourceParent);
        CResourceEntry *pEntry = mpModel->IndexEntry(Index);

        if (mpDirectory && !pEntry->IsInDirectory(mpDirectory))
            return false;

        if (!mSearchString.IsEmpty() && !pEntry->UppercaseName().Contains(mSearchString))
            return false;

        return true;
    }

    inline void SetSortMode(ESortMode Mode)
    {
        if (mSortMode != Mode)
        {
            mSortMode = Mode;
            sort(0, (mSortMode == eSortByName ? Qt::AscendingOrder : Qt::DescendingOrder));
        }
    }

    inline void SetDirectory(CVirtualDirectory *pDir)
    {
        mpDirectory = pDir;
        invalidate();
    }

public slots:
    void SetSearchString(const TWideString& rkString)
    {
        mSearchString = rkString.ToUpper();
        invalidate();
    }
};

#endif // CRESOURCEPROXYMODEL

