#include "CSkeletonHierarchyModel.h"
#include "Editor/UICommon.h"

CSkeletonHierarchyModel::CSkeletonHierarchyModel(QObject *pParent)
    : QAbstractItemModel(pParent)
{
}

QModelIndex CSkeletonHierarchyModel::index(int Row, int Column, const QModelIndex& rkParent) const
{
    if (!hasIndex(Row, Column, rkParent))
        return {};

    if (!rkParent.isValid())
    {
        if (mpSkeleton)
            return createIndex(Row, Column, mpSkeleton->RootBone());

        return {};
    }

    const auto* pBone = static_cast<const CBone*>(rkParent.internalPointer());
    if (Row < static_cast<int>(pBone->NumChildren()))
        return createIndex(Row, Column, pBone->ChildByIndex(Row));

    return {};
}

QModelIndex CSkeletonHierarchyModel::parent(const QModelIndex& rkChild) const
{
    const auto *pBone = static_cast<const CBone*>(rkChild.internalPointer());

    if (pBone->Parent())
    {
        // Determine parent index
        const CBone* pParent = pBone->Parent();

        if (pParent->Parent())
        {
            const CBone* pGrandparent = pParent->Parent();

            for (size_t iChild = 0; iChild < pGrandparent->NumChildren(); iChild++)
            {
                if (pGrandparent->ChildByIndex(iChild) == pParent)
                    return createIndex(static_cast<int>(iChild), 0, pParent);
            }
        }
        else
        {
            return createIndex(0, 0, pParent);
        }
    }

    return {};
}

int CSkeletonHierarchyModel::rowCount(const QModelIndex& rkParent) const
{
    if (!mpSkeleton)
        return 0;

    const auto* pBone = static_cast<const CBone*>(rkParent.internalPointer());
    return pBone ? static_cast<int>(pBone->NumChildren()) : 1;
}

int CSkeletonHierarchyModel::columnCount(const QModelIndex& /*rkParent*/) const
{
    return 1;
}

QVariant CSkeletonHierarchyModel::data(const QModelIndex& rkIndex, int Role) const
{
    if (Role == Qt::DisplayRole || Role == Qt::ToolTipRole)
    {
        const auto* pBone = static_cast<const CBone*>(rkIndex.internalPointer());
        return TO_QSTRING(pBone->Name());
    }

    return {};
}

CBone* CSkeletonHierarchyModel::BoneForIndex(const QModelIndex& rkIndex) const
{
    return static_cast<CBone*>(rkIndex.internalPointer());
}

QModelIndex CSkeletonHierarchyModel::IndexForBone(const CBone *pBone) const
{
    const CBone* pParent = pBone->Parent();
    if (!pParent)
        return index(0, 0);

    const QModelIndex ParentIndex = IndexForBone(pParent);

    for (size_t iChild = 0; iChild < pParent->NumChildren(); iChild++)
    {
       if (pParent->ChildByIndex(iChild) == pBone)
           return index(static_cast<int>(iChild), 0, ParentIndex);
    }

    return {};
}

void CSkeletonHierarchyModel::SetSkeleton(const CSkeleton *pSkel)
{
    if (mpSkeleton == pSkel)
        return;

    beginResetModel();
    mpSkeleton = pSkel;
    endResetModel();
}
