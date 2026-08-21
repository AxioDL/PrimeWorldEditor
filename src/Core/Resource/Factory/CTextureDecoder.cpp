#include "Core/Resource/Factory/CTextureDecoder.h"

#include "Core/Resource/CTexture.h"
#include <Common/CColor.h>
#include <Common/Log.h>
#include <Common/Macros.h>
#include <Common/FileIO/IOutputStream.h>
#include <Common/FileIO/CMemoryOutStream.h>
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

constexpr uint8_t Extend3to8(uint8_t In)
{
    In &= 0x7;
    return (In << 5) | (In << 2) | (In >> 1);
}

constexpr uint8_t Extend4to8(uint8_t In)
{
    In &= 0xF;
    return (In << 4) | In;
}

constexpr uint8_t Extend5to8(uint8_t In)
{
    In &= 0x1F;
    return (In << 3) | (In >> 2);
}

constexpr uint8_t Extend6to8(uint8_t In)
{
    In &= 0x3F;
    return (In << 2) | (In >> 4);
}

constexpr uint32_t CalculateShiftForMask(uint32_t BitMask)
{
    uint32_t Shift = 32;

    while (BitMask)
    {
        BitMask <<= 1;
        Shift--;
    }
    return Shift;
}

constexpr uint32_t CalculateMaskBitCount(uint32_t BitMask)
{
    uint32_t Count = 0;

    while (BitMask)
    {
        if (BitMask & 0x1)
            Count++;
        BitMask >>= 1;
    }
    return Count;
}
} // Anonymous namespace

CTextureDecoder::CTextureDecoder() = default;
CTextureDecoder::~CTextureDecoder() = default;

