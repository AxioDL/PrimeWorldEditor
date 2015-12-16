#ifndef CCOLLISIONMESH_H
#define CCOLLISIONMESH_H

#include "CResource.h"
#include "Core/OpenGL/CVertexBuffer.h"
#include "Core/OpenGL/CIndexBuffer.h"
#include <Math/CAABox.h>

class CCollisionMesh
{
    friend class CCollisionLoader;

    class CCollisionOctree
    {
        friend class CCollisionLoader;
        struct SOctreeNode {};

        struct SLeaf : public SOctreeNode
        {
            CAABox AABox;
            std::vector<u16> FaceIndices;
        };

        struct SBranch : public SOctreeNode
        {
            u16 Flags;
            SOctreeNode *pChildren[8];
        };

        SOctreeNode* Root;
    };

    struct SCollisionProperties
    {
        // todo: figure out what the other properties are
        bool Invert;
    };

    class CCollisionVertex
    {
    public:
        SCollisionProperties Properties;
        CVector3f Pos;
    };

    class CCollisionLine
    {
    public:
        SCollisionProperties Properties;
        u16 Vertices[2];
    };

    class CCollisionFace
    {
    public:
        SCollisionProperties Properties;
        u16 Lines[3];
    };

    CVertexBuffer mVBO;
    CIndexBuffer mIBO;
    u32 mVertexCount;
    u32 mLineCount;
    u32 mFaceCount;
    bool mBuffered;

    CAABox mAABox;
    CCollisionOctree *mOctree;
    std::vector<u32> mFlags;
    std::vector<CCollisionVertex> mCollisionVertices;
    std::vector<CCollisionLine> mCollisionLines;
    std::vector<CCollisionFace> mCollisionFaces;
    bool mOctreeLoaded;

    CCollisionVertex *GetVertex(u16 index);
    CCollisionLine *GetLine(u16 index);
    CCollisionFace *GetFace(u16 index);

public:
    CCollisionMesh();
    ~CCollisionMesh();

    void BufferGL();
    void Draw();
    void DrawWireframe();
};

#endif // CCOLLISIONMESH_H
