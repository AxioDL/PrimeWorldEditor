#ifndef CTEXTUREDECODER_H
#define CTEXTUREDECODER_H

#include "Core/Resource/CTexture.h"
#include "Core/Resource/ETexelFormat.h"
#include <Common/CColor.h>
#include <Common/types.h>

#include <FileIO/FileIO.h>

class CTextureDecoder
{
    ETexelFormat mTexelFormat;
    u16 mWidth, mHeight;
    u32 mNumMipMaps;

    bool mHasPalettes;
    EGXPaletteFormat mPaletteFormat;
    std::vector<u8> mPalettes;
    CMemoryInStream mPaletteInput;

    struct SDDSInfo
    {
        enum { DXT1, DXT2, DXT3, DXT4, DXT5, RGBA } Format;
        u32 Flags;
        u32 BitCount;
        u32 RBitMask, GBitMask, BBitMask, ABitMask;
        u32 RShift, GShift, BShift, AShift;
        u32 RSize, GSize, BSize, ASize;
    } mDDSInfo;

    u8 *mpDataBuffer;
    u32 mDataBufferSize;

    // Private Functions
    CTextureDecoder();
    ~CTextureDecoder();
    CTexture* CreateTexture();

    // Read
    void ReadTXTR(CInputStream& TXTR);
    void ReadDDS(CInputStream& DDS);

    // Decode
    void PartialDecodeGXTexture(CInputStream& TXTR);
    void FullDecodeGXTexture(CInputStream& TXTR);
    void DecodeDDS(CInputStream& DDS);

    // Decode Pixels (preserve compression)
    void ReadPixelsI4(CInputStream& src, COutputStream& dst);
    void ReadPixelI8(CInputStream& src, COutputStream& dst);
    void ReadPixelIA4(CInputStream& src, COutputStream& dst);
    void ReadPixelIA8(CInputStream& src, COutputStream& dst);
    void ReadPixelsC4(CInputStream& src, COutputStream& dst);
    void ReadPixelC8(CInputStream& src, COutputStream& dst);
    void ReadPixelRGB565(CInputStream& src, COutputStream& dst);
    void ReadPixelRGB5A3(CInputStream& src, COutputStream& dst);
    void ReadPixelRGBA8(CInputStream& src, COutputStream& dst);
    void ReadSubBlockCMPR(CInputStream& src, COutputStream& dst);

    // Decode Pixels (convert to RGBA8)
    CColor DecodePixelI4(u8 Byte, u8 WhichPixel);
    CColor DecodePixelI8(u8 Byte);
    CColor DecodePixelIA4(u8 Byte);
    CColor DecodePixelIA8(u16 Short);
    CColor DecodePixelC4(u8 Byte, u8 WhichPixel, CInputStream& PaletteStream);
    CColor DecodePixelC8(u8 Byte, CInputStream& PaletteStream);
    CColor DecodePixelRGB565(u16 Short);
    CColor DecodePixelRGB5A3(u16 Short);
    CColor DecodePixelRGBA8(CInputStream& src, COutputStream& dst);
    void DecodeSubBlockCMPR(CInputStream& src, COutputStream& dst, u16 Width);

    void DecodeBlockBC1(CInputStream& src, COutputStream& dst, u32 Width);
    void DecodeBlockBC2(CInputStream& src, COutputStream& dst, u32 Width);
    void DecodeBlockBC3(CInputStream& src, COutputStream& dst, u32 Width);
    CColor DecodeDDSPixel(CInputStream& DDS);

    // Static
public:
    static CTexture* LoadTXTR(CInputStream& TXTR);
    static CTexture* LoadDDS(CInputStream& DDS);
    static CTexture* DoFullDecode(CInputStream& TXTR);
    static CTexture* DoFullDecode(CTexture *pTexture);

    // Utility
    static u8 Extend3to8(u8 in);
    static u8 Extend4to8(u8 in);
    static u8 Extend5to8(u8 in);
    static u8 Extend6to8(u8 in);
    static u32 CalculateShiftForMask(u32 BitMask);
    static u32 CalculateMaskBitCount(u32 BitMask);
};

#endif // CTEXTUREDECODER_H
