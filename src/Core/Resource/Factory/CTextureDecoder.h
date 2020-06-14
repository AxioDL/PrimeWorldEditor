#ifndef CTEXTUREDECODER_H
#define CTEXTUREDECODER_H

#include "Core/Resource/CTexture.h"
#include "Core/Resource/ETexelFormat.h"
#include <Common/BasicTypes.h>
#include <Common/CColor.h>
#include <Common/FileIO.h>

#include <memory>

class CTextureDecoder
{
    CResourceEntry *mpEntry;
    ETexelFormat mTexelFormat;
    uint16 mWidth, mHeight;
    uint32 mNumMipMaps;

    bool mHasPalettes;
    EGXPaletteFormat mPaletteFormat;
    std::vector<uint8> mPalettes;
    CMemoryInStream mPaletteInput;

    struct SDDSInfo
    {
        enum { DXT1, DXT2, DXT3, DXT4, DXT5, RGBA } Format;
        uint32 Flags;
        uint32 BitCount;
        uint32 RBitMask, GBitMask, BBitMask, ABitMask;
        uint32 RShift, GShift, BShift, AShift;
        uint32 RSize, GSize, BSize, ASize;
    } mDDSInfo;

    uint8 *mpDataBuffer;
    uint32 mDataBufferSize;

    // Private Functions
    CTextureDecoder();
    ~CTextureDecoder();
    std::unique_ptr<CTexture> CreateTexture();

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
    CColor DecodePixelI4(uint8 Byte, uint8 WhichPixel);
    CColor DecodePixelI8(uint8 Byte);
    CColor DecodePixelIA4(uint8 Byte);
    CColor DecodePixelIA8(uint16 Short);
    CColor DecodePixelC4(uint8 Byte, uint8 WhichPixel, IInputStream& rPaletteStream);
    CColor DecodePixelC8(uint8 Byte, IInputStream& rPaletteStream);
    CColor DecodePixelRGB565(uint16 Short);
    CColor DecodePixelRGB5A3(uint16 Short);
    void DecodeSubBlockCMPR(IInputStream& rSrc, IOutputStream& rDst, uint16 Width);

    void DecodeBlockBC1(IInputStream& rSrc, IOutputStream& rDst, uint32 Width);
    void DecodeBlockBC2(IInputStream& rSrc, IOutputStream& rDst, uint32 Width);
    void DecodeBlockBC3(IInputStream& rSrc, IOutputStream& rDst, uint32 Width);
    CColor DecodeDDSPixel(IInputStream& rDDS);

    // Static
public:
    static std::unique_ptr<CTexture> LoadTXTR(IInputStream& rTXTR, CResourceEntry *pEntry);
    static std::unique_ptr<CTexture> LoadDDS(IInputStream& rDDS, CResourceEntry *pEntry);
    static std::unique_ptr<CTexture> DoFullDecode(IInputStream& rTXTR, CResourceEntry *pEntry);
    static CTexture* DoFullDecode(CTexture *pTexture);
};

#endif // CTEXTUREDECODER_H
