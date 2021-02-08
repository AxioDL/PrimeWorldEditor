#ifndef CANIMATIONLOADER_H
#define CANIMATIONLOADER_H

#include "Core/Resource/TResPtr.h"
#include "Core/Resource/Animation/CAnimation.h"
#include <Common/EGame.h>
#include <array>
#include <memory>

class CAnimationLoader
{
    TResPtr<CAnimation> mpAnim;
    IInputStream *mpInput = nullptr;
    EGame mGame{};

    // Compression data
    std::vector<bool> mKeyFlags;
    float mTranslationMultiplier = 0.0f;
    uint32 mRotationDivisor = 0;
    float mScaleMultiplier = 0.0f;

    struct SCompressedChannel
    {
        uint32 BoneID;
        uint16 NumRotationKeys;
        std::array<int16, 3> Rotation;
        std::array<uint8, 3> RotationBits;
        uint16 NumTranslationKeys;
        std::array<int16, 3> Translation;
        std::array<uint8, 3> TranslationBits;
        uint16 NumScaleKeys;
        std::array<int16, 3> Scale;
        std::array<uint8, 3> ScaleBits;
    };
    std::vector<SCompressedChannel> mCompressedChannels;

    CAnimationLoader() = default;
    bool UncompressedCheckEchoes();
    EGame UncompressedCheckVersion();
    void ReadUncompressedANIM();
    void ReadCompressedANIM();
    void ReadCompressedAnimationData();
    CQuaternion DequantizeRotation(bool Sign, int16 X, int16 Y, int16 Z) const;

public:
    static std::unique_ptr<CAnimation> LoadANIM(IInputStream& rANIM, CResourceEntry *pEntry);
};

#endif // CANIMATIONLOADER_H
