#ifndef CSKELETONHIERARCHYMODEL_H
#define CSKELETONHIERARCHYMODEL_H

#include <QAbstractItemModel>
#include <Core/Resource/Animation/CSkeleton.h>

class CSkeletonHierarchyModel : public QAbstractItemModel
{
    const CSkeleton *mpSkeleton = nullptr;

public:
    explicit CSkeletonHierarchyModel(QObject *pParent = nullptr);

    QModelIndex index(int Row, int Column, const QModelIndex& rkParent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& rkChild) const override;
    int rowCount(const QModelIndex& rkParent = QModelIndex()) const override;
    int columnCount(const QModelIndex& rkParent = QModelIndex()) const override;
    QVariant data(const QModelIndex& rkIndex, int Role) const override;

    CBone* BoneForIndex(const QModelIndex& rkIndex) const;
    QModelIndex IndexForBone(const CBone *pBone) const;
    void SetSkeleton(const CSkeleton *pSkel);
};

#endif // CSKELETONHIERARCHYMODEL_H

