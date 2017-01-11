#include "CCollisionMesh.h"
#include "Core/Render/CRenderer.h"
#include "Core/Render/CDrawUtil.h"
#include <Common/Assert.h>

CCollisionMesh::CCollisionMesh()
{
    mVBO.SetVertexDesc(ePosition | eNormal);
    mVertexCount = 0;
    mLineCount = 0;
    mFaceCount = 0;
    mBuffered = false;
    mIBO.SetPrimitiveType(GL_TRIANGLES);
}

CCollisionMesh::~CCollisionMesh()
{
    if (mBuffered)
    {
        mIBO.Clear();
        mVBO.Clear();
        mBuffered = false;
    }
}

void CCollisionMesh::BufferGL()
{
    if (mBuffered)
    {
        mIBO.Clear();
        mVBO.Clear();
        mBuffered = false;
    }

    // Create new list of collision faces sorted by material index
    std::vector<CCollisionFace> SortedTris = mCollisionFaces;
    std::sort(SortedTris.begin(), SortedTris.end(), [](CCollisionFace& rLeft, CCollisionFace& rRight) -> bool {
        return rLeft.MaterialIdx < rRight.MaterialIdx;
    });

    // Add all the relevant indices to the IBO
    mVBO.Reserve(SortedTris.size() * 3);
    mIBO.Reserve(SortedTris.size() * 3);

    mMaterialOffsets.reserve(mMaterials.size());
    u32 CurMat = 0;

    for (u32 iTri = 0; iTri < SortedTris.size(); iTri++)
    {
        u16 Verts[3];

        CCollisionFace *pFace = &SortedTris[iTri];
        CCollisionLine *pLineA = GetLine(pFace->Lines[0]);
        CCollisionLine *pLineB = GetLine(pFace->Lines[1]);
        Verts[0] = pLineA->Vertices[0];
        Verts[1] = pLineA->Vertices[1];

        // Check if we've reached a new material
        if (pFace->MaterialIdx != CurMat)
        {
            while (CurMat != pFace->MaterialIdx)
            {
                mMaterialOffsets.push_back(mIBO.GetSize());
                CurMat++;
            }
        }

        // We have two vertex indices; the last one is one of the ones on line B, but we're not sure which one
        if ((pLineB->Vertices[0] != Verts[0]) &&
            (pLineB->Vertices[0] != Verts[1]))
            Verts[2] = pLineB->Vertices[0];
        else
            Verts[2] = pLineB->Vertices[1];

        // Some faces have a property that indicates they need to be inverted
        if (GetMaterial(pFace->MaterialIdx) & eCF_FlippedTri)
        {
            u16 V0 = Verts[0];
            Verts[0] = Verts[2];
            Verts[2] = V0;
        }

        // Generate vertices - we don't share vertices between triangles in order to get the generated normals looking correct
        CCollisionVertex& rVert0 = mCollisionVertices[Verts[0]];
        CCollisionVertex& rVert1 = mCollisionVertices[Verts[1]];
        CCollisionVertex& rVert2 = mCollisionVertices[Verts[2]];

        CVector3f V0toV1 = (rVert1.Pos - rVert0.Pos).Normalized();
        CVector3f V0toV2 = (rVert2.Pos - rVert0.Pos).Normalized();
        CVector3f FaceNormal = V0toV1.Cross(V0toV2).Normalized();

        for (u32 iVtx = 0; iVtx < 3; iVtx++)
        {
            CVertex Vtx;
            Vtx.Position = mCollisionVertices[ Verts[iVtx] ].Pos;
            Vtx.Normal = FaceNormal;
            u16 Index = mVBO.AddVertex(Vtx);
            mIBO.AddIndex(Index);
        }
    }

    while (CurMat != mMaterials.size())
    {
        mMaterialOffsets.push_back(mIBO.GetSize());
        CurMat++;
    }

    ASSERT(mMaterialOffsets.size() == mMaterials.size());

    // Buffer, and done
    mVBO.Buffer();
    mIBO.Buffer();
    mBuffered = true;
}

void CCollisionMesh::Draw()
{
    if (!mBuffered) BufferGL();

    mVBO.Bind();
    mIBO.DrawElements();
    mVBO.Unbind();
}

void CCollisionMesh::DrawMaterial(u32 MatIdx)
{
    if (!mBuffered) BufferGL();
    ASSERT(MatIdx < mMaterials.size());

    mVBO.Bind();
    u32 StartIdx = (MatIdx == 0 ? 0 : mMaterialOffsets[MatIdx - 1]);
    u32 NumElements = mMaterialOffsets[MatIdx] - StartIdx;
    mIBO.DrawElements(StartIdx, NumElements);
    mVBO.Unbind();
}

void CCollisionMesh::DrawWireframe()
{
    CDrawUtil::UseColorShader(CColor::skBlack);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    Draw();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

CCollisionMesh::CCollisionVertex* CCollisionMesh::GetVertex(u16 Index)
{
    return &mCollisionVertices[Index];
}

CCollisionMesh::CCollisionLine* CCollisionMesh::GetLine(u16 Index)
{
    return &mCollisionLines[Index];
}

CCollisionMesh::CCollisionFace* CCollisionMesh::GetFace(u16 Index)
{
    return &mCollisionFaces[Index];
}
