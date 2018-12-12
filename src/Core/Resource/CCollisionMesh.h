#ifndef CCOLLISIONMESH_H
#define CCOLLISIONMESH_H

#include "CCollisionMaterial.h"
#include "CResource.h"
#include "Core/OpenGL/CVertexBuffer.h"
#include "Core/OpenGL/CIndexBuffer.h"
#include <Common/Math/CAABox.h>

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
            std::vector<uint16> FaceIndices;
        };

        struct SBranch : public SOctreeNode
        {
            uint16 Flags;
            SOctreeNode *pChildren[8];
        };

        SOctreeNode* mpRoot;
    };

    class CCollisionVertex
    {
    public:
        uint32 MaterialIdx;
        CVector3f Pos;
    };

    class CCollisionLine
    {
    public:
        uint32 MaterialIdx;
        uint16 Vertices[2];
    };

    class CCollisionFace
    {
    public:
        uint32 MaterialIdx;
        uint16 Lines[3];
    };

    CVertexBuffer mVBO;
    CIndexBuffer mIBO;
    uint32 mVertexCount;
    uint32 mLineCount;
    uint32 mFaceCount;
    bool mBuffered;

    CAABox mAABox;
    CCollisionOctree *mpOctree;
    std::vector<CCollisionMaterial> mMaterials;
    std::vector<CCollisionVertex> mCollisionVertices;
    std::vector<CCollisionLine> mCollisionLines;
    std::vector<CCollisionFace> mCollisionFaces;
    std::vector<uint32> mMaterialOffsets;
    bool mOctreeLoaded;

    CCollisionVertex *GetVertex(uint16 Index);
    CCollisionLine *GetLine(uint16 Index);
    CCollisionFace *GetFace(uint16 Index);

public:
    CCollisionMesh();
    ~CCollisionMesh();

    void BufferGL();
    void Draw();
    void DrawMaterial(uint32 MatIdx, bool Wireframe);
    void DrawWireframe();

    inline uint32 NumMaterials() const                      { return mMaterials.size(); }
    inline CCollisionMaterial& GetMaterial(uint32 Index)    { return mMaterials[Index]; }
    inline const CAABox& BoundingBox() const                { return mAABox; }
};

#endif // CCOLLISIONMESH_H
