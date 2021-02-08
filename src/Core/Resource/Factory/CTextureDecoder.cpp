#include "CTextureDecoder.h"
#include <Common/Log.h>
#include <Common/CColor.h>
#include <array>

// A cleanup is warranted at some point. Trying to support both partial + full decode ended up really messy.
namespace
{
// Number of pixels * this = number of bytes
constexpr std::array gskPixelsToBytes{
    2.f,
    2.f,
    2.f,
    2.f,
    4.f,
    4.f,
    0.f,
    2.f,
    4.f,
    4.f,
    0.5f,
};

// Bits per pixel for each GX texture format
constexpr std::array gskSourceBpp{
    4U,
    8U,
    8U,
    16U,
    4U,
    8U,
    16U,
    16U,
    16U,
    32U,
    4U,
};

// Bits per pixel for each GX texture format when decoded
constexpr std::array gskOutputBpp{
    16U,
    16U,
    16U,
    16U,
    16U,
    16U,
    16U,
    16U,
    32U,
    32U,
    4U,
};

// Size of one pixel in output data in bytes
constexpr std::array gskOutputPixelStride{
    2U,
    2U,
    2U,
    2U,
    2U,
    2U,
    2U,
    2U,
    4U,
    4U,
    8U,
};

// Block width for each GX texture format
constexpr std::array gskBlockWidth{
    8U,
    8U,
    8U,
    4U,
    8U,
    8U,
    4U,
    4U,
    4U,
    4U,
    2U,
};

// Block height for each GX texture format
constexpr std::array gskBlockHeight{
    8U,
    4U,
    4U,
    4U,
    8U,
    4U,
    4U,
    4U,
    4U,
    4U,
    2U,
};

constexpr uint8 Extend3to8(uint8 In)
{
    In &= 0x7;
    return (In << 5) | (In << 2) | (In >> 1);
}

constexpr uint8 Extend4to8(uint8 In)
{
    In &= 0xF;
    return (In << 4) | In;
}

constexpr uint8 Extend5to8(uint8 In)
{
    In &= 0x1F;
    return (In << 3) | (In >> 2);
}

constexpr uint8 Extend6to8(uint8 In)
{
    In &= 0x3F;
    return (In << 2) | (In >> 4);
}

constexpr uint32 CalculateShiftForMask(uint32 BitMask)
{
    uint32 Shift = 32;

    while (BitMask)
    {
        BitMask <<= 1;
        Shift--;
    }
    return Shift;
}

constexpr uint32 CalculateMaskBitCount(uint32 BitMask)
{
    uint32 Count = 0;

    while (BitMask)
    {
        if (BitMask & 0x1)
            Count++;
        BitMask >>= 1;
    }
    return Count;
}
} // Anonymous namespace

CTextureDecoder::CTextureDecoder()
{
}

CTextureDecoder::~CTextureDecoder() = default;

std::unique_ptr<CTexture> CTextureDecoder::CreateTexture()
{
    auto pTex = std::make_unique<CTexture>(mpEntry);
    pTex->mSourceTexelFormat = mTexelFormat;
    pTex->mWidth = mWidth;
    pTex->mHeight = mHeight;
    pTex->mNumMipMaps = mNumMipMaps;
    pTex->mLinearSize = static_cast<uint32>(mWidth * mHeight * gskPixelsToBytes[static_cast<size_t>(mTexelFormat)]);
    pTex->mpImgDataBuffer = mpDataBuffer;
    pTex->mImgDataSize = mDataBufferSize;
    pTex->mBufferExists = true;

    switch (mTexelFormat)
    {
        case ETexelFormat::GX_I4:
        case ETexelFormat::GX_I8:
        case ETexelFormat::GX_IA4:
        case ETexelFormat::GX_IA8:
            pTex->mTexelFormat = ETexelFormat::LuminanceAlpha;
            break;
        case ETexelFormat::GX_RGB565:
            pTex->mTexelFormat = ETexelFormat::RGB565;
            break;
        case ETexelFormat::GX_C4:
        case ETexelFormat::GX_C8:
            if (mPaletteFormat == EGXPaletteFormat::IA8)    pTex->mTexelFormat = ETexelFormat::LuminanceAlpha;
            if (mPaletteFormat == EGXPaletteFormat::RGB565) pTex->mTexelFormat = ETexelFormat::RGB565;
            if (mPaletteFormat == EGXPaletteFormat::RGB5A3) pTex->mTexelFormat = ETexelFormat::RGBA8;
            break;
        case ETexelFormat::GX_RGB5A3:
        case ETexelFormat::GX_RGBA8:
            pTex->mTexelFormat = ETexelFormat::RGBA8;
            break;
        case ETexelFormat::GX_CMPR:
            pTex->mTexelFormat = ETexelFormat::DXT1;
            break;
        case ETexelFormat::DXT1:
            pTex->mTexelFormat = ETexelFormat::DXT1;
            pTex->mLinearSize = mWidth * mHeight / 2;
            break;
        default:
            pTex->mTexelFormat = mTexelFormat;
            break;
    }

    return pTex;
}

