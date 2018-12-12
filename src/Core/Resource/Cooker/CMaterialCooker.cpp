#include "CMaterialCooker.h"
#include <algorithm>

CMaterialCooker::CMaterialCooker()
    : mpMat(nullptr)
{
}

uint32 CMaterialCooker::ConvertFromVertexDescription(FVertexDescription VtxDesc)
{
    uint32 Flags = 0;
    if (VtxDesc & ePosition)    Flags |= 0x00000003;
    if (VtxDesc & eNormal)      Flags |= 0x0000000C;
    if (VtxDesc & eColor0)      Flags |= 0x00000030;
    if (VtxDesc & eColor1)      Flags |= 0x000000C0;
    if (VtxDesc & eTex0)        Flags |= 0x00000300;
    if (VtxDesc & eTex1)        Flags |= 0x00000C00;
    if (VtxDesc & eTex2)        Flags |= 0x00003000;
    if (VtxDesc & eTex3)        Flags |= 0x0000C000;
    if (VtxDesc & eTex4)        Flags |= 0x00030000;
    if (VtxDesc & eTex5)        Flags |= 0x000C0000;
    if (VtxDesc & eTex6)        Flags |= 0x00300000;
    if (VtxDesc & eTex7)        Flags |= 0x00C00000;
    if (VtxDesc & ePosMtx)      Flags |= 0x01000000;
    if (VtxDesc & eTex0Mtx)     Flags |= 0x02000000;
    if (VtxDesc & eTex1Mtx)     Flags |= 0x04000000;
    if (VtxDesc & eTex2Mtx)     Flags |= 0x08000000;
    if (VtxDesc & eTex3Mtx)     Flags |= 0x10000000;
    if (VtxDesc & eTex4Mtx)     Flags |= 0x20000000;
    if (VtxDesc & eTex5Mtx)     Flags |= 0x40000000;
    if (VtxDesc & eTex6Mtx)     Flags |= 0x80000000;
    return Flags;
}

