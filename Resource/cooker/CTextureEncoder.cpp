#include "CTextureEncoder.h"
#include <iostream>

CTextureEncoder::CTextureEncoder()
{
    mpTexture = nullptr;
}

void CTextureEncoder::WriteTXTR(COutputStream& TXTR)
{
    // Only DXT1->CMPR supported at the moment
    TXTR.WriteLong(mOutputFormat);
    TXTR.WriteShort(mpTexture->mWidth);
    TXTR.WriteShort(mpTexture->mHeight);
    TXTR.WriteLong(mpTexture->mNumMipMaps);

    u32 MipW = mpTexture->Width() / 4;
    u32 MipH = mpTexture->Height() / 4;
    CMemoryInStream Image(mpTexture->mImgDataBuffer, mpTexture->mImgDataSize, IOUtil::LittleEndian);
    u32 MipOffset = Image.Tell();

    for (u32 iMip = 0; iMip < mpTexture->mNumMipMaps; iMip++)
    {
        for (u32 BlockY = 0; BlockY < MipH; BlockY += 2)
            for (u32 BlockX = 0; BlockX < MipW; BlockX += 2)
                for (u32 ImgY = BlockY; ImgY < BlockY + 2; ImgY++)
                    for (u32 ImgX = BlockX; ImgX < BlockX + 2; ImgX++)
                    {
                        u32 SrcPos = ((ImgY * MipW) + ImgX) * 8;
                        Image.Seek(MipOffset + SrcPos, SEEK_SET);

                        ReadSubBlockCMPR(Image, TXTR);
                    }

        MipOffset += MipW * MipH * 8;
        MipW /= 2;
        MipH /= 2;
        if (MipW < 2) MipW = 2;
        if (MipH < 2) MipH = 2;
    }
}

void CTextureEncoder::DetermineBestOutputFormat()
{
    // todo
}

void CTextureEncoder::ReadSubBlockCMPR(CInputStream& Source, COutputStream& Dest)
{
    Dest.WriteShort(Source.ReadShort());
    Dest.WriteShort(Source.ReadShort());

    for (u32 byte = 0; byte < 4; byte++) {
        u8 b = Source.ReadByte();
        b = ((b & 0x3) << 6) | ((b & 0xC) << 2) | ((b & 0x30) >> 2) | ((b & 0xC0) >> 6);
        Dest.WriteByte(b);
    }
}

// ************ STATIC ************
void CTextureEncoder::EncodeTXTR(COutputStream& TXTR, CTexture *pTex)
{
    if (pTex->mTexelFormat != eDXT1)
    {
        std::cout << "\rError: Unsupported texel format for decoding\n";
        return;
    }

    CTextureEncoder Encoder;
    Encoder.mpTexture = pTex;
    Encoder.mSourceFormat = eDXT1;
    Encoder.mOutputFormat = eGX_CMPR;
    Encoder.WriteTXTR(TXTR);
}

void CTextureEncoder::EncodeTXTR(COutputStream& TXTR, CTexture *pTex, ETexelFormat /*OutputFormat*/)
{
    // todo: support for encoding a specific format
    EncodeTXTR(TXTR, pTex);
}

ETexelFormat CTextureEncoder::GetGXFormat(ETexelFormat Format)
{
    switch (Format)
    {
    case eLuminance: return eGX_I8;
    case eLuminanceAlpha: return eGX_IA8;
    case eRGBA4: return eGX_RGB5A3;
    case eRGB565: return eGX_RGB565;
    case eRGBA8: return eGX_RGBA8;
    case eDXT1: return eGX_CMPR;
    default: return eInvalidTexelFormat;
    }
}

ETexelFormat CTextureEncoder::GetFormat(ETexelFormat Format)
{
    switch (Format)
    {
    case eGX_I4: return eLuminance;
    case eGX_I8: return eLuminance;
    case eGX_IA4: return eLuminanceAlpha;
    case eGX_IA8: return eLuminanceAlpha;
        // todo rest of these
    case eGX_CMPR: return eDXT1;
    default: return eInvalidTexelFormat;
    }
}
