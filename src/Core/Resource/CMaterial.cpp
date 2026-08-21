#include "Core/Resource/CMaterial.h"

#include "Core/NRangeUtils.h"
#include "Core/GameProject/CResourceStore.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/Render/CGraphics.h"
#include "Core/Render/CRenderer.h"
#include "Core/OpenGL/GLCommon.h"
#include "Core/OpenGL/CShaderGenerator.h"
#include <Common/Hash/CFNV1A.h>

#include <GL/glew.h>
#include <map>

namespace
{
// Reuse shaders between materials that have identical TEV setups
struct SMaterialShader
{
    int NumReferences{};
    CShader* pShader{};
};

std::map<uint64_t, SMaterialShader> smShaderMap;

// The hash for the currently bound material
uint64_t sCurrentMaterial = 0;
} // Anonymous namespace

CMaterial::CMaterial() = default;

CMaterial::CMaterial(EGame Version, FVertexDescription VtxDesc)
    : mVersion(Version)
    , mOptions(EMaterialOption::DepthWrite | EMaterialOption::ColorWrite)
    , mVtxDesc(VtxDesc)
{}

CMaterial::~CMaterial()
{
    ClearShader();
}

std::unique_ptr<CMaterial> CMaterial::Clone() const
{
    std::unique_ptr<CMaterial> pOut = std::make_unique<CMaterial>();
    pOut->mName = mName;
    pOut->mVersion = mVersion;
    pOut->mOptions = mOptions;
    pOut->mVtxDesc = mVtxDesc;
    pOut->mKonstColors = mKonstColors;
    pOut->mTevColors = mTevColors;
    pOut->mBlendSrcFac = mBlendSrcFac;
    pOut->mBlendDstFac = mBlendDstFac;
    pOut->mLightingEnabled = mLightingEnabled;
    pOut->mEchoesUnknownA = mEchoesUnknownA;
    pOut->mEchoesUnknownB = mEchoesUnknownB;
    pOut->mpIndirectTexture = mpIndirectTexture;

    pOut->mPasses.resize(mPasses.size());
    for (size_t iPass = 0; iPass < mPasses.size(); iPass++)
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
    if (mpShader == nullptr)
        return;

    const auto Find = smShaderMap.find(mParametersHash);
    ASSERT(Find != smShaderMap.cend());

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

bool CMaterial::SetCurrent(FRenderOptions Options)
{
    // Skip material setup if the currently bound material is identical
    if (sCurrentMaterial != HashParameters())
    {
        // Shader setup
        if (mShaderStatus == EShaderStatus::NoShader)
            GenerateShader();
        mpShader->SetCurrent();

        if (mShaderStatus == EShaderStatus::ShaderFailed)
            return false;

        // Set RGB blend equation - force to ZERO/ONE if alpha is disabled
        GLenum srcRGB, dstRGB, srcAlpha, dstAlpha;

        if (Options.HasFlag(ERenderOption::NoAlpha)) {
            srcRGB = GL_ONE;
            dstRGB = GL_ZERO;
        } else {
            srcRGB = mBlendSrcFac;
            dstRGB = mBlendDstFac;
        }

        if (mOptions.HasFlag(EMaterialOption::ZeroDestAlpha)) {
            srcAlpha = GL_ZERO;
            dstAlpha = GL_ZERO;
        } else {
            srcAlpha = mBlendSrcFac;
            dstAlpha = mBlendDstFac;
        }

        glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);

        // Set konst inputs
        CGraphics::sPixelBlock.Konst = mKonstColors;

        // Set TEV registers
        if (mVersion >= EGame::Corruption)
        {
            CGraphics::sPixelBlock.TevColor = mTevColors;
        }

        // Set color channels
        // COLOR0_Amb,Mat is initialized by the node instead of by the material

        // Set depth write - force on if alpha is disabled (lots of weird depth issues otherwise)
        if (mOptions.HasFlag(EMaterialOption::DepthWrite) || Options.HasFlag(ERenderOption::NoAlpha))
            glDepthMask(GL_TRUE);
        else
            glDepthMask(GL_FALSE);

        // Set color/alpha write
        const GLboolean bColorWrite = mOptions.HasFlag(EMaterialOption::ColorWrite);
        const GLboolean bAlphaWrite = mOptions.HasFlag(EMaterialOption::AlphaWrite);
        glColorMask(bColorWrite, bColorWrite, bColorWrite, bAlphaWrite);

        // Load uniforms
        for (const auto&& [idx, pass] : Utils::enumerate(mPasses))
            pass->SetAnimCurrent(Options, idx);

        sCurrentMaterial = HashParameters();
    }
    else // If the passes are otherwise the same, update UV anims that use the model matrix
    {
        for (const auto&& [idx, pass] : Utils::enumerate(mPasses))
        {
            const EUVAnimMode mode = pass->AnimMode();

            if (mode == EUVAnimMode::InverseMV || mode == EUVAnimMode::InverseMVTranslated ||
                mode == EUVAnimMode::ModelMatrix || mode == EUVAnimMode::SimpleMode)
            {
                pass->SetAnimCurrent(Options, idx);
            }
        }
    }

    // Set up shader uniforms
    for (const auto&& [idx, pass] : Utils::enumerate(mPasses))
        pass->LoadTexture(idx);

    CShader* pShader = CShader::CurrentShader();
    pShader->SetTextureUniforms(mPasses.size());
    pShader->SetNumLights(CGraphics::sNumLights);

    // Update shader blocks
    CGraphics::UpdateVertexBlock();
    CGraphics::UpdatePixelBlock();

    return true;
}

uint64_t CMaterial::HashParameters()
{
    if (mRecalcHash)
    {
        CFNV1A Hash(CFNV1A::EHashLength::k64Bit);

        Hash.HashData(mVersion);
        Hash.HashData(mOptions);
        Hash.HashData(mVtxDesc);
        Hash.HashData(mKonstColors.data(), sizeof(mKonstColors));
        Hash.HashData(mTevColors.data(), sizeof(mTevColors));
        Hash.HashData(mBlendSrcFac);
        Hash.HashData(mBlendDstFac);
        Hash.HashData(mLightingEnabled);
        Hash.HashData(mEchoesUnknownA);
        Hash.HashData(mEchoesUnknownB);

        for (auto& pass : mPasses)
            pass->HashParameters(Hash);

        const auto NewHash = Hash.GetHash64();

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

void CMaterial::SetNumPasses(size_t NumPasses)
{
    const size_t OldCount = mPasses.size();
    mPasses.resize(NumPasses);

    if (NumPasses > OldCount)
    {
        for (size_t i = OldCount; i < NumPasses; i++)
            mPasses[i] = std::make_unique<CMaterialPass>(this);
    }

    mRecalcHash = true;
}

void CMaterial::KillCachedMaterial()
{
    sCurrentMaterial = 0;
}
