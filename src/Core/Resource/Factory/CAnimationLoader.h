#ifndef CANIMATIONLOADER_H
#define CANIMATIONLOADER_H

#include "Core/Resource/TResPtr.h"
#include "Core/Resource/Animation/CAnimation.h"
#include <Common/EGame.h>
#include <memory>

class CAnimationLoader
{
    TResPtr<CAnimation> mpAnim;
    IInputStream *mpInput;
    EGame mGame;

    // Compression data
    std::vector<bool> mKeyFlags;
    float mTranslationMultiplier;
    uint32 mRotationDivisor;
    float mScaleMultiplier;

    struct SCompressedChannel
    {
        uint32 BoneID;
        uint16 NumRotationKeys;
        int16 Rotation[3];
        uint8 RotationBits[3];
        uint16 NumTranslationKeys;
        int16 Translation[3];
        uint8 TranslationBits[3];
        uint16 NumScaleKeys;
        int16 Scale[3];
        uint8 ScaleBits[3];
    };
    std::vector<SCompressedChannel> mCompressedChannels;

    CAnimationLoader() {}
    bool UncompressedCheckEchoes();
    EGame UncompressedCheckVersion();
    void ReadUncompressedANIM();
    void ReadCompressedANIM();
    void ReadCompressedAnimationData();
    CQuaternion DequantizeRotation(bool Sign, int16 X, int16 Y, int16 Z);

public:
    static std::unique_ptr<CAnimation> LoadANIM(IInputStream& rANIM, CResourceEntry *pEntry);
};

#endif // CANIMATIONLOADER_H
