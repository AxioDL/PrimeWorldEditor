#ifndef ETEVENUMS
#define ETEVENUMS

enum ETevColorInput
{
    ePrevRGB    = 0x0,
    ePrevAAA    = 0x1,
    eColor0RGB  = 0x2,
    eColor0AAA  = 0x3,
    eColor1RGB  = 0x4,
    eColor1AAA  = 0x5,
    eColor2RGB  = 0x6,
    eColor2AAA  = 0x7,
    eTextureRGB = 0x8,
    eTextureAAA = 0x9,
    eRasRGB     = 0xA,
    eRasAAA     = 0xB,
    eOneRGB     = 0xC,
    eHalfRGB    = 0xD,
    eKonstRGB   = 0xE,
    eZeroRGB    = 0xF
};

enum ETevAlphaInput
{
    ePrevAlpha    = 0x0,
    eColor0Alpha  = 0x1,
    eColor1Alpha  = 0x2,
    eColor2Alpha  = 0x3,
    eTextureAlpha = 0x4,
    eRasAlpha     = 0x5,
    eKonstAlpha   = 0x6,
    eZeroAlpha    = 0x7
};

enum ETevOutput
{
    ePrevReg   = 0x0,
    eColor0Reg = 0x1,
    eColor1Reg = 0x2,
    eColor2Reg = 0x3
};

enum ETevKSel
{
    eKonstOne          = 0x0,
    eKonstSevenEighths = 0x1,
    eKonstThreeFourths = 0x2,
    eKonstFiveEighths  = 0x3,
    eKonstOneHalf      = 0x4,
    eKonstThreeEighths = 0x5,
    eKonstOneFourth    = 0x6,
    eKonstOneEighth    = 0x7,
    eKonst0_RGB        = 0xC,
    eKonst1_RGB        = 0xD,
    eKonst2_RGB        = 0xE,
    eKonst3_RGB        = 0xF,
    eKonst0_R          = 0x10,
    eKonst1_R          = 0x11,
    eKonst2_R          = 0x12,
    eKonst3_R          = 0x13,
    eKonst0_G          = 0x14,
    eKonst1_G          = 0x15,
    eKonst2_G          = 0x16,
    eKonst3_G          = 0x17,
    eKonst0_B          = 0x18,
    eKonst1_B          = 0x19,
    eKonst2_B          = 0x1A,
    eKonst3_B          = 0x1B,
    eKonst0_A          = 0x1C,
    eKonst1_A          = 0x1D,
    eKonst2_A          = 0x1E,
    eKonst3_A          = 0x1F
};

enum ETevRasSel
{
    eRasColor0     = 0x0,
    eRasColor1     = 0x1,
    eRasAlpha0     = 0x2,
    eRasAlpha1     = 0x3,
    eRasColor0A0   = 0x4,
    eRasColor1A1   = 0x5,
    eRasColorZero  = 0x6,
    eRasAlphaBump  = 0x7,
    eRasAlphaBumpN = 0x8,
    eRasColorNull  = 0xFF
};

enum EUVAnimMode
{
    eInverseMV           = 0x0,
    eInverseMVTranslated = 0x1,
    eUVScroll            = 0x2,
    eUVRotation          = 0x3,
    eHFilmstrip          = 0x4,
    eVFilmstrip          = 0x5,
    eModelMatrix         = 0x6,
    eConvolutedModeA     = 0x7,
    eConvolutedModeB     = 0x8,
    eSimpleMode          = 0xA,
    eNoUVAnim            = 0xFFFFFFFF
};

#endif // ETEVENUMS

