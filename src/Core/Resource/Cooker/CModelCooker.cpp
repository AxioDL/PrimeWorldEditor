#include "CModelCooker.h"
#include "CMaterialCooker.h"
#include "CSectionMgrOut.h"

#include <algorithm>
#include <iostream>

CModelCooker::CModelCooker()
{
}

void CModelCooker::GenerateSurfaceData()
{
    // Need to gather metadata from the model before we can start
    mNumMatSets = mpModel->mMaterialSets.size();
    mNumSurfaces = mpModel->mSurfaces.size();
    mNumVertices = mpModel->mVertexCount;
    mVertices.resize(mNumVertices);

    // Get vertex attributes
    mVtxAttribs = eNoAttributes;

    for (u32 iMat = 0; iMat < mpModel->GetMatCount(); iMat++)
    {
        CMaterial *pMat = mpModel->GetMaterialByIndex(0, iMat);
        mVtxAttribs |= pMat->VtxDesc();
    }

    // Get vertices
    u32 MaxIndex = 0;

    for (u32 iSurf = 0; iSurf < mNumSurfaces; iSurf++)
    {
        u32 NumPrimitives = mpModel->mSurfaces[iSurf]->Primitives.size();

        for (u32 iPrim = 0; iPrim < NumPrimitives; iPrim++)
        {
            SSurface::SPrimitive *pPrim = &mpModel->mSurfaces[iSurf]->Primitives[iPrim];
            u32 NumVerts = pPrim->Vertices.size();

            for (u32 iVtx = 0; iVtx < NumVerts; iVtx++)
            {
                u32 VertIndex = pPrim->Vertices[iVtx].ArrayPosition;
                mVertices[VertIndex] = pPrim->Vertices[iVtx];

                if (VertIndex > MaxIndex) MaxIndex = VertIndex;
            }
        }
    }

    mVertices.resize(MaxIndex + 1);
    mNumVertices = mVertices.size();
}

void CModelCooker::WriteEditorModel(IOutputStream& /*rOut*/)
{
}

