#include "CCollisionMesh.h"
#include <Core/CRenderer.h>

CCollisionMesh::CCollisionMesh()
{
    mVBO.SetVertexDesc(ePosition);
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

    // Add all the verts to our VBO, first...
    mVBO.Reserve(mCollisionVertices.size());
    for (u16 v = 0; v < mCollisionVertices.size(); v++)
        mVBO.AddVertex(CVertex(mCollisionVertices[v].Pos));

    // Then add all the relevant indices to the IBO
    mIBO.Reserve(mCollisionFaces.size() * 3);
    for (u32 v = 0; v < mCollisionFaces.size(); v++)
    {
        u16 Verts[3];

        CCollisionFace *Face = &mCollisionFaces[v];
        CCollisionLine *LineA = GetLine(Face->Lines[0]);
        CCollisionLine *LineB = GetLine(Face->Lines[1]);
        Verts[0] = LineA->Vertices[0];
        Verts[1] = LineA->Vertices[1];

        // We have two vertex indices; the last one is one of the ones on line B, but we're not sure which one
        if ((LineB->Vertices[0] != Verts[0]) &&
            (LineB->Vertices[0] != Verts[1]))
            Verts[2] = LineB->Vertices[0];
        else
            Verts[2] = LineB->Vertices[1];

        // Some faces have a property that indicates they need to be inverted
        if (!Face->Properties.Invert)
            mIBO.AddIndices(&Verts[0], 3);
        else {
            mIBO.AddIndex(Verts[2]);
            mIBO.AddIndex(Verts[1]);
            mIBO.AddIndex(Verts[0]);
        }
    }

    // Buffer, and done
    mVBO.Buffer();
    mIBO.Buffer();
    mBuffered = true;
}

void CCollisionMesh::Draw()
{
    if (!mBuffered) BufferGL();

    mVBO.Bind();
    mIBO.Bind();

    glDrawElements(GL_TRIANGLES, mIBO.GetSize(), GL_UNSIGNED_SHORT, (void*) 0);
    gDrawCount++;
    mIBO.Unbind();
    mVBO.Unbind();
}

void CCollisionMesh::DrawWireframe()
{
    if (!mBuffered) BufferGL();

    mVBO.Bind();
    mIBO.Bind();
    for (u32 f = 0; f < mFaceCount; f++)
    {
        glDrawElements(GL_LINE_LOOP, 3, GL_UNSIGNED_SHORT, (void*) (f * 6));
        gDrawCount++;
    }
    mIBO.Unbind();
    mVBO.Unbind();
}

CCollisionMesh::CCollisionVertex* CCollisionMesh::GetVertex(u16 index)
{
    return &mCollisionVertices[index];
}

CCollisionMesh::CCollisionLine* CCollisionMesh::GetLine(u16 index)
{
    return &mCollisionLines[index];
}

CCollisionMesh::CCollisionFace* CCollisionMesh::GetFace(u16 index)
{
    return &mCollisionFaces[index];
}
