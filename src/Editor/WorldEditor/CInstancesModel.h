#ifndef CTYPESINSTANCEMODEL_H
#define CTYPESINSTANCEMODEL_H

#include "CWorldEditor.h"
#include <Core/Resource/Script/CGameTemplate.h>
#include <Core/Resource/Script/CScriptTemplate.h>
#include <Core/Scene/CSceneNode.h>

#include <QAbstractItemModel>
#include <QList>

class CInstancesModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum class EIndexType
    {
        Root, NodeType, ObjectType, Instance
    };

    enum class EInstanceModelType
    {
        Layers, Types
    };

private:
    CWorldEditor *mpEditor;
    CScene *mpScene;
    CGameArea *mpArea = nullptr;
    CGameTemplate *mpCurrentGame = nullptr;
    EInstanceModelType mModelType{EInstanceModelType::Layers};
    QList<CScriptTemplate*> mTemplateList;
    QStringList mBaseItems;
    bool mShowColumnEnabled = true;
    bool mChangingLayout = false;

public:
    explicit CInstancesModel(CWorldEditor *pEditor, QObject *pParent = nullptr);
    ~CInstancesModel() override;

    QVariant headerData(int Section, Qt::Orientation Orientation, int Role) const override;
    QModelIndex index(int Row, int Column, const QModelIndex& rkParent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& rkChild) const override;
    int rowCount(const QModelIndex& rkParent) const override;
    int columnCount(const QModelIndex& rkParent) const override;
    QVariant data(const QModelIndex& rkIndex, int Role) const override;

    void SetModelType(EInstanceModelType Type);
    void SetShowColumnEnabled(bool Enabled);
    CScriptLayer* IndexLayer(const QModelIndex& rkIndex) const;
    CScriptTemplate* IndexTemplate(const QModelIndex& rkIndex) const;
    CScriptObject* IndexObject(const QModelIndex& rkIndex) const;

public slots:
    void OnActiveProjectChanged(CGameProject *pProj);
    void OnMapChange();

    void NodeAboutToBeCreated();
    void NodeCreated(CSceneNode *pNode);
    void NodeAboutToBeDeleted(CSceneNode *pNode);
    void NodeDeleted();

    void PropertyModified(IProperty *pProp, CScriptObject *pInst);
    void InstancesLayerPreChange();
    void InstancesLayerPostChange(const QList<CScriptNode*>& rkInstanceList);

    // Static
    static EIndexType IndexType(const QModelIndex& rkIndex);
    static ENodeType IndexNodeType(const QModelIndex& rkIndex);

private:
    void GenerateList();
};

#endif // CTYPESINSTANCEMODEL_H