// ************ STATIC ************
std::unique_ptr<CTexture> CTextureDecoder::LoadTXTR(IInputStream& rTXTR, CResourceEntry *pEntry)
{
    CTextureDecoder Decoder;
    Decoder.mpEntry = pEntry;
    Decoder.ReadTXTR(rTXTR);
    Decoder.PartialDecodeGXTexture(rTXTR);
    return Decoder.CreateTexture();
}

std::unique_ptr<CTexture> CTextureDecoder::DoFullDecode(IInputStream& rTXTR, CResourceEntry *pEntry)
{
    CTextureDecoder Decoder;
    Decoder.mpEntry = pEntry;
    Decoder.ReadTXTR(rTXTR);
    Decoder.FullDecodeGXTexture(rTXTR);

    auto pTexture = Decoder.CreateTexture();
    pTexture->mTexelFormat = ETexelFormat::RGBA8;
    return pTexture;
}

std::unique_ptr<CTexture> CTextureDecoder::LoadDDS(IInputStream& rDDS, CResourceEntry *pEntry)
{
    CTextureDecoder Decoder;
    Decoder.mpEntry = pEntry;
    Decoder.ReadDDS(rDDS);
    Decoder.DecodeDDS(rDDS);
    return Decoder.CreateTexture();
}

CTexture* CTextureDecoder::DoFullDecode(CTexture* /*pTexture*/)
{
    return nullptr;
}

// ************ READ ************
void CTextureDecoder::ReadTXTR(IInputStream& rTXTR)
{
    // Read TXTR header
    mTexelFormat = ETexelFormat(rTXTR.ReadLong());
    mWidth = rTXTR.ReadUShort();
    mHeight = rTXTR.ReadUShort();
    mNumMipMaps = rTXTR.ReadULong();

    // For C4 and C8 images, read palette
    if (mTexelFormat == ETexelFormat::GX_C4 || mTexelFormat == ETexelFormat::GX_C8)
    {
        mHasPalettes = true;
        mPaletteFormat = EGXPaletteFormat(rTXTR.ReadLong());
        rTXTR.Seek(0x4, SEEK_CUR);

        const uint32 PaletteEntryCount = (mTexelFormat == ETexelFormat::GX_C4) ? 16 : 256;
        mPalettes.resize(PaletteEntryCount * 2);
        rTXTR.ReadBytes(mPalettes.data(), mPalettes.size());

        mPaletteInput.SetData(mPalettes.data(), mPalettes.size(), EEndian::BigEndian);
    }
    else
    {
        mHasPalettes = false;
    }
}

void CTextureDecoder::ReadDDS(IInputStream& rDDS)
{
    // Header
    const CFourCC Magic(rDDS);
    if (Magic != FOURCC('DDS '))
    {
        errorf("%s: Invalid DDS magic: 0x%08X", *rDDS.GetSourceString(), Magic.ToLong());
        return;
    }

    const uint32 ImageDataStart = rDDS.Tell() + rDDS.ReadULong();
    rDDS.Seek(0x4, SEEK_CUR); // Skipping flags
    mHeight = static_cast<uint16>(rDDS.ReadULong());
    mWidth = static_cast<uint16>(rDDS.ReadULong());
    rDDS.Seek(0x8, SEEK_CUR); // Skipping linear size + depth
    mNumMipMaps = rDDS.ReadULong() + 1; // DDS doesn't seem to count the first mipmap
    rDDS.Seek(0x2C, SEEK_CUR); // Skipping reserved

    // Pixel Format
    rDDS.Seek(0x4, SEEK_CUR); // Skipping size
    mDDSInfo.Flags = rDDS.ReadULong();
    const CFourCC Format(rDDS);

    if (Format == "DXT1")      mDDSInfo.Format = SDDSInfo::DXT1;
    else if (Format == "DXT2") mDDSInfo.Format = SDDSInfo::DXT2;
    else if (Format == "DXT3") mDDSInfo.Format = SDDSInfo::DXT3;
    else if (Format == "DXT4") mDDSInfo.Format = SDDSInfo::DXT4;
    else if (Format == "DXT5") mDDSInfo.Format = SDDSInfo::DXT5;
    else
    {
        mDDSInfo.Format = SDDSInfo::RGBA;
        mDDSInfo.BitCount = rDDS.ReadULong();
        mDDSInfo.RBitMask = rDDS.ReadULong();
        mDDSInfo.GBitMask = rDDS.ReadULong();
        mDDSInfo.BBitMask = rDDS.ReadULong();
        mDDSInfo.ABitMask = rDDS.ReadULong();
        mDDSInfo.RShift = CalculateShiftForMask(mDDSInfo.RBitMask);
        mDDSInfo.GShift = CalculateShiftForMask(mDDSInfo.GBitMask);
        mDDSInfo.BShift = CalculateShiftForMask(mDDSInfo.BBitMask);
        mDDSInfo.AShift = CalculateShiftForMask(mDDSInfo.ABitMask);
        mDDSInfo.RSize = CalculateMaskBitCount(mDDSInfo.RBitMask);
        mDDSInfo.GSize = CalculateMaskBitCount(mDDSInfo.GBitMask);
        mDDSInfo.BSize = CalculateMaskBitCount(mDDSInfo.BBitMask);
        mDDSInfo.ASize = CalculateMaskBitCount(mDDSInfo.ABitMask);
    }

    // Skip the rest
    rDDS.Seek(ImageDataStart, SEEK_SET);
}

