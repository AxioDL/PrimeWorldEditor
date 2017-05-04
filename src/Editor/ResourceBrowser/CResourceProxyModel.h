#ifndef CRESOURCEPROXYMODEL
#define CRESOURCEPROXYMODEL

#include "CResourceTableModel.h"
#include <QSet>
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
    TString mSearchString;
    ESortMode mSortMode;
    QSet<CResTypeInfo*> mTypeFilter;

public:
    explicit CResourceProxyModel(QObject *pParent = 0)
        : QSortFilterProxyModel(pParent)
        , mpModel(nullptr)
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

        if (pEntry && HasTypeFilter() && !mTypeFilter.contains(pEntry->TypeInfo()))
            return false;

        if (!mSearchString.IsEmpty())
        {
            if (pDir)
                return false;
            else
                return pEntry->UppercaseName().Contains(mSearchString);
        }

        return true;
    }

    inline void SetTypeFilter(CResTypeInfo *pInfo, bool Allow)
    {
        if (Allow)
            mTypeFilter.insert(pInfo);
        else
            mTypeFilter.remove(pInfo);
    }

    inline void ClearTypeFilter()
    {
        mTypeFilter.clear();
    }

    inline bool HasTypeFilter() const
    {
        return !mTypeFilter.isEmpty();
    }

    inline void SetSortMode(ESortMode Mode)
    {
        if (mSortMode != Mode)
        {
            mSortMode = Mode;
            sort(0, (mSortMode == eSortByName ? Qt::AscendingOrder : Qt::DescendingOrder));
        }
    }

public slots:
    void SetSearchString(const TString& rkString)
    {
        mSearchString = rkString.ToUpper();
    }
};

#endif // CRESOURCEPROXYMODEL

