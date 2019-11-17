#include "CMaterial.h"
#include "Core/GameProject/CResourceStore.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/Render/CRenderer.h"
#include "Core/OpenGL/GLCommon.h"
#include "Core/OpenGL/CShaderGenerator.h"
#include <Common/Hash/CFNV1A.h>

#include <iostream>
#include <GL/glew.h>

uint64 CMaterial::sCurrentMaterial = 0;
CColor CMaterial::sCurrentTint = CColor::skWhite;
std::map<uint64, CMaterial::SMaterialShader> CMaterial::smShaderMap;

CMaterial::CMaterial()
    : mpShader(nullptr)
    , mShaderStatus(EShaderStatus::NoShader)
    , mRecalcHash(true)
    , mVersion(EGame::Invalid)
    , mOptions(EMaterialOption::None)
    , mVtxDesc(EVertexAttribute::None)
    , mBlendSrcFac(GL_ONE)
    , mBlendDstFac(GL_ZERO)
    , mLightingEnabled(true)
    , mEchoesUnknownA(0)
    , mEchoesUnknownB(0)
    , mpIndirectTexture(nullptr)
    , mpNextDrawPassMaterial(nullptr)
    , mpBloomMaterial(nullptr)
{}

CMaterial::CMaterial(EGame Version, FVertexDescription VtxDesc)
    : mpShader(nullptr)
    , mShaderStatus(EShaderStatus::NoShader)
    , mRecalcHash(true)
    , mVersion(Version)
    , mOptions(EMaterialOption::DepthWrite | EMaterialOption::ColorWrite)
    , mVtxDesc(VtxDesc)
    , mBlendSrcFac(GL_ONE)
    , mBlendDstFac(GL_ZERO)
    , mLightingEnabled(true)
    , mEchoesUnknownA(0)
    , mEchoesUnknownB(0)
    , mpIndirectTexture(nullptr)
    , mpNextDrawPassMaterial(nullptr)
    , mpBloomMaterial(nullptr)
{}

CMaterial::~CMaterial()
{
    ClearShader();
}

std::unique_ptr<CMaterial> CMaterial::Clone()
{
    std::unique_ptr<CMaterial> pOut = std::make_unique<CMaterial>();
    pOut->mName = mName;
    pOut->mVersion = mVersion;
    pOut->mOptions = mOptions;
    pOut->mVtxDesc = mVtxDesc;
    for (uint32 iKonst = 0; iKonst < 4; iKonst++)
        pOut->mKonstColors[iKonst] = mKonstColors[iKonst];
    for (uint32 iTev = 0; iTev < 4; iTev++)
        pOut->mTevColors[iTev] = mTevColors[iTev];
    pOut->mBlendSrcFac = mBlendSrcFac;
    pOut->mBlendDstFac = mBlendDstFac;
    pOut->mLightingEnabled = mLightingEnabled;
    pOut->mEchoesUnknownA = mEchoesUnknownA;
    pOut->mEchoesUnknownB = mEchoesUnknownB;
    pOut->mpIndirectTexture = mpIndirectTexture;

    pOut->mPasses.resize(mPasses.size());
    for (uint32 iPass = 0; iPass < mPasses.size(); iPass++)
        pOut->mPasses[iPass] = mPasses[iPass]->Clone(pOut.get());

    if (mpNextDrawPassMaterial)
        pOut->mpNextDrawPassMaterial = mpNextDrawPassMaterial->Clone();

    if (mpBloomMaterial)
        pOut->mpBloomMaterial = mpBloomMaterial->Clone();

    return pOut;
}

void CMaterial::GenerateShader(bool AllowRegen /*= true*/)
{
    HashParameters(); // Calling HashParameters() may change mShaderStatus so call it before checking

    if (mShaderStatus != EShaderStatus::ShaderExists || AllowRegen)
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
                mShaderStatus = EShaderStatus::ShaderFailed;
                delete mpShader;
                mpShader = nullptr;
            }

            else
            {
                mShaderStatus = EShaderStatus::ShaderExists;
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
        mShaderStatus = EShaderStatus::NoShader;
    }
}

