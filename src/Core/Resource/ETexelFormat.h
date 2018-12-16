#ifndef ETEXELFORMAT
#define ETEXELFORMAT

// ETexelFormat - supported internal formats for decoded textures
enum class ETexelFormat
{
    // Supported texel formats in GX using Retro's numbering
    GX_I4     = 0x0,
    GX_I8     = 0x1,
    GX_IA4    = 0x2,
    GX_IA8    = 0x3,
    GX_C4     = 0x4,
    GX_C8     = 0x5,
    GX_C14x2  = 0x6,
    GX_RGB565 = 0x7,
    GX_RGB5A3 = 0x8,
    GX_RGBA8  = 0x9,
    GX_CMPR   = 0xA,
    // Supported internal texel formats for decoded textures
    Luminance,
    LuminanceAlpha,
    RGBA4,
    RGB565,
    RGBA8,
    DXT1,
    Invalid = -1
};

// EGXPaletteFormat - GX's supported palette texel formats for C4/C8
enum class EGXPaletteFormat
{
    IA8    = 0,
    RGB565 = 1,
    RGB5A3 = 2
};

#endif // ETEXELFORMAT

