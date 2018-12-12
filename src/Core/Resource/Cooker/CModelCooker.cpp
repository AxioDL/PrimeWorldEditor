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

    for (uint32 iMat = 0; iMat < mpModel->GetMatCount(); iMat++)
    {
        CMaterial *pMat = mpModel->GetMaterialByIndex(0, iMat);
        mVtxAttribs |= pMat->VtxDesc();
    }

    // Get vertices
    uint32 MaxIndex = 0;

    for (uint32 iSurf = 0; iSurf < mNumSurfaces; iSurf++)
    {
        uint32 NumPrimitives = mpModel->mSurfaces[iSurf]->Primitives.size();

        for (uint32 iPrim = 0; iPrim < NumPrimitives; iPrim++)
        {
            SSurface::SPrimitive *pPrim = &mpModel->mSurfaces[iSurf]->Primitives[iPrim];
            uint32 NumVerts = pPrim->Vertices.size();

            for (uint32 iVtx = 0; iVtx < NumVerts; iVtx++)
            {
                uint32 VertIndex = pPrim->Vertices[iVtx].ArrayPosition;
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

    uint32 NumSections = mNumMatSets + mNumSurfaces + 6;
    rOut.WriteLong(NumSections);
    rOut.WriteLong(mNumMatSets);

    uint32 SectionSizesOffset = rOut.Tell();
    for (uint32 iSec = 0; iSec < NumSections; iSec++)
        rOut.WriteLong(0);

    rOut.WriteToBoundary(32, 0);

    std::vector<uint32> SectionSizes;
    SectionSizes.reserve(NumSections);

    CSectionMgrOut SectionMgr;
    SectionMgr.SetSectionCount(NumSections);
    SectionMgr.Init(rOut);

    // Materials
    for (uint32 iSet = 0; iSet < mNumMatSets; iSet++)
    {
        CMaterialCooker::WriteCookedMatSet(mpModel->mMaterialSets[iSet], mVersion, rOut);
        rOut.WriteToBoundary(32, 0);
        SectionMgr.AddSize(rOut);
    }

    // Vertices
    for (uint32 iPos = 0; iPos < mNumVertices; iPos++)
        mVertices[iPos].Position.Write(rOut);

    rOut.WriteToBoundary(32, 0);
    SectionMgr.AddSize(rOut);

    // Normals
    for (uint32 iNrm = 0; iNrm < mNumVertices; iNrm++)
        mVertices[iNrm].Normal.Write(rOut);

    rOut.WriteToBoundary(32, 0);
    SectionMgr.AddSize(rOut);

    // Colors
    for (uint32 iColor = 0; iColor < mNumVertices; iColor++)
        mVertices[iColor].Color[0].Write(rOut);

    rOut.WriteToBoundary(32, 0);
    SectionMgr.AddSize(rOut);

    // Float UV coordinates
    for (uint32 iTexSlot = 0; iTexSlot < 8; iTexSlot++)
    {
        bool HasTexSlot = (mVtxAttribs & (eTex0 << iTexSlot)) != 0;
        if (HasTexSlot)
        {
            for (uint32 iTex = 0; iTex < mNumVertices; iTex++)
                mVertices[iTex].Tex[iTexSlot].Write(rOut);
        }
    }

    rOut.WriteToBoundary(32, 0);
    SectionMgr.AddSize(rOut);
    SectionMgr.AddSize(rOut); // Skipping short UV coordinates

    // Surface offsets
    rOut.WriteLong(mNumSurfaces);
    uint32 SurfaceOffsetsStart = rOut.Tell();

    for (uint32 iSurf = 0; iSurf < mNumSurfaces; iSurf++)
        rOut.WriteLong(0);

    rOut.WriteToBoundary(32, 0);
    SectionMgr.AddSize(rOut);

    // Surfaces
    uint32 SurfacesStart = rOut.Tell();
    std::vector<uint32> SurfaceEndOffsets(mNumSurfaces);

    for (uint32 iSurf = 0; iSurf < mNumSurfaces; iSurf++)
    {
        SSurface *pSurface = mpModel->GetSurface(iSurf);

        pSurface->CenterPoint.Write(rOut);
        rOut.WriteLong(pSurface->MaterialID);
        rOut.WriteShort((uint16) 0x8000);
        uint32 PrimTableSizeOffset = rOut.Tell();
        rOut.WriteShort(0);
        rOut.WriteLongLong(0);
        rOut.WriteLong(0);
        pSurface->ReflectionDirection.Write(rOut);
        rOut.WriteToBoundary(32, 0);

        uint32 PrimTableStart = rOut.Tell();
        FVertexDescription VtxAttribs = mpModel->GetMaterialBySurface(0, iSurf)->VtxDesc();

        for (uint32 iPrim = 0; iPrim < pSurface->Primitives.size(); iPrim++)
        {
            SSurface::SPrimitive *pPrimitive = &pSurface->Primitives[iPrim];
            rOut.WriteByte((uint8) pPrimitive->Type);
            rOut.WriteShort((uint16) pPrimitive->Vertices.size());

            for (uint32 iVert = 0; iVert < pPrimitive->Vertices.size(); iVert++)
            {
                CVertex *pVert = &pPrimitive->Vertices[iVert];

                if (mVersion == EGame::Echoes)
                {
                    for (uint32 iMtxAttribs = 0; iMtxAttribs < 8; iMtxAttribs++)
                        if (VtxAttribs & (ePosMtx << iMtxAttribs))
                            rOut.WriteByte(pVert->MatrixIndices[iMtxAttribs]);
                }

                uint16 VertexIndex = (uint16) pVert->ArrayPosition;

                if (VtxAttribs & ePosition)
                    rOut.WriteShort(VertexIndex);

                if (VtxAttribs & eNormal)
                    rOut.WriteShort(VertexIndex);

                if (VtxAttribs & eColor0)
                    rOut.WriteShort(VertexIndex);

                if (VtxAttribs & eColor1)
                    rOut.WriteShort(VertexIndex);

                uint16 TexOffset = 0;
                for (uint32 iTex = 0; iTex < 8; iTex++)
                {
                    if (VtxAttribs & (eTex0 << iTex))
                    {
                        rOut.WriteShort(VertexIndex + TexOffset);
                        TexOffset += (uint16) mNumVertices;
                    }
                }
            }
        }

        rOut.WriteToBoundary(32, 0);
        uint32 PrimTableEnd = rOut.Tell();
        uint32 PrimTableSize = PrimTableEnd - PrimTableStart;
        rOut.Seek(PrimTableSizeOffset, SEEK_SET);
        rOut.WriteShort((uint16) PrimTableSize);
        rOut.Seek(PrimTableEnd, SEEK_SET);

        SectionMgr.AddSize(rOut);
        SurfaceEndOffsets[iSurf] = rOut.Tell() - SurfacesStart;
    }

    // Done writing the file - now we go back to fill in surface offsets + section sizes
    rOut.Seek(SurfaceOffsetsStart, SEEK_SET);

    for (uint32 iSurf = 0; iSurf < mNumSurfaces; iSurf++)
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

uint32 CModelCooker::GetCMDLVersion(EGame Version)
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