// ************ DECODE ************
void CTextureDecoder::PartialDecodeGXTexture(IInputStream& TXTR)
{
    // TODO: This function doesn't handle very small mipmaps correctly.
    // The format applies padding when the size of a mipmap is less than the block size for that format.
    // The decode needs to be adjusted to account for the padding and skip over it (since we don't have padding in OpenGL).

    // Get image data size, create output buffer
    const uint32 ImageStart = TXTR.Tell();
    TXTR.Seek(0x0, SEEK_END);
    const uint32 ImageSize = TXTR.Tell() - ImageStart;
    TXTR.Seek(ImageStart, SEEK_SET);

    mDataBufferSize = ImageSize * (gskOutputBpp[static_cast<size_t>(mTexelFormat)] / gskSourceBpp[static_cast<size_t>(mTexelFormat)]);
    if (mHasPalettes && mPaletteFormat == EGXPaletteFormat::RGB5A3)
        mDataBufferSize *= 2;
    mpDataBuffer = new uint8[mDataBufferSize];

    CMemoryOutStream Out(mpDataBuffer, mDataBufferSize, EEndian::SystemEndian);

    // Initializing more stuff before we start the mipmap loop
    uint32 MipW = mWidth;
    uint32 MipH = mHeight;
    uint32 MipOffset = 0;

    const uint32 BWidth = gskBlockWidth[static_cast<size_t>(mTexelFormat)];
    const uint32 BHeight = gskBlockHeight[static_cast<size_t>(mTexelFormat)];

    uint32 PixelStride = gskOutputPixelStride[static_cast<size_t>(mTexelFormat)];
    if (mHasPalettes && mPaletteFormat == EGXPaletteFormat::RGB5A3)
        PixelStride = 4;

    // With CMPR, we're using a little trick.
    // CMPR stores pixels in 8x8 blocks, with four 4x4 subblocks.
    // An easy way to convert it is to pretend each block is 2x2 and each subblock is one pixel.
    // So to do that we need to calculate the "new" dimensions of the image, 1/4 the size of the original.
    if (mTexelFormat == ETexelFormat::GX_CMPR) {
        MipW /= 4;
        MipH /= 4;
    }

    // This value set to true if we hit the end of the file earlier than expected.
    // This is necessary due to a mistake Retro made in their cooker for I8 textures where very small mipmaps are cut off early, resulting in an out-of-bounds memory access.
    // This affects one texture that I know of - Echoes 3bb2c034.TXTR
    bool BreakEarly = false;

    for (uint32 iMip = 0; iMip < mNumMipMaps; iMip++)
    {
        if (MipW < BWidth)
            MipW = BWidth;

        if (MipH < BHeight)
            MipH = BHeight;

        for (uint32 iBlockY = 0; iBlockY < MipH; iBlockY += BHeight)
        {
            for (uint32 iBlockX = 0; iBlockX < MipW; iBlockX += BWidth)
            {
                for (uint32 iImgY = iBlockY; iImgY < iBlockY + BHeight; iImgY++)
                {
                    for (uint32 iImgX = iBlockX; iImgX < iBlockX + BWidth; iImgX++)
                    {
                        const uint32 DstPos = ((iImgY * MipW) + iImgX) * PixelStride;
                        Out.Seek(MipOffset + DstPos, SEEK_SET);

                        if (mTexelFormat == ETexelFormat::GX_I4)          ReadPixelsI4(TXTR, Out);
                        else if (mTexelFormat == ETexelFormat::GX_I8)     ReadPixelI8(TXTR, Out);
                        else if (mTexelFormat == ETexelFormat::GX_IA4)    ReadPixelIA4(TXTR, Out);
                        else if (mTexelFormat == ETexelFormat::GX_IA8)    ReadPixelIA8(TXTR, Out);
                        else if (mTexelFormat == ETexelFormat::GX_C4)     ReadPixelsC4(TXTR, Out);
                        else if (mTexelFormat == ETexelFormat::GX_C8)     ReadPixelC8(TXTR, Out);
                        else if (mTexelFormat == ETexelFormat::GX_RGB565) ReadPixelRGB565(TXTR, Out);
                        else if (mTexelFormat == ETexelFormat::GX_RGB5A3) ReadPixelRGB5A3(TXTR, Out);
                        else if (mTexelFormat == ETexelFormat::GX_RGBA8)  ReadPixelRGBA8(TXTR, Out);
                        else if (mTexelFormat == ETexelFormat::GX_CMPR)   ReadSubBlockCMPR(TXTR, Out);

                        // I4 and C4 have 4bpp images, so I'm forced to read two pixels at a time.
                        if (mTexelFormat == ETexelFormat::GX_I4 || mTexelFormat == ETexelFormat::GX_C4)
                            iImgX++;

                        // Check if we're at the end of the file.
                        if (TXTR.EoF())
                            BreakEarly = true;
                    }
                    if (BreakEarly)
                        break;
                }

                if (mTexelFormat == ETexelFormat::GX_RGBA8)
                    TXTR.Seek(0x20, SEEK_CUR);

                if (BreakEarly)
                    break;
            }

            if (BreakEarly)
                break;
        }

        uint32 MipSize = static_cast<uint32>(MipW * MipH * gskPixelsToBytes[static_cast<size_t>(mTexelFormat)]);
        if (mTexelFormat == ETexelFormat::GX_CMPR)
        {
            // Since we're pretending the image is 1/4 its actual size, we have to multiply the size by 16 to get the correct offset
            MipSize *= 16;
        }

        MipOffset += MipSize;
        MipW /= 2;
        MipH /= 2;

        if (BreakEarly)
            break;
    }
}

