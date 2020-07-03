#ifndef CRESOURCEPROXYMODEL
#define CRESOURCEPROXYMODEL

#include "CResourceTableModel.h"
#include <QSet>
#include <QSortFilterProxyModel>

class CResourceProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    enum class ESortMode
    {
        ByName, BySize
    };

private:
    CResourceTableModel *mpModel = nullptr;
    TString mSearchString;
    ESortMode mSortMode{};
    QSet<CResTypeInfo*> mTypeFilter;

    uint64 mCompareID = 0;
    uint64 mCompareMask = 0;
    uint32 mCompareBitLength = 0;

public:
    explicit CResourceProxyModel(QObject *pParent = nullptr)
        : QSortFilterProxyModel(pParent)
    {
        SetSortMode(ESortMode::ByName);
    }

    void setSourceModel(QAbstractItemModel* pSourceModel) override
    {
        CResourceTableModel *pResModel = qobject_cast<CResourceTableModel*>(pSourceModel);
        mpModel = pResModel;
        QSortFilterProxyModel::setSourceModel(pResModel);
        sort(0);
    }

    bool lessThan(const QModelIndex& rkLeft, const QModelIndex& rkRight) const override
    {
        // Parent directory is always row 0 and should always be at the top
        if (mpModel->HasParentDirectoryEntry())
        {
            if (rkLeft.row() == 0)
                return true;

            if (rkRight.row() == 0)
                return false;
        }

        // Fetch directories and compare them
        CVirtualDirectory *pLeftDir = mpModel->IndexDirectory(rkLeft);
        CVirtualDirectory *pRightDir = mpModel->IndexDirectory(rkRight);
        CResourceEntry *pLeftRes = mpModel->IndexEntry(rkLeft);
        CResourceEntry *pRightRes = mpModel->IndexEntry(rkRight);

        if (pLeftDir && !pRightDir)
            return true;

        else if (pRightDir && !pLeftDir)
            return false;

        else if (pLeftDir && pRightDir)
            return pLeftDir->Name().ToUpper() < pRightDir->Name().ToUpper();

        else if (mSortMode == ESortMode::ByName)
            return pLeftRes->UppercaseName() < pRightRes->UppercaseName();

        else
            return pLeftRes->Size() < pRightRes->Size();
    }

    bool filterAcceptsRow(int SourceRow, const QModelIndex& rkSourceParent) const override
    {
        QModelIndex Index = mpModel->index(SourceRow, 0, rkSourceParent);
        CResourceEntry *pEntry = mpModel->IndexEntry(Index);

        if (pEntry && !IsTypeAccepted(pEntry->TypeInfo()))
            return false;

        // Compare search results
        if (!mSearchString.IsEmpty())
        {
            if (!pEntry)
                return false;

            bool HasNameMatch = pEntry->UppercaseName().Contains(mSearchString);

            if (!HasNameMatch)
            {
                bool HasIDMatch = false;

                if (mCompareBitLength > 0)
                {
                    const auto IDBitLength = static_cast<uint32>(pEntry->ID().Length()) * 8;

                    if (mCompareBitLength <= IDBitLength)
                    {
                        uint64 ID = pEntry->ID().ToLongLong();
                        uint32 MaxShift = IDBitLength - mCompareBitLength;

                        for (uint32 Shift = 0; Shift <= MaxShift; Shift += 4)
                        {
                            uint64 ShiftCompare = mCompareID << Shift;
                            uint64 Mask = mCompareMask << Shift;

                            if ((ID & Mask) == ShiftCompare)
                            {
                                HasIDMatch = true;
                                break;
                            }
                        }
                    }
                }

                if (!HasIDMatch)
                    return false;
            }
        }

        return true;
    }

    void SetTypeFilter(CResTypeInfo *pInfo, bool Allow)
    {
        if (Allow)
            mTypeFilter.insert(pInfo);
        else
            mTypeFilter.remove(pInfo);
    }

    void ClearTypeFilter()
    {
        mTypeFilter.clear();
    }

    bool HasTypeFilter() const
    {
        return !mTypeFilter.isEmpty();
    }

    bool IsTypeAccepted(CResTypeInfo *pTypeInfo) const
    {
        return mTypeFilter.isEmpty() || mTypeFilter.contains(pTypeInfo);
    }

    void SetSortMode(ESortMode Mode)
    {
        if (mSortMode != Mode)
        {
            mSortMode = Mode;
            sort(0, (mSortMode == ESortMode::ByName ? Qt::AscendingOrder : Qt::DescendingOrder));
        }
    }

public slots:
    void SetSearchString(const TString& rkString)
    {
        mSearchString = rkString.ToUpper();

        // Check if this is an asset ID
        TString IDString = rkString;
        IDString.RemoveWhitespace();

        if (IDString.StartsWith("0x"))
            IDString = IDString.ChopFront(2);

        if (IDString.Size() <= 16 && IDString.IsHexString())
        {
            mCompareBitLength = IDString.Size() * 4;
            mCompareMask = ((uint64) 1 << mCompareBitLength) - 1;
            mCompareID = IDString.ToInt64(16);

            if (mCompareMask == 0)
                mCompareMask = -1;
        }
        else
        {
            mCompareID = -1;
            mCompareMask = 0;
            mCompareBitLength = 0;
        }
    }
};

#endif // CRESOURCEPROXYMODEL

