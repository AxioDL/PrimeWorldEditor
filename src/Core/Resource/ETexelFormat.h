#ifndef ETEXELFORMAT
#define ETEXELFORMAT

// ETexelFormat - supported internal formats for decoded textures
enum ETexelFormat
{
    // Supported texel formats in GX using Retro's numbering
    eGX_I4     = 0x0,
    eGX_I8     = 0x1,
    eGX_IA4    = 0x2,
    eGX_IA8    = 0x3,
    eGX_C4     = 0x4,
    eGX_C8     = 0x5,
    eGX_C14x2  = 0x6,
    eGX_RGB565 = 0x7,
    eGX_RGB5A3 = 0x8,
    eGX_RGBA8  = 0x9,
    eGX_CMPR   = 0xA,
    // Supported internal texel formats for decoded textures
    eLuminance,
    eLuminanceAlpha,
    eRGBA4,
    eRGB565,
    eRGBA8,
    eDXT1,
    eInvalidTexelFormat
};

// EGXPaletteFormat - GX's supported palette texel formats for C4/C8
enum EGXPaletteFormat
{
    ePalette_IA8    = 0,
    ePalette_RGB565 = 1,
    ePalette_RGB5A3 = 2
};

#endif // ETEXELFORMAT

