#ifndef CSTATICMODEL_H
#define CSTATICMODEL_H

#include "CBasicModel.h"
#include <Core/ERenderOptions.h>
#include <OpenGL/CIndexBuffer.h>

// A CStaticModel is meant for meshes that don't move. It's built specifically with terrain in mind.
// It only links to one material, and what it does best is combining submeshes from different models
// into shared VBOs and IBOs. This allows for a significantly reduced number of draw calls.

class CStaticModel : public CBasicModel
{
    CMaterial *mpMaterial;
    std::vector<CIndexBuffer> mIBOs;
    std::vector<std::vector<u32>> mSubmeshEndOffsets;
    bool mTransparent;

public:
    CStaticModel();
    CStaticModel(CMaterial *pMat);
    ~CStaticModel();
    void AddSurface(SSurface *pSurface);

    void BufferGL();
    void ClearGLBuffer();
    void Draw(ERenderOptions Options);
    void DrawSurface(ERenderOptions Options, u32 Surface);
    void DrawWireframe(ERenderOptions Options, CColor WireColor = CColor::skWhite);

    CMaterial* GetMaterial();
    void SetMaterial(CMaterial *pMat);
    bool IsTransparent();
    bool IsOccluder();

private:
    CIndexBuffer* InternalGetIBO(EGXPrimitiveType Primitive);
};

#endif // CSTATICMODEL_H
