#include "CTexture.h"
#include <cmath>

CTexture::CTexture(CResourceEntry *pEntry)
    : CResource(pEntry)
{
}

CTexture::CTexture(uint32 Width, uint32 Height)
    : mWidth(static_cast<uint16>(Width))
    , mHeight(static_cast<uint16>(Height))
    , mNumMipMaps(1)
    , mLinearSize(Width * Height * 4)
{
}

CTexture::~CTexture()
{
    DeleteBuffers();
}

bool CTexture::BufferGL()
{
    const GLenum BindTarget = (mEnableMultisampling ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D);
    glGenTextures(1, &mTextureID);
    glBindTexture(BindTarget, mTextureID);

    GLenum GLFormat = 0;
    GLenum GLType = 0;
    bool IsCompressed = false;

    switch (mTexelFormat)
    {
    case ETexelFormat::Luminance:
        GLFormat = GL_R;
        GLType = GL_UNSIGNED_BYTE;
        break;
    case ETexelFormat::LuminanceAlpha:
        GLFormat = GL_RG;
        GLType = GL_UNSIGNED_BYTE;
        break;
    case ETexelFormat::RGB565:
        GLFormat = GL_RGB;
        GLType = GL_UNSIGNED_SHORT_5_6_5;
        break;
    case ETexelFormat::RGBA4:
        GLFormat = GL_RGBA;
        GLType = GL_UNSIGNED_SHORT_4_4_4_4;
        break;
    case ETexelFormat::RGBA8:
        GLFormat = GL_RGBA;
        GLType = GL_UNSIGNED_BYTE;
        break;
    case ETexelFormat::DXT1:
        GLFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        IsCompressed = true;
        break;
    default:
        break;
    }

    // The smallest mipmaps are probably not being loaded correctly, because mipmaps in GX textures have a minimum size depending on the format, and these don't.
    // Not sure specifically what accomodations should be made to fix that though so whatever.
    uint32 MipSize = mLinearSize;
    uint32 MipOffset = 0;
    uint16 MipW = mWidth, MipH = mHeight;

    for (uint32 iMip = 0; iMip < mNumMipMaps; iMip++)
    {
        GLvoid *pData = (mBufferExists) ? (mpImgDataBuffer + MipOffset) : NULL;

        if (!IsCompressed)
        {
            if (mEnableMultisampling)
                glTexImage2DMultisample(BindTarget, 4, GLFormat, MipW, MipH, true);
            else
                glTexImage2D(BindTarget, iMip, GLFormat, MipW, MipH, 0, GLFormat, GLType, pData);
        }
        else
        {
            glCompressedTexImage2D(BindTarget, iMip, GLFormat, MipW, MipH, 0, MipSize, pData);
        }

        MipW /= 2;
        MipH /= 2;
        MipOffset += MipSize;
        MipSize /= 4;
    }

    glTexParameteri(BindTarget, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(BindTarget, GL_TEXTURE_MAX_LEVEL, mNumMipMaps - 1);

    // Swizzling for luminance textures:
    if (mTexelFormat == ETexelFormat::Luminance || mTexelFormat == ETexelFormat::LuminanceAlpha)
    {
        const GLint SwizzleMask[] = {GL_RED, GL_RED, GL_RED, GLFormat == GL_RG ? GL_GREEN : GL_ONE};
        glTexParameteriv(BindTarget, GL_TEXTURE_SWIZZLE_RGBA, SwizzleMask);
    }

    // Linear filtering on mipmaps:
    glTexParameteri(BindTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(BindTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    // Anisotropic filtering:
    float MaxAnisotropy;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &MaxAnisotropy);
    glTexParameterf(BindTarget, GL_TEXTURE_MAX_ANISOTROPY_EXT, MaxAnisotropy);

    mGLBufferExists = true;
    return true;
}

void CTexture::Bind(uint32 GLTextureUnit)
{
    glActiveTexture(GL_TEXTURE0 + GLTextureUnit);

    if (!mGLBufferExists)
        BufferGL();

    const GLenum BindTarget = (mEnableMultisampling ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D);
    glBindTexture(BindTarget, mTextureID);
}

void CTexture::Resize(uint32 Width, uint32 Height)
{
    if ((mWidth != Width) || (mHeight != Height))
    {
        DeleteBuffers();
        mWidth = static_cast<uint16>(Width);
        mHeight = static_cast<uint16>(Height);
        mNumMipMaps = 1;
        CalcLinearSize();
    }
}

float CTexture::ReadTexelAlpha(const CVector2f& rkTexCoord)
{
    // todo: support texel formats other than DXT1
    // DXT1 is definitely the most complicated one anyway; try reusing CTextureDecoder functions for other formats
    const auto TexelX = static_cast<uint32>((mWidth - 1) * rkTexCoord.X);
    const auto TexelY = static_cast<uint32>((mHeight - 1) * (1.f - std::fmod(rkTexCoord.Y, 1.f)));

    if (mTexelFormat == ETexelFormat::DXT1 && mBufferExists)
    {
        CMemoryInStream Buffer(mpImgDataBuffer, mImgDataSize, EEndian::SystemEndian);

        // 8 bytes per 4x4 16-pixel block, left-to-right top-to-bottom
        const uint32 BlockIdxX = TexelX / 4;
        const uint32 BlockIdxY = TexelY / 4;
        const uint32 BlocksPerRow = mWidth / 4;

        const uint32 BufferPos = (8 * BlockIdxX) + (8 * BlockIdxY * BlocksPerRow);
        Buffer.Seek(BufferPos, SEEK_SET);

        const uint16 PaletteA = Buffer.ReadUShort();
        const uint16 PaletteB = Buffer.ReadUShort();

        if (PaletteA > PaletteB)
        {
            // No palette colors have alpha
            return 1.f;
        }

        // We only care about alpha, which is only present on palette index 3.
        // We don't need to calculate/decode the actual palette colors.
        const uint32 BlockCol = (TexelX & 0xF) / 4;
        const uint32 BlockRow = (TexelY & 0xF) / 4;

        Buffer.Seek(BlockRow, SEEK_CUR);
        const uint8 Row = Buffer.ReadUByte();
        const uint8 Shift = static_cast<uint8>(6 - (BlockCol * 2));
        const uint8 PaletteIndex = (Row >> Shift) & 0x3;
        return (PaletteIndex == 3 ? 0.f : 1.f);
    }

    return 1.f;
}

bool CTexture::WriteDDS(IOutputStream& rOut)
{
    if (!rOut.IsValid()) return false;

    CopyGLBuffer();

    rOut.WriteFourCC(FOURCC('DDS ')); // "DDS " fourCC
    rOut.WriteULong(0x7C);            // dwSize
    rOut.WriteULong(0x21007);         // dwFlags
    rOut.WriteULong(mHeight);         // dwHeight
    rOut.WriteULong(mWidth);          // dwWidth
    rOut.WriteULong(mLinearSize);     // dwPitchOrLinearSize
    rOut.WriteULong(0);               // dwDepth
    rOut.WriteULong(mNumMipMaps - 1); // dwMipMapCount

    for (uint32 iRes = 0; iRes < 11; iRes++)
        rOut.WriteLong(0);           // dwReserved1[11]

    // DDS_PIXELFORMAT
    rOut.WriteLong(32); // DDS_PIXELFORMAT.dwSize
    uint32 PFFlags = 0, PFBpp = 0, PFRBitMask = 0, PFGBitMask = 0, PFBBitMask = 0, PFABitMask = 0;

    switch (mTexelFormat)
    {
    case ETexelFormat::Luminance:
        PFFlags = 0x20000;
        PFBpp = 0x8;
        PFRBitMask = 0xFF;
        break;
    case ETexelFormat::LuminanceAlpha:
        PFFlags = 0x20001;
        PFBpp = 0x10;
        PFRBitMask = 0x00FF;
        PFABitMask = 0xFF00;
        break;
    case ETexelFormat::RGBA4:
        PFFlags = 0x41;
        PFBpp = 0x10;
        PFRBitMask = 0x0F00;
        PFGBitMask = 0x00F0;
        PFBBitMask = 0x000F;
        PFABitMask = 0xF000;
        break;
    case ETexelFormat::RGB565:
        PFFlags = 0x40;
        PFBpp = 0x10;
        PFRBitMask = 0xF800;
        PFGBitMask = 0x7E0;
        PFBBitMask = 0x1F;
        break;
    case ETexelFormat::RGBA8:
        PFFlags = 0x41;
        PFBpp = 0x20;
        PFRBitMask = 0x00FF0000;
        PFGBitMask = 0x0000FF00;
        PFBBitMask = 0x000000FF;
        PFABitMask = 0xFF000000;
        break;
    case ETexelFormat::DXT1:
        PFFlags = 0x4;
        break;
    default:
        break;
    }

    rOut.WriteULong(PFFlags);    // DDS_PIXELFORMAT.dwFlags
    (mTexelFormat == ETexelFormat::DXT1) ? rOut.WriteFourCC(FOURCC('DXT1')) : rOut.WriteLong(0); // DDS_PIXELFORMAT.dwFourCC
    rOut.WriteULong(PFBpp);      // DDS_PIXELFORMAT.dwRGBBitCount
    rOut.WriteULong(PFRBitMask); // DDS_PIXELFORMAT.dwRBitMask
    rOut.WriteULong(PFGBitMask); // DDS_PIXELFORMAT.dwGBitMask
    rOut.WriteULong(PFBBitMask); // DDS_PIXELFORMAT.dwBBitMask
    rOut.WriteULong(PFABitMask); // DDS_PIXELFORMAT.dwABitMask

    rOut.WriteLong(0x401000); // dwCaps
    rOut.WriteLong(0);        // dwCaps2
    rOut.WriteLong(0);        // dwCaps3
    rOut.WriteLong(0);        // dwCaps4
    rOut.WriteLong(0);        // dwReserved2

    rOut.WriteBytes(mpImgDataBuffer, mImgDataSize); // Image data
    return true;
}

// ************ STATIC ************
uint32 CTexture::FormatBPP(ETexelFormat Format)
{
    switch (Format)
    {
    case ETexelFormat::GX_I4:          return 4;
    case ETexelFormat::GX_I8:          return 8;
    case ETexelFormat::GX_IA4:         return 8;
    case ETexelFormat::GX_IA8:         return 16;
    case ETexelFormat::GX_C4:          return 4;
    case ETexelFormat::GX_C8:          return 8;
    case ETexelFormat::GX_RGB565:      return 16;
    case ETexelFormat::GX_RGB5A3:      return 16;
    case ETexelFormat::GX_RGBA8:       return 32;
    case ETexelFormat::GX_CMPR:        return 4;
    case ETexelFormat::Luminance:      return 8;
    case ETexelFormat::LuminanceAlpha: return 16;
    case ETexelFormat::RGBA4:          return 16;
    case ETexelFormat::RGB565:         return 16;
    case ETexelFormat::RGBA8:          return 32;
    case ETexelFormat::DXT1:           return 4;
    default:                           return 0;
    }
}

// ************ PRIVATE ************
void CTexture::CalcLinearSize()
{
    const float BytesPerPixel = FormatBPP(mTexelFormat) / 8.f;
    mLinearSize = static_cast<uint32>(mWidth * mHeight * BytesPerPixel);
}

uint32 CTexture::CalcTotalSize() const
{
    const float BytesPerPixel = FormatBPP(mTexelFormat) / 8.f;
    uint32 MipW = mWidth, MipH = mHeight;
    uint32 Size = 0;

    for (uint32 iMip = 0; iMip < mNumMipMaps; iMip++)
    {
        Size += static_cast<uint32>(MipW * MipH * BytesPerPixel);
        MipW /= 2;
        MipH /= 2;
    }

    return Size;
}

void CTexture::CopyGLBuffer()
{
    if (!mGLBufferExists) return;

    // Clear existing buffer
    if (mBufferExists)
    {
        delete[] mpImgDataBuffer;
        mBufferExists = false;
        mpImgDataBuffer = nullptr;
        mImgDataSize = 0;
    }

    // Calculate buffer size
    mImgDataSize = CalcTotalSize();
    mpImgDataBuffer = new uint8[mImgDataSize];
    mBufferExists = true;

    // Get texture
    uint32 MipW = mWidth, MipH = mHeight, MipOffset = 0;
    const float BytesPerPixel = FormatBPP(mTexelFormat) / 8.f;

    const GLenum BindTarget = (mEnableMultisampling ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D);
    glBindTexture(BindTarget, mTextureID);

    for (uint32 iMip = 0; iMip < mNumMipMaps; iMip++)
    {
        void *pData = mpImgDataBuffer + MipOffset;

        glGetTexImage(BindTarget, iMip, GL_RGBA, GL_UNSIGNED_BYTE, pData);

        MipOffset += static_cast<uint32>(MipW * MipH * BytesPerPixel);
        MipW /= 2;
        MipH /= 2;
    }

    mTexelFormat = ETexelFormat::RGBA8;
    mLinearSize = mWidth * mHeight * 4;
}

void CTexture::DeleteBuffers()
{
    if (mBufferExists)
    {
        delete[] mpImgDataBuffer;
        mBufferExists = false;
        mpImgDataBuffer = nullptr;
        mImgDataSize = 0;
    }

    if (mGLBufferExists)
    {
        glDeleteTextures(1, &mTextureID);
        mGLBufferExists = false;
    }
}