void CTextureDecoder::FullDecodeGXTexture(IInputStream& rTXTR)
{
    // Get image data size, create output buffer
    const uint32 ImageStart = rTXTR.Tell();
    rTXTR.Seek(0x0, SEEK_END);
    const uint32 ImageSize = rTXTR.Tell() - ImageStart;
    rTXTR.Seek(ImageStart, SEEK_SET);

    mDataBufferSize = ImageSize * (32 / gskSourceBpp[static_cast<size_t>(mTexelFormat)]);
    mpDataBuffer = new uint8[mDataBufferSize];

    CMemoryOutStream Out(mpDataBuffer, mDataBufferSize, EEndian::SystemEndian);

    // Initializing more stuff before we start the mipmap loop
    uint32 MipW = mWidth;
    uint32 MipH = mHeight;
    uint32 MipOffset = 0;

    const uint32 BWidth = gskBlockWidth[static_cast<size_t>(mTexelFormat)];
    const uint32 BHeight = gskBlockHeight[static_cast<size_t>(mTexelFormat)];

    // With CMPR, we're using a little trick.
    // CMPR stores pixels in 8x8 blocks, with four 4x4 subblocks.
    // An easy way to convert it is to pretend each block is 2x2 and each subblock is one pixel.
    // So to do that we need to calculate the "new" dimensions of the image, 1/4 the size of the original.
    if (mTexelFormat == ETexelFormat::GX_CMPR)
    {
        MipW /= 4;
        MipH /= 4;
    }

    for (uint32 iMip = 0; iMip < mNumMipMaps; iMip++) {
        for (uint32 iBlockY = 0; iBlockY < MipH; iBlockY += BHeight) {
            for (uint32 iBlockX = 0; iBlockX < MipW; iBlockX += BWidth) {
                for (uint32 iImgY = iBlockY; iImgY < iBlockY + BHeight; iImgY++) {
                    for (uint32 iImgX = iBlockX; iImgX < iBlockX + BWidth; iImgX++) {
                        const uint32 DstPos = (mTexelFormat == ETexelFormat::GX_CMPR) ? ((iImgY * (MipW * 4)) + iImgX) * 16 : ((iImgY * MipW) + iImgX) * 4;
                        Out.Seek(MipOffset + DstPos, SEEK_SET);

                        // I4/C4/CMPR require reading more than one pixel at a time
                        if (mTexelFormat == ETexelFormat::GX_I4)
                        {
                            const uint8 Byte = rTXTR.ReadUByte();
                            Out.WriteLong(DecodePixelI4(Byte, 0).ToLongARGB());
                            Out.WriteLong(DecodePixelI4(Byte, 1).ToLongARGB());
                        }
                        else if (mTexelFormat == ETexelFormat::GX_C4)
                        {
                            const uint8 Byte = rTXTR.ReadUByte();
                            Out.WriteLong(DecodePixelC4(Byte, 0, mPaletteInput).ToLongARGB());
                            Out.WriteLong(DecodePixelC4(Byte, 1, mPaletteInput).ToLongARGB());
                        }
                        else if (mTexelFormat == ETexelFormat::GX_CMPR)
                        {
                            DecodeSubBlockCMPR(rTXTR, Out, static_cast<uint16>(MipW * 4));
                        }
                        else
                        {
                            CColor Pixel;

                            if (mTexelFormat == ETexelFormat::GX_I8)          Pixel = DecodePixelI8(rTXTR.ReadByte());
                            else if (mTexelFormat == ETexelFormat::GX_IA4)    Pixel = DecodePixelIA4(rTXTR.ReadByte());
                            else if (mTexelFormat == ETexelFormat::GX_IA8)    Pixel = DecodePixelIA8(rTXTR.ReadShort());
                            else if (mTexelFormat == ETexelFormat::GX_C8)     Pixel = DecodePixelC8(rTXTR.ReadByte(), mPaletteInput);
                            else if (mTexelFormat == ETexelFormat::GX_RGB565) Pixel = DecodePixelRGB565(rTXTR.ReadShort());
                            else if (mTexelFormat == ETexelFormat::GX_RGB5A3) Pixel = DecodePixelRGB5A3(rTXTR.ReadShort());
                            else if (mTexelFormat == ETexelFormat::GX_RGBA8)  Pixel = CColor(rTXTR, true);

                            Out.WriteLong(Pixel.ToLongARGB());
                        }
                    }
                }
                if (mTexelFormat == ETexelFormat::GX_RGBA8)
                    rTXTR.Seek(0x20, SEEK_CUR);
            }
        }

        uint32 MipSize = MipW * MipH * 4;
        if (mTexelFormat == ETexelFormat::GX_CMPR)
            MipSize *= 16;

        MipOffset += MipSize;
        MipW /= 2;
        MipH /= 2;

        if (MipW < BWidth)
            MipW = BWidth;

        if (MipH < BHeight)
            MipH = BHeight;
    }
}

