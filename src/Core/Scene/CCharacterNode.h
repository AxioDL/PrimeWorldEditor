#ifndef CCHARACTERNODE_H
#define CCHARACTERNODE_H

#include "CSceneNode.h"
#include "Core/Resource/CAnimSet.h"

class CCharacterNode : public CSceneNode
{
    TResPtr<CAnimSet> mpCharacter;
    u32 mActiveCharSet;
    u32 mActiveAnim;

public:
    explicit CCharacterNode(CScene *pScene, u32 NodeID, CAnimSet *pChar = 0, CSceneNode *pParent = 0);

    virtual ENodeType NodeType();
    virtual void PostLoad();
    virtual void AddToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo);
    virtual void Draw(FRenderOptions Options, int ComponentIndex, const SViewInfo& rkViewInfo);
    virtual SRayIntersection RayNodeIntersectTest(const CRay& rkRay, u32 AssetID, const SViewInfo& rkViewInfo);
    inline CAnimSet* Character() const  { return mpCharacter; }
    inline u32 ActiveCharSet() const    { return mActiveCharSet; }
    inline u32 ActiveAnim() const       { return mActiveAnim; }

    void SetCharacter(CAnimSet *pChar);
    void SetActiveCharSet(u32 CharIndex);
    void SetActiveAnim(u32 AnimIndex);
};

#endif // CCHARACTERNODE_H
