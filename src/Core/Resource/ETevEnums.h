#ifndef ETEVENUMS
#define ETEVENUMS

#include <Common/BasicTypes.h>

enum ETevColorInput
{
    kPrevRGB    = 0x0,
    kPrevAAA    = 0x1,
    kColor0RGB  = 0x2,
    kColor0AAA  = 0x3,
    kColor1RGB  = 0x4,
    kColor1AAA  = 0x5,
    kColor2RGB  = 0x6,
    kColor2AAA  = 0x7,
    kTextureRGB = 0x8,
    kTextureAAA = 0x9,
    kRasRGB     = 0xA,
    kRasAAA     = 0xB,
    kOneRGB     = 0xC,
    kHalfRGB    = 0xD,
    kKonstRGB   = 0xE,
    kZeroRGB    = 0xF
};

enum ETevAlphaInput
{
    kPrevAlpha    = 0x0,
    kColor0Alpha  = 0x1,
    kColor1Alpha  = 0x2,
    kColor2Alpha  = 0x3,
    kTextureAlpha = 0x4,
    kRasAlpha     = 0x5,
    kKonstAlpha   = 0x6,
    kZeroAlpha    = 0x7
};

enum ETevOutput
{
    kPrevReg   = 0x0,
    kColor0Reg = 0x1,
    kColor1Reg = 0x2,
    kColor2Reg = 0x3
};

enum ETevKSel
{
    kKonstOne          = 0x0,
    kKonstSevenEighths = 0x1,
    kKonstThreeFourths = 0x2,
    kKonstFiveEighths  = 0x3,
    kKonstOneHalf      = 0x4,
    kKonstThreeEighths = 0x5,
    kKonstOneFourth    = 0x6,
    kKonstOneEighth    = 0x7,
    kKonst0_RGB        = 0xC,
    kKonst1_RGB        = 0xD,
    kKonst2_RGB        = 0xE,
    kKonst3_RGB        = 0xF,
    kKonst0_R          = 0x10,
    kKonst1_R          = 0x11,
    kKonst2_R          = 0x12,
    kKonst3_R          = 0x13,
    kKonst0_G          = 0x14,
    kKonst1_G          = 0x15,
    kKonst2_G          = 0x16,
    kKonst3_G          = 0x17,
    kKonst0_B          = 0x18,
    kKonst1_B          = 0x19,
    kKonst2_B          = 0x1A,
    kKonst3_B          = 0x1B,
    kKonst0_A          = 0x1C,
    kKonst1_A          = 0x1D,
    kKonst2_A          = 0x1E,
    kKonst3_A          = 0x1F
};

enum ETevRasSel
{
    kRasColor0     = 0x0,
    kRasColor1     = 0x1,
    kRasAlpha0     = 0x2,
    kRasAlpha1     = 0x3,
    kRasColor0A0   = 0x4,
    kRasColor1A1   = 0x5,
    kRasColorZero  = 0x6,
    kRasAlphaBump  = 0x7,
    kRasAlphaBumpN = 0x8,
    kRasColorNull  = 0xFF
};

enum class EUVAnimMode
{
    InverseMV           = 0x0,
    InverseMVTranslated = 0x1,
    UVScroll            = 0x2,
    UVRotation          = 0x3,
    HFilmstrip          = 0x4,
    VFilmstrip          = 0x5,
    ModelMatrix         = 0x6,
    ConvolutedModeA     = 0x7,
    ConvolutedModeB     = 0x8,
    SimpleMode          = 0xA,
    Eleven              = 0xB,
    NoUVAnim            = -1
};

enum class EUVAnimUVSource : uint16
{
    Position,
    Normal,
    UV
};
enum class EUVAnimMatrixConfig : uint16
{
    NoMtxNoPost,
    MtxNoPost,
    NoMtxPost,
    MtxPost
};

enum class EUVConvolutedModeBType {
    Zero,
    One,
    Two,
    Three,
    Four
};

#endif // ETEVENUMS

