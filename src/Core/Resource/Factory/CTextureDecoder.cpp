#include "CTextureDecoder.h"
#include "Core/Resource/Texture/NTextureUtils.h"
#include <Common/Log.h>
#include <Common/CColor.h>
#include <Common/Math/MathUtil.h>

CTexture* CTextureDecoder::LoadTXTR(IInputStream& TXTR, CResourceEntry* pEntry)
{
    CTextureDecoder Decoder(pEntry);
    return Decoder.ReadTXTR(TXTR);
}

// ************ READ ************
CTexture* CTextureDecoder::ReadTXTR(IInputStream& TXTR)
{
    if (!TXTR.IsValid())
    {
        warnf("Texture stream is invalid");
        return nullptr;
    }

    mpTexture = new CTexture(mpEntry);

    // Read TXTR header
    mTexelFormat = EGXTexelFormat(TXTR.ReadLong());
    mSizeX = TXTR.ReadShort();
    mSizeY = TXTR.ReadShort();
    mNumMipMaps = TXTR.ReadLong();

    // For C4 and C8 images, read palette
    if ((mTexelFormat == EGXTexelFormat::C4) || (mTexelFormat == EGXTexelFormat::C8))
    {
        uint PaletteFormatID = TXTR.ReadLong();

        EGXTexelFormat PaletteFormat = (PaletteFormatID == 0 ? EGXTexelFormat::IA8 :
                                        PaletteFormatID == 1 ? EGXTexelFormat::RGB565 :
                                        PaletteFormatID == 2 ? EGXTexelFormat::RGB5A3 :
                                        EGXTexelFormat::Invalid);

        TXTR.Skip(4);

        // Parse in palette colors
        const STexelFormatInfo& EncodedFormatInfo = NTextureUtils::GetTexelFormatInfo(PaletteFormat);
        const STexelFormatInfo& DecodedFormatInfo = NTextureUtils::GetTexelFormatInfo(EncodedFormatInfo.EditorTexelFormat);
        uint32 PaletteEntryCount = (mTexelFormat == EGXTexelFormat::C4) ? 16 : 256;

        mPaletteTexelStride = DecodedFormatInfo.BitsPerPixel / 8;
        mPaletteData.resize(PaletteEntryCount * DecodedFormatInfo.BitsPerPixel / 8);
        CMemoryOutStream PaletteStream(mPaletteData.data(), mPaletteData.size(), EEndian::LittleEndian);

        for (uint EntryIdx = 0; EntryIdx < PaletteEntryCount; EntryIdx++)
        {
            ParseTexel(PaletteFormat, TXTR, PaletteStream);
        }
    }

    // Create mipmaps
    const STexelFormatInfo& FormatInfo = NTextureUtils::GetTexelFormatInfo(mTexelFormat);
    mpTexture->mMipData.resize(mNumMipMaps);
    uint SizeX = mSizeX, SizeY = mSizeY;

    for (uint MipIdx = 0; MipIdx < mNumMipMaps; MipIdx++)
    {
        SMipData& MipData = mpTexture->mMipData[MipIdx];
        MipData.SizeX = SizeX;
        MipData.SizeY = SizeY;

        uint MipDataSize = SizeX * SizeY * FormatInfo.BitsPerPixel / 8;
        MipData.GameDataBuffer.resize(MipDataSize);

        SizeX /= 2;
        SizeY /= 2;
    }

    // Read image data
    DecodeGXTexture(TXTR);
    return mpTexture;
}

