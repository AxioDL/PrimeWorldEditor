#include "CGraphics.h"
#include "Core/OpenGL/CShader.h"
#include "Core/Resource/CMaterial.h"
#include <Common/Log.h>

// ************ MEMBER INITIALIZATION ************
CUniformBuffer* CGraphics::mpMVPBlockBuffer;
CUniformBuffer* CGraphics::mpVertexBlockBuffer;
CUniformBuffer* CGraphics::mpPixelBlockBuffer;
CUniformBuffer* CGraphics::mpLightBlockBuffer;
CUniformBuffer* CGraphics::mpBoneTransformBuffer;
uint32 CGraphics::mContextIndices = 0;
uint32 CGraphics::mActiveContext = -1;
bool CGraphics::mInitialized = false;
std::vector<CVertexArrayManager*> CGraphics::mVAMs;
bool CGraphics::mIdentityBoneTransforms = false;

CGraphics::SMVPBlock    CGraphics::sMVPBlock;
CGraphics::SVertexBlock CGraphics::sVertexBlock;
CGraphics::SPixelBlock  CGraphics::sPixelBlock;
CGraphics::SLightBlock  CGraphics::sLightBlock;

CGraphics::ELightingMode CGraphics::sLightMode;
uint32 CGraphics::sNumLights;
CColor CGraphics::sAreaAmbientColor = CColor::TransparentBlack();
float CGraphics::sWorldLightMultiplier;
std::array<CLight, 3> CGraphics::sDefaultDirectionalLights{{
    CLight::BuildDirectional(CVector3f(0), CVector3f(0.f, -0.866025f, -0.5f), CColor(0.3f, 0.3f, 0.3f, 0.3f)),
    CLight::BuildDirectional(CVector3f(0), CVector3f(-0.75f, 0.433013f, -0.5f), CColor(0.3f, 0.3f, 0.3f, 0.3f)),
    CLight::BuildDirectional(CVector3f(0), CVector3f(0.75f, 0.433013f, -0.5f), CColor(0.3f, 0.3f, 0.3f, 0.3f)),
}};

// ************ FUNCTIONS ************
void CGraphics::Initialize()
{
    if (!mInitialized)
    {
        debugf("Initializing GLEW");
        glewExperimental = true;
        glewInit();
        glGetError(); // This is to work around a glew bug - error is always set after initializing

        debugf("Creating uniform buffers");
        mpMVPBlockBuffer = new CUniformBuffer(sizeof(sMVPBlock));
        mpVertexBlockBuffer = new CUniformBuffer(sizeof(sVertexBlock));
        mpPixelBlockBuffer = new CUniformBuffer(sizeof(sPixelBlock));
        mpLightBlockBuffer = new CUniformBuffer(sizeof(sLightBlock));
        mpBoneTransformBuffer = new CUniformBuffer(sizeof(CTransform4f) * 100);

        sLightMode = ELightingMode::World;
        sNumLights = 0;
        sWorldLightMultiplier = 1.f;

        mInitialized = true;
    }
    mpMVPBlockBuffer->BindBase(0);
    mpVertexBlockBuffer->BindBase(1);
    mpPixelBlockBuffer->BindBase(2);
    mpLightBlockBuffer->BindBase(3);
    mpBoneTransformBuffer->BindBase(4);
    LoadIdentityBoneTransforms();
}

void CGraphics::Shutdown()
{
    if (mInitialized)
    {
        debugf("Shutting down CGraphics");
        delete mpMVPBlockBuffer;
        delete mpVertexBlockBuffer;
        delete mpPixelBlockBuffer;
        delete mpLightBlockBuffer;
        delete mpBoneTransformBuffer;
        mInitialized = false;
    }
}

void CGraphics::UpdateMVPBlock()
{
    mpMVPBlockBuffer->Buffer(&sMVPBlock);
}

void CGraphics::UpdateVertexBlock()
{
    mpVertexBlockBuffer->Buffer(&sVertexBlock);
}

void CGraphics::UpdatePixelBlock()
{
    mpPixelBlockBuffer->Buffer(&sPixelBlock);
}

void CGraphics::UpdateLightBlock()
{
    mpLightBlockBuffer->Buffer(&sLightBlock);
}

GLuint CGraphics::MVPBlockBindingPoint()
{
    return 0;
}

GLuint CGraphics::VertexBlockBindingPoint()
{
    return 1;
}

GLuint CGraphics::PixelBlockBindingPoint()
{
    return 2;
}

GLuint CGraphics::LightBlockBindingPoint()
{
    return 3;
}

GLuint CGraphics::BoneTransformBlockBindingPoint()
{
    return 4;
}

uint32 CGraphics::GetContextIndex()
{
    for (uint32 iCon = 0; iCon < 32; iCon++)
    {
        uint32 Mask = (1 << iCon);
        if ((mContextIndices & Mask) == 0)
        {
            mContextIndices |= Mask;

            CVertexArrayManager *pVAM = new CVertexArrayManager;
            if (mVAMs.size() >= iCon) mVAMs.resize(iCon + 1);
            mVAMs[iCon] = pVAM;

            return iCon;
        }
    }

    return -1;
}

uint32 CGraphics::GetActiveContext()
{
    return mActiveContext;
}

void CGraphics::ReleaseContext(uint32 Index)
{
    if (Index < 32) mContextIndices &= ~(1 << Index);
    if (mActiveContext == Index) mActiveContext = -1;
    delete mVAMs[Index];
}

void CGraphics::SetActiveContext(uint32 Index)
{
    mActiveContext = Index;
    mVAMs[Index]->SetCurrent();
    CMaterial::KillCachedMaterial();
    CShader::KillCachedShader();
}

void CGraphics::SetDefaultLighting()
{
    sNumLights = 0; // CLight load function increments the light count by 1, which is why I set it to 0
    sDefaultDirectionalLights[0].Load();
    sDefaultDirectionalLights[1].Load();
    sDefaultDirectionalLights[2].Load();
    sNumLights = 0;
    UpdateLightBlock();

    sVertexBlock.COLOR0_Amb = CColor::Gray();
    sVertexBlock.COLOR0_Mat = CColor::White();
    UpdateVertexBlock();
}

void CGraphics::SetupAmbientColor()
{
    if (sLightMode == ELightingMode::World)
        sVertexBlock.COLOR0_Amb = sAreaAmbientColor * sWorldLightMultiplier;
    else if (sLightMode == ELightingMode::Basic)
        sVertexBlock.COLOR0_Amb = skDefaultAmbientColor;
    else
        sVertexBlock.COLOR0_Amb = CColor::TransparentWhite();
}

void CGraphics::SetIdentityMVP()
{
    sMVPBlock.ModelMatrix = CMatrix4f::skIdentity;
    sMVPBlock.ViewMatrix = CMatrix4f::skIdentity;
    sMVPBlock.ProjectionMatrix = CMatrix4f::skIdentity;
}

void CGraphics::LoadBoneTransforms(const CBoneTransformData& rkData)
{
    mpBoneTransformBuffer->BufferRange(rkData.Data(), 0, rkData.DataSize());
    mIdentityBoneTransforms = false;
}

void CGraphics::LoadIdentityBoneTransforms()
{
    static const CTransform4f skIdentityTransforms[100];

    if (!mIdentityBoneTransforms)
    {
        mpBoneTransformBuffer->Buffer(&skIdentityTransforms);
        mIdentityBoneTransforms = true;
    }
}