void CTextureDecoder::DecodeDDS(IInputStream& rDDS)
{
    // Get image data size, create output buffer
    const uint32 ImageStart = rDDS.Tell();
    rDDS.Seek(0x0, SEEK_END);
    const uint32 ImageSize = rDDS.Tell() - ImageStart;
    rDDS.Seek(ImageStart, SEEK_SET);

    mDataBufferSize = ImageSize;
    if (mDDSInfo.Format == SDDSInfo::DXT1)
        mDataBufferSize *= 8;
    else if (mDDSInfo.Format == SDDSInfo::RGBA)
        mDataBufferSize *= (32 / mDDSInfo.BitCount);
    else
        mDataBufferSize *= 4;
    mpDataBuffer = new uint8[mDataBufferSize];

    CMemoryOutStream Out(mpDataBuffer, mDataBufferSize, EEndian::SystemEndian);

    // Initializing more stuff before we start the mipmap loop
    uint32 MipW = mWidth;
    uint32 MipH = mHeight;
    uint32 MipOffset = 0;

    uint32 BPP;
    switch (mDDSInfo.Format)
    {
    case SDDSInfo::RGBA:
        BPP = mDDSInfo.BitCount;
        break;
    case SDDSInfo::DXT1:
        BPP = 4;
        break;
    case SDDSInfo::DXT2:
    case SDDSInfo::DXT3:
    case SDDSInfo::DXT4:
    case SDDSInfo::DXT5:
        BPP = 8;
        break;
    }

    // For DXT* decodes we can use the same trick as CMPR
    if (mDDSInfo.Format != SDDSInfo::RGBA && mDDSInfo.Format != SDDSInfo::DXT1)
    {
        MipW /= 4;
        MipH /= 4;
    }

    for (uint32 iMip = 0; iMip < mNumMipMaps; iMip++)
    {
        // For DXT1 we can copy the image data as-is to load it
        if (mDDSInfo.Format == SDDSInfo::DXT1)
        {
            Out.Seek(MipOffset, SEEK_SET);
            const uint32 MipSize = MipW * MipH / 2;
            std::vector<uint8> MipBuffer(MipSize);
            rDDS.ReadBytes(MipBuffer.data(), MipBuffer.size());
            Out.WriteBytes(MipBuffer.data(), MipBuffer.size());
            MipOffset += MipSize;

            MipW /= 2;
            MipH /= 2;

            if (MipW % 4)
                MipW += 4 - (MipW % 4);

            if (MipH % 4)
                MipH += 4 - (MipH % 4);
        }
        else // Otherwise we do a full decode to RGBA8
        {
            for (uint32 Y = 0; Y < MipH; Y++)
            {
                for (uint32 X = 0; X < MipW; X++)
                {
                    uint32 OutPos = MipOffset;

                    if (mDDSInfo.Format == SDDSInfo::RGBA)
                    {
                        OutPos += ((Y * MipW) + X) * 4;
                        Out.Seek(OutPos, SEEK_SET);

                        const CColor Pixel = DecodeDDSPixel(rDDS);
                        Out.WriteLong(Pixel.ToLongARGB());
                    }
                    else
                    {
                        OutPos += ((Y * (MipW * 4)) + X) * 16;
                        Out.Seek(OutPos, SEEK_SET);

                        if (mDDSInfo.Format == SDDSInfo::DXT1)
                            DecodeBlockBC1(rDDS, Out, MipW * 4);
                        else if (mDDSInfo.Format == SDDSInfo::DXT2 || mDDSInfo.Format == SDDSInfo::DXT3)
                            DecodeBlockBC2(rDDS, Out, MipW * 4);
                        else if (mDDSInfo.Format == SDDSInfo::DXT4 || mDDSInfo.Format == SDDSInfo::DXT5)
                            DecodeBlockBC3(rDDS, Out, MipW * 4);
                    }
                }
            }

            uint32 MipSize = (mWidth * mHeight) * 4;
            if (mDDSInfo.Format != SDDSInfo::RGBA)
                MipSize *= 16;
            MipOffset += MipSize;

            MipW /= 2;
            MipH /= 2;
        }
    }

    if (mDDSInfo.Format == SDDSInfo::DXT1)
        mTexelFormat = ETexelFormat::DXT1;
    else
        mTexelFormat = ETexelFormat::GX_RGBA8;
}

// ************ READ PIXELS (PARTIAL DECODE) ************
void CTextureDecoder::ReadPixelsI4(IInputStream& rSrc, IOutputStream& rDst)
{
    const uint8 Pixels = rSrc.ReadByte();
    rDst.WriteUByte(Extend4to8(Pixels >> 4));
    rDst.WriteUByte(Extend4to8(Pixels >> 4));
    rDst.WriteUByte(Extend4to8(Pixels));
    rDst.WriteUByte(Extend4to8(Pixels));
}

void CTextureDecoder::ReadPixelI8(IInputStream& rSrc, IOutputStream& rDst)
{
    const uint8 Pixel = rSrc.ReadUByte();
    rDst.WriteUByte(Pixel);
    rDst.WriteUByte(Pixel);
}

