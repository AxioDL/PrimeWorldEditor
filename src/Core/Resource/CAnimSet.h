#ifndef CANIMSET_H
#define CANIMSET_H

#include "TResPtr.h"
#include "CResource.h"
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
        u32 SkelID;

        SNode() { pModel = nullptr; }
    };
    std::vector<SNode> mNodes;

public:
    CAnimSet() : CResource() {}

    u32 NumNodes() const            { return mNodes.size(); }
    TString NodeName(u32 Index)     { if (Index >= mNodes.size()) Index = 0; return mNodes[Index].Name; }
    CModel* NodeModel(u32 Index)    { if (Index >= mNodes.size()) Index = 0; return mNodes[Index].pModel; }
};

#endif // CCHARACTERSET_H
