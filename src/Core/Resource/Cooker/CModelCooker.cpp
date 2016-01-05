#include "CModelCooker.h"
#include "CMaterialCooker.h"
#include "CSectionMgrOut.h"

#include <algorithm>
#include <iostream>

CModelCooker::CModelCooker()
{
}

bool SortVertsByArrayPos(const CVertex& A, const CVertex& B) {
    return (A.ArrayPosition < B.ArrayPosition);
}

bool CheckDuplicateVertsByArrayPos(const CVertex& A, const CVertex& B) {
    return (A.ArrayPosition == B.ArrayPosition);
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

void CModelCooker::WriteEditorModel(IOutputStream& /*Out*/)
{
}

void CModelCooker::WriteModelPrime(IOutputStream& Out)
{
    GenerateSurfaceData();

    // Header
    Out.WriteLong(0xDEADBABE);
    Out.WriteLong(GetCMDLVersion(mVersion));
    Out.WriteLong(5);
    mpModel->mAABox.Write(Out);

    u32 NumSections = mNumMatSets + mNumSurfaces + 6;
    Out.WriteLong(NumSections);
    Out.WriteLong(mNumMatSets);

    u32 SectionSizesOffset = Out.Tell();
    for (u32 iSec = 0; iSec < NumSections; iSec++)
        Out.WriteLong(0);

    Out.WriteToBoundary(32, 0);

    std::vector<u32> SectionSizes;
    SectionSizes.reserve(NumSections);

    CSectionMgrOut SectionMgr;
    SectionMgr.SetSectionCount(NumSections);
    SectionMgr.Init(Out);

    // Materials
    for (u32 iSet = 0; iSet < mNumMatSets; iSet++)
    {
        CMaterialCooker::WriteCookedMatSet(mpModel->mMaterialSets[iSet], mVersion, Out);
        Out.WriteToBoundary(32, 0);
        SectionMgr.AddSize(Out);
    }

    // Vertices
    for (u32 iPos = 0; iPos < mNumVertices; iPos++)
        mVertices[iPos].Position.Write(Out);

    Out.WriteToBoundary(32, 0);
    SectionMgr.AddSize(Out);

    // Normals
    for (u32 iNrm = 0; iNrm < mNumVertices; iNrm++)
        mVertices[iNrm].Normal.Write(Out);

    Out.WriteToBoundary(32, 0);
    SectionMgr.AddSize(Out);

    // Colors
    for (u32 iColor = 0; iColor < mNumVertices; iColor++)
        mVertices[iColor].Color[0].Write(Out);

    Out.WriteToBoundary(32, 0);
    SectionMgr.AddSize(Out);

    // Float UV coordinates
    for (u32 iTexSlot = 0; iTexSlot < 8; iTexSlot++)
    {
        bool HasTexSlot = (mVtxAttribs & (eTex0 << (iTexSlot * 2))) != 0;
        if (HasTexSlot)
        {
            for (u32 iTex = 0; iTex < mNumVertices; iTex++)
                mVertices[iTex].Tex[iTexSlot].Write(Out);
        }
    }

    Out.WriteToBoundary(32, 0);
    SectionMgr.AddSize(Out);
    SectionMgr.AddSize(Out); // Skipping short UV coordinates

    // Surface offsets
    Out.WriteLong(mNumSurfaces);
    u32 SurfaceOffsetsStart = Out.Tell();

    for (u32 iSurf = 0; iSurf < mNumSurfaces; iSurf++)
        Out.WriteLong(0);

    Out.WriteToBoundary(32, 0);
    SectionMgr.AddSize(Out);

    // Surfaces
    u32 SurfacesStart = Out.Tell();
    std::vector<u32> SurfaceEndOffsets(mNumSurfaces);

    for (u32 iSurf = 0; iSurf < mNumSurfaces; iSurf++)
    {
        SSurface *pSurface = mpModel->GetSurface(iSurf);

        pSurface->CenterPoint.Write(Out);
        Out.WriteLong(pSurface->MaterialID);
        Out.WriteShort((u16) 0x8000);
        u32 PrimTableSizeOffset = Out.Tell();
        Out.WriteShort(0);
        Out.WriteLongLong(0);
        Out.WriteLong(0);
        pSurface->ReflectionDirection.Write(Out);
        Out.WriteToBoundary(32, 0);

        u32 PrimTableStart = Out.Tell();
        FVertexDescription MatAttribs = mpModel->GetMaterialBySurface(0, iSurf)->VtxDesc();

        for (u32 iPrim = 0; iPrim < pSurface->Primitives.size(); iPrim++)
        {
            SSurface::SPrimitive *pPrimitive = &pSurface->Primitives[iPrim];
            Out.WriteByte((u8) pPrimitive->Type);
            Out.WriteShort((u16) pPrimitive->Vertices.size());

            for (u32 iVert = 0; iVert < pPrimitive->Vertices.size(); iVert++)
            {
                CVertex *pVert = &pPrimitive->Vertices[iVert];

                if (mVersion == eEchoes)
                {
                    for (u32 iMtxAttribs = 0; iMtxAttribs < 8; iMtxAttribs++)
                        if (MatAttribs & (ePosMtx << iMtxAttribs))
                            Out.WriteByte(pVert->MatrixIndices[iMtxAttribs]);
                }

                u16 VertexIndex = (u16) pVert->ArrayPosition;

                if (MatAttribs & ePosition)
                    Out.WriteShort(VertexIndex);

                if (MatAttribs & eNormal)
                    Out.WriteShort(VertexIndex);

                if (MatAttribs & eColor0)
                    Out.WriteShort(VertexIndex);

                if (MatAttribs & eColor1)
                    Out.WriteShort(VertexIndex);

                u16 TexOffset = 0;
                for (u32 iTex = 0; iTex < 8; iTex++)
                {
                    if (MatAttribs & (eTex0 << (iTex * 2)))
                    {
                        Out.WriteShort(VertexIndex + TexOffset);
                        TexOffset += (u16) mNumVertices;
                    }
                }
            }
        }

        Out.WriteToBoundary(32, 0);
        u32 PrimTableEnd = Out.Tell();
        u32 PrimTableSize = PrimTableEnd - PrimTableStart;
        Out.Seek(PrimTableSizeOffset, SEEK_SET);
        Out.WriteShort((u16) PrimTableSize);
        Out.Seek(PrimTableEnd, SEEK_SET);

        SectionMgr.AddSize(Out);
        SurfaceEndOffsets[iSurf] = Out.Tell() - SurfacesStart;
    }

    // Done writing the file - now we go back to fill in surface offsets + section sizes
    Out.Seek(SurfaceOffsetsStart, SEEK_SET);

    for (u32 iSurf = 0; iSurf < mNumSurfaces; iSurf++)
        Out.WriteLong(SurfaceEndOffsets[iSurf]);

    Out.Seek(SectionSizesOffset, SEEK_SET);
    SectionMgr.WriteSizes(Out);

    // Done!
}

void CModelCooker::WriteCookedModel(CModel *pModel, EGame Version, IOutputStream& CMDL)
{
    CModelCooker Cooker;
    Cooker.mpModel = pModel;
    Cooker.mVersion = Version;

    switch (Version)
    {
    case ePrimeDemo:
    case ePrime:
    case eEchoesDemo:
    case eEchoes:
        Cooker.WriteModelPrime(CMDL);
        break;
    }
}

void CModelCooker::WriteUncookedModel(CModel* /*pModel*/, IOutputStream& /*EMDL*/)
{
}

u32 CModelCooker::GetCMDLVersion(EGame Version)
{
    switch (Version)
    {
    case ePrimeDemo:
    case ePrime:
        return 0x2;
    case eEchoesDemo:
        return 0x3;
    case eEchoes:
        return 0x4;
    case eCorruptionProto:
    case eCorruption:
        return 0x5;
    case eReturns:
        return 0xA;
    default:
        return 0;
    }
}
