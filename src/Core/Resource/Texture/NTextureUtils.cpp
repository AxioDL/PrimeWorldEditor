#include "NTextureUtils.h"
#include <Common/Common.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT ASSERT
#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_image_write.h>

namespace NTextureUtils
{

/** Table of image format info; indexed by EGXTexelFormat. */
STexelFormatInfo kTexelFormatInfo[] =
{
    // GameTexelFormat          EditorTexelFormat               BlockSizeX  BlockSizeY  BitsPerPixel
    {  EGXTexelFormat::I4,      ETexelFormat::Luminance,        8,          8,          4   },
    {  EGXTexelFormat::I8,      ETexelFormat::Luminance,        8,          4,          8   },
    {  EGXTexelFormat::IA4,     ETexelFormat::LuminanceAlpha,   8,          4,          8   },
    {  EGXTexelFormat::IA8,     ETexelFormat::LuminanceAlpha,   4,          4,          16  },
    {  EGXTexelFormat::C4,      ETexelFormat::Invalid,          8,          8,          4   },
    {  EGXTexelFormat::C8,      ETexelFormat::Invalid,          8,          4,          8   },
    {  EGXTexelFormat::C14x2,   ETexelFormat::Invalid,          4,          4,          16  },
    {  EGXTexelFormat::RGB565,  ETexelFormat::RGB565,           4,          4,          16  },
    {  EGXTexelFormat::RGB5A3,  ETexelFormat::RGBA8,            4,          4,          16  },
    {  EGXTexelFormat::RGBA8,   ETexelFormat::RGBA8,            4,          4,          32  },
    {  EGXTexelFormat::CMPR,    ETexelFormat::RGBA8,            8,          8,          4   },
};

/** Remap an ETexelFormat to the closest EGXTexelFormat */
const EGXTexelFormat kEditorFormatToGameFormat[] =
{
    // Luminance
    EGXTexelFormat::I8,
    // LuminanceAlpha
    EGXTexelFormat::IA8,
    // RGB565
    EGXTexelFormat::RGB565,
    // RGBA8
    EGXTexelFormat::RGBA8,
};

/** Retrieve the format info for a given texel format */
const STexelFormatInfo& GetTexelFormatInfo(EGXTexelFormat Format)
{
    uint FormatIdx = (uint) Format;
    ASSERT( FormatIdx >= 0 && FormatIdx < ARRAY_SIZE(kTexelFormatInfo) );

    return kTexelFormatInfo[(uint) Format];
}

const STexelFormatInfo& GetTexelFormatInfo(ETexelFormat Format)
{
    return GetTexelFormatInfo( kEditorFormatToGameFormat[(uint) Format] );
}

/** Utility functions useful for converting image formats  */
FORCEINLINE static uint8 Extend3to8(uint8 In)
{
    In &= 0x7;
    return (In << 5) | (In << 2) | (In >> 1);
}

FORCEINLINE static uint8 Extend4to8(uint8 In)
{
    In &= 0xF;
    return (In << 4) | In;
}

FORCEINLINE static uint8 Extend5to8(uint8 In)
{
    In &= 0x1F;
    return (In << 3) | (In >> 2);
}

FORCEINLINE static uint8 Extend6to8(uint8 In)
{
    In &= 0x3F;
    return (In << 2) | (In >> 4);
}

/** Decompose an RGBA dword into 8-bit components */
FORCEINLINE static void DecomposeRGBA8(uint32 In,
                                       uint8& OutR,
                                       uint8& OutG,
                                       uint8& OutB,
                                       uint8& OutA)
{
    OutR = (In >> 24) & 0xFF;
    OutG = (In >> 16) & 0xFF;
    OutB = (In >>  8) & 0xFF;
    OutA = (In >>  0) & 0xFF;
}

/** Compose an RGBA dword from 8-bit components */
FORCEINLINE static uint32 ComposeRGBA8(uint8 R,
                                       uint8 G,
                                       uint8 B,
                                       uint8 A)
{
    return (R << 24) | (G << 16) | (B << 8) | A;
}

/** Convert an RGB5A3 word into an RGBA8 dword */
static uint32 RGB5A3toRGBA8(uint16 Texel)
{
    // RGB5A3 uses a per-texel sign bit to swap between two formats.
    // Based on this bit, the texel is either RGB555 or RGB4A3.
    uint8 R, G, B, A;

    if (Texel & 0x8000)
    {
        R = Extend5to8(Texel >> 10);
        G = Extend5to8(Texel >>  5);
        B = Extend5to8(Texel >>  0);
        A = 255;
    }
    else
    {
        R = Extend4to8(Texel >> 11);
        G = Extend4to8(Texel >>  7);
        B = Extend4to8(Texel >>  3);
        A = Extend3to8(Texel >>  0);
    }

    return ComposeRGBA8(R, G, B, A);
}

/** Convert an RGB565 word into an RGBA8 dword */
static uint32 RGB565toRGBA8(uint16 Texel)
{
    uint8 R = Extend5to8( (Texel >> 11) & 0x1F );
    uint8 G = Extend6to8( (Texel >>  5) & 0x3F );
    uint8 B = Extend5to8( (Texel >>  0) & 0x1F );
    return ComposeRGBA8(R, G, B, 255);
}

/** A texel block in the CMPR/BC1 format */
struct SCMPRBlock
{
    uint16 C0;
    uint16 C1;
    uint8 Idx[4];
};

/** Extract the four palette colors from a CMPR texel block */
static void DecomposeCMPRPalettes(const SCMPRBlock& kBlock,
                                  uint32& OutC0,
                                  uint32& OutC1,
                                  uint32& OutC2,
                                  uint32& OutC3)
{
    // Get block palette colors and decompose into RGBA components
    uint8 R0, G0, B0, A0,
          R1, G1, B1, A1,
          R2, G2, B2, A2,
          R3, G3, B3, A3;

    OutC0 = RGB565toRGBA8(kBlock.C0);
    OutC1 = RGB565toRGBA8(kBlock.C1);
    DecomposeRGBA8(OutC0, R0, G0, B0, A0);
    DecomposeRGBA8(OutC1, R1, G1, B1, A1);

    // Interpolate to get the remaining palette colors
    if (kBlock.C0 > kBlock.C1)
    {
        // GameCube hardware interpolates at 3/8 and 5/8 points
        // This differs from PC DXT1 implementation that interpolates at 1/3 and 2/3
        R2 = (R0*5 + R1*3) >> 3;
        G2 = (G0*5 + G1*3) >> 3;
        B2 = (B0*5 + B1*3) >> 3;
        A2 = 255;

        R3 = (R0*3 + R1*5) >> 3;
        G3 = (G0*3 + G1*5) >> 3;
        B3 = (B0*3 + B1*5) >> 3;
        A3 = 255;
    }
    else
    {
        // GameCube hardware sets the color of C3 as the same as C2 instead of black
        R2 = R3 = (R0 + R1) / 2;
        G2 = G3 = (G0 + G1) / 2;
        B2 = B3 = (B0 + B1) / 2;
        A2 = 255, A3 = 0;
    }

    OutC2 = ComposeRGBA8(R2, G2, B2, A2);
    OutC3 = ComposeRGBA8(R3, G3, B3, A3);
}

/**
 *  Converts game texel data to the corresponding editor format.
 *  "Game Data" is essentially the texture data that is contained
 *  in a TXTR file, except without swizzling and with fixed endianness.
 *  The output texel format is the same as specified by GetTexelFormatInfo().
 */
void ConvertGameDataToEditorData(EGXTexelFormat SrcFormat,
                                 uint32 SizeX,
                                 uint32 SizeY,
                                 const uint8* pkSrcData,
                                 uint SrcDataSize,
                                 std::vector<uint8>& DstData,
                                 ETexelFormat* pOutTexelFormat /*= nullptr*/)
{
    //@todo palette formats are unsupported
    const STexelFormatInfo& kSrcTexelFormat = GetTexelFormatInfo( SrcFormat );
    const STexelFormatInfo& kDstTexelFormat = GetTexelFormatInfo( kSrcTexelFormat.EditorTexelFormat );
    uint SrcSize = (SizeX * SizeY * kSrcTexelFormat.BitsPerPixel) / 8;
    uint DstSize = (SizeX * SizeY * kDstTexelFormat.BitsPerPixel) / 8;
    ASSERT( SrcDataSize >= SrcSize );

    DstData.resize( DstSize );
    memset(DstData.data(), 0xFF, DstData.size());
    uint8* pDstData = DstData.data();

    if (pOutTexelFormat)
    {
        *pOutTexelFormat = kDstTexelFormat.EditorTexelFormat;
    }

    // Some game formats are the same as the editor format.
    // In these cases we can simply copy the buffer over.
    if( SrcFormat == EGXTexelFormat::I8 ||
        SrcFormat == EGXTexelFormat::IA8 ||
        SrcFormat == EGXTexelFormat::RGB565 ||
        SrcFormat == EGXTexelFormat::RGBA8 )
    {
        memcpy( pDstData, pkSrcData, DstSize );
    }

    // CMPR requires special handling. There are small differences between CMPR and DXT1
    // that prevents us from using hardware DXT1 support, so it must be decoded to RGBA8.
    else if( SrcFormat == EGXTexelFormat::CMPR )
    {
        const SCMPRBlock* pkBlock = (const SCMPRBlock*) pkSrcData;
        uint32* pDst32 = (uint32*) pDstData;

        for (uint Y = 0; Y < SizeY; Y += 4)
        {
            for (uint X = 0; X < SizeX; X += 4)
            {
                uint32 Palettes[4];
                DecomposeCMPRPalettes(*pkBlock, Palettes[0], Palettes[1],
                                                Palettes[2], Palettes[3] );

                // Byte-swap the palettes so they will be written in RGBA order
                if (EEndian::SystemEndian == EEndian::LittleEndian)
                {
                    SwapBytes(Palettes[0]);
                    SwapBytes(Palettes[1]);
                    SwapBytes(Palettes[2]);
                    SwapBytes(Palettes[3]);
                }

                for (uint DY = 0; DY < 4; DY++)
                {
                    uint8 Bits = pkBlock->Idx[DY];

                    for (uint DX = 0; DX < 4; DX++)
                    {
                        uint Shift = DX*2;
                        uint Index = (Bits >> Shift) & 0x3;
                        uint DstOffset = (Y+DY)*SizeX + X+DX;
                        pDst32[DstOffset] = Palettes[Index];
                    }
                }

                pkBlock++;
            }
        }
    }

    // Remaining formats require conversion because they aren't supported by PC graphics cards.
    else
    {
        // Sweep texels left to right, top to bottom, and apply conversions.
        for (uint Y = 0; Y < SizeY; Y++)
        {
            for (uint X = 0; X < SizeX; X++)
            {
                switch ( SrcFormat )
                {
                // I4/IA4: Extend greyscale and alpha (for IA4) to 8-bit.
                // Note I4 has two 1-component texels, whereas IA4 has one 2-component texels.
                // But this data is converted to the output data the same way either way
                case EGXTexelFormat::I4:
                case EGXTexelFormat::IA4:
                {
                    uint8 I = *pkSrcData++;
                    *pDstData++ = Extend3to8( (I >> 4) & 0xF );
                    *pDstData++ = Extend3to8( (I >> 0) & 0xF );
                    break;
                }

                // RGB5A3: Convert to raw RGBA8.
                case EGXTexelFormat::RGB5A3:
                {
                    uint8 V0 = *pkSrcData++;
                    uint8 V1 = *pkSrcData++;
                    uint32 RGBA = RGB5A3toRGBA8( (V1 << 8) | V0 );
                    SwapBytes(RGBA);

                    *((uint32*) pDstData) = RGBA;
                    pDstData += 4;
                    break;
                }

                default:
                    // Unhandled
                    errorf("Unhandled texel format in ConvertGameDataToEditorData(): %s",
                           TEnumReflection<EGXTexelFormat>::ConvertValueToString(SrcFormat) );
                    return;
                }

                // Increment I4 again to account for the fact that we read two texels instead of one
                if( SrcFormat == EGXTexelFormat::I4 )
                {
                    X++;
                }
            }
        }
    }
}

/** Decode editor texel data to RGBA texels */
void ConvertEditorDataToRGBA(ETexelFormat SrcFormat,
                             uint32 SizeX,
                             uint32 SizeY,
                             const uint8* pkSrcData,
                             uint SrcDataSize,
                             std::vector<uint8>& DstData)
{
    const STexelFormatInfo& kFormatInfo = GetTexelFormatInfo(SrcFormat);
    uint32 SrcSize = (SizeX * SizeY * kFormatInfo.BitsPerPixel) / 8;
    uint32 DstSize = (SizeX * SizeY);
    ASSERT( SrcDataSize >= SrcSize);
    DstData.resize(DstSize);
    uint8* pDstData = DstData.data();

    // If data is already RGBA, then no conversion is needed.
    if (SrcFormat == ETexelFormat::RGBA8)
    {
        memcpy( pDstData, pkSrcData, DstSize );
        return;
    }

    // Other formats require conversion
    for (uint Y = 0; Y < SizeY; Y++)
    {
        for (uint X = 0; X < SizeX; X++)
        {
            uint8 R, G, B, A;

            switch (SrcFormat)
            {

            // Luminance: Replicate to all channels and set opaque alpha
            case ETexelFormat::Luminance:
            {
                R = G = B = *pkSrcData++;
                A = 255;
                break;
            }

            // LuminanceAlpha: Replicate greyscale to all channels and preserve alpha
            case ETexelFormat::LuminanceAlpha:
            {
                R = G = B = *pkSrcData++;
                A = *pkSrcData++;
                break;
            }

            // RGB565: Extend RGB components to 8-bit and set opaque alpha
            case ETexelFormat::RGB565:
            {
                uint8 Byte0 = *pkSrcData++;
                uint8 Byte1 = *pkSrcData++;
                uint16 Texel = (Byte1 << 8) | Byte0;
                uint32 Decoded = RGB565toRGBA8(Texel);
                DecomposeRGBA8(Decoded, R, G, B, A);
                break;
            }

            }

            *pDstData++ = R;
            *pDstData++ = G;
            *pDstData++ = B;
            *pDstData++ = A;
        }
    }
}

/** Encode RGBA texels into a game compression format */
void CompressRGBA(uint32 SizeX,
                  uint32 SizeY,
                  const uint8* pkSrcData,
                  uint SrcDataSize,
                  EGXTexelFormat DstFormat,
                  std::vector<uint8>& DstData)
{
    //@todo
}

/** Import the image file at the given path. Returns true if succeeded. */
bool LoadImageFromFile(const TString& kPath,
                       std::vector<uint8>& OutBuffer,
                       int& OutSizeX,
                       int& OutSizeY,
                       int& OutNumChannels,
                       int DesiredNumChannels /*= 4*/)
{
    std::vector<uint8> DataBuffer;
    FileUtil::LoadFileToBuffer(kPath, DataBuffer);
    return LoadImageFromMemory(DataBuffer.data(), DataBuffer.size(), OutBuffer, OutSizeX, OutSizeY, OutNumChannels, DesiredNumChannels);
}

/** Import an image file from a buffer. Returns true if succeeded. */
bool LoadImageFromMemory(void* pData,
                         uint DataSize,
                         std::vector<uint8>& OutBuffer,
                         int& OutSizeX,
                         int& OutSizeY,
                         int& OutNumChannels,
                         int DesiredNumChannels /*= 4*/)
{
    stbi_uc* pOutData = stbi_load_from_memory( (const stbi_uc*) pData, DataSize, &OutSizeX, &OutSizeY, &OutNumChannels, DesiredNumChannels );

    if (pOutData)
    {
        uint32 NumChannels = (DesiredNumChannels > 0 ? DesiredNumChannels : OutNumChannels);
        uint32 ImageSize = (OutSizeX * OutSizeY * NumChannels);
        OutBuffer.resize(ImageSize);
        memcpy(OutBuffer.data(), pOutData, ImageSize);

        STBI_FREE(pOutData);
        return true;
    }
    else
    {
        return false;
    }
}

} // end namespace NImageUtils
