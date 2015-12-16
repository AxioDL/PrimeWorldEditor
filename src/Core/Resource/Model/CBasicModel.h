#ifndef CBASICMODEL_H
#define CBASICMODEL_H

#include "SSurface.h"
#include "Core/Resource/CResource.h"
#include "Core/OpenGL/CVertexBuffer.h"
#include <Math/CAABox.h>

class CBasicModel : public CResource
{
    DECLARE_RESOURCE_TYPE(eModel)
protected:
    CAABox mAABox;
    u32 mVertexCount;
    u32 mTriangleCount;
    bool mBuffered;
    bool mHasOwnMaterials;
    bool mHasOwnSurfaces;

    CVertexBuffer mVBO;
    std::vector<SSurface*> mSurfaces;

public:
    CBasicModel();
    ~CBasicModel();

    u32 GetVertexCount();
    u32 GetTriangleCount();
    CAABox AABox();
    bool IsBuffered();
    u32 GetSurfaceCount();
    CAABox GetSurfaceAABox(u32 Surface);
    SSurface* GetSurface(u32 Surface);
    virtual void ClearGLBuffer() = 0;
};

#endif // CBASICMODEL_H
