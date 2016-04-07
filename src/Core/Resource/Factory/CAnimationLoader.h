#ifndef CANIMATIONLOADER_H
#define CANIMATIONLOADER_H

#include "Core/Resource/TResPtr.h"
#include "Core/Resource/CAnimation.h"

class CAnimationLoader
{
    TResPtr<CAnimation> mpAnim;
    IInputStream *mpInput;

    CAnimationLoader() {}
    void ReadUncompressedANIM();

public:
    static CAnimation* LoadANIM(IInputStream& rANIM);
};

#endif // CANIMATIONLOADER_H
