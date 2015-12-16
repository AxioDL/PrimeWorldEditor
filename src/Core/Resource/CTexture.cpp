#include "CTexture.h"

CTexture::CTexture() : CResource()
{
    mTexelFormat = eRGBA8;
    mSourceTexelFormat = eRGBA8;
    mWidth = 0;
    mHeight = 0;
    mNumMipMaps = 0;
    mLinearSize = 0;

    mBufferExists = false;
    mImgDataBuffer = nullptr;
    mImgDataSize = 0;

    mGLBufferExists = false;
}

CTexture::CTexture(const CTexture& Source)
{
    mTexelFormat = Source.mTexelFormat;
    mSourceTexelFormat = Source.mSourceTexelFormat;
    mWidth = Source.mWidth;
    mHeight = Source.mHeight;
    mLinearSize = Source.mLinearSize;

    mBufferExists = Source.mBufferExists;
    mImgDataSize = Source.mImgDataSize;
    mImgDataBuffer = new u8[mImgDataSize];
    memcpy(mImgDataBuffer, Source.mImgDataBuffer, mImgDataSize);

    mGLBufferExists = false;
}

CTexture::CTexture(u32 Width, u32 Height)
{
    mTexelFormat = eRGBA8;
    mSourceTexelFormat = eRGBA8;
    mWidth = (u16) Width;
    mHeight = (u16) Height;
    mNumMipMaps = 1;
    mLinearSize = Width * Height * 4;

    mBufferExists = false;
    mImgDataBuffer = nullptr;
    mImgDataSize = 0;

    mGLBufferExists = false;
}

CTexture::~CTexture()
{
    DeleteBuffers();
}

bool CTexture::BufferGL()
{
    glGenTextures(1, &mTextureID);
    glBindTexture(GL_TEXTURE_2D, mTextureID);

    GLenum GLFormat, GLType;
    bool IsCompressed = false;

    switch (mTexelFormat) {

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
    u32 MipSize = mLinearSize;
    u32 MipOffset = 0;
    u16 MipW = mWidth, MipH = mHeight;

    for (u32 iMip = 0; iMip < mNumMipMaps; iMip++)
    {
        GLvoid *pData = (mBufferExists) ? (mImgDataBuffer + MipOffset) : NULL;

        if (!IsCompressed)
            glTexImage2D(GL_TEXTURE_2D, iMip, GLFormat, MipW, MipH, 0, GLFormat, GLType, pData);
        else
            glCompressedTexImage2D(GL_TEXTURE_2D, iMip, GLFormat, MipW, MipH, 0, MipSize, pData);

        MipW /= 2;
        MipH /= 2;
        MipOffset += MipSize;
        MipSize /= 4;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mNumMipMaps - 1);

    // Linear filtering on mipmaps:
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    // Anisotropic filtering:
    float MaxAnisotropy;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &MaxAnisotropy);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, MaxAnisotropy);

    mGLBufferExists = true;
    return true;
}

void CTexture::Bind(u32 GLTextureUnit)
{
    glActiveTexture(GL_TEXTURE0 + GLTextureUnit);

    if (!mGLBufferExists)
        BufferGL();

    glBindTexture(GL_TEXTURE_2D, mTextureID);
}

void CTexture::Resize(u32 Width, u32 Height)
{
    if ((mWidth != Width) || (mHeight != Height))
    {
        DeleteBuffers();
        mWidth = (u16) Width;
        mHeight = (u16) Height;
        mNumMipMaps = 1;
    }
}

float CTexture::ReadTexelAlpha(const CVector2f& TexCoord)
{
    // todo: support texel formats other than DXT1
    // DXT1 is definitely the most complicated one anyway; try reusing CTextureDecoder functions for other formats
    u32 TexelX = (u32) ((mWidth - 1) * TexCoord.x);
    u32 TexelY = (u32) ((mHeight - 1) * (1.f - fmodf(TexCoord.y, 1.f)));

    if (mTexelFormat == eDXT1 && mBufferExists)
    {
        CMemoryInStream Buffer(mImgDataBuffer, mImgDataSize, IOUtil::kSystemEndianness);

        // 8 bytes per 4x4 16-pixel block, left-to-right top-to-bottom
        u32 BlockIdxX = TexelX / 4;
        u32 BlockIdxY = TexelY / 4;
        u32 BlocksPerRow = mWidth / 4;

        u32 BufferPos = (8 * BlockIdxX) + (8 * BlockIdxY * BlocksPerRow);
        Buffer.Seek(BufferPos, SEEK_SET);

        u16 PaletteA = Buffer.ReadShort();
        u16 PaletteB = Buffer.ReadShort();

        if (PaletteA > PaletteB)
        {
            // No palette colors have alpha
            return 1.f;
        }

        // We only care about alpha, which is only present on palette index 3.
        // We don't need to calculate/decode the actual palette colors.
        u32 BlockCol = (TexelX & 0xF) / 4;
        u32 BlockRow = (TexelY & 0xF) / 4;

        Buffer.Seek(BlockRow, SEEK_CUR);
        u8 Row = Buffer.ReadByte();
        u8 Shift = (u8) (6 - (BlockCol * 2));
        u8 PaletteIndex = (Row >> Shift) & 0x3;
        return (PaletteIndex == 3 ? 0.f : 1.f);
    }

    return 1.f;
}

