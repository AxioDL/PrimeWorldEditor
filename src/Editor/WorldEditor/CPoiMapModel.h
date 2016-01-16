#ifndef CPOIMAPMODEL_H
#define CPOIMAPMODEL_H

#include <Core/Resource/CPoiToWorld.h>
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
    CGameArea *mpArea;
    TResPtr<CPoiToWorld> mpPoiToWorld;

    QMap<CScriptNode*, QList<CModelNode*>*> mModelMap;

public:
    explicit CPoiMapModel(CWorldEditor *pEditor, QObject *pParent = 0);
    ~CPoiMapModel();
    QVariant headerData(int Section, Qt::Orientation Orientation, int Role) const;
    int rowCount(const QModelIndex& rkParent) const;
    QVariant data(const QModelIndex& rkIndex, int Role) const;

    void AddPOI(CScriptNode *pPOI);
    void AddMapping(const QModelIndex& rkIndex, CModelNode *pNode);
    void RemovePOI(const QModelIndex& rkIndex);
    void RemoveMapping(const QModelIndex& rkIndex, CModelNode *pNode);

    CScriptNode* PoiNodePointer(const QModelIndex& rkIndex) const;
    const QList<CModelNode*>& GetPoiMeshList(const QModelIndex& rkIndex) const;
    const QList<CModelNode*>& GetPoiMeshList(CScriptNode *pPOI) const;
};

#endif // CPOIMAPMODEL_H
