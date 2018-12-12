#include "CTextureEncoder.h"
#include <Common/Log.h>

CTextureEncoder::CTextureEncoder()
    : mpTexture(nullptr)
{
}

void CTextureEncoder::WriteTXTR(IOutputStream& rTXTR)
{
    // Only DXT1->CMPR supported at the moment
    rTXTR.WriteLong(mOutputFormat);
    rTXTR.WriteShort(mpTexture->mWidth);
    rTXTR.WriteShort(mpTexture->mHeight);
    rTXTR.WriteLong(mpTexture->mNumMipMaps);

    uint32 MipW = mpTexture->Width() / 4;
    uint32 MipH = mpTexture->Height() / 4;
    CMemoryInStream Image(mpTexture->mpImgDataBuffer, mpTexture->mImgDataSize, IOUtil::eLittleEndian);
    uint32 MipOffset = Image.Tell();

    for (uint32 iMip = 0; iMip < mpTexture->mNumMipMaps; iMip++)
    {
        for (uint32 iBlockY = 0; iBlockY < MipH; iBlockY += 2)
            for (uint32 iBlockX = 0; iBlockX < MipW; iBlockX += 2)
                for (uint32 iImgY = iBlockY; iImgY < iBlockY + 2; iImgY++)
                    for (uint32 iImgX = iBlockX; iImgX < iBlockX + 2; iImgX++)
                    {
                        uint32 SrcPos = ((iImgY * MipW) + iImgX) * 8;
                        Image.Seek(MipOffset + SrcPos, SEEK_SET);

                        ReadSubBlockCMPR(Image, rTXTR);
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

void CTextureEncoder::ReadSubBlockCMPR(IInputStream& rSource, IOutputStream& rDest)
{
    rDest.WriteShort(rSource.ReadShort());
    rDest.WriteShort(rSource.ReadShort());

    for (uint32 iByte = 0; iByte < 4; iByte++)
    {
        uint8 Byte = rSource.ReadByte();
        Byte = ((Byte & 0x3) << 6) | ((Byte & 0xC) << 2) | ((Byte & 0x30) >> 2) | ((Byte & 0xC0) >> 6);
        rDest.WriteByte(Byte);
    }
}

// ************ STATIC ************
void CTextureEncoder::EncodeTXTR(IOutputStream& rTXTR, CTexture *pTex)
{
    if (pTex->mTexelFormat != eDXT1)
    {
        errorf("Unsupported texel format for decoding");
        return;
    }

    CTextureEncoder Encoder;
    Encoder.mpTexture = pTex;
    Encoder.mSourceFormat = eDXT1;
    Encoder.mOutputFormat = eGX_CMPR;
    Encoder.WriteTXTR(rTXTR);
}

void CTextureEncoder::EncodeTXTR(IOutputStream& rTXTR, CTexture *pTex, ETexelFormat /*OutputFormat*/)
{
    // todo: support for encoding a specific format
    EncodeTXTR(rTXTR, pTex);
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
