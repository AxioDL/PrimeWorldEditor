#include "CMaterial.h"
#include "Core/GameProject/CResourceStore.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/Render/CRenderer.h"
#include "Core/OpenGL/GLCommon.h"
#include "Core/OpenGL/CShaderGenerator.h"
#include <Common/Hash/CFNV1A.h>

#include <iostream>
#include <GL/glew.h>

u64 CMaterial::sCurrentMaterial = 0;
CColor CMaterial::sCurrentTint = CColor::skWhite;
std::map<u64, CMaterial::SMaterialShader> CMaterial::smShaderMap;

CMaterial::CMaterial()
    : mpShader(nullptr)
    , mShaderStatus(eNoShader)
    , mRecalcHash(true)
    , mEnableBloom(false)
    , mVersion(EGame::Invalid)
    , mOptions(eNoSettings)
    , mVtxDesc(eNoAttributes)
    , mBlendSrcFac(GL_ONE)
    , mBlendDstFac(GL_ZERO)
    , mLightingEnabled(true)
    , mEchoesUnknownA(0)
    , mEchoesUnknownB(0)
    , mpIndirectTexture(nullptr)
{
}

CMaterial::CMaterial(EGame Version, FVertexDescription VtxDesc)
    : mpShader(nullptr)
    , mShaderStatus(eNoShader)
    , mRecalcHash(true)
    , mEnableBloom(Version == EGame::Corruption)
    , mVersion(Version)
    , mOptions(eDepthWrite)
    , mVtxDesc(VtxDesc)
    , mBlendSrcFac(GL_ONE)
    , mBlendDstFac(GL_ZERO)
    , mLightingEnabled(true)
    , mEchoesUnknownA(0)
    , mEchoesUnknownB(0)
    , mpIndirectTexture(nullptr)
{
    mpShader = nullptr;
    mShaderStatus = eNoShader;
    mRecalcHash = true;
    mEnableBloom = (Version == EGame::Corruption);
    mVersion = Version;
    mOptions = eDepthWrite;
    mVtxDesc = VtxDesc;
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

    ClearShader();
}

CMaterial* CMaterial::Clone()
{
    CMaterial *pOut = new CMaterial();
    pOut->mName = mName;
    pOut->mEnableBloom = mEnableBloom;
    pOut->mVersion = mVersion;
    pOut->mOptions = mOptions;
    pOut->mVtxDesc = mVtxDesc;
    for (u32 iKonst = 0; iKonst < 4; iKonst++)
        pOut->mKonstColors[iKonst] = mKonstColors[iKonst];
    pOut->mBlendSrcFac = mBlendSrcFac;
    pOut->mBlendDstFac = mBlendDstFac;
    pOut->mLightingEnabled = mLightingEnabled;
    pOut->mEchoesUnknownA = mEchoesUnknownA;
    pOut->mEchoesUnknownB = mEchoesUnknownB;
    pOut->mpIndirectTexture = mpIndirectTexture;

    pOut->mPasses.resize(mPasses.size());
    for (u32 iPass = 0; iPass < mPasses.size(); iPass++)
        pOut->mPasses[iPass] = mPasses[iPass]->Clone(pOut);

    return pOut;
}

void CMaterial::GenerateShader(bool AllowRegen /*= true*/)
{
    HashParameters(); // Calling HashParameters() may change mShaderStatus so call it before checking

    if (mShaderStatus != eShaderExists || AllowRegen)
    {
        auto Find = smShaderMap.find(mParametersHash);

        if (Find != smShaderMap.end())
        {
            SMaterialShader& rShader = Find->second;

            if (rShader.pShader == mpShader)
                return;

            ClearShader();
            mpShader = rShader.pShader;
            rShader.NumReferences++;
        }

        else
        {
            ClearShader();
            mpShader = CShaderGenerator::GenerateShader(*this);

            if (!mpShader->IsValidProgram())
            {
                mShaderStatus = eShaderFailed;
                delete mpShader;
                mpShader = nullptr;
            }

            else
            {
                mShaderStatus = eShaderExists;
                smShaderMap[mParametersHash] = SMaterialShader { 1, mpShader };
            }
        }
    }
}

void CMaterial::ClearShader()
{
    if (mpShader)
    {
        auto Find = smShaderMap.find(mParametersHash);
        ASSERT(Find != smShaderMap.end());

        SMaterialShader& rShader = Find->second;
        ASSERT(rShader.pShader == mpShader);

        rShader.NumReferences--;

        if (rShader.NumReferences == 0)
        {
            delete mpShader;
            smShaderMap.erase(Find);
        }

        mpShader = nullptr;
        mShaderStatus = eNoShader;
    }
}

bool CMaterial::SetCurrent(FRenderOptions Options)
{
    // Skip material setup if the currently bound material is identical
    if (sCurrentMaterial != HashParameters())
    {
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
            CGraphics::sPixelBlock.Konst[iKonst] = mKonstColors[iKonst];

        // Set color channels
        // COLOR0_Amb is initialized by the node instead of by the material
        CGraphics::sVertexBlock.COLOR0_Mat = CColor::skWhite;

        // Set depth write - force on if alpha is disabled (lots of weird depth issues otherwise)
        if ((mOptions & eDepthWrite) || (Options & eNoAlpha)) glDepthMask(GL_TRUE);
        else glDepthMask(GL_FALSE);

        // Load uniforms
        for (u32 iPass = 0; iPass < mPasses.size(); iPass++)
            mPasses[iPass]->SetAnimCurrent(Options, iPass);

        sCurrentMaterial = HashParameters();
    }

    // If the passes are otherwise the same, update UV anims that use the model matrix
    else
    {
        for (u32 iPass = 0; iPass < mPasses.size(); iPass++)
        {
            EUVAnimMode mode = mPasses[iPass]->AnimMode();

            if ((mode == eInverseMV) || (mode == eInverseMVTranslated) ||
                (mode == eModelMatrix) || (mode == eSimpleMode))
                mPasses[iPass]->SetAnimCurrent(Options, iPass);
        }
    }

    // Set up shader uniforms
    for (u32 iPass = 0; iPass < mPasses.size(); iPass++)
        mPasses[iPass]->LoadTexture(iPass);

    CShader *pShader = CShader::CurrentShader();
    pShader->SetTextureUniforms(mPasses.size());
    pShader->SetNumLights(CGraphics::sNumLights);

    // Update shader blocks
    CGraphics::UpdateVertexBlock();
    CGraphics::UpdatePixelBlock();

    return true;
}

u64 CMaterial::HashParameters()
{
    if (mRecalcHash)
    {
        CFNV1A Hash(CFNV1A::e64Bit);

        Hash.HashLong((int) mVersion);
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

        u64 NewHash = Hash.GetHash64();

        if (mParametersHash != NewHash)
            ClearShader();

        mParametersHash = NewHash;
        mRecalcHash = false;
    }

    return mParametersHash;
}

void CMaterial::Update()
{
    mRecalcHash = true;
    mShaderStatus = eNoShader;
}

void CMaterial::SetNumPasses(u32 NumPasses)
{
    if (NumPasses < mPasses.size())
    {
        for (u32 iPass = NumPasses; iPass < mPasses.size(); iPass++)
            delete mPasses[iPass];
    }

    u32 OldCount = mPasses.size();
    mPasses.resize(NumPasses);

    if (NumPasses > OldCount)
    {
        for (u32 iPass = OldCount; iPass < NumPasses; iPass++)
            mPasses[iPass] = new CMaterialPass(this);
    }

    mRecalcHash = true;
}
