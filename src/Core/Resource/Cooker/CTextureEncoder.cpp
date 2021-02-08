#include "CTextureEncoder.h"
#include <Common/Log.h>

CTextureEncoder::CTextureEncoder() = default;

void CTextureEncoder::WriteTXTR(IOutputStream& rTXTR)
{
    // Only DXT1->CMPR supported at the moment
    rTXTR.WriteULong(static_cast<uint>(mOutputFormat));
    rTXTR.WriteUShort(mpTexture->mWidth);
    rTXTR.WriteUShort(mpTexture->mHeight);
    rTXTR.WriteULong(mpTexture->mNumMipMaps);

    uint32 MipW = mpTexture->Width() / 4;
    uint32 MipH = mpTexture->Height() / 4;
    CMemoryInStream Image(mpTexture->mpImgDataBuffer, mpTexture->mImgDataSize, EEndian::LittleEndian);
    uint32 MipOffset = Image.Tell();

    for (uint32 iMip = 0; iMip < mpTexture->mNumMipMaps; iMip++)
    {
        for (uint32 iBlockY = 0; iBlockY < MipH; iBlockY += 2)
        {
            for (uint32 iBlockX = 0; iBlockX < MipW; iBlockX += 2)
            {
                for (uint32 iImgY = iBlockY; iImgY < iBlockY + 2; iImgY++)
                {
                    for (uint32 iImgX = iBlockX; iImgX < iBlockX + 2; iImgX++)
                    {
                        uint32 SrcPos = ((iImgY * MipW) + iImgX) * 8;
                        Image.Seek(MipOffset + SrcPos, SEEK_SET);

                        ReadSubBlockCMPR(Image, rTXTR);
                    }
                }
            }
        }

        MipOffset += MipW * MipH * 8;
        MipW /= 2;
        MipH /= 2;

        if (MipW < 2)
            MipW = 2;

        if (MipH < 2)
            MipH = 2;
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
        uint8 Byte = rSource.ReadUByte();
        Byte = ((Byte & 0x3) << 6) | ((Byte & 0xC) << 2) | ((Byte & 0x30) >> 2) | ((Byte & 0xC0) >> 6);
        rDest.WriteUByte(Byte);
    }
}

// ************ STATIC ************
void CTextureEncoder::EncodeTXTR(IOutputStream& rTXTR, CTexture *pTex)
{
    if (pTex->mTexelFormat != ETexelFormat::DXT1)
    {
        errorf("Unsupported texel format for decoding");
        return;
    }

    CTextureEncoder Encoder;
    Encoder.mpTexture = pTex;
    Encoder.mSourceFormat = ETexelFormat::DXT1;
    Encoder.mOutputFormat = ETexelFormat::GX_CMPR;
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
    case ETexelFormat::Luminance:       return ETexelFormat::GX_I8;
    case ETexelFormat::LuminanceAlpha:  return ETexelFormat::GX_IA8;
    case ETexelFormat::RGBA4:           return ETexelFormat::GX_RGB5A3;
    case ETexelFormat::RGB565:          return ETexelFormat::GX_RGB565;
    case ETexelFormat::RGBA8:           return ETexelFormat::GX_RGBA8;
    case ETexelFormat::DXT1:            return ETexelFormat::GX_CMPR;
    default:                            return ETexelFormat::Invalid;
    }
}

ETexelFormat CTextureEncoder::GetFormat(ETexelFormat Format)
{
    switch (Format)
    {
    case ETexelFormat::GX_I4:   return ETexelFormat::Luminance;
    case ETexelFormat::GX_I8:   return ETexelFormat::Luminance;
    case ETexelFormat::GX_IA4:  return ETexelFormat::LuminanceAlpha;
    case ETexelFormat::GX_IA8:  return ETexelFormat::LuminanceAlpha;
        // todo rest of these
    case ETexelFormat::GX_CMPR: return ETexelFormat::DXT1;
    default:                    return ETexelFormat::Invalid;
    }
}
