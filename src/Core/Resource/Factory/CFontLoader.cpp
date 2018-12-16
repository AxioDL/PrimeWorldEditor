#include "CFontLoader.h"
#include <Common/Log.h>
#include <iostream>

CFontLoader::CFontLoader()
{
}

CFont* CFontLoader::LoadFont(IInputStream& rFONT)
{
    // If I seek past a value without reading it, then it's because I don't know what it is
    mpFont->mUnknown = rFONT.ReadLong();
    mpFont->mLineHeight = rFONT.ReadLong();
    mpFont->mVerticalOffset = rFONT.ReadLong();
    mpFont->mLineMargin = rFONT.ReadLong();
    if (mVersion > EGame::PrimeDemo) rFONT.Seek(0x4, SEEK_CUR);
    rFONT.Seek(0x2, SEEK_CUR);
    mpFont->mDefaultSize = rFONT.ReadLong();
    mpFont->mFontName = rFONT.ReadString();
    mpFont->mpFontTexture = gpResourceStore->LoadResource(CAssetID(rFONT, mVersion), EResourceType::Texture);
    mpFont->mTextureFormat = rFONT.ReadLong();
    uint32 NumGlyphs = rFONT.ReadLong();
    mpFont->mGlyphs.reserve(NumGlyphs);

    for (uint32 iGlyph = 0; iGlyph < NumGlyphs; iGlyph++)
    {
        CFont::SGlyph Glyph;
        Glyph.Character = rFONT.ReadShort();

        float TexCoordL = rFONT.ReadFloat();
        float TexCoordU = rFONT.ReadFloat();
        float TexCoordR = rFONT.ReadFloat();
        float TexCoordD = rFONT.ReadFloat();
        Glyph.TexCoords[0] = CVector2f(TexCoordL, TexCoordU); // Upper-left
        Glyph.TexCoords[1] = CVector2f(TexCoordR, TexCoordU); // Upper-right
        Glyph.TexCoords[2] = CVector2f(TexCoordL, TexCoordD); // Lower-left
        Glyph.TexCoords[3] = CVector2f(TexCoordR, TexCoordD); // Lower-right

        if (mVersion <= EGame::Prime)
        {
            Glyph.RGBAChannel = 0;
            Glyph.LeftPadding = rFONT.ReadLong();
            Glyph.PrintAdvance = rFONT.ReadLong();
            Glyph.RightPadding = rFONT.ReadLong();
            Glyph.Width = rFONT.ReadLong();
            Glyph.Height = rFONT.ReadLong();
            Glyph.BaseOffset = rFONT.ReadLong();
            Glyph.KerningIndex = rFONT.ReadLong();
        }
        else if (mVersion >= EGame::Echoes)
        {
            Glyph.RGBAChannel = rFONT.ReadByte();
            Glyph.LeftPadding = rFONT.ReadByte();
            Glyph.PrintAdvance = rFONT.ReadByte();
            Glyph.RightPadding = rFONT.ReadByte();
            Glyph.Width = rFONT.ReadByte();
            Glyph.Height = rFONT.ReadByte();
            Glyph.BaseOffset = rFONT.ReadByte();
            Glyph.KerningIndex = rFONT.ReadShort();
        }
        mpFont->mGlyphs[Glyph.Character] = Glyph;
    }

    uint32 NumKerningPairs = rFONT.ReadLong();
    mpFont->mKerningTable.reserve(NumKerningPairs);

    for (uint32 iKern = 0; iKern < NumKerningPairs; iKern++)
    {
        CFont::SKerningPair Pair;
        Pair.CharacterA = rFONT.ReadShort();
        Pair.CharacterB = rFONT.ReadShort();
        Pair.Adjust = rFONT.ReadLong();
        mpFont->mKerningTable.push_back(Pair);
    }

    return mpFont;
}

CFont* CFontLoader::LoadFONT(IInputStream& rFONT, CResourceEntry *pEntry)
{
    if (!rFONT.IsValid()) return nullptr;

    CFourCC Magic(rFONT);
    if (Magic != FOURCC('FONT'))
    {
        errorf("%s: Invalid FONT magic: 0x%08X", *rFONT.GetSourceString(), Magic.ToLong());
        return nullptr;
    }

    uint32 FileVersion = rFONT.ReadLong();
    EGame Version = GetFormatVersion(FileVersion);
    if (Version == EGame::Invalid)
    {
        errorf("%s: Unsupported FONT version: %d", *rFONT.GetSourceString(), FileVersion);
        return nullptr;
    }

    CFontLoader Loader;
    Loader.mpFont = new CFont(pEntry);
    Loader.mVersion = Version;
    return Loader.LoadFont(rFONT);
}

EGame CFontLoader::GetFormatVersion(uint32 Version)
{
    switch (Version)
    {
    case 1: return EGame::PrimeDemo;
    case 2: return EGame::Prime;
    case 4: return EGame::Echoes;
    case 5: return EGame::Corruption;
    default: return EGame::Invalid;
    }
}
