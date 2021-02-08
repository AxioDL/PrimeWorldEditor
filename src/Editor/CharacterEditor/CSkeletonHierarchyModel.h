#ifndef CSKELETONHIERARCHYMODEL
#define CSKELETONHIERARCHYMODEL

#include <QAbstractItemModel>
#include <Core/Resource/Animation/CSkeleton.h>

class CSkeletonHierarchyModel : public QAbstractItemModel
{
    CSkeleton *mpSkeleton = nullptr;

public:
    explicit CSkeletonHierarchyModel(QObject *pParent = nullptr);

    QModelIndex index(int Row, int Column, const QModelIndex& rkParent) const override;
    QModelIndex parent(const QModelIndex& rkChild) const override;
    int rowCount(const QModelIndex& rkParent) const override;
    int columnCount(const QModelIndex& rkParent) const override;
    QVariant data(const QModelIndex& rkIndex, int Role) const override;

    CBone* BoneForIndex(const QModelIndex& rkIndex) const;
    QModelIndex IndexForBone(CBone *pBone) const;
    void SetSkeleton(CSkeleton *pSkel);
};

#endif // CSKELETONHIERARCHYMODEL

