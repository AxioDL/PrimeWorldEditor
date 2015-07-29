#include "CMaterial.h"
#include <Common/CHashFNV1A.h>
#include <Core/CDrawUtil.h>
#include <Core/CRenderer.h>
#include <Core/CResCache.h>
#include <OpenGL/GLCommon.h>
#include <OpenGL/CShaderGenerator.h>

#include <iostream>
#include <gl/glew.h>

u64 CMaterial::sCurrentMaterial = 0;
CColor CMaterial::sCurrentTint = CColor::skWhite;

CMaterial::CMaterial()
{
    mpShader = nullptr;
    mShaderStatus = eNoShader;
    mRecalcHash = true;
    mEnableBloom = false;
    mVersion = eUnknownVersion;
    mOptions = eNoSettings;
    mVtxDesc = eNoAttributes;
    mBlendSrcFac = GL_ONE;
    mBlendDstFac = GL_ZERO;
    mLightingEnabled = true;
    mEchoesUnknownA = 0;
    mEchoesUnknownB = 0;
    mpIndirectTexture = nullptr;
}

CMaterial::~CMaterial()
{
    for (u32 iPass = 0; iPass < mPasses.size(); iPass++)
        delete mPasses[iPass];

    delete mpShader;
}

void CMaterial::GenerateShader()
{
    delete mpShader;
    mpShader = CShaderGenerator::GenerateShader(*this);

    if (!mpShader->IsValidProgram()) mShaderStatus = eShaderFailed;
    else mShaderStatus = eShaderExists;
}

bool CMaterial::SetCurrent(ERenderOptions Options)
{
    // Bind textures
    for (u32 iPass = 0; iPass < mPasses.size(); iPass++)
        mPasses[iPass]->LoadTexture(iPass);

    // Skip material setup if the currently bound material is identical
    if (sCurrentMaterial == HashParameters())
    {
        GLuint NumLightsLoc = CShader::CurrentShader()->GetUniformLocation("NumLights");
        glUniform1i(NumLightsLoc, CGraphics::sNumLights);
        return true;
    }

    // Shader setup
    if (mShaderStatus == eNoShader) GenerateShader();
    mpShader->SetCurrent();

    if (mShaderStatus == eShaderFailed)
        return false;

    // Set RGB blend equation - force to ZERO/ONE if alpha is disabled
    GLenum srcRGB, dstRGB, srcAlpha, dstAlpha;

    if (Options & eNoAlpha) {
        srcRGB = GL_ONE;
        dstRGB = GL_ZERO;
    } else {
        srcRGB = mBlendSrcFac;
        dstRGB = mBlendDstFac;
    }

    // Set alpha blend equation
    bool AlphaBlended = ((mBlendSrcFac == GL_SRC_ALPHA) && (mBlendDstFac == GL_ONE_MINUS_SRC_ALPHA));

    if ((mEnableBloom) && (Options & eEnableBloom) && (!AlphaBlended)) {
        srcAlpha = mBlendSrcFac;
        dstAlpha = mBlendDstFac;
    } else {
        srcAlpha = GL_ZERO;
        dstAlpha = GL_ZERO;
    }

    glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);

    // Set konst inputs
    for (u32 iKonst = 0; iKonst < 4; iKonst++)
        CGraphics::sPixelBlock.Konst[iKonst] = mKonstColors[iKonst].ToVector4f();

    // Set color channels
    // COLOR0_Amb is initialized by the node instead of by the material
    CGraphics::sVertexBlock.COLOR0_Mat = CColor::skWhite.ToVector4f();

    // Set depth write - force on if alpha is disabled (lots of weird depth issues otherwise)
    if ((mOptions & eDepthWrite) || (Options & eNoAlpha)) glDepthMask(GL_TRUE);
    else glDepthMask(GL_FALSE);

    // Load uniforms
    for (u32 iPass = 0; iPass < mPasses.size(); iPass++)
        mPasses[iPass]->SetAnimCurrent(Options, iPass);

    GLuint NumLightsLoc = mpShader->GetUniformLocation("NumLights");
    glUniform1i(NumLightsLoc, CGraphics::sNumLights);

    CGraphics::UpdateVertexBlock();
    CGraphics::UpdatePixelBlock();
    sCurrentMaterial = HashParameters();
    return true;
}

u64 CMaterial::HashParameters()
{
    if (mRecalcHash)
    {
        CHashFNV1A Hash;
        Hash.Init64();

        Hash.HashLong(mVersion);
        Hash.HashLong(mOptions);
        Hash.HashLong(mVtxDesc);
        Hash.HashData(mKonstColors, sizeof(CColor) * 4);
        Hash.HashLong(mBlendSrcFac);
        Hash.HashLong(mBlendDstFac);
        Hash.HashByte(mLightingEnabled);
        Hash.HashLong(mEchoesUnknownA);
        Hash.HashLong(mEchoesUnknownB);

        for (u32 iPass = 0; iPass < mPasses.size(); iPass++)
            mPasses[iPass]->HashParameters(Hash);

        mParametersHash = Hash.GetHash64();
        mRecalcHash = false;
    }

    return mParametersHash;
}

void CMaterial::Update()
{
    mRecalcHash = true;
    mShaderStatus = eNoShader;
}

// ************ GETTERS ************
EGame CMaterial::Version() const
{
    return mVersion;
}

CMaterial::EMaterialOptions CMaterial::Options() const
{
    return mOptions;
}

EVertexDescription CMaterial::VtxDesc() const
{
    return mVtxDesc;
}

GLenum CMaterial::BlendSrcFac() const {
    return mBlendSrcFac;
}

GLenum CMaterial::BlendDstFac() const {
    return mBlendDstFac;
}

CColor CMaterial::Konst(u32 KIndex) const
{
    if (KIndex > 3) return CColor::skTransparentBlack;
    else return mKonstColors[KIndex];
}

CTexture* CMaterial::IndTexture() const
{
    return mpIndirectTexture;
}

bool CMaterial::IsLightingEnabled() const
{
    return mLightingEnabled;
}

u32 CMaterial::EchoesUnknownA() const
{
    return mEchoesUnknownA;
}

u32 CMaterial::EchoesUnknownB() const
{
    return mEchoesUnknownB;
}

u32 CMaterial::PassCount() const
{
    return mPasses.size();
}

CMaterialPass* CMaterial::Pass(u32 PassIndex) const
{
    return mPasses[PassIndex];
}


// ************ SETTERS ************
void CMaterial::SetOptions(EMaterialOptions Options)
{
    mOptions = Options;
    mRecalcHash = true;
}

void CMaterial::SetBlendMode(GLenum SrcFac, GLenum DstFac)
{
    mBlendSrcFac = SrcFac;
    mBlendDstFac = DstFac;
    mRecalcHash = true;
}

void CMaterial::SetKonst(CColor& Konst, u32 KIndex)
{
    mKonstColors[KIndex] = Konst;
    mRecalcHash = true;
}

void CMaterial::SetIndTexture(CTexture *pTex)
{
    mpIndirectTexture = pTex;
}

void CMaterial::SetLightingEnabled(bool Enabled)
{
    mLightingEnabled = Enabled;
    mRecalcHash = true;
}

// ************ STATIC ************
void CMaterial::KillCachedMaterial()
{
    sCurrentMaterial = 0;
    CShader::KillCachedShader();
}
