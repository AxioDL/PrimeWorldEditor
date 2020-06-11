#ifndef CSKELETONLOADER_H
#define CSKELETONLOADER_H

#include "Core/Resource/Animation/CSkeleton.h"
#include "Core/Resource/TResPtr.h"
#include <Common/EGame.h>

class CSkeletonLoader
{
    TResPtr<CSkeleton> mpSkeleton;
    EGame mVersion{};

    CSkeletonLoader() = default;
    void SetLocalBoneCoords(CBone *pBone);
    void CalculateBoneInverseBindMatrices();

public:
    static CSkeleton* LoadCINF(IInputStream& rCINF, CResourceEntry *pEntry);
};

#endif // CSKELETONLOADER_H
