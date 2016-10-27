#ifndef CSKELETONHIERARCHYMODEL
#define CSKELETONHIERARCHYMODEL

#include <QAbstractItemModel>
#include <Core/Resource/Animation/CSkeleton.h>

class CSkeletonHierarchyModel : public QAbstractItemModel
{
    CSkeleton *mpSkeleton;

public:
    explicit CSkeletonHierarchyModel(QObject *pParent = 0);
    QModelIndex index(int Row, int Column, const QModelIndex& rkParent) const;
    QModelIndex parent(const QModelIndex& rkChild) const;
    int rowCount(const QModelIndex& rkParent) const;
    int columnCount(const QModelIndex& rkParent) const;
    QVariant data(const QModelIndex& rkIndex, int Role) const;

    CBone* BoneForIndex(const QModelIndex& rkIndex) const;
    QModelIndex IndexForBone(CBone *pBone) const;
    void SetSkeleton(CSkeleton *pSkel);
};

#endif // CSKELETONHIERARCHYMODEL

