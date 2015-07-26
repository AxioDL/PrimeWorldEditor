#ifndef SMODELDATA_H
#define SMODELDATA_H

#include "SSurface.h"
#include <Common/CAABox.h>

struct SModelData
{
    CAABox mAABox;
    std::vector<SSurface*> mSurfaces;
};

#endif // SMODELDATA_H
