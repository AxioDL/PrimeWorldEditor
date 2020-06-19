#include "CFontLoader.h"
#include <Common/Log.h>

CFontLoader::CFontLoader() = default;

void CFontLoader::LoadFont(IInputStream& rFONT)
{
    // If I seek past a value without reading it, then it's because I don't know what it is
    mpFont->mUnknown = rFONT.ReadULong();
    mpFont->mLineHeight = rFONT.ReadULong();
    mpFont->mVerticalOffset = rFONT.ReadULong();
    mpFont->mLineMargin = rFONT.ReadULong();
    if (mVersion > EGame::PrimeDemo) rFONT.Seek(0x4, SEEK_CUR);
    rFONT.Seek(0x2, SEEK_CUR);
    mpFont->mDefaultSize = rFONT.ReadULong();
    mpFont->mFontName = rFONT.ReadString();
    mpFont->mpFontTexture = gpResourceStore->LoadResource(CAssetID(rFONT, mVersion), EResourceType::Texture);
    mpFont->mTextureFormat = rFONT.ReadULong();
    const uint32 NumGlyphs = rFONT.ReadULong();
    mpFont->mGlyphs.reserve(NumGlyphs);

    for (uint32 iGlyph = 0; iGlyph < NumGlyphs; iGlyph++)
    {
        CFont::SGlyph Glyph;
        Glyph.Character = rFONT.ReadUShort();

        const float TexCoordL = rFONT.ReadFloat();
        const float TexCoordU = rFONT.ReadFloat();
        const float TexCoordR = rFONT.ReadFloat();
        const float TexCoordD = rFONT.ReadFloat();
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
            Glyph.Width = rFONT.ReadULong();
            Glyph.Height = rFONT.ReadULong();
            Glyph.BaseOffset = rFONT.ReadULong();
            Glyph.KerningIndex = rFONT.ReadULong();
        }
        else if (mVersion >= EGame::Echoes)
        {
            Glyph.RGBAChannel = rFONT.ReadUByte();
            Glyph.LeftPadding = rFONT.ReadByte();
            Glyph.PrintAdvance = rFONT.ReadUByte();
            Glyph.RightPadding = rFONT.ReadByte();
            Glyph.Width = rFONT.ReadUByte();
            Glyph.Height = rFONT.ReadUByte();
            Glyph.BaseOffset = rFONT.ReadUByte();
            Glyph.KerningIndex = rFONT.ReadUShort();
        }
        mpFont->mGlyphs.insert_or_assign(Glyph.Character, Glyph);
    }

    const uint32 NumKerningPairs = rFONT.ReadULong();
    mpFont->mKerningTable.reserve(NumKerningPairs);

    for (uint32 iKern = 0; iKern < NumKerningPairs; iKern++)
    {
        auto& Pair = mpFont->mKerningTable.emplace_back();
        Pair.CharacterA = rFONT.ReadUShort();
        Pair.CharacterB = rFONT.ReadUShort();
        Pair.Adjust = rFONT.ReadLong();
    }
}

std::unique_ptr<CFont> CFontLoader::LoadFONT(IInputStream& rFONT, CResourceEntry *pEntry)
{
    if (!rFONT.IsValid())
        return nullptr;

    const CFourCC Magic(rFONT);
    if (Magic != FOURCC('FONT'))
    {
        errorf("%s: Invalid FONT magic: 0x%08X", *rFONT.GetSourceString(), Magic.ToLong());
        return nullptr;
    }

    const uint32 FileVersion = rFONT.ReadULong();
    const EGame Version = GetFormatVersion(FileVersion);
    if (Version == EGame::Invalid)
    {
        errorf("%s: Unsupported FONT version: %u", *rFONT.GetSourceString(), FileVersion);
        return nullptr;
    }

    auto ptr = std::make_unique<CFont>(pEntry);

    CFontLoader Loader;
    Loader.mpFont = ptr.get();
    Loader.mVersion = Version;
    Loader.LoadFont(rFONT);

    return ptr;
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
