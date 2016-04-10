#include "CAnimationLoader.h"
#include <Common/Log.h>
#include <Math/MathUtil.h>

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

void CAnimationLoader::ReadCompressedANIM()
{
    // Header
    mpInput->Seek(0xC, SEEK_CUR); // Skip alloc size, EVNT ID, unknown value
    mpAnim->mDuration = mpInput->ReadFloat();
    mpAnim->mTickInterval = mpInput->ReadFloat();
    mpInput->Seek(0x8, SEEK_CUR); // Skip two unknown values

    mRotationDivisor = mpInput->ReadLong();
    mTranslationMultiplier = mpInput->ReadFloat();
    u32 NumBoneChannels = mpInput->ReadLong();
    mpInput->Seek(0x4, SEEK_CUR); // Skip unknown value

    // Read key flags
    u32 NumKeys = mpInput->ReadLong();
    mpAnim->mNumKeys = NumKeys;
    mKeyFlags.resize(NumKeys);
    {
        CBitStreamInWrapper BitStream(mpInput);

        for (u32 iBit = 0; iBit < NumKeys; iBit++)
            mKeyFlags[iBit] = BitStream.ReadBit();
    }
    mpInput->Seek(0x8, SEEK_CUR);

    // Read bone channel descriptors
    mCompressedChannels.resize(NumBoneChannels);
    mpAnim->mRotationChannels.resize(NumBoneChannels);
    mpAnim->mTranslationChannels.resize(NumBoneChannels);

    for (u32 iChan = 0; iChan < NumBoneChannels; iChan++)
    {
        SCompressedChannel& rChan = mCompressedChannels[iChan];
        rChan.BoneID = mpInput->ReadLong();

        // Read rotation parameters
        rChan.NumRotationKeys = mpInput->ReadShort();

        if (rChan.NumRotationKeys > 0)
        {
            for (u32 iComp = 0; iComp < 3; iComp++)
            {
                rChan.Rotation[iComp] = mpInput->ReadShort();
                rChan.RotationBits[iComp] = mpInput->ReadByte();
            }

            mpAnim->mBoneInfo[rChan.BoneID].RotationChannelIdx = (u8) iChan;
        }
        else mpAnim->mBoneInfo[rChan.BoneID].RotationChannelIdx = 0xFF;

        // Read translation parameters
        rChan.NumTranslationKeys = mpInput->ReadShort();

        if (rChan.NumTranslationKeys > 0)
        {
            for (u32 iComp = 0; iComp < 3; iComp++)
            {
                rChan.Translation[iComp] = mpInput->ReadShort();
                rChan.TranslationBits[iComp] = mpInput->ReadByte();
            }

            mpAnim->mBoneInfo[rChan.BoneID].TranslationChannelIdx = (u8) iChan;
        }
        else
            mpAnim->mBoneInfo[rChan.BoneID].TranslationChannelIdx = 0xFF;
    }

    // Read animation data
    ReadCompressedAnimationData();
}

