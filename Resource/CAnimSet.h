#ifndef CANIMSET_H
#define CANIMSET_H

#include <Common/types.h>
#include <Core/CToken.h>
#include <vector>
#include "model/CModel.h"
#include "CResource.h"

// will expand later! this is where animation support will come in
class CAnimSet : public CResource
{
    friend class CAnimSetLoader;

    struct SNode
    {
        std::string name;
        CModel *model;
        u32 skinID;
        u32 skelID;
        CToken ModelToken;

        SNode() { model = nullptr; }
    };
    std::vector<SNode> nodes;

public:
    CAnimSet();
    ~CAnimSet();
    EResType Type();

    u32 getNodeCount();
    std::string getNodeName(u32 node);
    CModel* getNodeModel(u32 node);
};

#endif // CCHARACTERSET_H
