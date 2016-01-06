#ifndef CLAYERSINSTANCEMODEL_H
#define CLAYERSINSTANCEMODEL_H

#include "CWorldEditor.h"
#include <Core/Resource/Script/CScriptLayer.h>
#include <QAbstractItemModel>

// Only supports script layers atm - maybe light layers later...?
class CLayersInstanceModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum EIndexType {
        eRootIndex, eNodeTypeIndex, eLayerIndex, eInstanceIndex
    };

    enum ENodeType {
        eScriptType = 0x0,
        eLightType = 0x1,
        eInvalidType = 0xFF
    };

private:
    CWorldEditor *mpEditor;
    CScene *mpScene;
    TResPtr<CGameArea> mpArea;

public:
    explicit CLayersInstanceModel(QObject *pParent = 0);
    ~CLayersInstanceModel();
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    void SetEditor(CWorldEditor *pEditor);
    void NodeCreated(CSceneNode *pNode);
    void NodeDeleted(CSceneNode *pNode);
    CScriptLayer* IndexLayer(const QModelIndex& index) const;
    CScriptObject* IndexObject(const QModelIndex& index) const;

    // Static
    static EIndexType IndexType(const QModelIndex& index);
    static ENodeType IndexNodeType(const QModelIndex& index);
};

#endif // CLAYERSINSTANCEMODEL_H
