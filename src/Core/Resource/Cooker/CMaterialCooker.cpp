#include "Core/Resource/Cooker/CMaterialCooker.h"

#include "Core/NRangeUtils.h"
#include "Core/Resource/CMaterial.h"
#include "Core/Resource/CMaterialSet.h"
#include <Core/Resource/Model/EVertexAttribute.h>

#include <algorithm>

static uint32_t ConvertFromVertexDescription(FVertexDescription VtxDesc)
{
    uint32_t Flags = 0;
    if (VtxDesc.HasFlag(EVertexAttribute::Position)) Flags |= 0x00000003;
    if (VtxDesc.HasFlag(EVertexAttribute::Normal))   Flags |= 0x0000000C;
    if (VtxDesc.HasFlag(EVertexAttribute::Color0))   Flags |= 0x00000030;
    if (VtxDesc.HasFlag(EVertexAttribute::Color1))   Flags |= 0x000000C0;
    if (VtxDesc.HasFlag(EVertexAttribute::Tex0))     Flags |= 0x00000300;
    if (VtxDesc.HasFlag(EVertexAttribute::Tex1))     Flags |= 0x00000C00;
    if (VtxDesc.HasFlag(EVertexAttribute::Tex2))     Flags |= 0x00003000;
    if (VtxDesc.HasFlag(EVertexAttribute::Tex3))     Flags |= 0x0000C000;
    if (VtxDesc.HasFlag(EVertexAttribute::Tex4))     Flags |= 0x00030000;
    if (VtxDesc.HasFlag(EVertexAttribute::Tex5))     Flags |= 0x000C0000;
    if (VtxDesc.HasFlag(EVertexAttribute::Tex6))     Flags |= 0x00300000;
    if (VtxDesc.HasFlag(EVertexAttribute::Tex7))     Flags |= 0x00C00000;
    if (VtxDesc.HasFlag(EVertexAttribute::PosMtx))   Flags |= 0x01000000;
    if (VtxDesc.HasFlag(EVertexAttribute::Tex0Mtx))  Flags |= 0x02000000;
    if (VtxDesc.HasFlag(EVertexAttribute::Tex1Mtx))  Flags |= 0x04000000;
    if (VtxDesc.HasFlag(EVertexAttribute::Tex2Mtx))  Flags |= 0x08000000;
    if (VtxDesc.HasFlag(EVertexAttribute::Tex3Mtx))  Flags |= 0x10000000;
    if (VtxDesc.HasFlag(EVertexAttribute::Tex4Mtx))  Flags |= 0x20000000;
    if (VtxDesc.HasFlag(EVertexAttribute::Tex5Mtx))  Flags |= 0x40000000;
    if (VtxDesc.HasFlag(EVertexAttribute::Tex6Mtx))  Flags |= 0x80000000;
    return Flags;
}

CMaterialCooker::CMaterialCooker() = default;

void CMaterialCooker::WriteMatSetPrime(IOutputStream& rOut)
{
    // Gather texture list from the materials before starting
    mTextureIDs.clear();
    const size_t NumMats = mpSet->mMaterials.size();

    for (size_t iMat = 0; iMat < NumMats; iMat++)
    {
        const CMaterial* pMat = mpSet->mMaterials[iMat].get();

        for (const auto* pass : pMat->Passes())
        {
            if (const auto* tex = pass->Texture())
                mTextureIDs.push_back(tex->ID().ToU32());
        }
    }

    // Sort/remove duplicates
    {
        std::ranges::sort(mTextureIDs);
        const auto [begin, end] = std::ranges::unique(mTextureIDs);
        mTextureIDs.erase(begin, end);
    }

    // Write texture IDs
    rOut.WriteU32(static_cast<uint32_t>(mTextureIDs.size()));

    for (const auto id : mTextureIDs)
        rOut.WriteU32(id);

    // Write material offset filler
    rOut.WriteU32(static_cast<uint32_t>(NumMats));
    const uint32_t MatOffsetsStart = rOut.Tell();

    for (size_t iMat = 0; iMat < NumMats; iMat++)
        rOut.WriteU32(0);

    // Write materials
    const uint32_t MatsStart = rOut.Tell();
    std::vector<uint32_t> MatEndOffsets(NumMats);

    for (size_t iMat = 0; iMat < NumMats; iMat++)
    {
        mpMat = mpSet->mMaterials[iMat].get();
        WriteMaterialPrime(rOut);
        MatEndOffsets[iMat] = rOut.Tell() - MatsStart;
    }

    // Write material offsets
    const uint32_t MatsEnd = rOut.Tell();
    rOut.Seek(MatOffsetsStart, SEEK_SET);

    for (size_t iMat = 0; iMat < NumMats; iMat++)
        rOut.WriteU32(MatEndOffsets[iMat]);

    // Done!
    rOut.Seek(MatsEnd, SEEK_SET);
}

void CMaterialCooker::WriteMatSetCorruption(IOutputStream& /*rOut*/)
{
    // todo
}