// ************ DECODE ************
void CTextureDecoder::DecodeGXTexture(IInputStream& TXTR)
{
    const STexelFormatInfo& FormatInfo = NTextureUtils::GetTexelFormatInfo(mTexelFormat);

    uint MipSizeX = mSizeX;
    uint MipSizeY = mSizeY;
    uint BlockSizeX = FormatInfo.BlockSizeX;
    uint BlockSizeY = FormatInfo.BlockSizeY;
    uint BPP = FormatInfo.BitsPerPixel;

    // For CMPR, we parse per 4x4 block instead of per texel
    if (mTexelFormat == EGXTexelFormat::CMPR)
    {
        MipSizeX /= 4;
        MipSizeY /= 4;
        BlockSizeX /= 4;
        BlockSizeY /= 4;
        BPP *= 16;
    }

    // This value set to true if we hit the end of the file earlier than expected.
    // This is necessary due to a mistake Retro made in their cooker for I8 textures where very small mipmaps are cut off early, resulting in an out-of-bounds memory access.
    // This affects one texture that I know of - Echoes 3BB2C034
    bool bBreakEarly = false;

    for (uint MipIdx = 0; MipIdx < mNumMipMaps && !bBreakEarly; MipIdx++)
    {
        SMipData& MipData = mpTexture->mMipData[MipIdx];
        CMemoryOutStream MipStream( MipData.GameDataBuffer.data(), MipData.GameDataBuffer.size(), EEndian::LittleEndian );

        uint SizeX = Math::Max(MipSizeX, BlockSizeX);
        uint SizeY = Math::Max(MipSizeY, BlockSizeY);

        for (uint Y = 0; Y < SizeY && !bBreakEarly; Y += BlockSizeY)
        {
            for (uint X = 0; X < SizeX && !bBreakEarly; X += BlockSizeX)
            {
                for (uint BY = 0; BY < BlockSizeY && !bBreakEarly; BY++)
                {
                    for (uint BX = 0; BX < BlockSizeX && !bBreakEarly; BX++)
                    {
                        // For small mipmaps below the format's block size, skip texels outside the mipmap.
                        // The input data has texels for these dummy texels, but our output data doesn't.
                        if (BX >= BlockSizeX || BY >= BlockSizeY)
                        {
                            uint SkipAmount = Math::Max(BPP / 8, 1u);
                            TXTR.Skip(SkipAmount);
                        }
                        else
                        {
                            uint Col = X + BX;
                            uint Row = Y + BY;
                            uint DstPixel = (Row * SizeX) + Col;
                            uint DstOffset = (DstPixel * BPP) / 8;
                            MipStream.GoTo(DstOffset);
                            ParseTexel(mTexelFormat, TXTR, MipStream);
                        }

                        // ParseTexel parses two texels at a time for 4 bpp formats
                        if (FormatInfo.BitsPerPixel == 4 && mTexelFormat != EGXTexelFormat::CMPR)
                        {
                            BX++;
                        }

                        // Check if we reached the end of the file early
                        if (TXTR.EoF())
                        {
                            bBreakEarly = true;
                        }
                    }
                }
            }
        }

        MipSizeX /= 2;
        MipSizeY /= 2;
    }

    // Finalize texture
    mpTexture->mGameFormat = mTexelFormat;
    mpTexture->mEditorFormat = FormatInfo.EditorTexelFormat;
    mpTexture->GenerateEditorData();
    mpTexture->CreateRenderResources();
}

// ************ READ PIXELS (PARTIAL DECODE) ************
void CTextureDecoder::ParseTexel(EGXTexelFormat Format, IInputStream& Src, IOutputStream& Dst)
{
    switch (Format)
    {
    case EGXTexelFormat::I4:        ParseI4(Src, Dst);      break;
    case EGXTexelFormat::I8:        ParseI8(Src, Dst);      break;
    case EGXTexelFormat::IA4:       ParseIA4(Src, Dst);     break;
    case EGXTexelFormat::IA8:       ParseIA8(Src, Dst);     break;
    case EGXTexelFormat::C4:        ParseC4(Src, Dst);      break;
    case EGXTexelFormat::C8:        ParseC8(Src, Dst);      break;
    case EGXTexelFormat::C14x2:     /* Unsupported */       break;
    case EGXTexelFormat::RGB565:    ParseRGB565(Src, Dst);  break;
    case EGXTexelFormat::RGB5A3:    ParseRGB5A3(Src, Dst);  break;
    case EGXTexelFormat::RGBA8:     ParseRGBA8(Src, Dst);   break;
    case EGXTexelFormat::CMPR:      ParseCMPR(Src, Dst);    break;
    }
}

void CTextureDecoder::ParseI4(IInputStream& Src, IOutputStream& Dst)
{
    Dst.WriteByte( Src.ReadByte() );
}

