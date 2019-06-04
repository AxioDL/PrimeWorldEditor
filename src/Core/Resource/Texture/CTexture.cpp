#include "CTexture.h"
#include "NTextureUtils.h"
#include <Common/Math/MathUtil.h>

CTexture::CTexture(CResourceEntry *pEntry /*= 0*/)
    : CResource(pEntry)
    , mEditorFormat(ETexelFormat::RGBA8)
    , mGameFormat(EGXTexelFormat::RGBA8)
    , mEnableMultisampling(false)
    , mTextureResource(0)
{
}

CTexture::CTexture(uint32 SizeX, uint32 SizeY)
    : mEditorFormat(ETexelFormat::RGBA8)
    , mGameFormat(EGXTexelFormat::RGBA8)
    , mEnableMultisampling(false)
    , mTextureResource(0)
{
    mMipData.emplace_back();
    SMipData& Mip = mMipData.back();
    Mip.SizeX = SizeX;
    Mip.SizeY = SizeY;
    Mip.DataBuffer.resize(SizeX * SizeY * 4);
}

CTexture::~CTexture()
{
    ReleaseRenderResources();
}

void CTexture::CreateRenderResources()
{
    if (mTextureResource != 0)
    {
        ReleaseRenderResources();
    }

    GLenum BindTarget = (mEnableMultisampling ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D);
    glGenTextures(1, &mTextureResource);
    glBindTexture(BindTarget, mTextureResource);

    GLenum GLFormat, GLType;

    switch (mEditorFormat)
    {
        case ETexelFormat::Luminance:
            GLFormat = GL_R8;
            GLType = GL_UNSIGNED_BYTE;
            break;
        case ETexelFormat::LuminanceAlpha:
            GLFormat = GL_RG8;
            GLType = GL_UNSIGNED_BYTE;
            break;
        case ETexelFormat::RGB565:
            GLFormat = GL_RGB;
            GLType = GL_UNSIGNED_SHORT_5_6_5;
            break;
        case ETexelFormat::RGBA8:
            GLFormat = GL_RGBA;
            GLType = GL_UNSIGNED_BYTE;
            break;
    }

    // The smallest mipmaps are probably not being loaded correctly, because mipmaps in GX textures have a minimum size depending on the format, and these don't.
    // Not sure specifically what accomodations should be made to fix that though so whatever.
    for (uint MipIdx = 0; MipIdx < mMipData.size(); MipIdx++)
    {
        const SMipData& MipData = mMipData[MipIdx];
        uint SizeX = MipData.SizeX;
        uint SizeY = MipData.SizeY;
        const void* pkData = MipData.DataBuffer.data();

        if (mEnableMultisampling)
        {
            glTexImage2DMultisample(BindTarget, 4, GLFormat, SizeX, SizeY, true);
        }
        else
        {
            glTexImage2D(BindTarget, MipIdx, GLFormat, SizeX, SizeY, 0, GLFormat, GLType, pkData);
        }
    }

    glTexParameteri(BindTarget, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(BindTarget, GL_TEXTURE_MAX_LEVEL, mMipData.size() - 1);

    // Linear filtering on mipmaps:
    glTexParameteri(BindTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(BindTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    // Anisotropic filtering:
    float MaxAnisotropy;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &MaxAnisotropy);
    glTexParameterf(BindTarget, GL_TEXTURE_MAX_ANISOTROPY_EXT, MaxAnisotropy);

    // Swizzle for luminance formats
    if (mEditorFormat == ETexelFormat::Luminance)
    {
        glTexParameteri(BindTarget, GL_TEXTURE_SWIZZLE_RGBA, GL_RED);
    }
    else if (mEditorFormat == ETexelFormat::LuminanceAlpha)
    {
        glTexParameteri(BindTarget, GL_TEXTURE_SWIZZLE_R, GL_RED);
        glTexParameteri(BindTarget, GL_TEXTURE_SWIZZLE_G, GL_RED);
        glTexParameteri(BindTarget, GL_TEXTURE_SWIZZLE_B, GL_RED);
        glTexParameteri(BindTarget, GL_TEXTURE_SWIZZLE_A, GL_GREEN);
    }
}

void CTexture::ReleaseRenderResources()
{
    if (mTextureResource != 0)
    {
        glDeleteTextures(1, &mTextureResource);
        mTextureResource = 0;
    }
}

void CTexture::BindToSampler(uint SamplerIndex) const
{
    // CreateGraphicsResources() must have been called before calling this
    // @todo this should not be the responsibility of CTexture
    ASSERT( mTextureResource != 0 );
    GLenum BindTarget = (mEnableMultisampling ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D);

    glActiveTexture(GL_TEXTURE0 + SamplerIndex);
    glBindTexture(BindTarget, mTextureResource);
}

/** Generate mipmap chain based on the contents of the first mip */
void CTexture::GenerateMipTail(uint NumMips /*= 0*/)
{
    //@todo
}

/** Allocate mipmap data, but does not fill any data. Returns the new mipmap count. */
uint CTexture::AllocateMipTail(uint DesiredMipCount /*= 0*/)
{
    // We must have at least one mipmap to start with.
    if (mMipData.empty())
    {
        warnf("Failed to allocate mip tail; texture is empty, did not initialize correctly");
        return 0;
    }

    // Try to allocate the requested number of mipmaps, but don't allocate any below 1x1.
    // Also, we always need at least one mipmap.
    uint BaseSizeX = mMipData[0].SizeX;
    uint BaseSizeY = mMipData[0].SizeY;
    uint MaxMips = Math::Min( Math::FloorLog2(BaseSizeX), Math::FloorLog2(BaseSizeY) ) + 1;
    uint NewMipCount = Math::Min(MaxMips, DesiredMipCount);

    if (mMipData.size() != NewMipCount)
    {
        uint OldMipCount = mMipData.size();
        mMipData.resize(NewMipCount);

        // Allocate internal data for any new mips.
        if (NewMipCount > OldMipCount)
        {
            uint LastMipIdx = OldMipCount - 1;
            uint SizeX = mMipData[LastMipIdx].SizeX;
            uint SizeY = mMipData[LastMipIdx].SizeY;
            uint BPP = NTextureUtils::GetTexelFormatInfo( mEditorFormat ).BitsPerPixel;

            for (uint MipIdx = OldMipCount; MipIdx < NewMipCount; MipIdx++)
            {
                SizeX /= 2;
                SizeY /= 2;
                uint Size = (SizeX * SizeY * BPP) / 8;
                mMipData[MipIdx].SizeX = SizeX;
                mMipData[MipIdx].SizeY = SizeY;
                mMipData[MipIdx].DataBuffer.resize(Size);
            }
        }
    }

    return mMipData.size();
}

/** Compress the texture data into the format specified by Format. */
void CTexture::Compress(EGXTexelFormat Format)
{
    // @todo load source PNG data

}

/** Generate editor texel data based on the currently loaded game texel data */
void CTexture::GenerateEditorData(bool bClearGameData /*= true*/)
{
    for (uint MipIdx = 0; MipIdx < mMipData.size(); MipIdx++)
    {
        SMipData& MipData = mMipData[MipIdx];

        NTextureUtils::ConvertGameDataToEditorData(
            mGameFormat,
            MipData.SizeX,
            MipData.SizeY,
            MipData.GameDataBuffer.data(),
            MipData.GameDataBuffer.size(),
            MipData.DataBuffer,
            MipIdx == 0 ? &mEditorFormat : nullptr
        );

        if (bClearGameData)
        {
            MipData.GameDataBuffer.clear();
        }
    }
}

/**
 * Update the internal resolution of the texture; used for dynamically-scaling textures
 */
void CTexture::Resize(uint32 SizeX, uint32 SizeY)
{
    if (mMipData.size() > 0)
    {
        if (mMipData[0].SizeX == SizeX &&
            mMipData[0].SizeY == SizeY)
        {
            return;
        }
    }

    if (mMipData.size() == 0)
    {
        mMipData.emplace_back();
    }

    const STexelFormatInfo& kFormatInfo = NTextureUtils::GetTexelFormatInfo(mEditorFormat);
    mMipData.back().SizeX = SizeX;
    mMipData.back().SizeY = SizeY;
    mMipData.back().DataBuffer.resize( SizeX * SizeY * kFormatInfo.BitsPerPixel / 8 );

    if (mTextureResource != 0)
    {
        ReleaseRenderResources();
        CreateRenderResources();
    }
}

float CTexture::ReadTexelAlpha(const CVector2f& kTexCoord)
{
    // @todo: this is an inaccurate implementation because it
    // doesn't take into account mipmaps or texture filtering
    const SMipData& kMipData = mMipData[0];
    uint32 TexelX = (uint32) ((kMipData.SizeX - 1) * kTexCoord.X);
    uint32 TexelY = (uint32) ((kMipData.SizeY - 1) * (1.f - fmodf(kTexCoord.Y, 1.f)));


    if (mEditorFormat == ETexelFormat::Luminance || mEditorFormat == ETexelFormat::RGB565)
    {
        // No alpha in these formats
        return 1.f;
    }
    else if (mEditorFormat == ETexelFormat::LuminanceAlpha)
    {
        uint Offset = (TexelY * kMipData.SizeX * 2) + TexelX*2 + 1;
        uint8 Alpha = *((uint8*) &kMipData.DataBuffer[Offset]);
        return Alpha / 255.f;
    }
    else if (mEditorFormat == ETexelFormat::RGBA8)
    {
        uint Offset = (TexelY * kMipData.SizeX * 4) + TexelX*4 + 3;
        uint8 Alpha = *((uint8*) &kMipData.DataBuffer[Offset]);
        return Alpha / 255.f;
    }
    else
    {
        errorf("Unhandled texel format in ReadTexelAlpha(): %s",
               TEnumReflection<ETexelFormat>::ConvertValueToString(mEditorFormat));
        return 1.f;
    }
}
