#include <Common/CColor.h>
#include <Core/Log.h>
#include "CTextureDecoder.h"

// A cleanup is warranted at some point. Trying to support both partial + full decode ended up really messy.

// Number of pixels * this = number of bytes
static const float gskPixelsToBytes[] = {
    2.f, 2.f, 2.f, 2.f, 4.f, 4.f, 0.f, 2.f, 4.f, 4.f, 0.5f
};

// Bits per pixel for each GX texture format
static const u32 gskSourceBpp[] = {
    4, 8, 8, 16, 4, 8, 16, 16, 16, 32, 4
};

// Bits per pixel for each GX texture format when decoded
static const u32 gskOutputBpp[] = {
    16, 16, 16, 16, 16, 16, 16, 16, 32, 32, 4
};

// Size of one pixel in output data in bytes
static const u32 gskOutputPixelStride[] = {
    2, 2, 2, 2, 2, 2, 2, 2, 4, 4, 8
};

// Block width for each GX texture format
static const u32 gskBlockWidth[] = {
    8, 8, 8, 4, 8, 8, 4, 4, 4, 4, 2
};

// Block height for each GX texture format
static const u32 gskBlockHeight[] = {
    8, 4, 4, 4, 8, 4, 4, 4, 4, 4, 2
};

CTextureDecoder::CTextureDecoder()
{
}

CTextureDecoder::~CTextureDecoder()
{
}

CTexture* CTextureDecoder::CreateTexture()
{
    CTexture *pTex = new CTexture;
    pTex->mSourceTexelFormat = mTexelFormat;
    pTex->mWidth = mWidth;
    pTex->mHeight = mHeight;
    pTex->mNumMipMaps = mNumMipMaps;
    pTex->mLinearSize = (u32) (mWidth * mHeight * gskPixelsToBytes[mTexelFormat]);
    pTex->mImgDataBuffer = mpDataBuffer;
    pTex->mImgDataSize = mDataBufferSize;
    pTex->mBufferExists = true;

    switch (mTexelFormat) {
        case eGX_I4:
        case eGX_I8:
        case eGX_IA4:
        case eGX_IA8:
            pTex->mTexelFormat = eLuminanceAlpha;
            break;
        case eGX_RGB565:
            pTex->mTexelFormat = eRGB565;
            break;
        case eGX_C4:
        case eGX_C8:
            if (mPaletteFormat == ePalette_IA8)    pTex->mTexelFormat = eLuminanceAlpha;
            if (mPaletteFormat == ePalette_RGB565) pTex->mTexelFormat = eRGB565;
            if (mPaletteFormat == ePalette_RGB5A3) pTex->mTexelFormat = eRGBA8;
            break;
        case eGX_RGB5A3:
        case eGX_RGBA8:
            pTex->mTexelFormat = eRGBA8;
            break;
        case eGX_CMPR:
            pTex->mTexelFormat = eDXT1;
            break;
        case eDXT1:
            pTex->mTexelFormat = eDXT1;
            pTex->mLinearSize = mWidth * mHeight / 2;
            break;
        default:
            pTex->mTexelFormat = mTexelFormat;
            break;
    }

    return pTex;
}

// ************ STATIC ************
CTexture* CTextureDecoder::LoadTXTR(CInputStream& TXTR)
{
    CTextureDecoder Decoder;
    Decoder.ReadTXTR(TXTR);
    Decoder.PartialDecodeGXTexture(TXTR);
    return Decoder.CreateTexture();
}

CTexture* CTextureDecoder::DoFullDecode(CInputStream &TXTR)
{
    CTextureDecoder Decoder;
    Decoder.ReadTXTR(TXTR);
    Decoder.FullDecodeGXTexture(TXTR);

    CTexture *pTexture = Decoder.CreateTexture();
    pTexture->mTexelFormat = eRGBA8;
    return pTexture;
}

CTexture* CTextureDecoder::LoadDDS(CInputStream &DDS)
{
    CTextureDecoder Decoder;
    Decoder.ReadDDS(DDS);
    Decoder.DecodeDDS(DDS);
    return Decoder.CreateTexture();
}

CTexture* CTextureDecoder::DoFullDecode(CTexture*)
{
    // Not using parameter 1 (CTexture* - pTexture)
    return nullptr;
}

