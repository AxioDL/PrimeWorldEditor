#include "CMaterialCooker.h"
#include <algorithm>

CMaterialCooker::CMaterialCooker()
    : mpMat(nullptr)
{
}

void CMaterialCooker::WriteMatSetPrime(IOutputStream& rOut)
{
    // Gather texture list from the materials before starting
    mTextureIDs.clear();
    u32 NumMats = mpSet->mMaterials.size();

    for (u32 iMat = 0; iMat < NumMats; iMat++)
    {
        CMaterial *pMat = mpSet->mMaterials[iMat];

        u32 NumPasses  = pMat->PassCount();
        for (u32 iPass = 0; iPass < NumPasses; iPass++)
        {
            CTexture *pTex = pMat->Pass(iPass)->Texture();
            if (pTex)
                mTextureIDs.push_back(pTex->ResID().ToLong());
        }
    }

    // Sort/remove duplicates
    std::sort(mTextureIDs.begin(), mTextureIDs.end());
    mTextureIDs.erase(std::unique(mTextureIDs.begin(), mTextureIDs.end()), mTextureIDs.end());

    // Write texture IDs
    rOut.WriteLong(mTextureIDs.size());

    for (u32 iTex = 0; iTex < mTextureIDs.size(); iTex++)
        rOut.WriteLong(mTextureIDs[iTex]);

    // Write material offset filler
    rOut.WriteLong(NumMats);
    u32 MatOffsetsStart = rOut.Tell();

    for (u32 iMat = 0; iMat < NumMats; iMat++)
        rOut.WriteLong(0);

    // Write materials
    u32 MatsStart = rOut.Tell();
    std::vector<u32> MatEndOffsets(NumMats);

    for (u32 iMat = 0; iMat < NumMats; iMat++)
    {
        mpMat = mpSet->mMaterials[iMat];
        WriteMaterialPrime(rOut);
        MatEndOffsets[iMat] = rOut.Tell() - MatsStart;
    }

    // Write material offsets
    u32 MatsEnd = rOut.Tell();
    rOut.Seek(MatOffsetsStart, SEEK_SET);

    for (u32 iMat = 0; iMat < NumMats; iMat++)
        rOut.WriteLong(MatEndOffsets[iMat]);

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
    u32 TexFlags = 0;
    u32 NumKonst = 0;
    std::vector<u32> TexIndices;

    for (u32 iPass = 0; iPass < mpMat->mPasses.size(); iPass++)
    {
        CMaterialPass *pPass = mpMat->Pass(iPass);

        if ((pPass->KColorSel() >= 0xC) || (pPass->KAlphaSel() >= 0x10))
        {
            // Determine the highest Konst index being used
            u32 KColorIndex = pPass->KColorSel() % 4;
            u32 KAlphaIndex = pPass->KAlphaSel() % 4;

            if (KColorIndex >= NumKonst)
                NumKonst = KColorIndex + 1;
            if (KAlphaIndex >= NumKonst)
                NumKonst = KAlphaIndex + 1;
        }

        CTexture *pPassTex = pPass->Texture();
        if (pPassTex != nullptr)
        {
            TexFlags |= (1 << iPass);
            u32 TexID = pPassTex->ResID().ToLong();

            for (u32 iTex = 0; iTex < mTextureIDs.size(); iTex++)
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
    u32 GroupIndex;
    u64 MatHash = mpMat->HashParameters();
    bool NewHash = true;

    for (u32 iHash = 0; iHash < mMaterialHashes.size(); iHash++)
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
    bool HasKonst = (NumKonst > 0);
    u32 Flags;

    if (mVersion <= ePrime)
        Flags = 0x1003;
    else
        Flags = 0x4002;

    Flags |= (HasKonst ? 0x8 : 0x0) | (mpMat->Options() & ~0x8) | (TexFlags << 16);

    rOut.WriteLong(Flags);

    // Texture indices
    rOut.WriteLong(TexIndices.size());
    for (u32 iTex = 0; iTex < TexIndices.size(); iTex++)
        rOut.WriteLong(TexIndices[iTex]);

    // Vertex description
    FVertexDescription Desc = mpMat->VtxDesc();

    if (mVersion < eEchoes)
        Desc &= 0x00FFFFFF;

    rOut.WriteLong(Desc);

    // Echoes unknowns
    if (mVersion == eEchoes)
    {
        rOut.WriteLong(mpMat->EchoesUnknownA());
        rOut.WriteLong(mpMat->EchoesUnknownB());
    }

    // Group index
    rOut.WriteLong(GroupIndex);

    // Konst
    if (HasKonst)
    {
        rOut.WriteLong(NumKonst);
        for (u32 iKonst = 0; iKonst < NumKonst; iKonst++)
            rOut.WriteLong( mpMat->Konst(iKonst).ToLongRGBA() );
    }

    // Blend Mode
    // Some modifications are done to convert the GLenum to the corresponding GX enum
    u16 BlendSrcFac = (u16) mpMat->BlendSrcFac();
    u16 BlendDstFac = (u16) mpMat->BlendDstFac();
    if (BlendSrcFac >= 0x300) BlendSrcFac -= 0x2FE;
    if (BlendDstFac >= 0x300) BlendDstFac -= 0x2FE;
    rOut.WriteShort(BlendDstFac);
    rOut.WriteShort(BlendSrcFac);

    // Color Channels
    rOut.WriteLong(1);
    rOut.WriteLong(0x3000 | (mpMat->IsLightingEnabled() ? 1 : 0));

    // TEV
    u32 NumPasses = mpMat->PassCount();
    rOut.WriteLong(NumPasses);

    for (u32 iPass = 0; iPass < NumPasses; iPass++)
    {
        CMaterialPass *pPass = mpMat->Pass(iPass);

        u32 ColorInputFlags = ((pPass->ColorInput(0)) |
                               (pPass->ColorInput(1) << 5) |
                               (pPass->ColorInput(2) << 10) |
                               (pPass->ColorInput(3) << 15));
        u32 AlphaInputFlags = ((pPass->AlphaInput(0)) |
                               (pPass->AlphaInput(1) << 5) |
                               (pPass->AlphaInput(2) << 10) |
                               (pPass->AlphaInput(3) << 15));

        u32 ColorOpFlags = 0x100 | (pPass->ColorOutput() << 9);
        u32 AlphaOpFlags = 0x100 | (pPass->AlphaOutput() << 9);

        rOut.WriteLong(ColorInputFlags);
        rOut.WriteLong(AlphaInputFlags);
        rOut.WriteLong(ColorOpFlags);
        rOut.WriteLong(AlphaOpFlags);
        rOut.WriteByte(0); // Padding
        rOut.WriteByte(pPass->KAlphaSel());
        rOut.WriteByte(pPass->KColorSel());
        rOut.WriteByte(pPass->RasSel());
    }

    // TEV Tex/UV input selection
    u32 CurTexIdx = 0;

    for (u32 iPass = 0; iPass < NumPasses; iPass++)
    {
        rOut.WriteShort(0); // Padding

        if (mpMat->Pass(iPass)->Texture())
        {
            rOut.WriteByte((u8) CurTexIdx);
            rOut.WriteByte((u8) CurTexIdx);
            CurTexIdx++;
        }

        else
            rOut.WriteShort((u16) 0xFFFF);
    }

    // TexGen
    u32 NumTexCoords = CurTexIdx; // TexIdx is currently equal to the tex coord count
    rOut.WriteLong(NumTexCoords);
    u32 CurTexMtx = 0;

    for (u32 iPass = 0; iPass < NumPasses; iPass++)
    {
        CMaterialPass *pPass = mpMat->Pass(iPass);
        if (pPass->Texture() == nullptr) continue;

        u32 AnimType = pPass->AnimMode();
        u32 CoordSource = pPass->TexCoordSource();

        u32 TexMtxIdx, PostMtxIdx;
        bool Normalize;

        // No animation - set TexMtx and PostMtx to identity, disable normalization
        if (AnimType == eNoUVAnim)
        {
            TexMtxIdx = 30;
            PostMtxIdx = 61;
            Normalize = false;
        }

        // Animation - set parameters as the animation mode needs them
        else
        {
            TexMtxIdx = CurTexMtx;

            if ((AnimType < 2) || (AnimType > 5))
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

        u32 TexGenFlags = (CoordSource << 4) | (TexMtxIdx << 9) | (Normalize << 14) | (PostMtxIdx << 15);
        rOut.WriteLong(TexGenFlags);
    }

    // Animations
    u32 AnimSizeOffset = rOut.Tell();
    u32 NumAnims = CurTexMtx; // CurTexMtx is currently equal to the anim count
    rOut.WriteLong(0);         // Anim size filler
    u32 AnimsStart = rOut.Tell();
    rOut.WriteLong(NumAnims);

    for (u32 iPass = 0; iPass < NumPasses; iPass++)
    {
        CMaterialPass *pPass = mpMat->Pass(iPass);
        u32 AnimMode = pPass->AnimMode();
        if (AnimMode == eNoUVAnim) continue;

        rOut.WriteLong(AnimMode);

        if ((AnimMode > 1) && (AnimMode != 6))
        {
            rOut.WriteFloat(pPass->AnimParam(0));
            rOut.WriteFloat(pPass->AnimParam(1));

            if ((AnimMode == 2) || (AnimMode == 4) || (AnimMode == 5))
            {
                rOut.WriteFloat(pPass->AnimParam(2));
                rOut.WriteFloat(pPass->AnimParam(3));
            }
        }
    }

    u32 AnimsEnd = rOut.Tell();
    u32 AnimsSize = AnimsEnd - AnimsStart;
    rOut.Seek(AnimSizeOffset, SEEK_SET);
    rOut.WriteLong(AnimsSize);
    rOut.Seek(AnimsEnd, SEEK_SET);

    // Done!
}

void CMaterialCooker::WriteMaterialCorruption(IOutputStream& /*rOut*/)
{
    // todo
}

// ************ STATIC ************
void CMaterialCooker::WriteCookedMatSet(CMaterialSet *pSet, EGame Version, IOutputStream& rOut)
{
    CMaterialCooker Cooker;
    Cooker.mpSet = pSet;
    Cooker.mVersion = Version;

    switch (Version)
    {
    case ePrimeDemo:
    case ePrime:
    case eEchoesDemo:
    case eEchoes:
        Cooker.WriteMatSetPrime(rOut);
        break;
    }
}

void CMaterialCooker::WriteCookedMaterial(CMaterial *pMat, EGame Version, IOutputStream& rOut)
{
    CMaterialCooker Cooker;
    Cooker.mpMat = pMat;
    Cooker.mVersion = Version;

    switch (Version)
    {
    case ePrimeDemo:
    case ePrime:
    case eEchoesDemo:
    case eEchoes:
        Cooker.WriteMaterialPrime(rOut);
        break;
    // TODO: Corruption/Uncooked
    }
}
