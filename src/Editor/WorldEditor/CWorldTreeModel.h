#ifndef CWORLDTREEMODEL_H
#define CWORLDTREEMODEL_H

#include <Common/CAssetID.h>
#include <Core/Resource/TResPtr.h>

#include <QAbstractItemModel>
#include <QList>
#include <QSortFilterProxyModel>

class CGameProject;
class CResourceEntry;
class CWorld;
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
    ~CWorldTreeModel() override;

    int rowCount(const QModelIndex& rkParent = QModelIndex()) const override;
    int columnCount(const QModelIndex& rkParent = QModelIndex()) const override;
    QModelIndex index(int Row, int Column, const QModelIndex& rkParent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& rkChild) const override;
    QVariant data(const QModelIndex& rkIndex, int Role) const override;
    QVariant headerData(int Section, Qt::Orientation Orientation, int Role) const override;

    bool IndexIsWorld(const QModelIndex& rkIndex) const;
    int AreaIndexForIndex(const QModelIndex& rkIndex) const;
    CWorld* WorldForIndex(const QModelIndex& rkIndex) const;
    CResourceEntry* AreaEntryForIndex(const QModelIndex& rkIndex) const;

protected:
    const SWorldInfo& WorldInfoForIndex(const QModelIndex& rkIndex) const;

private slots:
    void OnProjectChanged(CGameProject *pProj);
    void OnMapChanged();
};

// Proxy Model
class CWorldTreeProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

protected:
    bool lessThan(const QModelIndex& rkSourceLeft, const QModelIndex& rkSourceRight) const override;
    bool filterAcceptsRow(int SourceRow, const QModelIndex& rkSourceParent) const override;
};

#endif // CWORLDTREEMODEL_H