void CMaterialCooker::WriteMaterialPrime(IOutputStream& rOut)
{
    // Gather data from the passes before we start writing
    uint32_t TexFlags = 0;
    uint32_t NumKonst = 0;
    std::vector<uint32_t> TexIndices;

    for (auto [idx, pass] : Utils::enumerate(mpMat->Passes()))
    {
        if ((pass->KColorSel() >= kKonst0_RGB) ||
            (pass->KAlphaSel() >= kKonst0_R))
        {
            // Determine the highest Konst index being used
            const auto KColorIndex = static_cast<uint32_t>(pass->KColorSel()) % 4;
            const auto KAlphaIndex = static_cast<uint32_t>(pass->KAlphaSel()) % 4;

            if (KColorIndex >= NumKonst)
                NumKonst = KColorIndex + 1;
            if (KAlphaIndex >= NumKonst)
                NumKonst = KAlphaIndex + 1;
        }

        CTexture* pPassTex = pass->Texture();
        if (pPassTex != nullptr)
        {
            TexFlags |= (1U << idx);
            const auto TexID = pPassTex->ID().ToU32();

            for (uint32_t iTex = 0; iTex < mTextureIDs.size(); iTex++)
            {
                if (mTextureIDs[iTex] == TexID)
                {
                    TexIndices.push_back(iTex);
                    break;
                }
            }
        }
    }

    // Get group index
    size_t GroupIndex = 0;
    const uint64_t MatHash = mpMat->HashParameters();
    bool NewHash = true;

    for (size_t iHash = 0; iHash < mMaterialHashes.size(); iHash++)
    {
        if (mMaterialHashes[iHash] == MatHash)
        {
            GroupIndex = iHash;
            NewHash = false;
            break;
        }
    }

    if (NewHash)
    {
        GroupIndex = mMaterialHashes.size();
        mMaterialHashes.push_back(MatHash);
    }

    // Start writing!
    // Generate flags value
    const bool HasKonst = NumKonst > 0;
    uint32_t Flags;

    if (mVersion <= EGame::Prime)
        Flags = 0x1003;
    else
        Flags = 0x4002;

    Flags |= (HasKonst ? 0x8 : 0x0) | (mpMat->Options().Value() & ~0x8) | (TexFlags << 16);

    rOut.WriteU32(Flags);

    // Texture indices
    rOut.WriteU32(static_cast<uint32_t>(TexIndices.size()));
    for (const auto index : TexIndices)
        rOut.WriteU32(index);

    // Vertex description
    uint32_t VtxFlags = ConvertFromVertexDescription(mpMat->VtxDesc());

    if (mVersion < EGame::Echoes)
        VtxFlags &= 0x00FFFFFF;

    rOut.WriteU32(VtxFlags);

    // Echoes unknowns
    if (mVersion == EGame::Echoes)
    {
        rOut.WriteU32(mpMat->EchoesUnknownA());
        rOut.WriteU32(mpMat->EchoesUnknownB());
    }

    // Group index
    rOut.WriteU32(static_cast<uint32_t>(GroupIndex));

    // Konst
    if (HasKonst)
    {
        rOut.WriteU32(NumKonst);
        for (size_t iKonst = 0; iKonst < NumKonst; iKonst++)
            rOut.WriteU32(mpMat->Konst(iKonst).ToRGBA());
    }

    // Blend Mode
    // Some modifications are done to convert the GLenum to the corresponding GX enum
    auto BlendSrcFac = static_cast<uint16_t>(mpMat->BlendSrcFac());
    auto BlendDstFac = static_cast<uint16_t>(mpMat->BlendDstFac());
    if (BlendSrcFac >= 0x300) BlendSrcFac -= 0x2FE;
    if (BlendDstFac >= 0x300) BlendDstFac -= 0x2FE;
    rOut.WriteU16(BlendDstFac);
    rOut.WriteU16(BlendSrcFac);

    // Color Channels
    rOut.WriteU32(1);
    rOut.WriteU32(0x3000 | (mpMat->IsLightingEnabled() ? 1 : 0));

    // TEV
    auto Passes = mpMat->Passes();
    rOut.WriteU32(uint32_t(std::ranges::size(Passes)));

    for (const auto* pass : Passes)
    {
        const uint32_t ColorInputFlags = ((pass->ColorInput(0)) |
                                          (pass->ColorInput(1) << 5) |
                                          (pass->ColorInput(2) << 10) |
                                          (pass->ColorInput(3) << 15));
        const uint32_t AlphaInputFlags = ((pass->AlphaInput(0)) |
                                          (pass->AlphaInput(1) << 5) |
                                          (pass->AlphaInput(2) << 10) |
                                          (pass->AlphaInput(3) << 15));

        const uint32_t ColorOpFlags = 0x100 | (pass->ColorOutput() << 9);
        const uint32_t AlphaOpFlags = 0x100 | (pass->AlphaOutput() << 9);

        rOut.WriteU32(ColorInputFlags);
        rOut.WriteU32(AlphaInputFlags);
        rOut.WriteU32(ColorOpFlags);
        rOut.WriteU32(AlphaOpFlags);
        rOut.WriteU8(0); // Padding
        rOut.WriteU8(static_cast<uint8_t>(pass->KAlphaSel()));
        rOut.WriteU8(static_cast<uint8_t>(pass->KColorSel()));
        rOut.WriteU8(static_cast<uint8_t>(pass->RasSel()));
    }

    // TEV Tex/UV input selection
    uint32_t CurTexIdx = 0;

    for (const auto* pass : mpMat->Passes())
    {
        rOut.WriteU16(0); // Padding

        if (pass->Texture() != nullptr)
        {
            rOut.WriteU8(static_cast<uint8_t>(CurTexIdx));
            rOut.WriteU8(static_cast<uint8_t>(CurTexIdx));
            CurTexIdx++;
        }
        else
        {
            rOut.WriteU16(static_cast<uint16_t>(0xFFFF));
        }
    }

    // TexGen
    const uint32_t NumTexCoords = CurTexIdx; // TexIdx is currently equal to the tex coord count
    rOut.WriteU32(NumTexCoords);
    uint32_t CurTexMtx = 0;

    for (const auto* pass : mpMat->Passes())
    {
        if (pass->Texture() == nullptr)
            continue;

        const EUVAnimMode AnimType = pass->AnimMode();
        const uint32_t CoordSource = pass->TexCoordSource();

        uint32_t TexMtxIdx, PostMtxIdx;
        bool Normalize;

        // No animation - set TexMtx and PostMtx to identity, disable normalization
        if (AnimType == EUVAnimMode::NoUVAnim)
        {
            TexMtxIdx = 30;
            PostMtxIdx = 61;
            Normalize = false;
        }
        else // Animation - set parameters as the animation mode needs them
        {
            TexMtxIdx = CurTexMtx;

            if (AnimType <= EUVAnimMode::InverseMVTranslated || AnimType >= EUVAnimMode::ModelMatrix)
            {
                PostMtxIdx = CurTexMtx;
                Normalize = true;
            }
            else
            {
                PostMtxIdx = 61;
                Normalize = false;
            }
            CurTexMtx += 3;
        }

        const uint32_t TexGenFlags = (CoordSource << 4) | (TexMtxIdx << 9) | (Normalize << 14) | (PostMtxIdx << 15);
        rOut.WriteU32(TexGenFlags);
    }

    // Animations
    const uint32_t AnimSizeOffset = rOut.Tell();
    const uint32_t NumAnims = CurTexMtx; // CurTexMtx is currently equal to the anim count
    rOut.WriteU32(0);         // Anim size filler
    const uint32_t AnimsStart = rOut.Tell();
    rOut.WriteU32(NumAnims);

    for (const auto* pass : mpMat->Passes())
    {
        const auto AnimMode = pass->AnimMode();
        if (AnimMode == EUVAnimMode::NoUVAnim)
            continue;

        rOut.WriteS32(static_cast<int>(AnimMode));

        if ((AnimMode >= EUVAnimMode::UVScroll) && (AnimMode != EUVAnimMode::ModelMatrix))
        {
            rOut.WriteF32(pass->AnimParam(0));
            rOut.WriteF32(pass->AnimParam(1));

            if ((AnimMode == EUVAnimMode::UVScroll) || (AnimMode == EUVAnimMode::HFilmstrip) || (AnimMode == EUVAnimMode::VFilmstrip))
            {
                rOut.WriteF32(pass->AnimParam(2));
                rOut.WriteF32(pass->AnimParam(3));
            }
        }
    }

    const uint32_t AnimsEnd = rOut.Tell();
    const uint32_t AnimsSize = AnimsEnd - AnimsStart;
    rOut.Seek(AnimSizeOffset, SEEK_SET);
    rOut.WriteU32(AnimsSize);
    rOut.Seek(AnimsEnd, SEEK_SET);

    // Done!
}

void CMaterialCooker::WriteMaterialCorruption(IOutputStream& /*rOut*/)
{
    // todo
}

// ************ STATIC ************
void CMaterialCooker::WriteCookedMatSet(CMaterialSet* pSet, EGame Version, IOutputStream& rOut)
{
    CMaterialCooker Cooker;
    Cooker.mpSet = pSet;
    Cooker.mVersion = Version;

    switch (Version)
    {
    case EGame::PrimeDemo:
    case EGame::Prime:
    case EGame::EchoesDemo:
    case EGame::Echoes:
        Cooker.WriteMatSetPrime(rOut);
        break;
    default:
        break;
    }
}

void CMaterialCooker::WriteCookedMaterial(CMaterial* pMat, EGame Version, IOutputStream& rOut)
{
    CMaterialCooker Cooker;
    Cooker.mpMat = pMat;
    Cooker.mVersion = Version;

    switch (Version)
    {
    case EGame::PrimeDemo:
    case EGame::Prime:
    case EGame::EchoesDemo:
    case EGame::Echoes:
        Cooker.WriteMaterialPrime(rOut);
        break;
    // TODO: Corruption/Uncooked
    default: break;
    }
}