std::unique_ptr<CTexture> CTextureDecoder::CreateTexture()
{
    auto pTex = std::make_unique<CTexture>(mpEntry);
    pTex->mSourceTexelFormat = mTexelFormat;
    pTex->mWidth = mWidth;
    pTex->mHeight = mHeight;
    pTex->mNumMipMaps = mNumMipMaps;
    pTex->mLinearSize = static_cast<uint32_t>(mWidth * mHeight * gskPixelsToBytes[static_cast<size_t>(mTexelFormat)]);
    pTex->mpImgDataBuffer = std::move(mpDataBuffer);
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
    mTexelFormat = ETexelFormat(rTXTR.ReadU32());
    mWidth = rTXTR.ReadU16();
    mHeight = rTXTR.ReadU16();
    mNumMipMaps = rTXTR.ReadU32();

    // For C4 and C8 images, read palette
    if (mTexelFormat == ETexelFormat::GX_C4 || mTexelFormat == ETexelFormat::GX_C8)
    {
        mHasPalettes = true;
        mPaletteFormat = EGXPaletteFormat(rTXTR.ReadU32());
        rTXTR.Seek(0x4, SEEK_CUR);

        const uint32_t PaletteEntryCount = (mTexelFormat == ETexelFormat::GX_C4) ? 16 : 256;
        mPalettes.resize(PaletteEntryCount * 2);
        rTXTR.ReadBytes(mPalettes.data(), mPalettes.size());

        mPaletteInput.SetData(mPalettes.data(), mPalettes.size(), std::endian::big);
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
    if (Magic != CFourCC("DDS "))
    {
        NLog::Error("{}: Invalid DDS magic: 0x{:08X}", rDDS.GetSourceString(), Magic.ToU32());
        return;
    }

    const uint32_t ImageDataStart = rDDS.Tell() + rDDS.ReadU32();
    rDDS.Seek(0x4, SEEK_CUR); // Skipping flags
    mHeight = static_cast<uint16_t>(rDDS.ReadU32());
    mWidth = static_cast<uint16_t>(rDDS.ReadU32());
    rDDS.Seek(0x8, SEEK_CUR); // Skipping linear size + depth
    mNumMipMaps = rDDS.ReadU32() + 1; // DDS doesn't seem to count the first mipmap
    rDDS.Seek(0x2C, SEEK_CUR); // Skipping reserved

    // Pixel Format
    rDDS.Seek(0x4, SEEK_CUR); // Skipping size
    mDDSInfo.Flags = rDDS.ReadU32();
    const CFourCC Format(rDDS);

    if (Format == CFourCC("DXT1"))      mDDSInfo.Format = SDDSInfo::DXT1;
    else if (Format == CFourCC("DXT2")) mDDSInfo.Format = SDDSInfo::DXT2;
    else if (Format == CFourCC("DXT3")) mDDSInfo.Format = SDDSInfo::DXT3;
    else if (Format == CFourCC("DXT4")) mDDSInfo.Format = SDDSInfo::DXT4;
    else if (Format == CFourCC("DXT5")) mDDSInfo.Format = SDDSInfo::DXT5;
    else
    {
        mDDSInfo.Format = SDDSInfo::RGBA;
        mDDSInfo.BitCount = rDDS.ReadU32();
        mDDSInfo.RBitMask = rDDS.ReadU32();
        mDDSInfo.GBitMask = rDDS.ReadU32();
        mDDSInfo.BBitMask = rDDS.ReadU32();
        mDDSInfo.ABitMask = rDDS.ReadU32();
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
    const uint32_t ImageStart = TXTR.Tell();
    TXTR.Seek(0x0, SEEK_END);
    const uint32_t ImageSize = TXTR.Tell() - ImageStart;
    TXTR.Seek(ImageStart, SEEK_SET);

    mDataBufferSize = ImageSize * (gskOutputBpp[static_cast<size_t>(mTexelFormat)] / gskSourceBpp[static_cast<size_t>(mTexelFormat)]);
    if (mHasPalettes && mPaletteFormat == EGXPaletteFormat::RGB5A3)
        mDataBufferSize *= 2;
    mpDataBuffer = std::make_unique_for_overwrite<uint8_t[]>(mDataBufferSize);

    CMemoryOutStream Out(mpDataBuffer.get(), mDataBufferSize, std::endian::native);

    // Initializing more stuff before we start the mipmap loop
    uint32_t MipW = mWidth;
    uint32_t MipH = mHeight;
    uint32_t MipOffset = 0;

    const uint32_t BWidth = gskBlockWidth[static_cast<size_t>(mTexelFormat)];
    const uint32_t BHeight = gskBlockHeight[static_cast<size_t>(mTexelFormat)];

    uint32_t PixelStride = gskOutputPixelStride[static_cast<size_t>(mTexelFormat)];
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

    for (uint32_t iMip = 0; iMip < mNumMipMaps; iMip++)
    {
        if (MipW < BWidth)
            MipW = BWidth;

        if (MipH < BHeight)
            MipH = BHeight;

        for (uint32_t iBlockY = 0; iBlockY < MipH; iBlockY += BHeight)
        {
            for (uint32_t iBlockX = 0; iBlockX < MipW; iBlockX += BWidth)
            {
                for (uint32_t iImgY = iBlockY; iImgY < iBlockY + BHeight; iImgY++)
                {
                    for (uint32_t iImgX = iBlockX; iImgX < iBlockX + BWidth; iImgX++)
                    {
                        const uint32_t DstPos = ((iImgY * MipW) + iImgX) * PixelStride;
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

        uint32_t MipSize = static_cast<uint32_t>(MipW * MipH * gskPixelsToBytes[static_cast<size_t>(mTexelFormat)]);
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
    const uint32_t ImageStart = rTXTR.Tell();
    rTXTR.Seek(0x0, SEEK_END);
    const uint32_t ImageSize = rTXTR.Tell() - ImageStart;
    rTXTR.Seek(ImageStart, SEEK_SET);

    mDataBufferSize = ImageSize * (32 / gskSourceBpp[static_cast<size_t>(mTexelFormat)]);
    mpDataBuffer = std::make_unique_for_overwrite<uint8_t[]>(mDataBufferSize);

    CMemoryOutStream Out(mpDataBuffer.get(), mDataBufferSize, std::endian::native);

    // Initializing more stuff before we start the mipmap loop
    uint32_t MipW = mWidth;
    uint32_t MipH = mHeight;
    uint32_t MipOffset = 0;

    const uint32_t BWidth = gskBlockWidth[static_cast<size_t>(mTexelFormat)];
    const uint32_t BHeight = gskBlockHeight[static_cast<size_t>(mTexelFormat)];

    // With CMPR, we're using a little trick.
    // CMPR stores pixels in 8x8 blocks, with four 4x4 subblocks.
    // An easy way to convert it is to pretend each block is 2x2 and each subblock is one pixel.
    // So to do that we need to calculate the "new" dimensions of the image, 1/4 the size of the original.
    if (mTexelFormat == ETexelFormat::GX_CMPR)
    {
        MipW /= 4;
        MipH /= 4;
    }

    for (uint32_t iMip = 0; iMip < mNumMipMaps; iMip++) {
        for (uint32_t iBlockY = 0; iBlockY < MipH; iBlockY += BHeight) {
            for (uint32_t iBlockX = 0; iBlockX < MipW; iBlockX += BWidth) {
                for (uint32_t iImgY = iBlockY; iImgY < iBlockY + BHeight; iImgY++) {
                    for (uint32_t iImgX = iBlockX; iImgX < iBlockX + BWidth; iImgX++) {
                        const uint32_t DstPos = (mTexelFormat == ETexelFormat::GX_CMPR) ? ((iImgY * (MipW * 4)) + iImgX) * 16 : ((iImgY * MipW) + iImgX) * 4;
                        Out.Seek(MipOffset + DstPos, SEEK_SET);

                        // I4/C4/CMPR require reading more than one pixel at a time
                        if (mTexelFormat == ETexelFormat::GX_I4)
                        {
                            const auto Byte = rTXTR.ReadU8();
                            Out.WriteU32(DecodePixelI4(Byte, 0).ToARGB());
                            Out.WriteU32(DecodePixelI4(Byte, 1).ToARGB());
                        }
                        else if (mTexelFormat == ETexelFormat::GX_C4)
                        {
                            const auto Byte = rTXTR.ReadU8();
                            Out.WriteU32(DecodePixelC4(Byte, 0, mPaletteInput).ToARGB());
                            Out.WriteU32(DecodePixelC4(Byte, 1, mPaletteInput).ToARGB());
                        }
                        else if (mTexelFormat == ETexelFormat::GX_CMPR)
                        {
                            DecodeSubBlockCMPR(rTXTR, Out, static_cast<uint16_t>(MipW * 4));
                        }
                        else
                        {
                            CColor Pixel;

                            if (mTexelFormat == ETexelFormat::GX_I8)          Pixel = DecodePixelI8(rTXTR.ReadU8());
                            else if (mTexelFormat == ETexelFormat::GX_IA4)    Pixel = DecodePixelIA4(rTXTR.ReadU8());
                            else if (mTexelFormat == ETexelFormat::GX_IA8)    Pixel = DecodePixelIA8(rTXTR.ReadU16());
                            else if (mTexelFormat == ETexelFormat::GX_C8)     Pixel = DecodePixelC8(rTXTR.ReadU8(), mPaletteInput);
                            else if (mTexelFormat == ETexelFormat::GX_RGB565) Pixel = DecodePixelRGB565(rTXTR.ReadU16());
                            else if (mTexelFormat == ETexelFormat::GX_RGB5A3) Pixel = DecodePixelRGB5A3(rTXTR.ReadU16());
                            else if (mTexelFormat == ETexelFormat::GX_RGBA8)  Pixel = CColor(rTXTR, true);

                            Out.WriteU32(Pixel.ToARGB());
                        }
                    }
                }
                if (mTexelFormat == ETexelFormat::GX_RGBA8)
                    rTXTR.Seek(0x20, SEEK_CUR);
            }
        }

        uint32_t MipSize = MipW * MipH * 4;
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
    const uint32_t ImageStart = rDDS.Tell();
    rDDS.Seek(0x0, SEEK_END);
    const uint32_t ImageSize = rDDS.Tell() - ImageStart;
    rDDS.Seek(ImageStart, SEEK_SET);

    mDataBufferSize = ImageSize;
    if (mDDSInfo.Format == SDDSInfo::DXT1)
        mDataBufferSize *= 8;
    else if (mDDSInfo.Format == SDDSInfo::RGBA)
        mDataBufferSize *= (32 / mDDSInfo.BitCount);
    else
        mDataBufferSize *= 4;
    mpDataBuffer = std::make_unique_for_overwrite<uint8_t[]>(mDataBufferSize);

    CMemoryOutStream Out(mpDataBuffer.get(), mDataBufferSize, std::endian::native);

    // Initializing more stuff before we start the mipmap loop
    uint32_t MipW = mWidth;
    uint32_t MipH = mHeight;
    uint32_t MipOffset = 0;

    uint32_t BPP;
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

    for (uint32_t iMip = 0; iMip < mNumMipMaps; iMip++)
    {
        // For DXT1 we can copy the image data as-is to load it
        if (mDDSInfo.Format == SDDSInfo::DXT1)
        {
            Out.Seek(MipOffset, SEEK_SET);
            const uint32_t MipSize = MipW * MipH / 2;
            std::vector<uint8_t> MipBuffer(MipSize);
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
            for (uint32_t Y = 0; Y < MipH; Y++)
            {
                for (uint32_t X = 0; X < MipW; X++)
                {
                    uint32_t OutPos = MipOffset;

                    if (mDDSInfo.Format == SDDSInfo::RGBA)
                    {
                        OutPos += ((Y * MipW) + X) * 4;
                        Out.Seek(OutPos, SEEK_SET);

                        const CColor Pixel = DecodeDDSPixel(rDDS);
                        Out.WriteU32(Pixel.ToARGB());
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

            uint32_t MipSize = (mWidth * mHeight) * 4;
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
    const auto Pixels = rSrc.ReadU8();
    rDst.WriteU8(Extend4to8(Pixels >> 4));
    rDst.WriteU8(Extend4to8(Pixels >> 4));
    rDst.WriteU8(Extend4to8(Pixels));
    rDst.WriteU8(Extend4to8(Pixels));
}

void CTextureDecoder::ReadPixelI8(IInputStream& rSrc, IOutputStream& rDst)
{
    const auto Pixel = rSrc.ReadU8();
    rDst.WriteU8(Pixel);
    rDst.WriteU8(Pixel);
}

void CTextureDecoder::ReadPixelIA4(IInputStream& rSrc, IOutputStream& rDst)
{
    // this can be left as-is for DDS conversion, but opengl doesn't support two components in one byte...
    const auto Byte = rSrc.ReadU8();
    const auto Alpha = Extend4to8(Byte >> 4);
    const auto Lum = Extend4to8(Byte);
    rDst.WriteU16((Lum << 8) | Alpha);
}

void CTextureDecoder::ReadPixelIA8(IInputStream& rSrc, IOutputStream& rDst)
{
    rDst.WriteS16(rSrc.ReadS16());
}

void CTextureDecoder::ReadPixelsC4(IInputStream& rSrc, IOutputStream& rDst)
{
    // This isn't how C4 works, but due to the way Retro packed font textures (which use C4)
    // this is the only way to get them to decode correctly for now.
    // Commented-out code is proper C4 decoding. Dedicated font texture-decoding function
    // is probably going to be necessary in the future.
    const auto Byte = rSrc.ReadU8();
    std::array<uint8_t, 2> Indices;
    Indices[0] = (Byte >> 4) & 0xF;
    Indices[1] = Byte & 0xF;

    for (uint32_t iIdx = 0; iIdx < 2; iIdx++)
    {
        uint8_t R, G, B, A;
        ((Indices[iIdx] >> 3) & 0x1) ? R = 0xFF : R = 0x0;
        ((Indices[iIdx] >> 2) & 0x1) ? G = 0xFF : G = 0x0;
        ((Indices[iIdx] >> 1) & 0x1) ? B = 0xFF : B = 0x0;
        ((Indices[iIdx] >> 0) & 0x1) ? A = 0xFF : A = 0x0;
        const uint32_t RGBA = (R << 24) | (G << 16) | (B << 8) | (A);
        rDst.WriteU32(RGBA);

      /*mPaletteInput.Seek(indices[i] * 2, SEEK_SET);

             if (mPaletteFormat == ePalette_IA8)    readPixelIA8(mPaletteInput, rDst);
        else if (mPaletteFormat == ePalette_RGB565) readPixelRGB565(mPaletteInput, rDst);
        else if (mPaletteFormat == ePalette_RGB5A3) readPixelRGB5A3(mPaletteInput, rDst);*/
    }
}

void CTextureDecoder::ReadPixelC8(IInputStream& rSrc, IOutputStream& rDst)
{
    // DKCR fonts use C8 :|
    const auto Index = rSrc.ReadU8();

    /*u8 R, G, B, A;
    ((Index >> 3) & 0x1) ? R = 0xFF : R = 0x0;
    ((Index >> 2) & 0x1) ? G = 0xFF : G = 0x0;
    ((Index >> 1) & 0x1) ? B = 0xFF : B = 0x0;
    ((Index >> 0) & 0x1) ? A = 0xFF : A = 0x0;
    const uint32_t RGBA = (R << 24) | (G << 16) | (B << 8) | (A);
    dst.WriteU32(RGBA);*/

    mPaletteInput.Seek(Index * 2, SEEK_SET);

         if (mPaletteFormat == EGXPaletteFormat::IA8)    ReadPixelIA8(mPaletteInput, rDst);
    else if (mPaletteFormat == EGXPaletteFormat::RGB565) ReadPixelRGB565(mPaletteInput, rDst);
    else if (mPaletteFormat == EGXPaletteFormat::RGB5A3) ReadPixelRGB5A3(mPaletteInput, rDst);
}

void CTextureDecoder::ReadPixelRGB565(IInputStream& rSrc, IOutputStream& rDst)
{
    // RGB565 can be used as-is.
    rDst.WriteS16(rSrc.ReadS16());
}

void CTextureDecoder::ReadPixelRGB5A3(IInputStream& rSrc, IOutputStream& rDst)
{
    const auto Pixel = rSrc.ReadU16();
    uint8_t R, G, B, A;

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

    const uint32_t Color = (A << 24) | (R << 16) | (G << 8) | B;
    rDst.WriteU32(Color);
}

void CTextureDecoder::ReadPixelRGBA8(IInputStream& rSrc, IOutputStream& rDst)
{
    const auto AR = rSrc.ReadU16();
    rSrc.Seek(0x1E, SEEK_CUR);
    const auto GB = rSrc.ReadU16();
    rSrc.Seek(-0x20, SEEK_CUR);
    const auto Pixel = uint32_t(AR << 16) | GB;
    rDst.WriteU32(Pixel);
}

void CTextureDecoder::ReadSubBlockCMPR(IInputStream& rSrc, IOutputStream& rDst)
{
    rDst.WriteS16(rSrc.ReadS16());
    rDst.WriteS16(rSrc.ReadS16());

    for (uint32_t iByte = 0; iByte < 4; iByte++)
    {
        auto Byte = rSrc.ReadU8();
        Byte = ((Byte & 0x3) << 6) | ((Byte & 0xC) << 2) | ((Byte & 0x30) >> 2) | ((Byte & 0xC0) >> 6);
        rDst.WriteU8(Byte);
    }
}

// ************ DECODE PIXELS (FULL DECODE TO RGBA8) ************
CColor CTextureDecoder::DecodePixelI4(uint8_t Byte, uint8_t WhichPixel)
{
    if (WhichPixel == 1)
        Byte >>= 4;

    const uint8_t Pixel = Extend4to8(Byte);
    return CColor::Integral(Pixel, Pixel, Pixel);
}

CColor CTextureDecoder::DecodePixelI8(uint8_t Byte)
{
    return CColor::Integral(Byte, Byte, Byte);
}

CColor CTextureDecoder::DecodePixelIA4(uint8_t Byte)
{
    const uint8_t Alpha = Extend4to8(Byte >> 4);
    const uint8_t Lum = Extend4to8(Byte);
    return CColor::Integral(Lum, Lum, Lum, Alpha);
}

CColor CTextureDecoder::DecodePixelIA8(uint16_t Short)
{
    const uint8_t Alpha = (Short >> 8) & 0xFF;
    const uint8_t Lum = Short & 0xFF;
    return CColor::Integral(Lum, Lum, Lum, Alpha);
}

CColor CTextureDecoder::DecodePixelC4(uint8_t Byte, uint8_t WhichPixel, IInputStream& rPaletteStream)
{
    if (WhichPixel == 1)
        Byte >>= 4;

    Byte &= 0xF;

    rPaletteStream.Seek(Byte * 2, SEEK_SET);

    if (mPaletteFormat == EGXPaletteFormat::IA8)
        return DecodePixelIA8(rPaletteStream.ReadU16());

    if (mPaletteFormat == EGXPaletteFormat::RGB565)
        return DecodePixelIA8(rPaletteStream.ReadU16());

    if (mPaletteFormat == EGXPaletteFormat::RGB5A3)
        return DecodePixelIA8(rPaletteStream.ReadU16());

    return CColor::TransparentBlack();
}

CColor CTextureDecoder::DecodePixelC8(uint8_t Byte, IInputStream& rPaletteStream)
{
    rPaletteStream.Seek(Byte * 2, SEEK_SET);

    if (mPaletteFormat == EGXPaletteFormat::IA8)
        return DecodePixelIA8(rPaletteStream.ReadU16());

    if (mPaletteFormat == EGXPaletteFormat::RGB565)
        return DecodePixelIA8(rPaletteStream.ReadU16());

    if (mPaletteFormat == EGXPaletteFormat::RGB5A3)
        return DecodePixelIA8(rPaletteStream.ReadU16());

    return CColor::TransparentBlack();
}

CColor CTextureDecoder::DecodePixelRGB565(uint16_t Short)
{
    const uint8_t B = Extend5to8(static_cast<uint8_t>(Short >> 11));
    const uint8_t G = Extend6to8(static_cast<uint8_t>(Short >> 5));
    const uint8_t R = Extend5to8(static_cast<uint8_t>(Short));
    return CColor::Integral(R, G, B, 0xFF);
}

CColor CTextureDecoder::DecodePixelRGB5A3(uint16_t Short)
{
    if (Short & 0x8000) // RGB5
    {
        const uint8_t B = Extend5to8(static_cast<uint8_t>(Short >> 10));
        const uint8_t G = Extend5to8(static_cast<uint8_t>(Short >> 5));
        const uint8_t R = Extend5to8(static_cast<uint8_t>(Short));
        return CColor::Integral(R, G, B, 0xFF);
    }
    else // RGB4A3
    {
        const uint8_t A = Extend3to8(static_cast<uint8_t>(Short >> 12));
        const uint8_t B = Extend4to8(static_cast<uint8_t>(Short >> 8));
        const uint8_t G = Extend4to8(static_cast<uint8_t>(Short >> 4));
        const uint8_t R = Extend4to8(static_cast<uint8_t>(Short));
        return CColor::Integral(R, G, B, A);
    }
}

void CTextureDecoder::DecodeSubBlockCMPR(IInputStream& rSrc, IOutputStream& rDst, uint16_t Width)
{
    const auto PaletteA = rSrc.ReadU16();
    const auto PaletteB = rSrc.ReadU16();

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

    for (uint32_t iBlockY = 0; iBlockY < 4; iBlockY++)
    {
        const auto Byte = rSrc.ReadU8();

        for (uint32_t iBlockX = 0; iBlockX < 4; iBlockX++)
        {
            const uint8_t Shift = static_cast<uint8_t>(6 - (iBlockX * 2));
            const uint8_t PaletteIndex = (Byte >> Shift) & 0x3;
            const CColor& Pixel = Palettes[PaletteIndex];
            rDst.WriteU32(Pixel.ToARGB());
        }

        rDst.Seek((Width - 4) * 4, SEEK_CUR);
    }
}

void CTextureDecoder::DecodeBlockBC1(IInputStream& rSrc, IOutputStream& rDst, uint32_t Width)
{
    // Very similar to the CMPR subblock function, but unfortunately a slight
    // difference in the order the pixel indices are read requires a separate function
    const auto PaletteA = rSrc.ReadU16();
    const auto PaletteB = rSrc.ReadU16();

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

    for (uint32_t iBlockY = 0; iBlockY < 4; iBlockY++)
    {
        const auto Byte = rSrc.ReadU8();

        for (uint32_t iBlockX = 0; iBlockX < 4; iBlockX++)
        {
            const uint8_t Shift = static_cast<uint8_t>(iBlockX * 2);
            const uint8_t PaletteIndex = (Byte >> Shift) & 0x3;
            const CColor& Pixel = Palettes[PaletteIndex];
            rDst.WriteU32(Pixel.ToARGB());
        }

        rDst.Seek((Width - 4) * 4, SEEK_CUR);
    }
}

void CTextureDecoder::DecodeBlockBC2(IInputStream& rSrc, IOutputStream& rDst, uint32_t Width)
{
    const auto PaletteA = rSrc.ReadU16();
    const auto PaletteB = rSrc.ReadU16();

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

    for (uint32_t iBlockY = 0; iBlockY < 4; iBlockY++)
    {
        const auto Byte = rSrc.ReadU8();

        for (uint32_t iBlockX = 0; iBlockX < 4; iBlockX++)
        {
            const uint8_t Shift = static_cast<uint8_t>(iBlockX * 2);
            const uint8_t PaletteIndex = (Byte >> Shift) & 0x3;
            const CColor& Pixel = CPalettes[PaletteIndex];
            rDst.WriteU32(Pixel.ToARGB());
        }

        rDst.Seek((Width - 4) * 4, SEEK_CUR);
    }
}

void CTextureDecoder::DecodeBlockBC3(IInputStream& rSrc, IOutputStream& rDst, uint32_t Width)
{
    const auto PaletteA = rSrc.ReadU16();
    const auto PaletteB = rSrc.ReadU16();

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

    for (uint32_t iBlockY = 0; iBlockY < 4; iBlockY++)
    {
        const auto Byte = rSrc.ReadU8();

        for (uint32_t iBlockX = 0; iBlockX < 4; iBlockX++)
        {
            const uint8_t Shift = static_cast<uint8_t>(iBlockX * 2);
            const uint8_t PaletteIndex = (Byte >> Shift) & 0x3;
            const CColor& Pixel = Palettes[PaletteIndex];
            rDst.WriteU32(Pixel.ToARGB());
        }

        rDst.Seek((Width - 4) * 4, SEEK_CUR);
    }
}

CColor CTextureDecoder::DecodeDDSPixel(IInputStream& /*rDDS*/)
{
    return CColor::White();
}
