#include "CTexture.h"

CTexture::CTexture(CResourceEntry *pEntry /*= 0*/)
    : CResource(pEntry)
    , mTexelFormat(eRGBA8)
    , mSourceTexelFormat(eRGBA8)
    , mWidth(0)
    , mHeight(0)
    , mNumMipMaps(0)
    , mLinearSize(0)
    , mEnableMultisampling(false)
    , mBufferExists(false)
    , mpImgDataBuffer(nullptr)
    , mImgDataSize(0)
    , mGLBufferExists(false)
{
}

CTexture::CTexture(uint32 Width, uint32 Height)
    : mTexelFormat(eRGBA8)
    , mSourceTexelFormat(eRGBA8)
    , mWidth((uint16) Width)
    , mHeight((uint16) Height)
    , mNumMipMaps(1)
    , mLinearSize(Width * Height * 4)
    , mEnableMultisampling(false)
    , mBufferExists(false)
    , mpImgDataBuffer(nullptr)
    , mImgDataSize(0)
    , mGLBufferExists(false)
{
}

CTexture::~CTexture()
{
    DeleteBuffers();
}

bool CTexture::BufferGL()
{
    GLenum BindTarget = (mEnableMultisampling ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D);
    glGenTextures(1, &mTextureID);
    glBindTexture(BindTarget, mTextureID);

    GLenum GLFormat, GLType;
    bool IsCompressed = false;

    switch (mTexelFormat)
    {
        case eLuminance:
            GLFormat = GL_LUMINANCE;
            GLType = GL_UNSIGNED_BYTE;
            break;
        case eLuminanceAlpha:
            GLFormat = GL_LUMINANCE_ALPHA;
            GLType = GL_UNSIGNED_BYTE;
            break;
        case eRGB565:
            GLFormat = GL_RGB;
            GLType = GL_UNSIGNED_SHORT_5_6_5;
            break;
        case eRGBA4:
            GLFormat = GL_RGBA;
            GLType = GL_UNSIGNED_SHORT_4_4_4_4;
            break;
        case eRGBA8:
            GLFormat = GL_RGBA;
            GLType = GL_UNSIGNED_BYTE;
            break;
        case eDXT1:
            GLFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
            IsCompressed = true;
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
            glCompressedTexImage2D(BindTarget, iMip, GLFormat, MipW, MipH, 0, MipSize, pData);

        MipW /= 2;
        MipH /= 2;
        MipOffset += MipSize;
        MipSize /= 4;
    }

    glTexParameteri(BindTarget, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(BindTarget, GL_TEXTURE_MAX_LEVEL, mNumMipMaps - 1);

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

    GLenum BindTarget = (mEnableMultisampling ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D);
    glBindTexture(BindTarget, mTextureID);
}

void CTexture::Resize(uint32 Width, uint32 Height)
{
    if ((mWidth != Width) || (mHeight != Height))
    {
        DeleteBuffers();
        mWidth = (uint16) Width;
        mHeight = (uint16) Height;
        mNumMipMaps = 1;
        CalcLinearSize();
    }
}

float CTexture::ReadTexelAlpha(const CVector2f& rkTexCoord)
{
    // todo: support texel formats other than DXT1
    // DXT1 is definitely the most complicated one anyway; try reusing CTextureDecoder functions for other formats
    uint32 TexelX = (uint32) ((mWidth - 1) * rkTexCoord.X);
    uint32 TexelY = (uint32) ((mHeight - 1) * (1.f - fmodf(rkTexCoord.Y, 1.f)));

    if (mTexelFormat == eDXT1 && mBufferExists)
    {
        CMemoryInStream Buffer(mpImgDataBuffer, mImgDataSize, IOUtil::kSystemEndianness);

        // 8 bytes per 4x4 16-pixel block, left-to-right top-to-bottom
        uint32 BlockIdxX = TexelX / 4;
        uint32 BlockIdxY = TexelY / 4;
        uint32 BlocksPerRow = mWidth / 4;

        uint32 BufferPos = (8 * BlockIdxX) + (8 * BlockIdxY * BlocksPerRow);
        Buffer.Seek(BufferPos, SEEK_SET);

        uint16 PaletteA = Buffer.ReadShort();
        uint16 PaletteB = Buffer.ReadShort();

        if (PaletteA > PaletteB)
        {
            // No palette colors have alpha
            return 1.f;
        }

        // We only care about alpha, which is only present on palette index 3.
        // We don't need to calculate/decode the actual palette colors.
        uint32 BlockCol = (TexelX & 0xF) / 4;
        uint32 BlockRow = (TexelY & 0xF) / 4;

        Buffer.Seek(BlockRow, SEEK_CUR);
        uint8 Row = Buffer.ReadByte();
        uint8 Shift = (uint8) (6 - (BlockCol * 2));
        uint8 PaletteIndex = (Row >> Shift) & 0x3;
        return (PaletteIndex == 3 ? 0.f : 1.f);
    }

    return 1.f;
}

bool CTexture::WriteDDS(IOutputStream& rOut)
{
    if (!rOut.IsValid()) return false;

    CopyGLBuffer();

    rOut.WriteFourCC(FOURCC('DDS')); // "DDS " fourCC
    rOut.WriteLong(0x7C);            // dwSize
    rOut.WriteLong(0x21007);         // dwFlags
    rOut.WriteLong(mHeight);         // dwHeight
    rOut.WriteLong(mWidth);          // dwWidth
    rOut.WriteLong(mLinearSize);     // dwPitchOrLinearSize
    rOut.WriteLong(0);               // dwDepth
    rOut.WriteLong(mNumMipMaps - 1); // dwMipMapCount

    for (uint32 iRes = 0; iRes < 11; iRes++)
        rOut.WriteLong(0);           // dwReserved1[11]

    // DDS_PIXELFORMAT
    rOut.WriteLong(32); // DDS_PIXELFORMAT.dwSize
    uint32 PFFlags = 0, PFBpp = 0, PFRBitMask = 0, PFGBitMask = 0, PFBBitMask = 0, PFABitMask = 0;

    switch (mTexelFormat)
    {
        case eLuminance:
            PFFlags = 0x20000;
            PFBpp = 0x8;
            PFRBitMask = 0xFF;
            break;
        case eLuminanceAlpha:
            PFFlags = 0x20001;
            PFBpp = 0x10;
            PFRBitMask = 0x00FF;
            PFABitMask = 0xFF00;
            break;
        case eRGBA4:
            PFFlags = 0x41;
            PFBpp = 0x10;
            PFRBitMask = 0x0F00;
            PFGBitMask = 0x00F0;
            PFBBitMask = 0x000F;
            PFABitMask = 0xF000;
            break;
        case eRGB565:
            PFFlags = 0x40;
            PFBpp = 0x10;
            PFRBitMask = 0xF800;
            PFGBitMask = 0x7E0;
            PFBBitMask = 0x1F;
            break;
        case eRGBA8:
            PFFlags = 0x41;
            PFBpp = 0x20;
            PFRBitMask = 0x00FF0000;
            PFGBitMask = 0x0000FF00;
            PFBBitMask = 0x000000FF;
            PFABitMask = 0xFF000000;
            break;
        case eDXT1:
            PFFlags = 0x4;
            break;
    }

    rOut.WriteLong(PFFlags);    // DDS_PIXELFORMAT.dwFlags
    (mTexelFormat == eDXT1) ? rOut.WriteFourCC(FOURCC('DXT1')) : rOut.WriteLong(0); // DDS_PIXELFORMAT.dwFourCC
    rOut.WriteLong(PFBpp);      // DDS_PIXELFORMAT.dwRGBBitCount
    rOut.WriteLong(PFRBitMask); // DDS_PIXELFORMAT.dwRBitMask
    rOut.WriteLong(PFGBitMask); // DDS_PIXELFORMAT.dwGBitMask
    rOut.WriteLong(PFBBitMask); // DDS_PIXELFORMAT.dwBBitMask
    rOut.WriteLong(PFABitMask); // DDS_PIXELFORMAT.dwABitMask

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
    case eGX_I4:          return 4;
    case eGX_I8:          return 8;
    case eGX_IA4:         return 8;
    case eGX_IA8:         return 16;
    case eGX_C4:          return 4;
    case eGX_C8:          return 8;
    case eGX_RGB565:      return 16;
    case eGX_RGB5A3:      return 16;
    case eGX_RGBA8:       return 32;
    case eGX_CMPR:        return 4;
    case eLuminance:      return 8;
    case eLuminanceAlpha: return 16;
    case eRGBA4:          return 16;
    case eRGB565:         return 16;
    case eRGBA8:          return 32;
    case eDXT1:           return 4;
    default:              return 0;
    }
}

