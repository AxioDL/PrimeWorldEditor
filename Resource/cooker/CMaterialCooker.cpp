#include "CMaterialCooker.h"

#include <algorithm>

CMaterialCooker::CMaterialCooker()
{
    mpMat = nullptr;
}

void CMaterialCooker::WriteMatSetPrime(COutputStream& Out)
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
    Out.WriteLong(mTextureIDs.size());

    for (u32 iTex = 0; iTex < mTextureIDs.size(); iTex++)
        Out.WriteLong(mTextureIDs[iTex]);

    // Write material offset filler
    Out.WriteLong(NumMats);
    u32 MatOffsetsStart = Out.Tell();

    for (u32 iMat = 0; iMat < NumMats; iMat++)
        Out.WriteLong(0);

    // Write materials
    u32 MatsStart = Out.Tell();
    std::vector<u32> MatEndOffsets(NumMats);

    for (u32 iMat = 0; iMat < NumMats; iMat++)
    {
        mpMat = mpSet->mMaterials[iMat];
        WriteMaterialPrime(Out);
        MatEndOffsets[iMat] = Out.Tell() - MatsStart;
    }

    // Write material offsets
    u32 MatsEnd = Out.Tell();
    Out.Seek(MatOffsetsStart, SEEK_SET);

    for (u32 iMat = 0; iMat < NumMats; iMat++)
        Out.WriteLong(MatEndOffsets[iMat]);

    // Done!
    Out.Seek(MatsEnd, SEEK_SET);
}

void CMaterialCooker::WriteMatSetCorruption(COutputStream&)
{
    // Not using parameter 1 (COutputStream& - Out)
    // todo
}

void CMaterialCooker::WriteMaterialPrime(COutputStream& Out)
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

    Flags |= (HasKonst << 3) | mpMat->Options() | (TexFlags << 16);

    Out.WriteLong(Flags);

    // Texture indices
    Out.WriteLong(TexIndices.size());
    for (u32 iTex = 0; iTex < TexIndices.size(); iTex++)
        Out.WriteLong(TexIndices[iTex]);

    // Vertex description
    EVertexDescription Desc = mpMat->VtxDesc();

    if (mVersion < eEchoes)
        Desc = (EVertexDescription) (Desc & 0x00FFFFFF);

    Out.WriteLong(Desc);

    // Echoes unknowns
    if (mVersion == eEchoes)
    {
        Out.WriteLong(mpMat->EchoesUnknownA());
        Out.WriteLong(mpMat->EchoesUnknownB());
    }

    // Group index
    Out.WriteLong(GroupIndex);

    // Konst
    if (HasKonst)
    {
        Out.WriteLong(NumKonst);
        for (u32 iKonst = 0; iKonst < NumKonst; iKonst++)
            Out.WriteLong( mpMat->Konst(iKonst).AsLongRGBA() );
    }

    // Blend Mode
    // Some modifications are done to convert the GLenum to the corresponding GX enum
    u16 BlendSrcFac = (u16) mpMat->BlendSrcFac();
    u16 BlendDstFac = (u16) mpMat->BlendDstFac();
    if (BlendSrcFac >= 0x300) BlendSrcFac -= 0x2FE;
    if (BlendDstFac >= 0x300) BlendDstFac -= 0x2FE;
    Out.WriteShort(BlendDstFac);
    Out.WriteShort(BlendSrcFac);

    // Color Channels
    Out.WriteLong(1);
    Out.WriteLong(0x3000 | (mpMat->IsLightingEnabled() ? 1 : 0));

    // TEV
    u32 NumPasses = mpMat->PassCount();
    Out.WriteLong(NumPasses);

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

        Out.WriteLong(ColorInputFlags);
        Out.WriteLong(AlphaInputFlags);
        Out.WriteLong(ColorOpFlags);
        Out.WriteLong(AlphaOpFlags);
        Out.WriteByte(0); // Padding
        Out.WriteByte(pPass->KAlphaSel());
        Out.WriteByte(pPass->KColorSel());
        Out.WriteByte(pPass->RasSel());
    }

    // TEV Tex/UV input selection
    u32 CurTexIdx = 0;

    for (u32 iPass = 0; iPass < NumPasses; iPass++)
    {
        Out.WriteShort(0); // Padding

        if (mpMat->Pass(iPass)->Texture())
        {
            Out.WriteByte((u8) CurTexIdx);
            Out.WriteByte((u8) CurTexIdx);
            CurTexIdx++;
        }

        else
            Out.WriteShort((u16) 0xFFFF);
    }

    // TexGen
    u32 NumTexCoords = CurTexIdx; // TexIdx is currently equal to the tex coord count
    Out.WriteLong(NumTexCoords);
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
        Out.WriteLong(TexGenFlags);
    }

    // Animations
    u32 AnimSizeOffset = Out.Tell();
    u32 NumAnims = CurTexMtx; // CurTexMtx is currently equal to the anim count
    Out.WriteLong(0);         // Anim size filler
    u32 AnimsStart = Out.Tell();
    Out.WriteLong(NumAnims);

    for (u32 iPass = 0; iPass < NumPasses; iPass++)
    {
        CMaterialPass *pPass = mpMat->Pass(iPass);
        u32 AnimMode = pPass->AnimMode();
        if (AnimMode == eNoUVAnim) continue;

        Out.WriteLong(AnimMode);

        if ((AnimMode > 1) && (AnimMode != 6))
        {
            Out.WriteFloat(pPass->AnimParam(0));
            Out.WriteFloat(pPass->AnimParam(1));

            if ((AnimMode == 2) || (AnimMode == 4) || (AnimMode == 5))
            {
                Out.WriteFloat(pPass->AnimParam(2));
                Out.WriteFloat(pPass->AnimParam(3));
            }
        }
    }

    u32 AnimsEnd = Out.Tell();
    u32 AnimsSize = AnimsEnd - AnimsStart;
    Out.Seek(AnimSizeOffset, SEEK_SET);
    Out.WriteLong(AnimsSize);
    Out.Seek(AnimsEnd, SEEK_SET);

    // Done!
}

void CMaterialCooker::WriteMaterialCorruption(COutputStream&)
{
    // Not using parameter 1 (COutputStream& - Out)
    // todo
}

// ************ STATIC ************
void CMaterialCooker::WriteCookedMatSet(CMaterialSet *pSet, EGame Version, COutputStream &Out)
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
        Cooker.WriteMatSetPrime(Out);
        break;
    }
}

void CMaterialCooker::WriteCookedMaterial(CMaterial *pMat, EGame Version, COutputStream &Out)
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
        Cooker.WriteMaterialPrime(Out);
        break;
    // TODO: Corruption/Uncooked
    }
}
