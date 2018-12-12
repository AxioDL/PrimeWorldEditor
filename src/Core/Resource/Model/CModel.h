#ifndef CMODEL_H
#define CMODEL_H

#include "CBasicModel.h"
#include "SSurface.h"
#include "Core/Resource/CMaterialSet.h"
#include "Core/Resource/Animation/CSkeleton.h"
#include "Core/Resource/Animation/CSkin.h"
#include "Core/OpenGL/CIndexBuffer.h"
#include "Core/OpenGL/GLCommon.h"
#include "Core/Render/FRenderOptions.h"

class CModel : public CBasicModel
{
    friend class CModelLoader;
    friend class CModelCooker;

    TResPtr<CSkin> mpSkin;
    std::vector<CMaterialSet*> mMaterialSets;
    std::vector<std::vector<CIndexBuffer>> mSurfaceIndexBuffers;
    bool mHasOwnMaterials;
    
public:
    CModel(CResourceEntry *pEntry = 0);
    CModel(CMaterialSet *pSet, bool OwnsMatSet);
    ~CModel();

    CDependencyTree* BuildDependencyTree() const;
    void BufferGL();
    void GenerateMaterialShaders();
    void ClearGLBuffer();
    void Draw(FRenderOptions Options, uint32 MatSet);
    void DrawSurface(FRenderOptions Options, uint32 Surface, uint32 MatSet);
    void DrawWireframe(FRenderOptions Options, CColor WireColor = CColor::skWhite);
    void SetSkin(CSkin *pSkin);

    uint32 GetMatSetCount();
    uint32 GetMatCount();
    CMaterialSet* GetMatSet(uint32 MatSet);
    CMaterial* GetMaterialByIndex(uint32 MatSet, uint32 Index);
    CMaterial* GetMaterialBySurface(uint32 MatSet, uint32 Surface);
    bool HasTransparency(uint32 MatSet);
    bool IsSurfaceTransparent(uint32 Surface, uint32 MatSet);
    bool IsLightmapped() const;

    inline bool IsSkinned() const       { return (mpSkin != nullptr); }

private:
    CIndexBuffer* InternalGetIBO(uint32 Surface, EGXPrimitiveType Primitive);
};

#endif // MODEL_H
