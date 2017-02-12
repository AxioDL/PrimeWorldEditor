#ifndef CWORLDTREEMODEL_H
#define CWORLDTREEMODEL_H

#include <Core/Resource/CWorld.h>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
class CWorldEditor;

class CWorldTreeModel : public QAbstractItemModel
{
    Q_OBJECT

    struct SWorldInfo
    {
        TResPtr<CWorld> pWorld;
        QList<CResourceEntry*> Areas;
    };
    QList<SWorldInfo> mWorldList;

public:
    CWorldTreeModel(CWorldEditor *pEditor);

    int rowCount(const QModelIndex& rkParent) const;
    int columnCount(const QModelIndex& rkParent) const;
    QModelIndex index(int Row, int Column, const QModelIndex& rkParent) const;
    QModelIndex parent(const QModelIndex& rkChild) const;
    QVariant data(const QModelIndex& rkIndex, int Role) const;
    QVariant headerData(int Section, Qt::Orientation Orientation, int Role) const;

    bool IndexIsWorld(const QModelIndex& rkIndex) const;
    CWorld* WorldForIndex(const QModelIndex& rkIndex) const;
    int AreaIndexForIndex(const QModelIndex& rkIndex) const;
    CResourceEntry* AreaEntryForIndex(const QModelIndex& rkIndex) const;

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
    bool lessThan(const QModelIndex& rkSourceLeft, const QModelIndex& rkSourceRight) const;
    bool filterAcceptsRow(int SourceRow, const QModelIndex& rkSourceParent) const;

    inline void SetFilterString(const QString& rkFilter)    { mFilterString = rkFilter; invalidate(); }
};

#endif // CWORLDTREEMODEL_H