void CTextureDecoder::ReadPixelIA4(IInputStream& rSrc, IOutputStream& rDst)
{
    // this can be left as-is for DDS conversion, but opengl doesn't support two components in one byte...
    const uint8 Byte = rSrc.ReadUByte();
    const uint8 Alpha = Extend4to8(Byte >> 4);
    const uint8 Lum = Extend4to8(Byte);
    rDst.WriteShort((Lum << 8) | Alpha);
}

void CTextureDecoder::ReadPixelIA8(IInputStream& rSrc, IOutputStream& rDst)
{
    rDst.WriteShort(rSrc.ReadShort());
}

void CTextureDecoder::ReadPixelsC4(IInputStream& rSrc, IOutputStream& rDst)
{
    // This isn't how C4 works, but due to the way Retro packed font textures (which use C4)
    // this is the only way to get them to decode correctly for now.
    // Commented-out code is proper C4 decoding. Dedicated font texture-decoding function
    // is probably going to be necessary in the future.
    const uint8 Byte = rSrc.ReadUByte();
    std::array<uint8, 2> Indices;
    Indices[0] = (Byte >> 4) & 0xF;
    Indices[1] = Byte & 0xF;

    for (uint32 iIdx = 0; iIdx < 2; iIdx++)
    {
        uint8 R, G, B, A;
        ((Indices[iIdx] >> 3) & 0x1) ? R = 0xFF : R = 0x0;
        ((Indices[iIdx] >> 2) & 0x1) ? G = 0xFF : G = 0x0;
        ((Indices[iIdx] >> 1) & 0x1) ? B = 0xFF : B = 0x0;
        ((Indices[iIdx] >> 0) & 0x1) ? A = 0xFF : A = 0x0;
        const uint32 RGBA = (R << 24) | (G << 16) | (B << 8) | (A);
        rDst.WriteLong(RGBA);

      /*mPaletteInput.Seek(indices[i] * 2, SEEK_SET);

             if (mPaletteFormat == ePalette_IA8)    readPixelIA8(mPaletteInput, rDst);
        else if (mPaletteFormat == ePalette_RGB565) readPixelRGB565(mPaletteInput, rDst);
        else if (mPaletteFormat == ePalette_RGB5A3) readPixelRGB5A3(mPaletteInput, rDst);*/
    }
}

void CTextureDecoder::ReadPixelC8(IInputStream& rSrc, IOutputStream& rDst)
{
    // DKCR fonts use C8 :|
    const uint8 Index = rSrc.ReadByte();

    /*u8 R, G, B, A;
    ((Index >> 3) & 0x1) ? R = 0xFF : R = 0x0;
    ((Index >> 2) & 0x1) ? G = 0xFF : G = 0x0;
    ((Index >> 1) & 0x1) ? B = 0xFF : B = 0x0;
    ((Index >> 0) & 0x1) ? A = 0xFF : A = 0x0;
    uint32 RGBA = (R << 24) | (G << 16) | (B << 8) | (A);
    dst.WriteLong(RGBA);*/

    mPaletteInput.Seek(Index * 2, SEEK_SET);

         if (mPaletteFormat == EGXPaletteFormat::IA8)    ReadPixelIA8(mPaletteInput, rDst);
    else if (mPaletteFormat == EGXPaletteFormat::RGB565) ReadPixelRGB565(mPaletteInput, rDst);
    else if (mPaletteFormat == EGXPaletteFormat::RGB5A3) ReadPixelRGB5A3(mPaletteInput, rDst);
}

void CTextureDecoder::ReadPixelRGB565(IInputStream& rSrc, IOutputStream& rDst)
{
    // RGB565 can be used as-is.
    rDst.WriteShort(rSrc.ReadShort());
}

void CTextureDecoder::ReadPixelRGB5A3(IInputStream& rSrc, IOutputStream& rDst)
{
    const uint16 Pixel = rSrc.ReadUShort();
    uint8 R, G, B, A;

    if (Pixel & 0x8000) // RGB5
    {
        B = Extend5to8(Pixel >> 10);
        G = Extend5to8(Pixel >>  5);
        R = Extend5to8(Pixel >>  0);
        A = 255;
    }
    else // RGB4A3
    {
        A = Extend3to8(Pixel >> 12);
        B = Extend4to8(Pixel >>  8);
        G = Extend4to8(Pixel >>  4);
        R = Extend4to8(Pixel >>  0);
    }

    const uint32 Color = (A << 24) | (R << 16) | (G << 8) | B;
    rDst.WriteLong(Color);
}

void CTextureDecoder::ReadPixelRGBA8(IInputStream& rSrc, IOutputStream& rDst)
{
    const uint16 AR = rSrc.ReadUShort();
    rSrc.Seek(0x1E, SEEK_CUR);
    const uint16 GB = rSrc.ReadUShort();
    rSrc.Seek(-0x20, SEEK_CUR);
    const uint32 Pixel = (AR << 16) | GB;
    rDst.WriteULong(Pixel);
}