void CMaterialCooker::WriteMatSetPrime(IOutputStream& rOut)
{
    // Gather texture list from the materials before starting
    mTextureIDs.clear();
    uint32 NumMats = mpSet->mMaterials.size();

    for (uint32 iMat = 0; iMat < NumMats; iMat++)
    {
        CMaterial *pMat = mpSet->mMaterials[iMat];

        uint32 NumPasses  = pMat->PassCount();
        for (uint32 iPass = 0; iPass < NumPasses; iPass++)
        {
            CTexture *pTex = pMat->Pass(iPass)->Texture();
            if (pTex)
                mTextureIDs.push_back(pTex->ID().ToLong());
        }
    }

    // Sort/remove duplicates
    std::sort(mTextureIDs.begin(), mTextureIDs.end());
    mTextureIDs.erase(std::unique(mTextureIDs.begin(), mTextureIDs.end()), mTextureIDs.end());

    // Write texture IDs
    rOut.WriteLong(mTextureIDs.size());

    for (uint32 iTex = 0; iTex < mTextureIDs.size(); iTex++)
        rOut.WriteLong(mTextureIDs[iTex]);

    // Write material offset filler
    rOut.WriteLong(NumMats);
    uint32 MatOffsetsStart = rOut.Tell();

    for (uint32 iMat = 0; iMat < NumMats; iMat++)
        rOut.WriteLong(0);

    // Write materials
    uint32 MatsStart = rOut.Tell();
    std::vector<uint32> MatEndOffsets(NumMats);

    for (uint32 iMat = 0; iMat < NumMats; iMat++)
    {
        mpMat = mpSet->mMaterials[iMat];
        WriteMaterialPrime(rOut);
        MatEndOffsets[iMat] = rOut.Tell() - MatsStart;
    }

    // Write material offsets
    uint32 MatsEnd = rOut.Tell();
    rOut.Seek(MatOffsetsStart, SEEK_SET);

    for (uint32 iMat = 0; iMat < NumMats; iMat++)
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
    uint32 TexFlags = 0;
    uint32 NumKonst = 0;
    std::vector<uint32> TexIndices;

    for (uint32 iPass = 0; iPass < mpMat->mPasses.size(); iPass++)
    {
        CMaterialPass *pPass = mpMat->Pass(iPass);

        if ((pPass->KColorSel() >= 0xC) || (pPass->KAlphaSel() >= 0x10))
        {
            // Determine the highest Konst index being used
            uint32 KColorIndex = pPass->KColorSel() % 4;
            uint32 KAlphaIndex = pPass->KAlphaSel() % 4;

            if (KColorIndex >= NumKonst)
                NumKonst = KColorIndex + 1;
            if (KAlphaIndex >= NumKonst)
                NumKonst = KAlphaIndex + 1;
        }

        CTexture *pPassTex = pPass->Texture();
        if (pPassTex != nullptr)
        {
            TexFlags |= (1 << iPass);
            uint32 TexID = pPassTex->ID().ToLong();

            for (uint32 iTex = 0; iTex < mTextureIDs.size(); iTex++)
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
    uint32 GroupIndex;
    uint64 MatHash = mpMat->HashParameters();
    bool NewHash = true;

    for (uint32 iHash = 0; iHash < mMaterialHashes.size(); iHash++)
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
    uint32 Flags;

    if (mVersion <= EGame::Prime)
        Flags = 0x1003;
    else
        Flags = 0x4002;

    Flags |= (HasKonst ? 0x8 : 0x0) | (mpMat->Options() & ~0x8) | (TexFlags << 16);

    rOut.WriteLong(Flags);

    // Texture indices
    rOut.WriteLong(TexIndices.size());
    for (uint32 iTex = 0; iTex < TexIndices.size(); iTex++)
        rOut.WriteLong(TexIndices[iTex]);

    // Vertex description
    uint32 VtxFlags = ConvertFromVertexDescription( mpMat->VtxDesc() );

    if (mVersion < EGame::Echoes)
        VtxFlags &= 0x00FFFFFF;

    rOut.WriteLong(VtxFlags);

    // Echoes unknowns
    if (mVersion == EGame::Echoes)
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
        for (uint32 iKonst = 0; iKonst < NumKonst; iKonst++)
            rOut.WriteLong( mpMat->Konst(iKonst).ToLongRGBA() );
    }

    // Blend Mode
    // Some modifications are done to convert the GLenum to the corresponding GX enum
    uint16 BlendSrcFac = (uint16) mpMat->BlendSrcFac();
    uint16 BlendDstFac = (uint16) mpMat->BlendDstFac();
    if (BlendSrcFac >= 0x300) BlendSrcFac -= 0x2FE;
    if (BlendDstFac >= 0x300) BlendDstFac -= 0x2FE;
    rOut.WriteShort(BlendDstFac);
    rOut.WriteShort(BlendSrcFac);

    // Color Channels
    rOut.WriteLong(1);
    rOut.WriteLong(0x3000 | (mpMat->IsLightingEnabled() ? 1 : 0));

    // TEV
    uint32 NumPasses = mpMat->PassCount();
    rOut.WriteLong(NumPasses);

    for (uint32 iPass = 0; iPass < NumPasses; iPass++)
    {
        CMaterialPass *pPass = mpMat->Pass(iPass);

        uint32 ColorInputFlags = ((pPass->ColorInput(0)) |
                               (pPass->ColorInput(1) << 5) |
                               (pPass->ColorInput(2) << 10) |
                               (pPass->ColorInput(3) << 15));
        uint32 AlphaInputFlags = ((pPass->AlphaInput(0)) |
                               (pPass->AlphaInput(1) << 5) |
                               (pPass->AlphaInput(2) << 10) |
                               (pPass->AlphaInput(3) << 15));

        uint32 ColorOpFlags = 0x100 | (pPass->ColorOutput() << 9);
        uint32 AlphaOpFlags = 0x100 | (pPass->AlphaOutput() << 9);

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
    uint32 CurTexIdx = 0;

    for (uint32 iPass = 0; iPass < NumPasses; iPass++)
    {
        rOut.WriteShort(0); // Padding

        if (mpMat->Pass(iPass)->Texture())
        {
            rOut.WriteByte((uint8) CurTexIdx);
            rOut.WriteByte((uint8) CurTexIdx);
            CurTexIdx++;
        }

        else
            rOut.WriteShort((uint16) 0xFFFF);
    }

    // TexGen
    uint32 NumTexCoords = CurTexIdx; // TexIdx is currently equal to the tex coord count
    rOut.WriteLong(NumTexCoords);
    uint32 CurTexMtx = 0;

    for (uint32 iPass = 0; iPass < NumPasses; iPass++)
    {
        CMaterialPass *pPass = mpMat->Pass(iPass);
        if (pPass->Texture() == nullptr) continue;

        uint32 AnimType = pPass->AnimMode();
        uint32 CoordSource = pPass->TexCoordSource();

        uint32 TexMtxIdx, PostMtxIdx;
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

        uint32 TexGenFlags = (CoordSource << 4) | (TexMtxIdx << 9) | (Normalize << 14) | (PostMtxIdx << 15);
        rOut.WriteLong(TexGenFlags);
    }

    // Animations
    uint32 AnimSizeOffset = rOut.Tell();
    uint32 NumAnims = CurTexMtx; // CurTexMtx is currently equal to the anim count
    rOut.WriteLong(0);         // Anim size filler
    uint32 AnimsStart = rOut.Tell();
    rOut.WriteLong(NumAnims);

    for (uint32 iPass = 0; iPass < NumPasses; iPass++)
    {
        CMaterialPass *pPass = mpMat->Pass(iPass);
        uint32 AnimMode = pPass->AnimMode();
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

    uint32 AnimsEnd = rOut.Tell();
    uint32 AnimsSize = AnimsEnd - AnimsStart;
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
    case EGame::PrimeDemo:
    case EGame::Prime:
    case EGame::EchoesDemo:
    case EGame::Echoes:
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
    case EGame::PrimeDemo:
    case EGame::Prime:
    case EGame::EchoesDemo:
    case EGame::Echoes:
        Cooker.WriteMaterialPrime(rOut);
        break;
    // TODO: Corruption/Uncooked
    }
}