void CModelCooker::WriteModelPrime(IOutputStream& rOut)
{
    GenerateSurfaceData();

    // Header
    rOut.WriteLong(0xDEADBABE);
    rOut.WriteLong(GetCMDLVersion(mVersion));
    rOut.WriteLong(mpModel->IsSkinned() ? 6 : 5);
    mpModel->mAABox.Write(rOut);

    u32 NumSections = mNumMatSets + mNumSurfaces + 6;
    rOut.WriteLong(NumSections);
    rOut.WriteLong(mNumMatSets);

    u32 SectionSizesOffset = rOut.Tell();
    for (u32 iSec = 0; iSec < NumSections; iSec++)
        rOut.WriteLong(0);

    rOut.WriteToBoundary(32, 0);

    std::vector<u32> SectionSizes;
    SectionSizes.reserve(NumSections);

    CSectionMgrOut SectionMgr;
    SectionMgr.SetSectionCount(NumSections);
    SectionMgr.Init(rOut);

    // Materials
    for (u32 iSet = 0; iSet < mNumMatSets; iSet++)
    {
        CMaterialCooker::WriteCookedMatSet(mpModel->mMaterialSets[iSet], mVersion, rOut);
        rOut.WriteToBoundary(32, 0);
        SectionMgr.AddSize(rOut);
    }

    // Vertices
    for (u32 iPos = 0; iPos < mNumVertices; iPos++)
        mVertices[iPos].Position.Write(rOut);

    rOut.WriteToBoundary(32, 0);
    SectionMgr.AddSize(rOut);

    // Normals
    for (u32 iNrm = 0; iNrm < mNumVertices; iNrm++)
        mVertices[iNrm].Normal.Write(rOut);

    rOut.WriteToBoundary(32, 0);
    SectionMgr.AddSize(rOut);

    // Colors
    for (u32 iColor = 0; iColor < mNumVertices; iColor++)
        mVertices[iColor].Color[0].Write(rOut);

    rOut.WriteToBoundary(32, 0);
    SectionMgr.AddSize(rOut);

    // Float UV coordinates
    for (u32 iTexSlot = 0; iTexSlot < 8; iTexSlot++)
    {
        bool HasTexSlot = (mVtxAttribs & (eTex0 << iTexSlot)) != 0;
        if (HasTexSlot)
        {
            for (u32 iTex = 0; iTex < mNumVertices; iTex++)
                mVertices[iTex].Tex[iTexSlot].Write(rOut);
        }
    }

    rOut.WriteToBoundary(32, 0);
    SectionMgr.AddSize(rOut);
    SectionMgr.AddSize(rOut); // Skipping short UV coordinates

    // Surface offsets
    rOut.WriteLong(mNumSurfaces);
    u32 SurfaceOffsetsStart = rOut.Tell();

    for (u32 iSurf = 0; iSurf < mNumSurfaces; iSurf++)
        rOut.WriteLong(0);

    rOut.WriteToBoundary(32, 0);
    SectionMgr.AddSize(rOut);

    // Surfaces
    u32 SurfacesStart = rOut.Tell();
    std::vector<u32> SurfaceEndOffsets(mNumSurfaces);

    for (u32 iSurf = 0; iSurf < mNumSurfaces; iSurf++)
    {
        SSurface *pSurface = mpModel->GetSurface(iSurf);

        pSurface->CenterPoint.Write(rOut);
        rOut.WriteLong(pSurface->MaterialID);
        rOut.WriteShort((u16) 0x8000);
        u32 PrimTableSizeOffset = rOut.Tell();
        rOut.WriteShort(0);
        rOut.WriteLongLong(0);
        rOut.WriteLong(0);
        pSurface->ReflectionDirection.Write(rOut);
        rOut.WriteToBoundary(32, 0);

        u32 PrimTableStart = rOut.Tell();
        FVertexDescription VtxAttribs = mpModel->GetMaterialBySurface(0, iSurf)->VtxDesc();

        for (u32 iPrim = 0; iPrim < pSurface->Primitives.size(); iPrim++)
        {
            SSurface::SPrimitive *pPrimitive = &pSurface->Primitives[iPrim];
            rOut.WriteByte((u8) pPrimitive->Type);
            rOut.WriteShort((u16) pPrimitive->Vertices.size());

            for (u32 iVert = 0; iVert < pPrimitive->Vertices.size(); iVert++)
            {
                CVertex *pVert = &pPrimitive->Vertices[iVert];

                if (mVersion == EGame::Echoes)
                {
                    for (u32 iMtxAttribs = 0; iMtxAttribs < 8; iMtxAttribs++)
                        if (VtxAttribs & (ePosMtx << iMtxAttribs))
                            rOut.WriteByte(pVert->MatrixIndices[iMtxAttribs]);
                }

                u16 VertexIndex = (u16) pVert->ArrayPosition;

                if (VtxAttribs & ePosition)
                    rOut.WriteShort(VertexIndex);

                if (VtxAttribs & eNormal)
                    rOut.WriteShort(VertexIndex);

                if (VtxAttribs & eColor0)
                    rOut.WriteShort(VertexIndex);

                if (VtxAttribs & eColor1)
                    rOut.WriteShort(VertexIndex);

                u16 TexOffset = 0;
                for (u32 iTex = 0; iTex < 8; iTex++)
                {
                    if (VtxAttribs & (eTex0 << iTex))
                    {
                        rOut.WriteShort(VertexIndex + TexOffset);
                        TexOffset += (u16) mNumVertices;
                    }
                }
            }
        }

        rOut.WriteToBoundary(32, 0);
        u32 PrimTableEnd = rOut.Tell();
        u32 PrimTableSize = PrimTableEnd - PrimTableStart;
        rOut.Seek(PrimTableSizeOffset, SEEK_SET);
        rOut.WriteShort((u16) PrimTableSize);
        rOut.Seek(PrimTableEnd, SEEK_SET);

        SectionMgr.AddSize(rOut);
        SurfaceEndOffsets[iSurf] = rOut.Tell() - SurfacesStart;
    }

    // Done writing the file - now we go back to fill in surface offsets + section sizes
    rOut.Seek(SurfaceOffsetsStart, SEEK_SET);

    for (u32 iSurf = 0; iSurf < mNumSurfaces; iSurf++)
        rOut.WriteLong(SurfaceEndOffsets[iSurf]);

    rOut.Seek(SectionSizesOffset, SEEK_SET);
    SectionMgr.WriteSizes(rOut);

    // Done!
}

bool CModelCooker::CookCMDL(CModel *pModel, IOutputStream& rOut)
{
    CModelCooker Cooker;
    Cooker.mpModel = pModel;
    Cooker.mVersion = pModel->Game();

    switch (pModel->Game())
    {
    case EGame::PrimeDemo:
    case EGame::Prime:
    case EGame::EchoesDemo:
    case EGame::Echoes:
        Cooker.WriteModelPrime(rOut);
        return true;

    default:
        return false;
    }
}

u32 CModelCooker::GetCMDLVersion(EGame Version)
{
    switch (Version)
    {
    case EGame::PrimeDemo:
    case EGame::Prime:
        return 0x2;
    case EGame::EchoesDemo:
        return 0x3;
    case EGame::Echoes:
        return 0x4;
    case EGame::CorruptionProto:
    case EGame::Corruption:
        return 0x5;
    case EGame::DKCReturns:
        return 0xA;
    default:
        return 0;
    }
}
