#include "Core/Resource/Factory/CFontLoader.h"

#include <Common/Log.h>
#include "Core/Resource/CFont.h"

CFontLoader::CFontLoader() = default;

void CFontLoader::LoadFontImpl(IInputStream& rFONT)
{
    // If I seek past a value without reading it, then it's because I don't know what it is
    mpFont->mUnknown = rFONT.ReadU32();
    mpFont->mLineHeight = rFONT.ReadU32();
    mpFont->mVerticalOffset = rFONT.ReadU32();
    mpFont->mLineMargin = rFONT.ReadU32();

    if (mVersion > EGame::PrimeDemo)
        rFONT.Seek(0x4, SEEK_CUR);

    rFONT.Seek(0x2, SEEK_CUR);
    mpFont->mDefaultSize = rFONT.ReadU32();
    mpFont->mFontName = rFONT.ReadString();
    mpFont->mpFontTexture = mpFont->Entry()->ResourceStore()->LoadResource(CAssetID(rFONT, mVersion), EResourceType::Texture);
    mpFont->mTextureFormat = rFONT.ReadU32();
    const auto NumGlyphs = rFONT.ReadU32();
    mpFont->mGlyphs.reserve(NumGlyphs);

    for (uint32_t iGlyph = 0; iGlyph < NumGlyphs; iGlyph++)
    {
        CFont::SGlyph Glyph;
        Glyph.Character = rFONT.ReadU16();

        const auto TexCoordL = rFONT.ReadF32();
        const auto TexCoordU = rFONT.ReadF32();
        const auto TexCoordR = rFONT.ReadF32();
        const auto TexCoordD = rFONT.ReadF32();
        Glyph.TexCoords[0] = CVector2f(TexCoordL, TexCoordU); // Upper-left
        Glyph.TexCoords[1] = CVector2f(TexCoordR, TexCoordU); // Upper-right
        Glyph.TexCoords[2] = CVector2f(TexCoordL, TexCoordD); // Lower-left
        Glyph.TexCoords[3] = CVector2f(TexCoordR, TexCoordD); // Lower-right

        if (mVersion <= EGame::Prime)
        {
            Glyph.RGBAChannel = 0;
            Glyph.LeftPadding = rFONT.ReadS32();
            Glyph.PrintAdvance = rFONT.ReadS32();
            Glyph.RightPadding = rFONT.ReadS32();
            Glyph.Width = rFONT.ReadU32();
            Glyph.Height = rFONT.ReadU32();
            Glyph.BaseOffset = rFONT.ReadU32();
            Glyph.KerningIndex = rFONT.ReadU32();
        }
        else if (mVersion >= EGame::Echoes)
        {
            Glyph.RGBAChannel = rFONT.ReadU8();
            Glyph.LeftPadding = rFONT.ReadS8();
            Glyph.PrintAdvance = rFONT.ReadU8();
            Glyph.RightPadding = rFONT.ReadS8();
            Glyph.Width = rFONT.ReadU8();
            Glyph.Height = rFONT.ReadU8();
            Glyph.BaseOffset = rFONT.ReadU8();
            Glyph.KerningIndex = rFONT.ReadU16();
        }
        mpFont->mGlyphs.insert_or_assign(Glyph.Character, Glyph);
    }

    const auto NumKerningPairs = rFONT.ReadU32();
    mpFont->mKerningTable.reserve(NumKerningPairs);

    for (uint32_t iKern = 0; iKern < NumKerningPairs; iKern++)
    {
        auto& Pair = mpFont->mKerningTable.emplace_back();
        Pair.CharacterA = rFONT.ReadU16();
        Pair.CharacterB = rFONT.ReadU16();
        Pair.Adjust = rFONT.ReadS32();
    }
}

std::unique_ptr<CFont> CFontLoader::LoadFONT(IInputStream& rFONT, CResourceEntry *pEntry)
{
    if (!rFONT.IsValid())
        return nullptr;

    const CFourCC Magic(rFONT);
    if (Magic != CFourCC("FONT"))
    {
        NLog::Error("{}: Invalid FONT magic: 0x{:08X}", rFONT.GetSourceString(), Magic.ToU32());
        return nullptr;
    }

    const auto FileVersion = rFONT.ReadU32();
    const auto Version = GetFormatVersion(FileVersion);
    if (Version == EGame::Invalid)
    {
        NLog::Error("{}: Unsupported FONT version: {}", rFONT.GetSourceString(), FileVersion);
        return nullptr;
    }

    auto ptr = std::make_unique<CFont>(pEntry);

    CFontLoader Loader;
    Loader.mpFont = ptr.get();
    Loader.mVersion = Version;
    Loader.LoadFontImpl(rFONT);

    return ptr;
}

EGame CFontLoader::GetFormatVersion(uint32_t Version)
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