void CTextureDecoder::ReadSubBlockCMPR(IInputStream& rSrc, IOutputStream& rDst)
{
    rDst.WriteShort(rSrc.ReadShort());
    rDst.WriteShort(rSrc.ReadShort());

    for (uint32 iByte = 0; iByte < 4; iByte++)
    {
        uint8 Byte = rSrc.ReadUByte();
        Byte = ((Byte & 0x3) << 6) | ((Byte & 0xC) << 2) | ((Byte & 0x30) >> 2) | ((Byte & 0xC0) >> 6);
        rDst.WriteUByte(Byte);
    }
}

// ************ DECODE PIXELS (FULL DECODE TO RGBA8) ************
CColor CTextureDecoder::DecodePixelI4(uint8 Byte, uint8 WhichPixel)
{
    if (WhichPixel == 1)
        Byte >>= 4;

    const uint8 Pixel = Extend4to8(Byte);
    return CColor::Integral(Pixel, Pixel, Pixel);
}

CColor CTextureDecoder::DecodePixelI8(uint8 Byte)
{
    return CColor::Integral(Byte, Byte, Byte);
}

CColor CTextureDecoder::DecodePixelIA4(uint8 Byte)
{
    const uint8 Alpha = Extend4to8(Byte >> 4);
    const uint8 Lum = Extend4to8(Byte);
    return CColor::Integral(Lum, Lum, Lum, Alpha);
}

CColor CTextureDecoder::DecodePixelIA8(uint16 Short)
{
    const uint8 Alpha = (Short >> 8) & 0xFF;
    const uint8 Lum = Short & 0xFF;
    return CColor::Integral(Lum, Lum, Lum, Alpha);
}

CColor CTextureDecoder::DecodePixelC4(uint8 Byte, uint8 WhichPixel, IInputStream& rPaletteStream)
{
    if (WhichPixel == 1)
        Byte >>= 4;

    Byte &= 0xF;

    rPaletteStream.Seek(Byte * 2, SEEK_SET);

    if (mPaletteFormat == EGXPaletteFormat::IA8)
        return DecodePixelIA8(rPaletteStream.ReadShort());

    if (mPaletteFormat == EGXPaletteFormat::RGB565)
        return DecodePixelIA8(rPaletteStream.ReadShort());

    if (mPaletteFormat == EGXPaletteFormat::RGB5A3)
        return DecodePixelIA8(rPaletteStream.ReadShort());

    return CColor::TransparentBlack();
}

CColor CTextureDecoder::DecodePixelC8(uint8 Byte, IInputStream& rPaletteStream)
{
    rPaletteStream.Seek(Byte * 2, SEEK_SET);

    if (mPaletteFormat == EGXPaletteFormat::IA8)
        return DecodePixelIA8(rPaletteStream.ReadShort());

    if (mPaletteFormat == EGXPaletteFormat::RGB565)
        return DecodePixelIA8(rPaletteStream.ReadShort());

    if (mPaletteFormat == EGXPaletteFormat::RGB5A3)
        return DecodePixelIA8(rPaletteStream.ReadShort());

    return CColor::TransparentBlack();
}

CColor CTextureDecoder::DecodePixelRGB565(uint16 Short)
{
    const uint8 B = Extend5to8(static_cast<uint8>(Short >> 11));
    const uint8 G = Extend6to8(static_cast<uint8>(Short >> 5));
    const uint8 R = Extend5to8(static_cast<uint8>(Short));
    return CColor::Integral(R, G, B, 0xFF);
}

CColor CTextureDecoder::DecodePixelRGB5A3(uint16 Short)
{
    if (Short & 0x8000) // RGB5
    {
        const uint8 B = Extend5to8(static_cast<uint8>(Short >> 10));
        const uint8 G = Extend5to8(static_cast<uint8>(Short >> 5));
        const uint8 R = Extend5to8(static_cast<uint8>(Short));
        return CColor::Integral(R, G, B, 0xFF);
    }
    else // RGB4A3
    {
        const uint8 A = Extend3to8(static_cast<uint8>(Short >> 12));
        const uint8 B = Extend4to8(static_cast<uint8>(Short >> 8));
        const uint8 G = Extend4to8(static_cast<uint8>(Short >> 4));
        const uint8 R = Extend4to8(static_cast<uint8>(Short));
        return CColor::Integral(R, G, B, A);
    }
}

void CTextureDecoder::DecodeSubBlockCMPR(IInputStream& rSrc, IOutputStream& rDst, uint16 Width)
{
    const uint16 PaletteA = rSrc.ReadUShort();
    const uint16 PaletteB = rSrc.ReadUShort();

    std::array<CColor, 4> Palettes{
        DecodePixelRGB565(PaletteA),
        DecodePixelRGB565(PaletteB),
    };

    if (PaletteA > PaletteB)
    {
        Palettes[2] = (Palettes[0] * 0.666666666f) + (Palettes[1] * 0.333333333f);
        Palettes[3] = (Palettes[0] * 0.333333333f) + (Palettes[1] * 0.666666666f);
    }
    else
    {
        Palettes[2] = (Palettes[0] * 0.5f) + (Palettes[1] * 0.5f);
        Palettes[3] = CColor::TransparentBlack();
    }

    for (uint32 iBlockY = 0; iBlockY < 4; iBlockY++)
    {
        const uint8 Byte = rSrc.ReadUByte();

        for (uint32 iBlockX = 0; iBlockX < 4; iBlockX++)
        {
            const uint8 Shift = static_cast<uint8>(6 - (iBlockX * 2));
            const uint8 PaletteIndex = (Byte >> Shift) & 0x3;
            const CColor& Pixel = Palettes[PaletteIndex];
            rDst.WriteLong(Pixel.ToLongARGB());
        }

        rDst.Seek((Width - 4) * 4, SEEK_CUR);
    }
}

