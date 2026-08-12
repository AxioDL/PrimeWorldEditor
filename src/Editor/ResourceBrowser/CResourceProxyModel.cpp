#include "Editor/ResourceBrowser/CResourceProxyModel.h"

#include <Core/GameProject/CResourceEntry.h>
#include <Core/GameProject/CVirtualDirectory.h>
#include "Editor/ResourceBrowser/CResourceTableModel.h"

CResourceProxyModel::CResourceProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    SetSortMode(ESortMode::ByName);
}

CResourceProxyModel::~CResourceProxyModel() = default;

void CResourceProxyModel::setSourceModel(QAbstractItemModel* sourceModel)
{
    mpModel = qobject_cast<CResourceTableModel*>(sourceModel);
    QSortFilterProxyModel::setSourceModel(mpModel);
    sort(0);
}

bool CResourceProxyModel::lessThan(const QModelIndex& rkLeft, const QModelIndex& rkRight) const
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
    const CVirtualDirectory* pLeftDir = mpModel->IndexDirectory(rkLeft);
    const CVirtualDirectory* pRightDir = mpModel->IndexDirectory(rkRight);
    const CResourceEntry* pLeftRes = mpModel->IndexEntry(rkLeft);
    const CResourceEntry* pRightRes = mpModel->IndexEntry(rkRight);

    if (pLeftDir && !pRightDir)
        return true;

    if (pRightDir && !pLeftDir)
        return false;

    if (pLeftDir && pRightDir)
        return pLeftDir->Name().ToUpper() < pRightDir->Name().ToUpper();

    if (mSortMode == ESortMode::ByName)
        return pLeftRes->UppercaseName() < pRightRes->UppercaseName();

    return pLeftRes->Size() < pRightRes->Size();
}

bool CResourceProxyModel::filterAcceptsRow(int SourceRow, const QModelIndex& rkSourceParent) const
{
    const QModelIndex Index = mpModel->index(SourceRow, 0, rkSourceParent);
    const CResourceEntry* pEntry = mpModel->IndexEntry(Index);

    if (pEntry && !IsTypeAccepted(pEntry->TypeInfo()))
        return false;

    if (mSearchString.IsEmpty())
        return true;

    if (!pEntry)
        return false;

    // Compare search results
    const bool HasNameMatch = pEntry->UppercaseName().Contains(mSearchString);
    if (HasNameMatch)
        return true;

    bool HasIDMatch = false;

    if (mCompareBitLength > 0)
    {
        const auto IDBitLength = static_cast<uint32_t>(pEntry->ID().Length()) * 8;

        if (mCompareBitLength <= IDBitLength)
        {
            const auto ID = pEntry->ID().ToU64();
            const auto MaxShift = IDBitLength - mCompareBitLength;

            for (uint32_t Shift = 0; Shift <= MaxShift; Shift += 4)
            {
                const uint64_t ShiftCompare = mCompareID << Shift;
                const uint64_t Mask = mCompareMask << Shift;

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

    return true;
}

void CResourceProxyModel::SetTypeFilter(const CResTypeInfo *pInfo, bool Allow)
{
    if (Allow)
        mTypeFilter.insert(pInfo);
    else
        mTypeFilter.remove(pInfo);
}

bool CResourceProxyModel::IsTypeAccepted(const CResTypeInfo* pTypeInfo) const
{
    return mTypeFilter.isEmpty() || mTypeFilter.contains(pTypeInfo);
}

void CResourceProxyModel::SetSortMode(ESortMode Mode)
{
    if (mSortMode == Mode)
        return;

    mSortMode = Mode;
    sort(0, (mSortMode == ESortMode::ByName ? Qt::AscendingOrder : Qt::DescendingOrder));
}

// ************ SLOTS ************
void CResourceProxyModel::SetSearchString(const TString& rkString)
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
        mCompareMask = (mCompareBitLength == 64) ? 0 : (1ULL << mCompareBitLength) - 1;
        mCompareID = IDString.ToInt64(16);

        if (mCompareMask == 0)
            mCompareMask = UINT64_MAX;
    }
    else
    {
        mCompareID = UINT64_MAX;
        mCompareMask = 0;
        mCompareBitLength = 0;
    }
}