void CTextureDecoder::ParseI8(IInputStream& Src, IOutputStream& Dst)
{
    Dst.WriteByte( Src.ReadByte() );
}

void CTextureDecoder::ParseIA4(IInputStream& Src, IOutputStream& Dst)
{
    Dst.WriteByte(Src.ReadByte());
}

void CTextureDecoder::ParseIA8(IInputStream& Src, IOutputStream& Dst)
{
    Dst.WriteShort(Src.ReadShort());
}

void CTextureDecoder::ParseC4(IInputStream& Src, IOutputStream& Dst)
{
    // This isn't how C4 works, but due to the way Retro packed font textures (which use C4)
    // this is the only way to get them to decode correctly for now.
    // Commented-out code is proper C4 decoding. Dedicated font texture-decoding function
    // is probably going to be necessary in the future.
    uint8 Byte = Src.ReadByte();
    uint8 Indices[2];
    Indices[0] = (Byte >> 4) & 0xF;
    Indices[1] = Byte & 0xF;

    for (uint32 Idx = 0; Idx < 2; Idx++)
    {
        uint8 R, G, B, A;
        ((Indices[Idx] >> 3) & 0x1) ? R = 0xFF : R = 0x0;
        ((Indices[Idx] >> 2) & 0x1) ? G = 0xFF : G = 0x0;
        ((Indices[Idx] >> 1) & 0x1) ? B = 0xFF : B = 0x0;
        ((Indices[Idx] >> 0) & 0x1) ? A = 0xFF : A = 0x0;
        uint32 RGBA = (R << 24) | (G << 16) | (B << 8) | (A);
        Dst.WriteLong(RGBA);

      /*void* pPaletteTexel = mPaletteData.data() + (Indices[iIdx] * mPaletteTexelStride);
        Dst.WriteBytes(pPaletteTexel, mPaletteTexelStride);*/
    }
}

void CTextureDecoder::ParseC8(IInputStream& Src, IOutputStream& Dst)
{
    // DKCR fonts use C8 :|
    uint8 Index = Src.ReadByte();

    uint8 R, G, B, A;
    ((Index >> 3) & 0x1) ? R = 0xFF : R = 0x0;
    ((Index >> 2) & 0x1) ? G = 0xFF : G = 0x0;
    ((Index >> 1) & 0x1) ? B = 0xFF : B = 0x0;
    ((Index >> 0) & 0x1) ? A = 0xFF : A = 0x0;
    uint32 RGBA = (R << 24) | (G << 16) | (B << 8) | (A);
    Dst.WriteLong(RGBA);

    /*void* pPaletteTexel = mPaletteData.data() + (Index * mPaletteTexelStride);
    Dst.WriteBytes(pPaletteTexel, mPaletteTexelStride);*/
}

void CTextureDecoder::ParseRGB565(IInputStream& Src, IOutputStream& Dst)
{
    // RGB565 can be used as-is.
    Dst.WriteShort(Src.ReadShort());
}

void CTextureDecoder::ParseRGB5A3(IInputStream& Src, IOutputStream& Dst)
{
    Dst.WriteShort(Src.ReadShort());
}

void CTextureDecoder::ParseRGBA8(IInputStream& Src, IOutputStream& Dst)
{
    Dst.WriteLong(Src.ReadLong());
}

void CTextureDecoder::ParseCMPR(IInputStream& Src, IOutputStream& Dst)
{
    Dst.WriteShort(Src.ReadShort());
    Dst.WriteShort(Src.ReadShort());

    for (uint i=0; i<4; i++)
    {
        uint8 Byte = Src.ReadByte();
        Byte = ((Byte & 0x03) << 6) |
               ((Byte & 0x0C) << 2) |
               ((Byte & 0x30) >> 2) |
               ((Byte & 0xC0) >> 6) ;
        Dst.WriteByte(Byte);
    }
}

CTextureDecoder::CTextureDecoder(CResourceEntry* pEntry)
    : mpEntry(pEntry)
{
}

CTextureDecoder::~CTextureDecoder()
{
}
