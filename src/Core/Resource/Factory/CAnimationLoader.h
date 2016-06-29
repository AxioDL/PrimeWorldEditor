#ifndef CANIMATIONLOADER_H
#define CANIMATIONLOADER_H

#include "Core/Resource/TResPtr.h"
#include "Core/Resource/CAnimation.h"
#include "Core/Resource/EGame.h"

class CAnimationLoader
{
    TResPtr<CAnimation> mpAnim;
    IInputStream *mpInput;
    EGame mGame;

    // Compression data
    std::vector<bool> mKeyFlags;
    float mTranslationMultiplier;
    u32 mRotationDivisor;
    float mScaleMultiplier;

    struct SCompressedChannel
    {
        u32 BoneID;
        u16 NumRotationKeys;
        s16 Rotation[3];
        u8 RotationBits[3];
        u16 NumTranslationKeys;
        s16 Translation[3];
        u8 TranslationBits[3];
        u16 NumScaleKeys;
        s16 Scale[3];
        u8 ScaleBits[3];
    };
    std::vector<SCompressedChannel> mCompressedChannels;

    CAnimationLoader() {}
    bool UncompressedCheckEchoes();
    EGame UncompressedCheckVersion();
    void ReadUncompressedANIM();
    void ReadCompressedANIM();
    void ReadCompressedAnimationData();
    CQuaternion DequantizeRotation(bool Sign, s16 X, s16 Y, s16 Z);

public:
    static CAnimation* LoadANIM(IInputStream& rANIM, CResourceEntry *pEntry);
};

#endif // CANIMATIONLOADER_H
