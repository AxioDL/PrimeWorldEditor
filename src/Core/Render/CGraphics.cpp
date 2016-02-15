#include "CGraphics.h"
#include "Core/OpenGL/CShader.h"
#include "Core/Resource/CMaterial.h"
#include <Common/Log.h>

// ************ MEMBER INITIALIZATION ************
CUniformBuffer* CGraphics::mpMVPBlockBuffer;
CUniformBuffer* CGraphics::mpVertexBlockBuffer;
CUniformBuffer* CGraphics::mpPixelBlockBuffer;
CUniformBuffer* CGraphics::mpLightBlockBuffer;
u32 CGraphics::mContextIndices = 0;
u32 CGraphics::mActiveContext = -1;
bool CGraphics::mInitialized = false;
std::vector<CVertexArrayManager*> CGraphics::mVAMs;

CGraphics::SMVPBlock    CGraphics::sMVPBlock;
CGraphics::SVertexBlock CGraphics::sVertexBlock;
CGraphics::SPixelBlock  CGraphics::sPixelBlock;
CGraphics::SLightBlock  CGraphics::sLightBlock;

CGraphics::ELightingMode CGraphics::sLightMode;
u32 CGraphics::sNumLights;
const CColor CGraphics::skDefaultAmbientColor = CColor(0.5f, 0.5f, 0.5f, 1.f);
CColor CGraphics::sAreaAmbientColor = CColor::skBlack;
float CGraphics::sWorldLightMultiplier;
CLight CGraphics::sDefaultDirectionalLights[3] = {
    *CLight::BuildDirectional(CVector3f(0), CVector3f   (0.f, -0.866025f, -0.5f), CColor(0.3f, 0.3f, 0.3f, 1.f)),
    *CLight::BuildDirectional(CVector3f(0), CVector3f(-0.75f,  0.433013f, -0.5f), CColor(0.3f, 0.3f, 0.3f, 1.f)),
    *CLight::BuildDirectional(CVector3f(0), CVector3f( 0.75f,  0.433013f, -0.5f), CColor(0.3f, 0.3f, 0.3f, 1.f))
};

// ************ FUNCTIONS ************
void CGraphics::Initialize()
{
    if (!mInitialized)
    {
        Log::Write("Initializing GLEW");
        glewExperimental = true;
        glewInit();
        glGetError(); // This is to work around a glew bug - error is always set after initializing

        Log::Write("Creating uniform buffers");
        mpMVPBlockBuffer = new CUniformBuffer(sizeof(sMVPBlock));
        mpVertexBlockBuffer = new CUniformBuffer(sizeof(sVertexBlock));
        mpPixelBlockBuffer = new CUniformBuffer(sizeof(sPixelBlock));
        mpLightBlockBuffer = new CUniformBuffer(sizeof(sLightBlock));

        sLightMode = eWorldLighting;
        sNumLights = 0;
        sWorldLightMultiplier = 1.f;

        mInitialized = true;
    }
    mpMVPBlockBuffer->BindBase(0);
    mpVertexBlockBuffer->BindBase(1);
    mpPixelBlockBuffer->BindBase(2);
    mpLightBlockBuffer->BindBase(3);
}

void CGraphics::Shutdown()
{
    if (mInitialized)
    {
        Log::Write("Shutting down CGraphics");
        delete mpMVPBlockBuffer;
        delete mpVertexBlockBuffer;
        delete mpPixelBlockBuffer;
        delete mpLightBlockBuffer;
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

u32 CGraphics::GetContextIndex()
{
    for (u32 iCon = 0; iCon < 32; iCon++)
    {
        u32 Mask = (1 << iCon);
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

u32 CGraphics::GetActiveContext()
{
    return mActiveContext;
}

void CGraphics::ReleaseContext(u32 Index)
{
    if (Index < 32) mContextIndices &= ~(1 << Index);
    if (mActiveContext == Index) mActiveContext = -1;
    delete mVAMs[Index];
}

void CGraphics::SetActiveContext(u32 Index)
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
    UpdateLightBlock();

    sVertexBlock.COLOR0_Amb = CColor::skGray;
    UpdateVertexBlock();
}

void CGraphics::SetupAmbientColor()
{
    if (sLightMode == eWorldLighting)
        sVertexBlock.COLOR0_Amb = sAreaAmbientColor * sWorldLightMultiplier;
    else if (sLightMode == eBasicLighting)
        sVertexBlock.COLOR0_Amb = skDefaultAmbientColor;
    else
        sVertexBlock.COLOR0_Amb = CColor::skWhite;
}

void CGraphics::SetIdentityMVP()
{
    sMVPBlock.ModelMatrix = CMatrix4f::skIdentity;
    sMVPBlock.ViewMatrix = CMatrix4f::skIdentity;
    sMVPBlock.ProjectionMatrix = CMatrix4f::skIdentity;
}
