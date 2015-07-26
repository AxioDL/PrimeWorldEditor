#include "CAnimSet.h"
#include <Core/CResCache.h>

CAnimSet::CAnimSet() : CResource()
{
}

CAnimSet::~CAnimSet()
{
}

EResType CAnimSet::Type()
{
    return eCharacter;
}

u32 CAnimSet::getNodeCount()
{
    return nodes.size();
}

std::string CAnimSet::getNodeName(u32 node)
{
    if (node >= nodes.size())
        return nodes[0].name;
    else
       return nodes[node].name;
}

CModel* CAnimSet::getNodeModel(u32 node)
{
    if (node >= nodes.size())
        return nodes[0].model;
    else
        return nodes[node].model;
}
