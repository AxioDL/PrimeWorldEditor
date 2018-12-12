#ifndef CBASICMODEL_H
#define CBASICMODEL_H

#include "SSurface.h"
#include "Core/Resource/CResource.h"
#include "Core/OpenGL/CVertexBuffer.h"
#include <Common/Math/CAABox.h>

class CBasicModel : public CResource
{
    DECLARE_RESOURCE_TYPE(eModel)
protected:
    CAABox mAABox;
    uint32 mVertexCount;
    uint32 mTriangleCount;
    bool mBuffered;
    bool mHasOwnMaterials;
    bool mHasOwnSurfaces;

    CVertexBuffer mVBO;
    std::vector<SSurface*> mSurfaces;

public:
    CBasicModel(CResourceEntry *pEntry = 0);
    ~CBasicModel();

    uint32 GetVertexCount();
    uint32 GetTriangleCount();
    CAABox AABox();
    bool IsBuffered();
    uint32 GetSurfaceCount();
    CAABox GetSurfaceAABox(uint32 Surface);
    SSurface* GetSurface(uint32 Surface);
    virtual void ClearGLBuffer() = 0;
};

#endif // CBASICMODEL_H
