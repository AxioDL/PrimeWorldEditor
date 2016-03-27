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
    void ReadTXTR(IInputStream& rTXTR);
    void ReadDDS(IInputStream& rDDS);

    // Decode
    void PartialDecodeGXTexture(IInputStream& rTXTR);
    void FullDecodeGXTexture(IInputStream& rTXTR);
    void DecodeDDS(IInputStream& rDDS);

    // Decode Pixels (preserve compression)
    void ReadPixelsI4(IInputStream& rSrc, IOutputStream& rDst);
    void ReadPixelI8(IInputStream& rSrc, IOutputStream& rDst);
    void ReadPixelIA4(IInputStream& rSrc, IOutputStream& rDst);
    void ReadPixelIA8(IInputStream& rSrc, IOutputStream& rDst);
    void ReadPixelsC4(IInputStream& rSrc, IOutputStream& rDst);
    void ReadPixelC8(IInputStream& rSrc, IOutputStream& rDst);
    void ReadPixelRGB565(IInputStream& rSrc, IOutputStream& rDst);
    void ReadPixelRGB5A3(IInputStream& rSrc, IOutputStream& rDst);
    void ReadPixelRGBA8(IInputStream& rSrc, IOutputStream& rDst);
    void ReadSubBlockCMPR(IInputStream& rSrc, IOutputStream& rDst);

    // Decode Pixels (convert to RGBA8)
    CColor DecodePixelI4(u8 Byte, u8 WhichPixel);
    CColor DecodePixelI8(u8 Byte);
    CColor DecodePixelIA4(u8 Byte);
    CColor DecodePixelIA8(u16 Short);
    CColor DecodePixelC4(u8 Byte, u8 WhichPixel, IInputStream& rPaletteStream);
    CColor DecodePixelC8(u8 Byte, IInputStream& rPaletteStream);
    CColor DecodePixelRGB565(u16 Short);
    CColor DecodePixelRGB5A3(u16 Short);
    CColor DecodePixelRGBA8(IInputStream& rSrc, IOutputStream& rDst);
    void DecodeSubBlockCMPR(IInputStream& rSrc, IOutputStream& rDst, u16 Width);

    void DecodeBlockBC1(IInputStream& rSrc, IOutputStream& rDst, u32 Width);
    void DecodeBlockBC2(IInputStream& rSrc, IOutputStream& rDst, u32 Width);
    void DecodeBlockBC3(IInputStream& rSrc, IOutputStream& rDst, u32 Width);
    CColor DecodeDDSPixel(IInputStream& rDDS);

    // Static
public:
    static CTexture* LoadTXTR(IInputStream& rTXTR);
    static CTexture* LoadDDS(IInputStream& rDDS);
    static CTexture* DoFullDecode(IInputStream& rTXTR);
    static CTexture* DoFullDecode(CTexture *pTexture);

    // Utility
    static u8 Extend3to8(u8 In);
    static u8 Extend4to8(u8 In);
    static u8 Extend5to8(u8 In);
    static u8 Extend6to8(u8 In);
    static u32 CalculateShiftForMask(u32 BitMask);
    static u32 CalculateMaskBitCount(u32 BitMask);
};

#endif // CTEXTUREDECODER_H
