#include "CAnimationLoader.h"
#include <Common/Log.h>

void CAnimationLoader::ReadUncompressedANIM()
{
    mpAnim->mDuration = mpInput->ReadFloat();
    mpInput->Seek(0x4, SEEK_CUR); // Skip unknown
    mpAnim->mTickInterval = mpInput->ReadFloat();
    mpInput->Seek(0x4, SEEK_CUR); // Skip unknown

    mpAnim->mNumKeys = mpInput->ReadLong();
    mpInput->Seek(0x4, SEEK_CUR); // Skip unknown

    // Read bone channel info
    u32 NumRotationChannels = 0;
    u32 NumTranslationChannels = 0;

    u32 NumRotIndices = mpInput->ReadLong();
    std::vector<u8> RotIndices(NumRotIndices);

    for (u32 iRot = 0; iRot < NumRotIndices; iRot++)
    {
        RotIndices[iRot] = mpInput->ReadByte();

        if (RotIndices[iRot] != 0xFF)
            NumRotationChannels++;
    }

    u32 NumTransIndices = mpInput->ReadLong();
    std::vector<u8> TransIndices(NumTransIndices);

    for (u32 iTrans = 0; iTrans < NumTransIndices; iTrans++)
    {
        TransIndices[iTrans] = mpInput->ReadByte();

        if (TransIndices[iTrans] != 0xFF)
            NumTranslationChannels++;
    }

    // Set up bone channel info
    mpAnim->mBoneInfo.resize(NumRotIndices);

    for (u32 iRot = 0, iTrans = 0; iRot < NumRotIndices; iRot++)
    {
        u8 RotIdx = RotIndices[iRot];
        mpAnim->mBoneInfo[iRot].RotationChannelIdx = RotIdx;

        if (RotIdx != 0xFF)
        {
            mpAnim->mBoneInfo[iRot].TranslationChannelIdx = TransIndices[iTrans];
            iTrans++;
        }
        else
            mpAnim->mBoneInfo[iRot].TranslationChannelIdx = 0xFF;
    }

    // Read bone transforms
    mpInput->Seek(0x4, SEEK_CUR); // Skipping quaternion count
    mpAnim->mRotationChannels.resize(NumRotationChannels);

    for (u32 iRot = 0; iRot < NumRotationChannels; iRot++)
    {
        mpAnim->mRotationChannels[iRot].resize(mpAnim->mNumKeys);

        for (u32 iKey = 0; iKey < mpAnim->mNumKeys; iKey++)
            mpAnim->mRotationChannels[iRot][iKey] = CQuaternion(*mpInput);
    }

    mpInput->Seek(0x4, SEEK_CUR); // Skipping vector3f count
    mpAnim->mTranslationChannels.resize(NumTranslationChannels);

    for (u32 iTrans = 0; iTrans < NumTranslationChannels; iTrans++)
    {
        mpAnim->mTranslationChannels[iTrans].resize(mpAnim->mNumKeys);

        for (u32 iKey = 0; iKey < mpAnim->mNumKeys; iKey++)
            mpAnim->mTranslationChannels[iTrans][iKey] = CVector3f(*mpInput);
    }

    // Skip EVNT file
}

// ************ STATIC ************
CAnimation* CAnimationLoader::LoadANIM(IInputStream& rANIM)
{
    u32 CompressionType = rANIM.ReadLong();

    if (CompressionType != 0 && CompressionType != 2)
    {
        Log::FileError(rANIM.GetSourceString(), "Unknown ANIM compression type: " + TString::HexString(CompressionType, 2));
        return nullptr;
    }

    if (CompressionType == 2)
    {
        Log::FileError(rANIM.GetSourceString(), "Compressed ANIMs not supported yet");
        return nullptr;
    }

    CAnimationLoader Loader;
    Loader.mpAnim = new CAnimation();
    Loader.mpInput = &rANIM;
    Loader.ReadUncompressedANIM();
    return Loader.mpAnim;
}
