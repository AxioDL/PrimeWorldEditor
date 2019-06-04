#ifndef CTEXTUREDECODER_H
#define CTEXTUREDECODER_H

#include "Core/Resource/Texture/CTexture.h"
#include "Core/Resource/Texture/ETexelFormat.h"
#include <Common/BasicTypes.h>
#include <Common/CColor.h>

#include <Common/FileIO.h>

class CTextureDecoder
{
    CResourceEntry* mpEntry;
    CTexture* mpTexture;

    // Texture asset data
    EGXTexelFormat mTexelFormat;
    uint16 mSizeX, mSizeY;
    uint32 mNumMipMaps;

    // Palette data
    std::vector<uint8> mPaletteData;
    uint32 mPaletteTexelStride;

    // Decode
    void DecodeGXTexture(IInputStream& TXTR);
    void ParseTexel(EGXTexelFormat Format, IInputStream& Src, IOutputStream& Dst);
    void ParseI4(IInputStream& Src, IOutputStream& Dst);
    void ParseI8(IInputStream& Src, IOutputStream& Dst);
    void ParseIA4(IInputStream& Src, IOutputStream& Dst);
    void ParseIA8(IInputStream& Src, IOutputStream& Dst);
    void ParseC4(IInputStream& Src, IOutputStream& Dst);
    void ParseC8(IInputStream& Src, IOutputStream& Dst);
    void ParseRGB565(IInputStream& Src, IOutputStream& Dst);
    void ParseRGB5A3(IInputStream& Src, IOutputStream& Dst);
    void ParseRGBA8(IInputStream& Src, IOutputStream& Dst);
    void ParseCMPR(IInputStream& Src, IOutputStream& Dst);

    CTextureDecoder(CResourceEntry* pEntry);
    ~CTextureDecoder();
    CTexture* ReadTXTR(IInputStream& TXTR);

public:
    static CTexture* LoadTXTR(IInputStream& TXTR, CResourceEntry* pEntry);

    // Utility
    static uint8 Extend3to8(uint8 In);
    static uint8 Extend4to8(uint8 In);
    static uint8 Extend5to8(uint8 In);
    static uint8 Extend6to8(uint8 In);
};

#endif // CTEXTUREDECODER_H