// ************ PRIVATE ************
void CTexture::CalcLinearSize()
{
    float BytesPerPixel = FormatBPP(mTexelFormat) / 8.f;
    mLinearSize = (uint32) (mWidth * mHeight * BytesPerPixel);
}

uint32 CTexture::CalcTotalSize()
{
    float BytesPerPixel = FormatBPP(mTexelFormat) / 8.f;
    uint32 MipW = mWidth, MipH = mHeight;
    uint32 Size = 0;

    for (uint32 iMip = 0; iMip < mNumMipMaps; iMip++)
    {
        Size += (uint32) (MipW * MipH * BytesPerPixel);
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
    float BytesPerPixel = FormatBPP(mTexelFormat) / 8.f;

    GLenum BindTarget = (mEnableMultisampling ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D);
    glBindTexture(BindTarget, mTextureID);

    for (uint32 iMip = 0; iMip < mNumMipMaps; iMip++)
    {
        void *pData = mpImgDataBuffer + MipOffset;

        glGetTexImage(BindTarget, iMip, GL_RGBA, GL_UNSIGNED_BYTE, pData);

        MipOffset += (uint32) (MipW * MipH * BytesPerPixel);
        MipW /= 2;
        MipH /= 2;
    }

    mTexelFormat = eRGBA8;
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
