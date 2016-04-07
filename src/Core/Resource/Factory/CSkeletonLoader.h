#ifndef CSKELETONLOADER_H
#define CSKELETONLOADER_H

#include "Core/Resource/CSkeleton.h"
#include "Core/Resource/EGame.h"
#include "Core/Resource/TResPtr.h"

class CSkeletonLoader
{
    TResPtr<CSkeleton> mpSkeleton;
    EGame mVersion;

    CSkeletonLoader() {}
    void SetLocalBoneCoords(CBone *pBone);

public:
    static CSkeleton* LoadCINF(IInputStream& rCINF);
};

#endif // CSKELETONLOADER_H