void CAnimationLoader::ReadCompressedAnimationData()
{
    CBitStreamInWrapper BitStream(mpInput);

    // Initialize
    for (u32 iChan = 0; iChan < mCompressedChannels.size(); iChan++)
    {
        SCompressedChannel& rChan = mCompressedChannels[iChan];

        // Set initial rotation/translation
        if (rChan.NumRotationKeys > 0)
        {
            mpAnim->mRotationChannels[iChan].reserve(rChan.NumRotationKeys + 1);
            CQuaternion Rotation = DequantizeRotation(false, rChan.Rotation[0], rChan.Rotation[1], rChan.Rotation[2]);
            mpAnim->mRotationChannels[iChan].push_back(Rotation);
        }

        if (rChan.NumTranslationKeys > 0)
        {
            mpAnim->mTranslationChannels[iChan].reserve(rChan.NumTranslationKeys + 1);
            CVector3f Translate = CVector3f(rChan.Translation[0], rChan.Translation[1], rChan.Translation[2]) * mTranslationMultiplier;
            mpAnim->mTranslationChannels[iChan].push_back(Translate);
        }
    }

    // Read keys
    for (u32 iKey = 0; iKey < mpAnim->mNumKeys - 1; iKey++)
    {
        bool KeyPresent = mKeyFlags[iKey+1];

        for (u32 iChan = 0; iChan < mCompressedChannels.size(); iChan++)
        {
            SCompressedChannel& rChan = mCompressedChannels[iChan];

            // Read rotation
            if (rChan.NumRotationKeys > 0)
            {
                // Note if KeyPresent is false, this isn't the correct value of WSign.
                // However, we're going to recreate this key later via interpolation, so it doesn't matter what value we use here.
                bool WSign = (KeyPresent ? BitStream.ReadBit() : false);

                if (KeyPresent)
                {
                    rChan.Rotation[0] += (s16) BitStream.ReadBits(rChan.RotationBits[0]);
                    rChan.Rotation[1] += (s16) BitStream.ReadBits(rChan.RotationBits[1]);
                    rChan.Rotation[2] += (s16) BitStream.ReadBits(rChan.RotationBits[2]);
                }

                CQuaternion Rotation = DequantizeRotation(WSign, rChan.Rotation[0], rChan.Rotation[1], rChan.Rotation[2]);
                mpAnim->mRotationChannels[iChan].push_back(Rotation);
            }

            // Read translation
            if (rChan.NumTranslationKeys > 0)
            {
                if (KeyPresent)
                {
                    rChan.Translation[0] += (s16) BitStream.ReadBits(rChan.TranslationBits[0]);
                    rChan.Translation[1] += (s16) BitStream.ReadBits(rChan.TranslationBits[1]);
                    rChan.Translation[2] += (s16) BitStream.ReadBits(rChan.TranslationBits[2]);
                }

                CVector3f Translate = CVector3f(rChan.Translation[0], rChan.Translation[1], rChan.Translation[2]) * mTranslationMultiplier;
                mpAnim->mTranslationChannels[iChan].push_back(Translate);
            }
        }
    }

    // Fill in missing keys
    u32 NumMissedKeys = 0;

    for (u32 iKey = 0; iKey < mpAnim->mNumKeys; iKey++)
    {
        if (!mKeyFlags[iKey])
            NumMissedKeys++;

        else if (NumMissedKeys > 0)
        {
            u32 FirstIndex = iKey - NumMissedKeys - 1;
            u32 LastIndex = iKey;
            u32 RelLastIndex = LastIndex - FirstIndex;

            for (u32 iMissed = 0; iMissed < NumMissedKeys; iMissed++)
            {
                u32 KeyIndex = FirstIndex + iMissed + 1;
                u32 RelKeyIndex = (KeyIndex - FirstIndex);
                float Interp = (float) RelKeyIndex / (float) RelLastIndex;

                for (u32 iChan = 0; iChan < mCompressedChannels.size(); iChan++)
                {
                    bool HasTranslationKeys = mCompressedChannels[iChan].NumTranslationKeys > 0;
                    bool HasRotationKeys = mCompressedChannels[iChan].NumRotationKeys > 0;

                    if (HasRotationKeys)
                    {
                        CQuaternion Left = mpAnim->mRotationChannels[iChan][FirstIndex];
                        CQuaternion Right = mpAnim->mRotationChannels[iChan][LastIndex];
                        mpAnim->mRotationChannels[iChan][KeyIndex] = Left.Slerp(Right, Interp);
                    }

                    if (HasTranslationKeys)
                    {
                        CVector3f Left = mpAnim->mTranslationChannels[iChan][FirstIndex];
                        CVector3f Right = mpAnim->mTranslationChannels[iChan][LastIndex];
                        mpAnim->mTranslationChannels[iChan][KeyIndex] = Math::Lerp<CVector3f>(Left, Right, Interp);
                    }
                }
            }

            NumMissedKeys = 0;
        }
    }
}

CQuaternion CAnimationLoader::DequantizeRotation(bool Sign, s16 X, s16 Y, s16 Z)
{
    CQuaternion Out;
    float Multiplier = Math::skHalfPi / (float) mRotationDivisor;
    Out.X = sinf(X * Multiplier);
    Out.Y = sinf(Y * Multiplier);
    Out.Z = sinf(Z * Multiplier);
    Out.W = Math::Sqrt( fmax(1.f - ((Out.X * Out.X) + (Out.Y * Out.Y) + (Out.Z * Out.Z)), 0.f) );
    if (Sign) Out.W = -Out.W;
    return Out;
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

    CAnimationLoader Loader;
    Loader.mpAnim = new CAnimation();
    Loader.mpInput = &rANIM;

    if (CompressionType == 0)
        Loader.ReadUncompressedANIM();
    else
        Loader.ReadCompressedANIM();

    return Loader.mpAnim;
}