bool CMaterial::SetCurrent(FRenderOptions Options)
{
    // Skip material setup if the currently bound material is identical
    if (sCurrentMaterial != HashParameters())
    {
        // Shader setup
        if (mShaderStatus == EShaderStatus::NoShader) GenerateShader();
        mpShader->SetCurrent();

        if (mShaderStatus == EShaderStatus::ShaderFailed)
            return false;

        // Set RGB blend equation - force to ZERO/ONE if alpha is disabled
        GLenum srcRGB, dstRGB, srcAlpha, dstAlpha;

        if (Options & ERenderOption::NoAlpha) {
            srcRGB = GL_ONE;
            dstRGB = GL_ZERO;
        } else {
            srcRGB = mBlendSrcFac;
            dstRGB = mBlendDstFac;
        }

        if (mOptions & EMaterialOption::ZeroDestAlpha) {
            srcAlpha = GL_ZERO;
            dstAlpha = GL_ZERO;
        } else {
            srcAlpha = mBlendSrcFac;
            dstAlpha = mBlendDstFac;
        }

        glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);

        // Set konst inputs
        for (uint32 iKonst = 0; iKonst < 4; iKonst++)
            CGraphics::sPixelBlock.Konst[iKonst] = mKonstColors[iKonst];

        // Set TEV registers
        if (mVersion >= EGame::Corruption)
            for (uint32 iTev = 0; iTev < 4; iTev++)
                CGraphics::sPixelBlock.TevColor[iTev] = mTevColors[iTev];

        // Set color channels
        // COLOR0_Amb is initialized by the node instead of by the material
        CGraphics::sVertexBlock.COLOR0_Mat = CColor::skWhite;

        // Set depth write - force on if alpha is disabled (lots of weird depth issues otherwise)
        if ((mOptions & EMaterialOption::DepthWrite) || (Options & ERenderOption::NoAlpha)) glDepthMask(GL_TRUE);
        else glDepthMask(GL_FALSE);

        // Set color/alpha write
        GLboolean bColorWrite = mOptions.HasFlag(EMaterialOption::ColorWrite);
        GLboolean bAlphaWrite = mOptions.HasFlag(EMaterialOption::AlphaWrite);
        glColorMask(bColorWrite, bColorWrite, bColorWrite, bAlphaWrite);

        // Load uniforms
        for (uint32 iPass = 0; iPass < mPasses.size(); iPass++)
            mPasses[iPass]->SetAnimCurrent(Options, iPass);

        sCurrentMaterial = HashParameters();
    }

    // If the passes are otherwise the same, update UV anims that use the model matrix
    else
    {
        for (uint32 iPass = 0; iPass < mPasses.size(); iPass++)
        {
            EUVAnimMode mode = mPasses[iPass]->AnimMode();

            if ((mode == EUVAnimMode::InverseMV) || (mode == EUVAnimMode::InverseMVTranslated) ||
                (mode == EUVAnimMode::ModelMatrix) || (mode == EUVAnimMode::SimpleMode))
                mPasses[iPass]->SetAnimCurrent(Options, iPass);
        }
    }

    // Set up shader uniforms
    for (uint32 iPass = 0; iPass < mPasses.size(); iPass++)
        mPasses[iPass]->LoadTexture(iPass);

    CShader *pShader = CShader::CurrentShader();
    pShader->SetTextureUniforms(mPasses.size());
    pShader->SetNumLights(CGraphics::sNumLights);

    // Update shader blocks
    CGraphics::UpdateVertexBlock();
    CGraphics::UpdatePixelBlock();

    return true;
}

uint64 CMaterial::HashParameters()
{
    if (mRecalcHash)
    {
        CFNV1A Hash(CFNV1A::k64Bit);

        Hash.HashLong((int) mVersion);
        Hash.HashLong(mOptions);
        Hash.HashLong(mVtxDesc);
        Hash.HashData(mKonstColors, sizeof(CColor) * 4);
        Hash.HashData(mTevColors, sizeof(CColor) * 4);
        Hash.HashLong(mBlendSrcFac);
        Hash.HashLong(mBlendDstFac);
        Hash.HashByte(mLightingEnabled);
        Hash.HashLong(mEchoesUnknownA);
        Hash.HashLong(mEchoesUnknownB);

        for (uint32 iPass = 0; iPass < mPasses.size(); iPass++)
            mPasses[iPass]->HashParameters(Hash);

        uint64 NewHash = Hash.GetHash64();

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
    mShaderStatus = EShaderStatus::NoShader;
}

void CMaterial::SetNumPasses(uint32 NumPasses)
{
    uint32 OldCount = mPasses.size();
    mPasses.resize(NumPasses);

    if (NumPasses > OldCount)
    {
        for (uint32 iPass = OldCount; iPass < NumPasses; iPass++)
            mPasses[iPass] = std::make_unique<CMaterialPass>(this);
    }

    mRecalcHash = true;
}
