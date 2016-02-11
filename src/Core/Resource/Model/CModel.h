#ifndef CMODEL_H
#define CMODEL_H

#include "CBasicModel.h"
#include "SSurface.h"
#include "Core/Resource/CMaterialSet.h"
#include "Core/OpenGL/CIndexBuffer.h"
#include "Core/OpenGL/GLCommon.h"
#include "Core/Render/FRenderOptions.h"

class CModel : public CBasicModel
{
    friend class CModelLoader;
    friend class CModelCooker;

    std::vector<CMaterialSet*> mMaterialSets;
    std::vector<std::vector<CIndexBuffer>> mSubmeshIndexBuffers;
    bool mHasOwnMaterials;
    
public:
    CModel();
    CModel(CMaterialSet *pSet, bool ownsMatSet);
    ~CModel();

    void BufferGL();
    void GenerateMaterialShaders();
    void ClearGLBuffer();
    void Draw(FRenderOptions Options, u32 MatSet);
    void DrawSurface(FRenderOptions Options, u32 Surface, u32 MatSet);
    void DrawWireframe(FRenderOptions Options, CColor WireColor = CColor::skWhite);

    u32 GetMatSetCount();
    u32 GetMatCount();
    CMaterialSet* GetMatSet(u32 MatSet);
    CMaterial* GetMaterialByIndex(u32 MatSet, u32 Index);
    CMaterial* GetMaterialBySurface(u32 MatSet, u32 Surface);
    bool HasTransparency(u32 MatSet);
    bool IsSurfaceTransparent(u32 Surface, u32 MatSet);

private:
    CIndexBuffer* InternalGetIBO(u32 Surface, EGXPrimitiveType Primitive);
};

#endif // MODEL_H
