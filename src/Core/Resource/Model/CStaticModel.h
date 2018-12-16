#ifndef CSTATICMODEL_H
#define CSTATICMODEL_H

#include "CBasicModel.h"
#include "Core/Render/FRenderOptions.h"
#include "Core/OpenGL/CIndexBuffer.h"

/* A CStaticModel is meant for meshes that don't move. It only links to one material,
 * and is used to combine surfaces from different world models into shared VBOs and
 * IBOs. This allows for a significantly reduced number of draw calls. */
class CStaticModel : public CBasicModel
{
    CMaterial *mpMaterial;
    std::vector<CIndexBuffer> mIBOs;
    std::vector<std::vector<uint32>> mSurfaceEndOffsets;
    bool mTransparent;

public:
    CStaticModel();
    CStaticModel(CMaterial *pMat);
    ~CStaticModel();
    void AddSurface(SSurface *pSurface);

    void BufferGL();
    void GenerateMaterialShaders();
    void ClearGLBuffer();
    void Draw(FRenderOptions Options);
    void DrawSurface(FRenderOptions Options, uint32 Surface);
    void DrawWireframe(FRenderOptions Options, CColor WireColor = CColor::skWhite);

    CMaterial* GetMaterial();
    void SetMaterial(CMaterial *pMat);
    bool IsTransparent();
    bool IsOccluder();

private:
    CIndexBuffer* InternalGetIBO(EPrimitiveType Primitive);
};

#endif // CSTATICMODEL_H
