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

public:

private:
    CWorldEditor *mpEditor;
    TResPtr<CPoiToWorld> mpPoiToWorld;

    struct SEditorPoiMap
    {
        CScriptNode *pPOI;
        QList<CModelNode*> Models;
    };
    QList<SEditorPoiMap> mMaps;
    QMap<CScriptNode*,SEditorPoiMap*> mPoiLookupMap;

public:
    explicit CPoiMapModel(CWorldEditor *pEditor, QObject *pParent = 0);
    QVariant headerData(int Section, Qt::Orientation Orientation, int Role) const;
    int rowCount(const QModelIndex& rkParent) const;
    QVariant data(const QModelIndex& rkIndex, int Role) const;

    CScriptNode* PoiNodePointer(const QModelIndex& rkIndex) const;
    const QList<CModelNode*>& GetPoiMeshList(const QModelIndex& rkIndex) const;
    const QList<CModelNode*>& GetPoiMeshList(CScriptNode *pPOI) const;
};

#endif // CPOIMAPMODEL_H
