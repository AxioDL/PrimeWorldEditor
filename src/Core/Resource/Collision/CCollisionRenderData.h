#ifndef CCOLLISIONRENDERDATA_H
#define CCOLLISIONRENDERDATA_H

#include "SCollisionIndexData.h"
#include "SOBBTreeNode.h"
#include "Core/OpenGL/CVertexBuffer.h"
#include "Core/OpenGL/CIndexBuffer.h"

class CCollidableOBBTree;

/** Data for rendering a collision model */
class CCollisionRenderData
{
    /** Vertex/index buffer for the collision geometry */
    CVertexBuffer       mVertexBuffer;
    CIndexBuffer        mIndexBuffer;
    CIndexBuffer        mWireframeIndexBuffer;

    /** Index buffer offset for the start of each collision material.
     *  This has an extra index at the end, which is the end index for the last material. */
    std::vector<uint>   mMaterialIndexOffsets;
    std::vector<uint>   mMaterialWireIndexOffsets;

    /** Cached vertex/index buffer for the bounding hierarchy (octree or OBB tree) */
    CVertexBuffer       mBoundingVertexBuffer;
    CIndexBuffer        mBoundingIndexBuffer;

    /** Index buffer offset for different depth levels of the bounding hierarchy.
     *  This allows you to i.e. render only the first (n) levels of the hierarchy. */
    std::vector<uint>   mBoundingDepthOffsets;

    /** Whether render data has been built */
    bool mBuilt = false;

    /** Whether bounding hierarchy render data has been built */
    bool mBoundingHierarchyBuilt = false;

public:
    CCollisionRenderData() = default;

    /** Build from collision data */
    void BuildRenderData(const SCollisionIndexData& kIndexData);
    void BuildBoundingHierarchyRenderData(const SOBBTreeNode* pOBBTree);

    /** Render */
    void Render(bool Wireframe, int MaterialIndex = -1);
    void RenderBoundingHierarchy(int MaxDepthLevel = -1);
    int MaxBoundingHierarchyDepth() const;

    /** Accessors */
    bool IsBuilt() const                     { return mBuilt; }
};

#endif // CCOLLISIONRENDERDATA_H
