#ifndef CTYPESINSTANCEMODEL_H
#define CTYPESINSTANCEMODEL_H

#include "CWorldEditor.h"
#include <Core/Resource/Script/CMasterTemplate.h>
#include <Core/Resource/Script/CScriptTemplate.h>
#include <Core/Scene/CSceneNode.h>

#include <QAbstractItemModel>
#include <QList>

class CInstancesModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum EIndexType {
        eRootIndex, eNodeTypeIndex, eObjectTypeIndex, eInstanceIndex
    };

    enum ENodeType {
        eScriptType = 0x0,
        eLightType = 0x1,
        eInvalidType = 0xFF
    };

    enum EInstanceModelType {
        eLayers, eTypes
    };

private:
    CWorldEditor *mpEditor;
    CScene *mpScene;
    TResPtr<CGameArea> mpArea;
    CMasterTemplate *mpCurrentMaster;
    EInstanceModelType mModelType;
    QList<CScriptTemplate*> mTemplateList;
    QStringList mBaseItems;

public:
    explicit CInstancesModel(QObject *pParent = 0);
    ~CInstancesModel();
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    void SetEditor(CWorldEditor *pEditor);
    void SetMaster(CMasterTemplate *pMaster);
    void SetArea(CGameArea *pArea);
    void SetModelType(EInstanceModelType type);
    void NodeCreated(CSceneNode *pNode);
    void NodeDeleted(CSceneNode *pNode);
    CScriptLayer* IndexLayer(const QModelIndex& index) const;
    CScriptTemplate* IndexTemplate(const QModelIndex& index) const;
    CScriptObject* IndexObject(const QModelIndex& index) const;

    // Static
    static EIndexType IndexType(const QModelIndex& index);
    static ENodeType IndexNodeType(const QModelIndex& index);

private:
    void GenerateList();

};

#endif // CTYPESINSTANCEMODEL_H
