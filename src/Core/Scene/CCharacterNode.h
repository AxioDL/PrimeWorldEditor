#ifndef CCHARACTERNODE_H
#define CCHARACTERNODE_H

#include "CSceneNode.h"
#include "Core/Resource/CAnimSet.h"

class CCharacterNode : public CSceneNode
{
    TResPtr<CAnimSet> mpCharacter;
    u32 mActiveCharSet;

public:
    explicit CCharacterNode(CScene *pScene, u32 NodeID, CAnimSet *pChar = 0, CSceneNode *pParent = 0);

    virtual ENodeType NodeType();
    virtual void PostLoad();
    virtual void AddToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo);
    virtual void Draw(FRenderOptions Options, int ComponentIndex, const SViewInfo& rkViewInfo);
    virtual SRayIntersection RayNodeIntersectTest(const CRay& rkRay, u32 AssetID, const SViewInfo& rkViewInfo);
    inline CAnimSet* Character() const  { return mpCharacter; }
    inline u32 ActiveCharSet() const    { return mActiveCharSet; }

    void SetCharacter(CAnimSet *pChar);
    void SetActiveCharSet(u32 CharIndex);
};

#endif // CCHARACTERNODE_H