// ************ READ ************
void CTextureDecoder::ReadTXTR(CInputStream& TXTR)
{
    // Read TXTR header
    Log::Write("Loading " + TXTR.GetSourceString());
    mTexelFormat = ETexelFormat(TXTR.ReadLong());
    mWidth = TXTR.ReadShort();
    mHeight = TXTR.ReadShort();
    mNumMipMaps = TXTR.ReadLong();

    // For C4 and C8 images, read palette
    if ((mTexelFormat == eGX_C4) || (mTexelFormat == eGX_C8))
    {
        mHasPalettes = true;
        mPaletteFormat = EGXPaletteFormat(TXTR.ReadLong());
        TXTR.Seek(0x4, SEEK_CUR);

        u32 PaletteEntryCount = (mTexelFormat == eGX_C4) ? 16 : 256;
        mPalettes.resize(PaletteEntryCount * 2);
        TXTR.ReadBytes(mPalettes.data(), mPalettes.size());

        mPaletteInput.SetData(mPalettes.data(), mPalettes.size(), IOUtil::BigEndian);
    }
    else mHasPalettes = false;
}

void CTextureDecoder::ReadDDS(CInputStream& DDS)
{
    Log::Write("Loading " + DDS.GetSourceString());

    // Header
    CFourCC Magic(DDS);
    if (Magic != "DDS ")
    {
        Log::FileError(DDS.GetSourceString(), "Invalid DDS magic: " + TString::HexString((u32) Magic.ToLong()));
        return;
    }

    u32 ImageDataStart = DDS.Tell() + DDS.ReadLong();
    DDS.Seek(0x4, SEEK_CUR); // Skipping flags
    mHeight = (u16) DDS.ReadLong();
    mWidth = (u16) DDS.ReadLong();
    DDS.Seek(0x8, SEEK_CUR); // Skipping linear size + depth
    mNumMipMaps = DDS.ReadLong() + 1; // DDS doesn't seem to count the first mipmap
    DDS.Seek(0x2C, SEEK_CUR); // Skipping reserved

    // Pixel Format
    DDS.Seek(0x4, SEEK_CUR); // Skipping size
    mDDSInfo.Flags = DDS.ReadLong();
    CFourCC Format(DDS);

    if (Format == "DXT1")      mDDSInfo.Format = SDDSInfo::DXT1;
    else if (Format == "DXT2") mDDSInfo.Format = SDDSInfo::DXT2;
    else if (Format == "DXT3") mDDSInfo.Format = SDDSInfo::DXT3;
    else if (Format == "DXT4") mDDSInfo.Format = SDDSInfo::DXT4;
    else if (Format == "DXT5") mDDSInfo.Format = SDDSInfo::DXT5;
    else
    {
        mDDSInfo.Format = SDDSInfo::RGBA;
        mDDSInfo.BitCount = DDS.ReadLong();
        mDDSInfo.RBitMask = DDS.ReadLong();
        mDDSInfo.GBitMask = DDS.ReadLong();
        mDDSInfo.BBitMask = DDS.ReadLong();
        mDDSInfo.ABitMask = DDS.ReadLong();
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
    DDS.Seek(ImageDataStart, SEEK_SET);
}

// ************ DECODE ************
void CTextureDecoder::PartialDecodeGXTexture(CInputStream& TXTR)
{
    // Get image data size, create output buffer
    u32 ImageStart = TXTR.Tell();
    TXTR.Seek(0x0, SEEK_END);
    u32 ImageSize = TXTR.Tell() - ImageStart;
    TXTR.Seek(ImageStart, SEEK_SET);

    mDataBufferSize = ImageSize * (gskOutputBpp[mTexelFormat] / gskSourceBpp[mTexelFormat]);
    if ((mHasPalettes) && (mPaletteFormat == ePalette_RGB5A3)) mDataBufferSize *= 2;
    mpDataBuffer = new u8[mDataBufferSize];

    CMemoryOutStream Out(mpDataBuffer, mDataBufferSize, IOUtil::SystemEndianness);

    // Initializing more stuff before we start the mipmap loop
    u32 MipW = mWidth, MipH = mHeight;
    u32 MipOffset = 0;

    u32 BWidth = gskBlockWidth[mTexelFormat];
    u32 BHeight = gskBlockHeight[mTexelFormat];

    u32 PixelStride = gskOutputPixelStride[mTexelFormat];
    if (mHasPalettes && (mPaletteFormat == ePalette_RGB5A3))
        PixelStride = 4;

    // With CMPR, we're using a little trick.
    // CMPR stores pixels in 8x8 blocks, with four 4x4 subblocks.
    // An easy way to convert it is to pretend each block is 2x2 and each subblock is one pixel.
    // So to do that we need to calculate the "new" dimensions of the image, 1/4 the size of the original.
    if (mTexelFormat == eGX_CMPR) {
        MipW /= 4;
        MipH /= 4;
    }

    for (u32 iMip = 0; iMip < mNumMipMaps; iMip++)
    {
        for (u32 BlockY = 0; BlockY < MipH; BlockY += BHeight)
            for (u32 BlockX = 0; BlockX < MipW; BlockX += BWidth) {
                for (u32 ImgY = BlockY; ImgY < BlockY + BHeight; ImgY++) {
                    for (u32 ImgX = BlockX; ImgX < BlockX + BWidth; ImgX++)
                    {
                        u32 DstPos = ((ImgY * MipW) + ImgX) * PixelStride;
                        Out.Seek(MipOffset + DstPos, SEEK_SET);

                        if (mTexelFormat == eGX_I4)          ReadPixelsI4(TXTR, Out);
                        else if (mTexelFormat == eGX_I8)     ReadPixelI8(TXTR, Out);
                        else if (mTexelFormat == eGX_IA4)    ReadPixelIA4(TXTR, Out);
                        else if (mTexelFormat == eGX_IA8)    ReadPixelIA8(TXTR, Out);
                        else if (mTexelFormat == eGX_C4)     ReadPixelsC4(TXTR, Out);
                        else if (mTexelFormat == eGX_C8)     ReadPixelC8(TXTR, Out);
                        else if (mTexelFormat == eGX_RGB565) ReadPixelRGB565(TXTR, Out);
                        else if (mTexelFormat == eGX_RGB5A3) ReadPixelRGB5A3(TXTR, Out);
                        else if (mTexelFormat == eGX_RGBA8)  ReadPixelRGBA8(TXTR, Out);
                        else if (mTexelFormat == eGX_CMPR)   ReadSubBlockCMPR(TXTR, Out);

                        // I4 and C4 have 4bpp images, so I'm forced to read two pixels at a time.
                        if ((mTexelFormat == eGX_I4) || (mTexelFormat == eGX_C4)) ImgX++;
                    }
                }
                if (mTexelFormat == eGX_RGBA8) TXTR.Seek(0x20, SEEK_CUR);
            }

        u32 MipSize = (u32) (MipW * MipH * gskPixelsToBytes[mTexelFormat]);
        if (mTexelFormat == eGX_CMPR) MipSize *= 16; // Since we're pretending the image is 1/4 its actual size, we have to multiply the size by 16 to get the correct offset

        MipOffset += MipSize;
        MipW /= 2;
        MipH /= 2;
        if (MipW < BWidth) MipW = BWidth;
        if (MipH < BHeight) MipH = BHeight;
    }
}

void CTextureDecoder::FullDecodeGXTexture(CInputStream& TXTR)
{
    // Get image data size, create output buffer
    u32 ImageStart = TXTR.Tell();
    TXTR.Seek(0x0, SEEK_END);
    u32 ImageSize = TXTR.Tell() - ImageStart;
    TXTR.Seek(ImageStart, SEEK_SET);

    mDataBufferSize = ImageSize * (32 / gskSourceBpp[mTexelFormat]);
    mpDataBuffer = new u8[mDataBufferSize];

    CMemoryOutStream Out(mpDataBuffer, mDataBufferSize, IOUtil::SystemEndianness);

    // Initializing more stuff before we start the mipmap loop
    u32 MipW = mWidth, MipH = mHeight;
    u32 MipOffset = 0;

    u32 BWidth = gskBlockWidth[mTexelFormat];
    u32 BHeight = gskBlockHeight[mTexelFormat];

    // With CMPR, we're using a little trick.
    // CMPR stores pixels in 8x8 blocks, with four 4x4 subblocks.
    // An easy way to convert it is to pretend each block is 2x2 and each subblock is one pixel.
    // So to do that we need to calculate the "new" dimensions of the image, 1/4 the size of the original.
    if (mTexelFormat == eGX_CMPR) {
        MipW /= 4;
        MipH /= 4;
    }

    for (u32 iMip = 0; iMip < mNumMipMaps; iMip++)
    {
        for (u32 BlockY = 0; BlockY < MipH; BlockY += BHeight)
            for (u32 BlockX = 0; BlockX < MipW; BlockX += BWidth) {
                for (u32 ImgY = BlockY; ImgY < BlockY + BHeight; ImgY++) {
                    for (u32 ImgX = BlockX; ImgX < BlockX + BWidth; ImgX++)
                    {
                        u32 DstPos = (mTexelFormat == eGX_CMPR) ? ((ImgY * (MipW * 4)) + ImgX) * 16 : ((ImgY * MipW) + ImgX) * 4;
                        Out.Seek(MipOffset + DstPos, SEEK_SET);

                        // I4/C4/CMPR require reading more than one pixel at a time
                        if (mTexelFormat == eGX_I4)
                        {
                            u8 Byte = TXTR.ReadByte();
                            Out.WriteLong( DecodePixelI4(Byte, 0).ToLongARGB() );
                            Out.WriteLong( DecodePixelI4(Byte, 1).ToLongARGB() );
                        }
                        else if (mTexelFormat == eGX_C4)
                        {
                            u8 Byte = TXTR.ReadByte();
                            Out.WriteLong( DecodePixelC4(Byte, 0, mPaletteInput).ToLongARGB() );
                            Out.WriteLong( DecodePixelC4(Byte, 1, mPaletteInput).ToLongARGB() );
                        }
                        else if (mTexelFormat == eGX_CMPR) DecodeSubBlockCMPR(TXTR, Out, (u16) (MipW * 4));

                        else
                        {
                            CColor Pixel;

                            if (mTexelFormat == eGX_I8)          Pixel = DecodePixelI8(TXTR.ReadByte());
                            else if (mTexelFormat == eGX_IA4)    Pixel = DecodePixelIA4(TXTR.ReadByte());
                            else if (mTexelFormat == eGX_IA8)    Pixel = DecodePixelIA8(TXTR.ReadShort());
                            else if (mTexelFormat == eGX_C8)     Pixel = DecodePixelC8(TXTR.ReadByte(), mPaletteInput);
                            else if (mTexelFormat == eGX_RGB565) Pixel = DecodePixelRGB565(TXTR.ReadShort());
                            else if (mTexelFormat == eGX_RGB5A3) Pixel = DecodePixelRGB5A3(TXTR.ReadShort());
                            else if (mTexelFormat == eGX_RGBA8)  Pixel = CColor(TXTR);

                            Out.WriteLong(Pixel.ToLongARGB());
                        }
                    }
                }
                if (mTexelFormat == eGX_RGBA8) TXTR.Seek(0x20, SEEK_CUR);
            }

        u32 MipSize = MipW * MipH * 4;
        if (mTexelFormat == eGX_CMPR) MipSize *= 16;

        MipOffset += MipSize;
        MipW /= 2;
        MipH /= 2;
        if (MipW < BWidth) MipW = BWidth;
        if (MipH < BHeight) MipH = BHeight;
    }
}

void CTextureDecoder::DecodeDDS(CInputStream &DDS)
{
    // Get image data size, create output buffer
    u32 ImageStart = DDS.Tell();
    DDS.Seek(0x0, SEEK_END);
    u32 ImageSize = DDS.Tell() - ImageStart;
    DDS.Seek(ImageStart, SEEK_SET);

    mDataBufferSize = ImageSize;
    if (mDDSInfo.Format == SDDSInfo::DXT1) mDataBufferSize *= 8;
    else if (mDDSInfo.Format == SDDSInfo::RGBA) mDataBufferSize *= (32 / mDDSInfo.BitCount);
    else mDataBufferSize *= 4;
    mpDataBuffer = new u8[mDataBufferSize];

    CMemoryOutStream Out(mpDataBuffer, mDataBufferSize, IOUtil::SystemEndianness);

    // Initializing more stuff before we start the mipmap loop
    u32 MipW = mWidth, MipH = mHeight;
    u32 MipOffset = 0;

    u32 BPP;
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
    if ((mDDSInfo.Format != SDDSInfo::RGBA) && (mDDSInfo.Format != SDDSInfo::DXT1))
    {
        MipW /= 4;
        MipH /= 4;
    }

    for (u32 iMip = 0; iMip < mNumMipMaps; iMip++)
    {
        // For DXT1 we can copy the image data as-is to load it
        if (mDDSInfo.Format == SDDSInfo::DXT1)
        {
            Out.Seek(MipOffset, SEEK_SET);
            u32 MipSize = MipW * MipH / 2;
            std::vector<u8> MipBuffer(MipSize);
            DDS.ReadBytes(MipBuffer.data(), MipBuffer.size());
            Out.WriteBytes(MipBuffer.data(), MipBuffer.size());
            MipOffset += MipSize;

            MipW /= 2;
            MipH /= 2;
            if (MipW % 4) MipW += (4 - (MipW % 4));
            if (MipH % 4) MipH += (4 - (MipH % 4));
        }

        // Otherwise we do a full decode to RGBA8
        else
        {
            for (u32 y = 0; y < MipH; y++)
            {
                for (u32 x = 0; x < MipW; x++)
                {
                    u32 OutPos = MipOffset;

                    if (mDDSInfo.Format == SDDSInfo::RGBA)
                    {
                        OutPos += ((y * MipW) + x) * 4;
                        Out.Seek(OutPos, SEEK_SET);

                        CColor Pixel = DecodeDDSPixel(DDS);
                        Out.WriteLong(Pixel.ToLongARGB());
                    }

                    else
                    {
                        OutPos += ((y * (MipW * 4)) + x) * 16;
                        Out.Seek(OutPos, SEEK_SET);

                        if (mDDSInfo.Format == SDDSInfo::DXT1)
                            DecodeBlockBC1(DDS, Out, MipW * 4);
                        else if ((mDDSInfo.Format == SDDSInfo::DXT2) || (mDDSInfo.Format == SDDSInfo::DXT3))
                            DecodeBlockBC2(DDS, Out, MipW * 4);
                        else if ((mDDSInfo.Format == SDDSInfo::DXT4) || (mDDSInfo.Format == SDDSInfo::DXT5))
                            DecodeBlockBC3(DDS, Out, MipW * 4);
                    }
                }
            }

            u32 MipSize = (mWidth * mHeight) * 4;
            if (mDDSInfo.Format != SDDSInfo::RGBA) MipSize *= 16;
            MipOffset += MipSize;

            MipW /= 2;
            MipH /= 2;
        }
    }

    if (mDDSInfo.Format == SDDSInfo::DXT1)
        mTexelFormat = eDXT1;
    else
        mTexelFormat = eGX_RGBA8;
}

// ************ READ PIXELS (PARTIAL DECODE) ************
void CTextureDecoder::ReadPixelsI4(CInputStream& src, COutputStream& dst)
{
    u8 px = src.ReadByte();
    dst.WriteByte(Extend4to8(px >> 4));
    dst.WriteByte(Extend4to8(px >> 4));
    dst.WriteByte(Extend4to8(px));
    dst.WriteByte(Extend4to8(px));
}

void CTextureDecoder::ReadPixelI8(CInputStream& src, COutputStream& dst)
{
    u8 px = src.ReadByte();
    dst.WriteByte(px);
    dst.WriteByte(px);
}

void CTextureDecoder::ReadPixelIA4(CInputStream& src, COutputStream& dst)
{
    // this can be left as-is for DDS conversion, but opengl doesn't support two components in one byte...
    u8 byte = src.ReadByte();
    u8 a = Extend4to8(byte >> 4);
    u8 l = Extend4to8(byte);
    dst.WriteShort((l << 8) | a);
}

void CTextureDecoder::ReadPixelIA8(CInputStream& src, COutputStream& dst)
{
    dst.WriteShort(src.ReadShort());
}

void CTextureDecoder::ReadPixelsC4(CInputStream& src, COutputStream& dst)
{
    // This isn't how C4 works, but due to the way Retro packed font textures (which use C4)
    // this is the only way to get them to decode correctly for now.
    // Commented-out code is proper C4 decoding. Dedicated font texture-decoding function
    // is probably going to be necessary in the future.
    u8 byte = src.ReadByte();
    u8 indices[2];
    indices[0] = (byte >> 4) & 0xF;
    indices[1] = byte & 0xF;

    for (u32 i = 0; i < 2; i++)
    {
        u8 r, g, b, a;
        ((indices[i] >> 3) & 0x1) ? r = 0xFF : r = 0x0;
        ((indices[i] >> 2) & 0x1) ? g = 0xFF : g = 0x0;
        ((indices[i] >> 1) & 0x1) ? b = 0xFF : b = 0x0;
        ((indices[i] >> 0) & 0x1) ? a = 0xFF : a = 0x0;
        u32 rgba = (r << 24) | (g << 16) | (b << 8) | (a);
        dst.WriteLong(rgba);

      /*paletteInput->Seek(indices[i] * 2, SEEK_SET);

             if (paletteFormat == PaletteIA8)    readPixelIA8(*paletteInput, dst);
        else if (paletteFormat == PaletteRGB565) readPixelRGB565(*paletteInput, dst);
        else if (paletteFormat == PaletteRGB5A3) readPixelRGB5A3(*paletteInput, dst);*/
    }
}

void CTextureDecoder::ReadPixelC8(CInputStream& src, COutputStream& dst)
{
    // DKCR fonts use C8 :|
    u8 index = src.ReadByte();

    /*u8 r, g, b, a;
    ((index >> 3) & 0x1) ? r = 0xFF : r = 0x0;
    ((index >> 2) & 0x1) ? g = 0xFF : g = 0x0;
    ((index >> 1) & 0x1) ? b = 0xFF : b = 0x0;
    ((index >> 0) & 0x1) ? a = 0xFF : a = 0x0;
    u32 rgba = (r << 24) | (g << 16) | (b << 8) | (a);
    dst.WriteLong(rgba);*/

    mPaletteInput.Seek(index * 2, SEEK_SET);

         if (mPaletteFormat == ePalette_IA8)    ReadPixelIA8(mPaletteInput, dst);
    else if (mPaletteFormat == ePalette_RGB565) ReadPixelRGB565(mPaletteInput, dst);
    else if (mPaletteFormat == ePalette_RGB5A3) ReadPixelRGB5A3(mPaletteInput, dst);
}

void CTextureDecoder::ReadPixelRGB565(CInputStream& src, COutputStream& dst)
{
    // RGB565 can be used as-is.
    dst.WriteShort(src.ReadShort());
}

void CTextureDecoder::ReadPixelRGB5A3(CInputStream& src, COutputStream& dst)
{
    u16 px = src.ReadShort();
    CColor c;

    if (px & 0x8000) // RGB5
    {
        c.b = Extend5to8(px >> 10);
        c.g = Extend5to8(px >>  5);
        c.r = Extend5to8(px >>  0);
        c.a = 0xFF;
    }

    else // RGB4A3
    {
        c.a = Extend3to8(px >> 12);
        c.b = Extend4to8(px >>  8);
        c.g = Extend4to8(px >>  4);
        c.r = Extend4to8(px >>  0);
    }

    dst.WriteLong(c.ToLongARGB());
}

void CTextureDecoder::ReadPixelRGBA8(CInputStream& src, COutputStream& dst)
{
    u16 ar = src.ReadShort();
    src.Seek(0x1E, SEEK_CUR);
    u16 gb = src.ReadShort();
    src.Seek(-0x20, SEEK_CUR);
    u32 px = (ar << 16) | gb;
    dst.WriteLong(px);
}

void CTextureDecoder::ReadSubBlockCMPR(CInputStream& src, COutputStream& dst)
{
    dst.WriteShort(src.ReadShort());
    dst.WriteShort(src.ReadShort());

    for (u32 byte = 0; byte < 4; byte++) {
        u8 b = src.ReadByte();
        b = ((b & 0x3) << 6) | ((b & 0xC) << 2) | ((b & 0x30) >> 2) | ((b & 0xC0) >> 6);
        dst.WriteByte(b);
    }
}

// ************ DECODE PIXELS (FULL DECODE TO RGBA8) ************
CColor CTextureDecoder::DecodePixelI4(u8 Byte, u8 WhichPixel)
{
    if (WhichPixel == 1) Byte >>= 4;
    u8 px = Extend4to8(Byte);
    return CColor(px, px, px, 0xFF);
}

CColor CTextureDecoder::DecodePixelI8(u8 Byte)
{
    return CColor(Byte, Byte, Byte, 0xFF);
}

CColor CTextureDecoder::DecodePixelIA4(u8 Byte)
{
    u8 Alpha = Extend4to8(Byte >> 4);
    u8 Lum = Extend4to8(Byte);
    return CColor(Lum, Lum, Lum, Alpha);
}

CColor CTextureDecoder::DecodePixelIA8(u16 Short)
{
    u8 Alpha = (Short >> 8) & 0xFF;
    u8 Lum = Short & 0xFF;
    return CColor(Lum, Lum, Lum, Alpha);
}

CColor CTextureDecoder::DecodePixelC4(u8 Byte, u8 WhichPixel, CInputStream& PaletteStream)
{
    if (WhichPixel == 1) Byte >>= 4;
    Byte &= 0xF;

    PaletteStream.Seek(Byte * 2, SEEK_SET);
    if (mPaletteFormat == ePalette_IA8)         return DecodePixelIA8(PaletteStream.ReadShort());
    else if (mPaletteFormat == ePalette_RGB565) return DecodePixelIA8(PaletteStream.ReadShort());
    else if (mPaletteFormat == ePalette_RGB5A3) return DecodePixelIA8(PaletteStream.ReadShort());
    else return CColor::skTransparentBlack;
}

CColor CTextureDecoder::DecodePixelC8(u8 Byte, CInputStream& PaletteStream)
{
    PaletteStream.Seek(Byte * 2, SEEK_SET);
    if (mPaletteFormat == ePalette_IA8)         return DecodePixelIA8(PaletteStream.ReadShort());
    else if (mPaletteFormat == ePalette_RGB565) return DecodePixelIA8(PaletteStream.ReadShort());
    else if (mPaletteFormat == ePalette_RGB5A3) return DecodePixelIA8(PaletteStream.ReadShort());
    else return CColor::skTransparentBlack;
}

CColor CTextureDecoder::DecodePixelRGB565(u16 Short)
{
    u8 b = Extend5to8( (u8) (Short >> 11) );
    u8 g = Extend6to8( (u8) (Short >> 5) );
    u8 r = Extend5to8( (u8) (Short) );
    return CColor(r, g, b, 0xFF);
}

CColor CTextureDecoder::DecodePixelRGB5A3(u16 Short)
{
    if (Short & 0x8000) // RGB5
    {
        u8 b = Extend5to8( (u8) (Short >> 10));
        u8 g = Extend5to8( (u8) (Short >> 5));
        u8 r = Extend5to8( (u8) (Short) );
        return CColor(r, g, b, 0xFF);
    }

    else // RGB4A3
    {
        u8 a = Extend3to8( (u8) (Short >> 12) );
        u8 b = Extend4to8( (u8) (Short >> 8) );
        u8 g = Extend4to8( (u8) (Short >> 4) );
        u8 r = Extend4to8( (u8) (Short) );
        return CColor(r, g, b, a);
    }
}

void CTextureDecoder::DecodeSubBlockCMPR(CInputStream& src, COutputStream& dst, u16 Width)
{
    CColor Palettes[4];
    u16 PaletteA = src.ReadShort();
    u16 PaletteB = src.ReadShort();
    Palettes[0] = DecodePixelRGB565(PaletteA);
    Palettes[1] = DecodePixelRGB565(PaletteB);

    if (PaletteA > PaletteB)
    {
        Palettes[2] = (Palettes[0] * 0.666666666f) + (Palettes[1] * 0.333333333f);
        Palettes[3] = (Palettes[0] * 0.333333333f) + (Palettes[1] * 0.666666666f);
    }
    else
    {
        Palettes[2] = (Palettes[0] * 0.5f) + (Palettes[1] * 0.5f);
        Palettes[3] = CColor::skTransparentBlack;
    }

    for (u32 y = 0; y < 4; y++)
    {
        u8 Byte = src.ReadByte();

        for (u32 x = 0; x < 4; x++)
        {
            u8 Shift = (u8) (6 - (x * 2));
            u8 PaletteIndex = (Byte >> Shift) & 0x3;
            CColor Pixel = Palettes[PaletteIndex];
            dst.WriteLong(Pixel.ToLongARGB());
        }

        dst.Seek((Width - 4) * 4, SEEK_CUR);
    }
}

void CTextureDecoder::DecodeBlockBC1(CInputStream& src, COutputStream& dst, u32 Width)
{
    // Very similar to the CMPR subblock function, but unfortunately a slight
    // difference in the order the pixel indices are read requires a separate function
    CColor Palettes[4];
    u16 PaletteA = src.ReadShort();
    u16 PaletteB = src.ReadShort();
    Palettes[0] = DecodePixelRGB565(PaletteA);
    Palettes[1] = DecodePixelRGB565(PaletteB);

    if (PaletteA > PaletteB)
    {
        Palettes[2] = (Palettes[0] * 0.666666666f) + (Palettes[1] * 0.333333333f);
        Palettes[3] = (Palettes[0] * 0.333333333f) + (Palettes[1] * 0.666666666f);
    }
    else
    {
        Palettes[2] = (Palettes[0] * 0.5f) + (Palettes[1] * 0.5f);
        Palettes[3] = CColor::skTransparentBlack;
    }

    for (u32 y = 0; y < 4; y++)
    {
        u8 Byte = src.ReadByte();

        for (u32 x = 0; x < 4; x++)
        {
            u8 Shift = (u8) (x * 2);
            u8 PaletteIndex = (Byte >> Shift) & 0x3;
            CColor Pixel = Palettes[PaletteIndex];
            dst.WriteLong(Pixel.ToLongARGB());
        }

        dst.Seek((Width - 4) * 4, SEEK_CUR);
    }
}

void CTextureDecoder::DecodeBlockBC2(CInputStream& src, COutputStream& dst, u32 Width)
{
    CColor CPalettes[4];
    u16 PaletteA = src.ReadShort();
    u16 PaletteB = src.ReadShort();
    CPalettes[0] = DecodePixelRGB565(PaletteA);
    CPalettes[1] = DecodePixelRGB565(PaletteB);

    if (PaletteA > PaletteB)
    {
        CPalettes[2] = (CPalettes[0] * 0.666666666f) + (CPalettes[1] * 0.333333333f);
        CPalettes[3] = (CPalettes[0] * 0.333333333f) + (CPalettes[1] * 0.666666666f);
    }
    else
    {
        CPalettes[2] = (CPalettes[0] * 0.5f) + (CPalettes[1] * 0.5f);
        CPalettes[3] = CColor::skTransparentBlack;
    }

    for (u32 y = 0; y < 4; y++)
    {
        u8 Byte = src.ReadByte();

        for (u32 x = 0; x < 4; x++)
        {
            u8 Shift = (u8) (x * 2);
            u8 PaletteIndex = (Byte >> Shift) & 0x3;
            CColor Pixel = CPalettes[PaletteIndex];
            dst.WriteLong(Pixel.ToLongARGB());
        }

        dst.Seek((Width - 4) * 4, SEEK_CUR);
    }
}

void CTextureDecoder::DecodeBlockBC3(CInputStream& src, COutputStream& dst, u32 Width)
{
    CColor Palettes[4];
    u16 PaletteA = src.ReadShort();
    u16 PaletteB = src.ReadShort();
    Palettes[0] = DecodePixelRGB565(PaletteA);
    Palettes[1] = DecodePixelRGB565(PaletteB);

    if (PaletteA > PaletteB)
    {
        Palettes[2] = (Palettes[0] * 0.666666666f) + (Palettes[1] * 0.333333333f);
        Palettes[3] = (Palettes[0] * 0.333333333f) + (Palettes[1] * 0.666666666f);
    }
    else
    {
        Palettes[2] = (Palettes[0] * 0.5f) + (Palettes[1] * 0.5f);
        Palettes[3] = CColor::skTransparentBlack;
    }

    for (u32 y = 0; y < 4; y++)
    {
        u8 Byte = src.ReadByte();

        for (u32 x = 0; x < 4; x++)
        {
            u8 Shift = (u8) (x * 2);
            u8 PaletteIndex = (Byte >> Shift) & 0x3;
            CColor Pixel = Palettes[PaletteIndex];
            dst.WriteLong(Pixel.ToLongARGB());
        }

        dst.Seek((Width - 4) * 4, SEEK_CUR);
    }
}

CColor CTextureDecoder::DecodeDDSPixel(CInputStream&)
{
    // Not using parameter 1 (CInputStream& - DDS)
    return CColor::skWhite;
}

// ************ UTILITY ************
u8 CTextureDecoder::Extend3to8(u8 in)
{
    in &= 0x7;
    return (in << 5) | (in << 2) | (in >> 1);
}

u8 CTextureDecoder::Extend4to8(u8 in)
{
    in &= 0xF;
    return (in << 4) | in;
}

u8 CTextureDecoder::Extend5to8(u8 in)
{
    in &= 0x1F;
    return (in << 3) | (in >> 2);
}

u8 CTextureDecoder::Extend6to8(u8 in)
{
    in &= 0x3F;
    return (in << 2) | (in >> 4);
}

u32 CTextureDecoder::CalculateShiftForMask(u32 BitMask)
{
    u32 Shift = 32;

    while (BitMask)
    {
        BitMask <<= 1;
        Shift--;
    }
    return Shift;
}

u32 CTextureDecoder::CalculateMaskBitCount(u32 BitMask)
{
    u32 Count = 0;

    while (BitMask)
    {
        if (BitMask & 0x1) Count++;
        BitMask >>= 1;
    }
    return Count;
}
