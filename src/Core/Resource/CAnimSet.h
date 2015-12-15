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
        TString name;
        TResPtr<CModel> model;
        u32 skinID;
        u32 skelID;

        SNode() { model = nullptr; }
    };
    std::vector<SNode> nodes;

public:
    CAnimSet();
    ~CAnimSet();

    u32 getNodeCount();
    TString getNodeName(u32 node);
    CModel* getNodeModel(u32 node);
};

#endif // CCHARACTERSET_H
