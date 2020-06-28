#ifndef CWORLDTREEMODEL_H
#define CWORLDTREEMODEL_H

#include <Core/Resource/CWorld.h>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
class CWorldEditor;

struct STreeArea
{
    CAssetID WorldID;
    int AreaIndex;
};

class CWorldTreeModel : public QAbstractItemModel
{
    Q_OBJECT

    struct SWorldInfo
    {
        QString WorldName;
        TResPtr<CWorld> pWorld;
        QList<CResourceEntry*> Areas;
    };
    QList<SWorldInfo> mWorldList;

public:
    explicit CWorldTreeModel(CWorldEditor *pEditor);

    int rowCount(const QModelIndex& rkParent) const override;
    int columnCount(const QModelIndex& rkParent) const override;
    QModelIndex index(int Row, int Column, const QModelIndex& rkParent) const override;
    QModelIndex parent(const QModelIndex& rkChild) const override;
    QVariant data(const QModelIndex& rkIndex, int Role) const override;
    QVariant headerData(int Section, Qt::Orientation Orientation, int Role) const override;

    bool IndexIsWorld(const QModelIndex& rkIndex) const;
    int AreaIndexForIndex(const QModelIndex& rkIndex) const;
    CWorld* WorldForIndex(const QModelIndex& rkIndex) const;
    CResourceEntry* AreaEntryForIndex(const QModelIndex& rkIndex) const;

protected:
    const SWorldInfo& WorldInfoForIndex(const QModelIndex& rkIndex) const;

public slots:
    void OnProjectChanged(CGameProject *pProj);
    void OnMapChanged();
};

// Proxy Model
class CWorldTreeProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QString mFilterString;

public:
    bool lessThan(const QModelIndex& rkSourceLeft, const QModelIndex& rkSourceRight) const override;
    bool filterAcceptsRow(int SourceRow, const QModelIndex& rkSourceParent) const override;

    void SetFilterString(const QString& rkFilter)
    {
        mFilterString = rkFilter;
        invalidate();
    }
};

#endif // CWORLDTREEMODEL_H
