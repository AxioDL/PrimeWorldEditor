#include "CModelCooker.h"
#include "CMaterialCooker.h"
#include "CSectionMgrOut.h"

#include <algorithm>

CModelCooker::CModelCooker() = default;

void CModelCooker::GenerateSurfaceData()
{
    // Need to gather metadata from the model before we can start
    mNumMatSets = mpModel->mMaterialSets.size();
    mNumSurfaces = mpModel->mSurfaces.size();
    mNumVertices = mpModel->mVertexCount;
    mVertices.resize(mNumVertices);

    // Get vertex attributes
    mVtxAttribs = EVertexAttribute::None;

    for (size_t iMat = 0; iMat < mpModel->GetMatCount(); iMat++)
    {
        CMaterial *pMat = mpModel->GetMaterialByIndex(0, iMat);
        mVtxAttribs |= pMat->VtxDesc();
    }

    // Get vertices
    size_t MaxIndex = 0;

    for (size_t iSurf = 0; iSurf < mNumSurfaces; iSurf++)
    {
        const size_t NumPrimitives = mpModel->mSurfaces[iSurf]->Primitives.size();

        for (size_t iPrim = 0; iPrim < NumPrimitives; iPrim++)
        {
            const SSurface::SPrimitive *pPrim = &mpModel->mSurfaces[iSurf]->Primitives[iPrim];
            const size_t NumVerts = pPrim->Vertices.size();

            for (size_t iVtx = 0; iVtx < NumVerts; iVtx++)
            {
                const uint32 VertIndex = pPrim->Vertices[iVtx].ArrayPosition;
                mVertices[VertIndex] = pPrim->Vertices[iVtx];

                if (VertIndex > MaxIndex)
                    MaxIndex = VertIndex;
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
    rOut.WriteULong(0xDEADBABE);
    rOut.WriteULong(GetCMDLVersion(mVersion));
    rOut.WriteULong(mpModel->IsSkinned() ? 6 : 5);
    mpModel->mAABox.Write(rOut);

    const uint32 NumSections = mNumMatSets + mNumSurfaces + 6;
    rOut.WriteULong(NumSections);
    rOut.WriteULong(mNumMatSets);

    const uint32 SectionSizesOffset = rOut.Tell();
    for (uint32 iSec = 0; iSec < NumSections; iSec++)
        rOut.WriteULong(0);

    rOut.WriteToBoundary(32, 0);

    std::vector<uint32> SectionSizes;
    SectionSizes.reserve(NumSections);

    CSectionMgrOut SectionMgr;
    SectionMgr.SetSectionCount(NumSections);
    SectionMgr.Init(rOut);

    // Materials
    for (size_t iSet = 0; iSet < mNumMatSets; iSet++)
    {
        CMaterialCooker::WriteCookedMatSet(mpModel->mMaterialSets[iSet], mVersion, rOut);
        rOut.WriteToBoundary(32, 0);
        SectionMgr.AddSize(rOut);
    }

    // Vertices
    for (size_t iPos = 0; iPos < mNumVertices; iPos++)
        mVertices[iPos].Position.Write(rOut);

    rOut.WriteToBoundary(32, 0);
    SectionMgr.AddSize(rOut);

    // Normals
    for (size_t iNrm = 0; iNrm < mNumVertices; iNrm++)
        mVertices[iNrm].Normal.Write(rOut);

    rOut.WriteToBoundary(32, 0);
    SectionMgr.AddSize(rOut);

    // Colors
    for (size_t iColor = 0; iColor < mNumVertices; iColor++)
        mVertices[iColor].Color[0].Write(rOut);

    rOut.WriteToBoundary(32, 0);
    SectionMgr.AddSize(rOut);

    // Float UV coordinates
    for (size_t iTexSlot = 0; iTexSlot < 8; iTexSlot++)
    {
        const auto TexSlotBit = static_cast<uint32>(EVertexAttribute::Tex0 << iTexSlot);
        const bool HasTexSlot = (mVtxAttribs & TexSlotBit) != 0;
        if (HasTexSlot)
        {
            for (size_t iTex = 0; iTex < mNumVertices; iTex++)
                mVertices[iTex].Tex[iTexSlot].Write(rOut);
        }
    }

    rOut.WriteToBoundary(32, 0);
    SectionMgr.AddSize(rOut);
    SectionMgr.AddSize(rOut); // Skipping short UV coordinates

    // Surface offsets
    rOut.WriteULong(mNumSurfaces);
    const uint32 SurfaceOffsetsStart = rOut.Tell();

    for (uint32 iSurf = 0; iSurf < mNumSurfaces; iSurf++)
        rOut.WriteULong(0);

    rOut.WriteToBoundary(32, 0);
    SectionMgr.AddSize(rOut);

    // Surfaces
    const uint32 SurfacesStart = rOut.Tell();
    std::vector<uint32> SurfaceEndOffsets(mNumSurfaces);

    for (size_t iSurf = 0; iSurf < mNumSurfaces; iSurf++)
    {
        SSurface *pSurface = mpModel->GetSurface(iSurf);

        pSurface->CenterPoint.Write(rOut);
        rOut.WriteULong(pSurface->MaterialID);
        rOut.WriteUShort(0x8000);
        const uint32 PrimTableSizeOffset = rOut.Tell();
        rOut.WriteUShort(0);
        rOut.WriteULongLong(0);
        rOut.WriteULong(0);
        pSurface->ReflectionDirection.Write(rOut);
        rOut.WriteToBoundary(32, 0);

        const uint32 PrimTableStart = rOut.Tell();
        const FVertexDescription VtxAttribs = mpModel->GetMaterialBySurface(0, iSurf)->VtxDesc();

        for (const SSurface::SPrimitive& pPrimitive : pSurface->Primitives)
        {
            rOut.WriteUByte(static_cast<uint8>(pPrimitive.Type));
            rOut.WriteUShort(static_cast<uint16>(pPrimitive.Vertices.size()));

            for (const CVertex& pVert : pPrimitive.Vertices)
            {
                if (mVersion == EGame::Echoes)
                {
                    for (size_t iMtxAttribs = 0; iMtxAttribs < pVert.MatrixIndices.size(); iMtxAttribs++)
                    {
                        const auto  MatrixBit = static_cast<uint32>(EVertexAttribute::PosMtx << iMtxAttribs);
                        if ((VtxAttribs & MatrixBit) != 0)
                        {
                            rOut.WriteUByte(pVert.MatrixIndices[iMtxAttribs]);
                        }
                    }
                }

                const auto VertexIndex = static_cast<uint16>(pVert.ArrayPosition);

                if ((VtxAttribs & EVertexAttribute::Position) != 0)
                    rOut.WriteUShort(VertexIndex);

                if ((VtxAttribs & EVertexAttribute::Normal) != 0)
                    rOut.WriteUShort(VertexIndex);

                if ((VtxAttribs & EVertexAttribute::Color0) != 0)
                    rOut.WriteUShort(VertexIndex);

                if ((VtxAttribs & EVertexAttribute::Color1) != 0)
                    rOut.WriteUShort(VertexIndex);

                uint16 TexOffset = 0;
                for (uint32 iTex = 0; iTex < 8; iTex++)
                {
                    const auto TexBit = static_cast<uint32>(EVertexAttribute::Tex0 << iTex);

                    if ((VtxAttribs & TexBit) != 0)
                    {
                        rOut.WriteUShort(static_cast<uint16>(VertexIndex + TexOffset));
                        TexOffset += static_cast<uint16>(mNumVertices);
                    }
                }
            }
        }

        rOut.WriteToBoundary(32, 0);
        const uint32 PrimTableEnd = rOut.Tell();
        const uint32 PrimTableSize = PrimTableEnd - PrimTableStart;
        rOut.Seek(PrimTableSizeOffset, SEEK_SET);
        rOut.WriteUShort(static_cast<uint16>(PrimTableSize));
        rOut.Seek(PrimTableEnd, SEEK_SET);

        SectionMgr.AddSize(rOut);
        SurfaceEndOffsets[iSurf] = rOut.Tell() - SurfacesStart;
    }

    // Done writing the file - now we go back to fill in surface offsets + section sizes
    rOut.Seek(SurfaceOffsetsStart, SEEK_SET);

    for (size_t iSurf = 0; iSurf < mNumSurfaces; iSurf++)
        rOut.WriteULong(SurfaceEndOffsets[iSurf]);

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
