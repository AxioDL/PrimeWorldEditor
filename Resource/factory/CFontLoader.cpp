#include "CFontLoader.h"
#include <Core/Log.h>
#include <iostream>

CFontLoader::CFontLoader()
{
}

CFont* CFontLoader::LoadFont(CInputStream& FONT)
{
    // If I seek past a value without reading it, then it's because I don't know what it is
    mpFont->mUnknown = FONT.ReadLong();
    mpFont->mLineHeight = FONT.ReadLong();
    mpFont->mVerticalOffset = FONT.ReadLong();
    mpFont->mLineMargin = FONT.ReadLong();
    if (mVersion > ePrimeDemo) FONT.Seek(0x4, SEEK_CUR);
    FONT.Seek(0x2, SEEK_CUR);
    mpFont->mDefaultSize = FONT.ReadLong();
    mpFont->mFontName = FONT.ReadString();

    if (mVersion <= eEchoes) mpFont->mpFontTexture = gResCache.GetResource(FONT.ReadLong(), "TXTR");
    else                     mpFont->mpFontTexture = gResCache.GetResource(FONT.ReadLongLong(), "TXTR");

    mpFont->mTextureFormat = FONT.ReadLong();
    u32 NumGlyphs = FONT.ReadLong();
    mpFont->mGlyphs.reserve(NumGlyphs);

    for (u32 iGlyph = 0; iGlyph < NumGlyphs; iGlyph++)
    {
        CFont::SGlyph Glyph;
        Glyph.Character = FONT.ReadShort();

        float TexCoordL = FONT.ReadFloat();
        float TexCoordU = FONT.ReadFloat();
        float TexCoordR = FONT.ReadFloat();
        float TexCoordD = FONT.ReadFloat();
        Glyph.TexCoords[0] = CVector2f(TexCoordL, TexCoordU); // Upper-left
        Glyph.TexCoords[1] = CVector2f(TexCoordR, TexCoordU); // Upper-right
        Glyph.TexCoords[2] = CVector2f(TexCoordL, TexCoordD); // Lower-left
        Glyph.TexCoords[3] = CVector2f(TexCoordR, TexCoordD); // Lower-right

        if (mVersion <= ePrime)
        {
            Glyph.RGBAChannel = 0;
            Glyph.LeftPadding = FONT.ReadLong();
            Glyph.PrintAdvance = FONT.ReadLong();
            Glyph.RightPadding = FONT.ReadLong();
            Glyph.Width = FONT.ReadLong();
            Glyph.Height = FONT.ReadLong();
            Glyph.BaseOffset = FONT.ReadLong();
            Glyph.KerningIndex = FONT.ReadLong();
        }
        else if (mVersion >= eEchoes)
        {
            Glyph.RGBAChannel = FONT.ReadByte();
            Glyph.LeftPadding = FONT.ReadByte();
            Glyph.PrintAdvance = FONT.ReadByte();
            Glyph.RightPadding = FONT.ReadByte();
            Glyph.Width = FONT.ReadByte();
            Glyph.Height = FONT.ReadByte();
            Glyph.BaseOffset = FONT.ReadByte();
            Glyph.KerningIndex = FONT.ReadShort();
        }
        mpFont->mGlyphs[Glyph.Character] = Glyph;
    }

    u32 NumKerningPairs = FONT.ReadLong();
    mpFont->mKerningTable.reserve(NumKerningPairs);

    for (u32 iKern = 0; iKern < NumKerningPairs; iKern++)
    {
        CFont::SKerningPair Pair;
        Pair.CharacterA = FONT.ReadShort();
        Pair.CharacterB = FONT.ReadShort();
        Pair.Adjust = FONT.ReadLong();
        mpFont->mKerningTable.push_back(Pair);
    }

    return mpFont;
}

CFont* CFontLoader::LoadFONT(CInputStream& FONT)
{
    if (!FONT.IsValid()) return nullptr;
    Log::Write("Loading " + FONT.GetSourceString());

    CFourCC Magic(FONT);
    if (Magic != "FONT")
    {
        Log::FileError(FONT.GetSourceString(), "Invalid FONT magic: " + TString::HexString((u32) Magic.ToLong()));
        return nullptr;
    }

    u32 FileVersion = FONT.ReadLong();
    EGame Version = GetFormatVersion(FileVersion);
    if (Version == eUnknownVersion)
    {
        Log::FileError(FONT.GetSourceString(), "Unsupported FONT version: " + TString::HexString(FileVersion));
        return nullptr;
    }

    CFontLoader Loader;
    Loader.mpFont = new CFont();
    Loader.mVersion = Version;
    return Loader.LoadFont(FONT);
}

EGame CFontLoader::GetFormatVersion(u32 Version)
{
    switch (Version)
    {
    case 1: return ePrimeDemo;
    case 2: return ePrime;
    case 4: return eEchoes;
    case 5: return eCorruption;
    default: return eUnknownVersion;
    }
}
