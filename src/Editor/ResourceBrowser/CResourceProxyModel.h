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
        CVirtualDirectory *pLeftDir = mpModel->IndexDirectory(rkLeft);
        CVirtualDirectory *pRightDir = mpModel->IndexDirectory(rkRight);
        CResourceEntry *pLeftRes = mpModel->IndexEntry(rkLeft);
        CResourceEntry *pRightRes = mpModel->IndexEntry(rkRight);

        if (pLeftDir && !pRightDir)
            return true;

        else if (pRightDir && !pLeftDir)
            return false;

        else if (pLeftDir && pRightDir)
            return rkLeft.row() < rkRight.row(); // leave original directory order intact

        else if (mSortMode == eSortByName)
            return pLeftRes->UppercaseName() < pRightRes->UppercaseName();

        else
            return pLeftRes->Size() < pRightRes->Size();
    }

    bool filterAcceptsRow(int SourceRow, const QModelIndex& rkSourceParent) const
    {
        QModelIndex Index = mpModel->index(SourceRow, 0, rkSourceParent);
        CVirtualDirectory *pDir = mpModel->IndexDirectory(Index);
        CResourceEntry *pEntry = mpModel->IndexEntry(Index);

        if (!mSearchString.IsEmpty())
        {
            if (pDir)
                return false;
            else
                return pEntry->UppercaseName().Contains(mSearchString);
        }

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

