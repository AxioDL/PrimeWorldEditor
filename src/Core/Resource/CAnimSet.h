#ifndef CANIMSET_H
#define CANIMSET_H

#include "TResPtr.h"
#include "CAnimation.h"
#include "CResource.h"
#include "CSkeleton.h"
#include "Core/Resource/Model/CModel.h"
#include <Common/types.h>

#include <vector>

// will expand later! this is where animation support will come in
class CAnimSet : public CResource
{
    DECLARE_RESOURCE_TYPE(eAnimSet)
    friend class CAnimSetLoader;

    struct SNode
    {
        TString Name;
        TResPtr<CModel> pModel;
        u32 SkinID;
        TResPtr<CSkeleton> pSkeleton;

        SNode() { pModel = nullptr; }
    };
    std::vector<SNode> mNodes;

    struct SAnimation
    {
        TString Name;
        TResPtr<CAnimation> pAnim;
    };
    std::vector<SAnimation> mAnims;

public:
    CAnimSet() : CResource() {}

    u32 NumNodes() const                { return mNodes.size(); }
    TString NodeName(u32 Index)         { if (Index >= mNodes.size()) Index = 0; return mNodes[Index].Name; }
    CModel* NodeModel(u32 Index)        { if (Index >= mNodes.size()) Index = 0; return mNodes[Index].pModel; }
    CSkeleton* NodeSkeleton(u32 Index)  { if (Index >= mNodes.size()) Index = 0; return mNodes[Index].pSkeleton; }

    u32 NumAnims() const                { return mAnims.size(); }
    CAnimation* Animation(u32 Index)    { if (Index >= mAnims.size()) Index = 0; return mAnims[Index].pAnim; }
    TString AnimName(u32 Index)         { if (Index >= mAnims.size()) Index = 0; return mAnims[Index].Name; }
};

#endif // CCHARACTERSET_H
