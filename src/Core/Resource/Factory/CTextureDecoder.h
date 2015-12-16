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
    void ReadTXTR(IInputStream& TXTR);
    void ReadDDS(IInputStream& DDS);

    // Decode
    void PartialDecodeGXTexture(IInputStream& TXTR);
    void FullDecodeGXTexture(IInputStream& TXTR);
    void DecodeDDS(IInputStream& DDS);

    // Decode Pixels (preserve compression)
    void ReadPixelsI4(IInputStream& src, IOutputStream& dst);
    void ReadPixelI8(IInputStream& src, IOutputStream& dst);
    void ReadPixelIA4(IInputStream& src, IOutputStream& dst);
    void ReadPixelIA8(IInputStream& src, IOutputStream& dst);
    void ReadPixelsC4(IInputStream& src, IOutputStream& dst);
    void ReadPixelC8(IInputStream& src, IOutputStream& dst);
    void ReadPixelRGB565(IInputStream& src, IOutputStream& dst);
    void ReadPixelRGB5A3(IInputStream& src, IOutputStream& dst);
    void ReadPixelRGBA8(IInputStream& src, IOutputStream& dst);
    void ReadSubBlockCMPR(IInputStream& src, IOutputStream& dst);

    // Decode Pixels (convert to RGBA8)
    CColor DecodePixelI4(u8 Byte, u8 WhichPixel);
    CColor DecodePixelI8(u8 Byte);
    CColor DecodePixelIA4(u8 Byte);
    CColor DecodePixelIA8(u16 Short);
    CColor DecodePixelC4(u8 Byte, u8 WhichPixel, IInputStream& PaletteStream);
    CColor DecodePixelC8(u8 Byte, IInputStream& PaletteStream);
    CColor DecodePixelRGB565(u16 Short);
    CColor DecodePixelRGB5A3(u16 Short);
    CColor DecodePixelRGBA8(IInputStream& src, IOutputStream& dst);
    void DecodeSubBlockCMPR(IInputStream& src, IOutputStream& dst, u16 Width);

    void DecodeBlockBC1(IInputStream& src, IOutputStream& dst, u32 Width);
    void DecodeBlockBC2(IInputStream& src, IOutputStream& dst, u32 Width);
    void DecodeBlockBC3(IInputStream& src, IOutputStream& dst, u32 Width);
    CColor DecodeDDSPixel(IInputStream& DDS);

    // Static
public:
    static CTexture* LoadTXTR(IInputStream& TXTR);
    static CTexture* LoadDDS(IInputStream& DDS);
    static CTexture* DoFullDecode(IInputStream& TXTR);
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
