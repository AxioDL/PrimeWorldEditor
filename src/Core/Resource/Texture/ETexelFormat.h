#ifndef ETEXELFORMAT
#define ETEXELFORMAT

#include <Common/BasicTypes.h>

/** Texel formats in GX using Retro's indexing from the TXTR format */
enum class EGXTexelFormat
{
    // 4-bit single-channel greyscale
    I4     = 0x0,
    // 8-bit single-channel greyscale
    I8     = 0x1,
    // 4-bit two-channel greyscale with alpha
    IA4    = 0x2,
    // 8-bit two-channel greyscale with alpha
    IA8    = 0x3,
    // Palette with 4-bit indices
    C4     = 0x4,
    // Palette with 8-bit indices
    C8     = 0x5,
    // Unused
    C14x2  = 0x6,
    // 16-bit three-channel RGB texture
    RGB565 = 0x7,
    // 16-bit four-channel RGBA texture
    RGB5A3 = 0x8,
    // 32-bit four-channel uncompressed RGBA texture
    RGBA8  = 0x9,
    // Compressed CMPR (similar to BC1/DXT1) RGBA texture with one-bit alpha
    CMPR   = 0xA,

    Invalid = -1
};

/** Texel formats useed internally in the editor */
enum class ETexelFormat
{
    // Single-channel greyscale
    Luminance,
    // Two-channel greyscale with alpha
    LuminanceAlpha,
    // Three-channel 16-bit RGB texture
    RGB565,
    // Four-channel 32-bit uncompressed RGBA texture
    RGBA8,

    Invalid = -1
};

/** Info about texel formats; retrieve via NTextureUtils */
struct STexelFormatInfo
{
    EGXTexelFormat  GameTexelFormat;
    ETexelFormat    EditorTexelFormat;
    uint            BlockSizeX;
    uint            BlockSizeY;
    uint            BitsPerPixel;
};

#endif // ETEXELFORMAT
