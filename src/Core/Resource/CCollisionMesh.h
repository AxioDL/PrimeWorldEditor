#ifndef CCOLLISIONMESH_H
#define CCOLLISIONMESH_H

#include "CCollisionMaterial.h"
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

        SOctreeNode* mpRoot;
    };

    class CCollisionVertex
    {
    public:
        u32 MaterialIdx;
        CVector3f Pos;
    };

    class CCollisionLine
    {
    public:
        u32 MaterialIdx;
        u16 Vertices[2];
    };

    class CCollisionFace
    {
    public:
        u32 MaterialIdx;
        u16 Lines[3];
    };

    CVertexBuffer mVBO;
    CIndexBuffer mIBO;
    u32 mVertexCount;
    u32 mLineCount;
    u32 mFaceCount;
    bool mBuffered;

    CAABox mAABox;
    CCollisionOctree *mpOctree;
    std::vector<CCollisionMaterial> mMaterials;
    std::vector<CCollisionVertex> mCollisionVertices;
    std::vector<CCollisionLine> mCollisionLines;
    std::vector<CCollisionFace> mCollisionFaces;
    std::vector<u32> mMaterialOffsets;
    bool mOctreeLoaded;

    CCollisionVertex *GetVertex(u16 Index);
    CCollisionLine *GetLine(u16 Index);
    CCollisionFace *GetFace(u16 Index);

public:
    CCollisionMesh();
    ~CCollisionMesh();

    void BufferGL();
    void Draw();
    void DrawMaterial(u32 MatIdx);
    void DrawWireframe();

    inline u32 NumMaterials() const                     { return mMaterials.size(); }
    inline CCollisionMaterial& GetMaterial(u32 Index)   { return mMaterials[Index]; }
    inline const CAABox& BoundingBox() const            { return mAABox; }
};

#endif // CCOLLISIONMESH_H
