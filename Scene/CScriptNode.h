#ifndef CSCRIPTNODE_H
#define CSCRIPTNODE_H

#include "CSceneNode.h"
#include "CModelNode.h"
#include "CCollisionNode.h"
#include <Resource/script/CScriptObject.h>

class CScriptNode : public CSceneNode
{
    CScriptObject *mpInstance;
    CModel *mpActiveModel;
    CToken mModelToken;
    CCollisionNode *mpCollisionNode;

    bool mHasValidPosition;
    bool mHasVolumePreview;
    CModelNode *mpVolumePreviewNode;

public:
    CScriptNode(CSceneManager *pScene, CSceneNode *pParent = 0, CScriptObject *pObject = 0);
    ENodeType NodeType();
    std::string PrefixedName() const;
    void AddToRenderer(CRenderer *pRenderer);
    void Draw(ERenderOptions Options);
    void DrawAsset(ERenderOptions Options, u32 Asset);
    void DrawSelection();
    void RayAABoxIntersectTest(CRayCollisionTester &Tester);
    SRayIntersection RayNodeIntersectTest(const CRay &Ray, u32 AssetID, ERenderOptions options);
    bool IsVisible() const;
    CScriptObject* Object();
    CModel* ActiveModel();
    void GeneratePosition();
    bool HasPreviewVolume();
    CAABox PreviewVolumeAABox();
};

#endif // CSCRIPTNODE_H