bool CTexture::WriteDDS(IOutputStream& out)
{
    if (!out.IsValid()) return false;

    CopyGLBuffer();

    out.WriteString("DDS ", 4);     // "DDS " fourCC
    out.WriteLong(0x7C);            // dwSize
    out.WriteLong(0x21007);         // dwFlags
    out.WriteLong(mHeight);         // dwHeight
    out.WriteLong(mWidth);          // dwWidth
    out.WriteLong(mLinearSize);     // dwPitchOrLinearSize
    out.WriteLong(0);               // dwDepth
    out.WriteLong(mNumMipMaps - 1); // dwMipMapCount

    for (u32 i = 0; i < 11; i++)
        out.WriteLong(0);           // dwReserved1[11]

    // DDS_PIXELFORMAT
    out.WriteLong(32); // DDS_PIXELFORMAT.dwSize

    u32 pfFlags = 0, pfBpp = 0, pfRBitMask = 0, pfGBitMask = 0, pfBBitMask = 0, pfABitMask = 0;
    switch (mTexelFormat) {
        case eLuminance:
            pfFlags = 0x20000;
            pfBpp = 0x8;
            pfRBitMask = 0xFF;
            break;
        case eLuminanceAlpha:
            pfFlags = 0x20001;
            pfBpp = 0x10;
            pfRBitMask = 0x00FF;
            pfABitMask = 0xFF00;
            break;
        case eRGBA4:
            pfFlags = 0x41;
            pfBpp = 0x10;
            pfRBitMask = 0x0F00;
            pfGBitMask = 0x00F0;
            pfBBitMask = 0x000F;
            pfABitMask = 0xF000;
            break;
        case eRGB565:
            pfFlags = 0x40;
            pfBpp = 0x10;
            pfRBitMask = 0xF800;
            pfGBitMask = 0x7E0;
            pfBBitMask = 0x1F;
            break;
        case eRGBA8:
            pfFlags = 0x41;
            pfBpp = 0x20;
            pfRBitMask = 0x00FF0000;
            pfGBitMask = 0x0000FF00;
            pfBBitMask = 0x000000FF;
            pfABitMask = 0xFF000000;
            break;
        case eDXT1:
            pfFlags = 0x4;
            break;
    }

    out.WriteLong(pfFlags);    // DDS_PIXELFORMAT.dwFlags
    (mTexelFormat == eDXT1) ? out.WriteString("DXT1", 4) : out.WriteLong(0); // DDS_PIXELFORMAT.dwFourCC
    out.WriteLong(pfBpp);      // DDS_PIXELFORMAT.dwRGBBitCount
    out.WriteLong(pfRBitMask); // DDS_PIXELFORMAT.dwRBitMask
    out.WriteLong(pfGBitMask); // DDS_PIXELFORMAT.dwGBitMask
    out.WriteLong(pfBBitMask); // DDS_PIXELFORMAT.dwBBitMask
    out.WriteLong(pfABitMask); // DDS_PIXELFORMAT.dwABitMask

    out.WriteLong(0x401000); // dwCaps
    out.WriteLong(0);        // dwCaps2
    out.WriteLong(0);        // dwCaps3
    out.WriteLong(0);        // dwCaps4
    out.WriteLong(0);        // dwReserved2

    out.WriteBytes(mImgDataBuffer, mImgDataSize); // Image data
    return true;
}

// ************ STATIC ************
u32 CTexture::FormatBPP(ETexelFormat Format)
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
    mLinearSize = (u32) (mWidth * mHeight * BytesPerPixel);
}

u32 CTexture::CalcTotalSize()
{
    float BytesPerPixel = FormatBPP(mTexelFormat) / 8.f;
    u32 MipW = mWidth, MipH = mHeight;
    u32 Size = 0;

    for (u32 iMip = 0; iMip < mNumMipMaps; iMip++)
    {
        Size += (u32) (MipW * MipH * BytesPerPixel);
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
        delete[] mImgDataBuffer;
        mBufferExists = false;
        mImgDataBuffer = nullptr;
        mImgDataSize = 0;
    }

    // Calculate buffer size
    mImgDataSize = CalcTotalSize();
    mImgDataBuffer = new u8[mImgDataSize];
    mBufferExists = true;

    // Get texture
    u32 MipW = mWidth, MipH = mHeight, MipOffset = 0;
    float BytesPerPixel = FormatBPP(mTexelFormat) / 8.f;

    glBindTexture(GL_TEXTURE_2D, mTextureID);

    for (u32 iMip = 0; iMip < mNumMipMaps; iMip++)
    {
        void *pData = mImgDataBuffer + MipOffset;

        glGetTexImage(GL_TEXTURE_2D, iMip, GL_RGBA, GL_UNSIGNED_BYTE, pData);

        MipOffset += (u32) (MipW * MipH * BytesPerPixel);
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
        delete[] mImgDataBuffer;
        mBufferExists = false;
        mImgDataBuffer = nullptr;
        mImgDataSize = 0;
    }

    if (mGLBufferExists)
    {
        glDeleteTextures(1, &mTextureID);
        mGLBufferExists = false;
    }
}
