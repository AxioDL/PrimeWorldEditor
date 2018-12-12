#include "CSkeletonHierarchyModel.h"
#include "Editor/UICommon.h"

CSkeletonHierarchyModel::CSkeletonHierarchyModel(QObject *pParent /*= 0*/)
    : QAbstractItemModel(pParent)
    , mpSkeleton(nullptr)
{
}

QModelIndex CSkeletonHierarchyModel::index(int Row, int Column, const QModelIndex& rkParent) const
{
    if (!hasIndex(Row, Column, rkParent))
        return QModelIndex();

    if (!rkParent.isValid())
    {
        if (mpSkeleton)
            return createIndex(Row, Column, mpSkeleton->RootBone());
        else
            return QModelIndex();
    }

    CBone *pBone = (CBone*) rkParent.internalPointer();
    if (Row < (int) pBone->NumChildren())
        return createIndex(Row, Column, pBone->ChildByIndex(Row));
    else
        return QModelIndex();
}

QModelIndex CSkeletonHierarchyModel::parent(const QModelIndex& rkChild) const
{
    CBone *pBone = (CBone*) rkChild.internalPointer();

    if (pBone->Parent())
    {
        // Determine parent index
        CBone *pParent = pBone->Parent();

        if (pParent->Parent())
        {
            CBone *pGrandparent = pParent->Parent();

            for (uint32 iChild = 0; iChild < pGrandparent->NumChildren(); iChild++)
            {
                if (pGrandparent->ChildByIndex(iChild) == pParent)
                    return createIndex(iChild, 0, pParent);
            }
        }

        else return createIndex(0, 0, pParent);
    }

    return QModelIndex();
}

int CSkeletonHierarchyModel::rowCount(const QModelIndex& rkParent) const
{
    if (!mpSkeleton) return 0;
    CBone *pBone = (CBone*) rkParent.internalPointer();
    return (pBone ? pBone->NumChildren() : 1);
}

int CSkeletonHierarchyModel::columnCount(const QModelIndex& /*rkParent*/) const
{
    return 1;
}

QVariant CSkeletonHierarchyModel::data(const QModelIndex& rkIndex, int Role) const
{
    if (Role == Qt::DisplayRole || Role == Qt::ToolTipRole)
    {
        CBone *pBone = (CBone*) rkIndex.internalPointer();
        return TO_QSTRING(pBone->Name());
    }

    return QVariant::Invalid;
}

CBone* CSkeletonHierarchyModel::BoneForIndex(const QModelIndex& rkIndex) const
{
    return (CBone*) (rkIndex.internalPointer());
}

QModelIndex CSkeletonHierarchyModel::IndexForBone(CBone *pBone) const
{
    CBone *pParent = pBone->Parent();
    if (!pParent) return index(0, 0, QModelIndex());

    QModelIndex ParentIndex = IndexForBone(pParent);

    for (uint32 iChild = 0; iChild < pParent->NumChildren(); iChild++)
    {
       if (pParent->ChildByIndex(iChild) == pBone)
           return index(iChild, 0, ParentIndex);
    }

    return QModelIndex();
}

void CSkeletonHierarchyModel::SetSkeleton(CSkeleton *pSkel)
{
    if (mpSkeleton != pSkel)
    {
        beginResetModel();
        mpSkeleton = pSkel;
        endResetModel();
    }
}
