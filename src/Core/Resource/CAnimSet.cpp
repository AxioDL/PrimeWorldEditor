#include "CAnimSet.h"

CAnimSet::CAnimSet() : CResource()
{
}

CAnimSet::~CAnimSet()
{
}

u32 CAnimSet::getNodeCount()
{
    return nodes.size();
}

TString CAnimSet::getNodeName(u32 node)
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
