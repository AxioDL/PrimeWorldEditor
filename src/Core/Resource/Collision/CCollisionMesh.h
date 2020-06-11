#ifndef CCOLLISIONMESH_H
#define CCOLLISIONMESH_H

#include "CCollisionMaterial.h"
#include "CCollisionRenderData.h"
#include "SCollisionIndexData.h"
#include <Common/Math/CAABox.h>

/** Base class of collision geometry */
class CCollisionMesh
{
    friend class CCollisionLoader;

protected:
    CAABox                  mAABox;
    SCollisionIndexData     mIndexData;
    CCollisionRenderData    mRenderData;

public:
    virtual ~CCollisionMesh() = default;
    virtual void BuildRenderData();

    /** Accessors */
    CAABox Bounds() const
    {
        return mAABox;
    }

    const SCollisionIndexData& GetIndexData() const
    {
        return mIndexData;
    }

    const CCollisionRenderData& GetRenderData() const
    {
        return mRenderData;
    }

    CCollisionRenderData& GetRenderData()
    {
        return mRenderData;
    }
};

#endif // CCOLLISIONMESH_H
