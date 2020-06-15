#ifndef CBASICMODEL_H
#define CBASICMODEL_H

#include "SSurface.h"
#include "Core/Resource/CResource.h"
#include "Core/OpenGL/CVertexBuffer.h"
#include <Common/Math/CAABox.h>

class CBasicModel : public CResource
{
    DECLARE_RESOURCE_TYPE(Model)
protected:
    CAABox mAABox;
    uint32 mVertexCount = 0;
    uint32 mTriangleCount = 0;
    bool mBuffered = false;
    bool mHasOwnMaterials = false;
    bool mHasOwnSurfaces = false;

    CVertexBuffer mVBO;
    std::vector<SSurface*> mSurfaces;

public:
    explicit CBasicModel(CResourceEntry *pEntry = nullptr);
    ~CBasicModel();

    uint32 GetVertexCount() const;
    uint32 GetTriangleCount() const;
    CAABox AABox() const;
    bool IsBuffered() const;
    uint32 GetSurfaceCount() const;
    CAABox GetSurfaceAABox(uint32 Surface) const;
    SSurface* GetSurface(uint32 Surface);
    virtual void ClearGLBuffer() = 0;
};

#endif // CBASICMODEL_H
