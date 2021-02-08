#ifndef CPOIMAPMODEL_H
#define CPOIMAPMODEL_H

#include <Core/Resource/CPoiToWorld.h>
#include <Core/Resource/CWorld.h>
#include <Core/Resource/TResPtr.h>
#include <Core/Scene/CModelNode.h>
#include <Core/Scene/CScriptNode.h>
#include <QAbstractTableModel>
#include <QVector>

class CWorldEditor;

class CPoiMapModel : public QAbstractListModel
{
    Q_OBJECT

    CWorldEditor *mpEditor;
    CGameArea *mpArea = nullptr;
    TResPtr<CPoiToWorld> mpPoiToWorld;

    QMap<CScriptNode*, QList<CModelNode*>*> mModelMap;

public:
    explicit CPoiMapModel(CWorldEditor *pEditor, QObject *pParent = nullptr);

    QVariant headerData(int Section, Qt::Orientation Orientation, int Role) const override;
    int rowCount(const QModelIndex& rkParent) const override;
    QVariant data(const QModelIndex& rkIndex, int Role) const override;

    void AddPOI(CScriptNode *pPOI);
    void AddMapping(const QModelIndex& rkIndex, CModelNode *pNode);
    void RemovePOI(const QModelIndex& rkIndex);
    void RemoveMapping(const QModelIndex& rkIndex, CModelNode *pNode);
    bool IsPoiTracked(CScriptNode *pPOI) const;
    bool IsModelMapped(const QModelIndex& rkIndex, CModelNode *pNode) const;

    CScriptNode* PoiNodePointer(const QModelIndex& rkIndex) const;
    const QList<CModelNode*>& GetPoiMeshList(const QModelIndex& rkIndex) const;
    const QList<CModelNode*>& GetPoiMeshList(CScriptNode *pPOI) const;

public slots:
    void OnMapChange(CWorld*, CGameArea *pArea);
};

#endif // CPOIMAPMODEL_H
