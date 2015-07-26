#include "CMaterialSet.h"
#include <Core/CResCache.h>
#include <iostream>

CMaterialSet::CMaterialSet() {}

CMaterialSet::~CMaterialSet()
{
    for (u32 m = 0; m < materials.size(); m++)
        delete materials[m];
}
