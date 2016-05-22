#include "CGameProject.h"

void CGameProject::AddPackage(CPackage *pPackage, bool WorldPak)
{
    if (WorldPak)
        mWorldPaks.push_back(pPackage);
    else
        mResourcePaks.push_back(pPackage);
}