void CTextureDecoder::DecodeBlockBC1(IInputStream& rSrc, IOutputStream& rDst, uint32 Width)
{
    // Very similar to the CMPR subblock function, but unfortunately a slight
    // difference in the order the pixel indices are read requires a separate function
    const uint16 PaletteA = rSrc.ReadUShort();
    const uint16 PaletteB = rSrc.ReadUShort();

    std::array<CColor, 4> Palettes{
        DecodePixelRGB565(PaletteA),
        DecodePixelRGB565(PaletteB),
    };

    if (PaletteA > PaletteB)
    {
        Palettes[2] = (Palettes[0] * 0.666666666f) + (Palettes[1] * 0.333333333f);
        Palettes[3] = (Palettes[0] * 0.333333333f) + (Palettes[1] * 0.666666666f);
    }
    else
    {
        Palettes[2] = (Palettes[0] * 0.5f) + (Palettes[1] * 0.5f);
        Palettes[3] = CColor::TransparentBlack();
    }

    for (uint32 iBlockY = 0; iBlockY < 4; iBlockY++)
    {
        const uint8 Byte = rSrc.ReadByte();

        for (uint32 iBlockX = 0; iBlockX < 4; iBlockX++)
        {
            const uint8 Shift = static_cast<uint8>(iBlockX * 2);
            const uint8 PaletteIndex = (Byte >> Shift) & 0x3;
            const CColor& Pixel = Palettes[PaletteIndex];
            rDst.WriteLong(Pixel.ToLongARGB());
        }

        rDst.Seek((Width - 4) * 4, SEEK_CUR);
    }
}

void CTextureDecoder::DecodeBlockBC2(IInputStream& rSrc, IOutputStream& rDst, uint32 Width)
{
    const uint16 PaletteA = rSrc.ReadUShort();
    const uint16 PaletteB = rSrc.ReadUShort();

    std::array<CColor, 4> CPalettes{
        DecodePixelRGB565(PaletteA),
        DecodePixelRGB565(PaletteB),
    };

    if (PaletteA > PaletteB)
    {
        CPalettes[2] = (CPalettes[0] * 0.666666666f) + (CPalettes[1] * 0.333333333f);
        CPalettes[3] = (CPalettes[0] * 0.333333333f) + (CPalettes[1] * 0.666666666f);
    }
    else
    {
        CPalettes[2] = (CPalettes[0] * 0.5f) + (CPalettes[1] * 0.5f);
        CPalettes[3] = CColor::TransparentBlack();
    }

    for (uint32 iBlockY = 0; iBlockY < 4; iBlockY++)
    {
        const uint8 Byte = rSrc.ReadUByte();

        for (uint32 iBlockX = 0; iBlockX < 4; iBlockX++)
        {
            const uint8 Shift = static_cast<uint8>(iBlockX * 2);
            const uint8 PaletteIndex = (Byte >> Shift) & 0x3;
            const CColor& Pixel = CPalettes[PaletteIndex];
            rDst.WriteLong(Pixel.ToLongARGB());
        }

        rDst.Seek((Width - 4) * 4, SEEK_CUR);
    }
}

void CTextureDecoder::DecodeBlockBC3(IInputStream& rSrc, IOutputStream& rDst, uint32 Width)
{
    const uint16 PaletteA = rSrc.ReadUShort();
    const uint16 PaletteB = rSrc.ReadUShort();

    std::array<CColor, 4> Palettes{
        DecodePixelRGB565(PaletteA),
        DecodePixelRGB565(PaletteB),
    };

    if (PaletteA > PaletteB)
    {
        Palettes[2] = (Palettes[0] * 0.666666666f) + (Palettes[1] * 0.333333333f);
        Palettes[3] = (Palettes[0] * 0.333333333f) + (Palettes[1] * 0.666666666f);
    }
    else
    {
        Palettes[2] = (Palettes[0] * 0.5f) + (Palettes[1] * 0.5f);
        Palettes[3] = CColor::TransparentBlack();
    }

    for (uint32 iBlockY = 0; iBlockY < 4; iBlockY++)
    {
        const uint8 Byte = rSrc.ReadUByte();

        for (uint32 iBlockX = 0; iBlockX < 4; iBlockX++)
        {
            const uint8 Shift = static_cast<uint8>(iBlockX * 2);
            const uint8 PaletteIndex = (Byte >> Shift) & 0x3;
            const CColor& Pixel = Palettes[PaletteIndex];
            rDst.WriteLong(Pixel.ToLongARGB());
        }

        rDst.Seek((Width - 4) * 4, SEEK_CUR);
    }
}

CColor CTextureDecoder::DecodeDDSPixel(IInputStream& /*rDDS*/)
{
    return CColor::White();
}
