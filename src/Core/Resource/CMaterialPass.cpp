#include "CMaterialPass.h"
#include "CMaterial.h"
#include "Core/Render/CGraphics.h"
#include <Common/CTimer.h>

CMaterialPass::CMaterialPass(CMaterial *pParent)
    : mpParentMat(pParent)
{
}

CMaterialPass::~CMaterialPass() = default;

std::unique_ptr<CMaterialPass> CMaterialPass::Clone(CMaterial* pParent) const
{
    auto pOut = std::make_unique<CMaterialPass>(pParent);

    pOut->mPassType = mPassType;
    pOut->mSettings = mSettings;
    pOut->mColorInputs = mColorInputs;
    pOut->mAlphaInputs = mAlphaInputs;
    pOut->mColorOutput = mColorOutput;
    pOut->mAlphaOutput = mAlphaOutput;
    pOut->mKColorSel = mKColorSel;
    pOut->mKAlphaSel = mKAlphaSel;
    pOut->mRasSel = mRasSel;
    pOut->mTevColorScale = mTevColorScale;
    pOut->mTevAlphaScale = mTevAlphaScale;
    pOut->mTexCoordSource = mTexCoordSource;
    pOut->mpTexture = mpTexture;
    pOut->mAnimMode = mAnimMode;
    pOut->mAnimParams = mAnimParams;
    pOut->mTexSwapComps = mTexSwapComps;
    pOut->mEnabled = mEnabled;

    return pOut;
}

void CMaterialPass::HashParameters(CFNV1A& rHash)
{
    if (!mEnabled)
        return;

    rHash.HashLong(mPassType.ToLong());
    rHash.HashLong(mSettings);
    rHash.HashData(mColorInputs.data(), sizeof(mColorInputs));
    rHash.HashData(mAlphaInputs.data(), sizeof(mAlphaInputs));
    rHash.HashLong(mColorOutput);
    rHash.HashLong(mAlphaOutput);
    rHash.HashLong(mKColorSel);
    rHash.HashLong(mKAlphaSel);
    rHash.HashLong(mRasSel);
    rHash.HashFloat(mTevColorScale);
    rHash.HashFloat(mTevAlphaScale);
    rHash.HashLong(mTexCoordSource);
    rHash.HashLong(static_cast<uint>(mAnimMode));
    rHash.HashData(mAnimParams.data(), sizeof(mAnimParams));
    rHash.HashData(mTexSwapComps.data(), sizeof(mTexSwapComps));
    rHash.HashByte(mEnabled);
}

void CMaterialPass::LoadTexture(uint32 PassIndex)
{
    if (mpTexture)
        mpTexture->Bind(PassIndex);
}

void CMaterialPass::SetAnimCurrent(FRenderOptions Options, size_t PassIndex)
{
    if (mAnimMode == EUVAnimMode::NoUVAnim) return;

    float Seconds = CTimer::SecondsMod900();
    const CMatrix4f& ModelMtx = CGraphics::sMVPBlock.ModelMatrix;
    const CMatrix4f& ViewMtx = CGraphics::sMVPBlock.ViewMatrix;

    CTransform4f TexMtx = CMatrix4f::skIdentity;
    CTransform4f PostMtx = CMatrix4f::skIdentity;

    switch (mAnimMode)
    {

    case EUVAnimMode::InverseMV: // Mode 0
    case EUVAnimMode::SimpleMode: // Mode 10 - maybe not correct?
    {
        TexMtx = (ViewMtx.Inverse().Transpose() * ModelMtx);
        TexMtx[0][3] = TexMtx[1][3] = TexMtx[2][3] = 0.f;
        PostMtx = CMatrix4f(0.5f, 0.0f, 0.0f, 0.5f,
                            0.0f, 0.5f, 0.0f, 0.5f,
                            0.0f, 0.0f, 0.0f, 1.0f,
                            0.0f, 0.0f, 0.0f, 1.0f);
        break;
    }

    case EUVAnimMode::InverseMVTranslated: // Mode 1
    {
        TexMtx = (ViewMtx.Inverse().Transpose() * ModelMtx);
        PostMtx = CMatrix4f(0.5f, 0.0f, 0.0f, 0.5f,
                            0.0f, 0.5f, 0.0f, 0.5f,
                            0.0f, 0.0f, 0.0f, 1.0f,
                            0.0f, 0.0f, 0.0f, 1.0f);
        break;
    }

    case EUVAnimMode::UVScroll: // Mode 2
    {
        if (Options & ERenderOption::EnableUVScroll)
        {
            TexMtx[0][3] = (Seconds * mAnimParams[2]) + mAnimParams[0];
            TexMtx[1][3] = (Seconds * mAnimParams[3]) + mAnimParams[1];
        }
        break;
    }

    case EUVAnimMode::UVRotation: // Mode 3
    {
        if (Options & ERenderOption::EnableUVScroll)
        {
            float Angle = (Seconds * mAnimParams[1]) + mAnimParams[0];

            float ACos = cos(Angle);
            float ASin = sin(Angle);
            float TransX = (1.f - (ACos - ASin)) * 0.5f;
            float TransY = (1.f - (ASin + ACos)) * 0.5f;

            TexMtx = CMatrix4f(ACos, -ASin, 0.0f, TransX,
                               ASin,  ACos, 0.0f, TransY,
                               0.0f,  0.0f, 1.0f,   0.0f,
                               0.0f,  0.0f, 0.0f,   1.0f);
        }
        break;
    }

    case EUVAnimMode::HFilmstrip: // Mode 4
    case EUVAnimMode::VFilmstrip: // Mode 5
    {
        if (Options & ERenderOption::EnableUVScroll)
        {
            float Offset = mAnimParams[2] * mAnimParams[0] * (mAnimParams[3] + Seconds);
            Offset = (float)(short)(float)(mAnimParams[1] * fmod(Offset, 1.0f)) * mAnimParams[2];
            if (mAnimMode == EUVAnimMode::HFilmstrip) TexMtx[0][3] = Offset;
            if (mAnimMode == EUVAnimMode::VFilmstrip) TexMtx[1][3] = Offset;
        }
        break;
    }

    case EUVAnimMode::ModelMatrix: // Mode 6
    {
        // It looks ok, but I can't tell whether it's correct...
        TexMtx  = ModelMtx;
        PostMtx = CMatrix4f(0.5f, 0.0f, 0.0f, TexMtx[0][3] * 0.5f,
                            0.0f, 0.5f, 0.0f, TexMtx[1][3] * 0.5f,
                            0.0f, 0.0f, 0.0f, 1.0f,
                            0.0f, 0.0f, 0.0f, 1.0f);
        TexMtx[0][3] = 0.f;
        TexMtx[1][3] = 0.f;
        TexMtx[2][3] = 0.f;
        break;
    }

    case EUVAnimMode::ConvolutedModeA: // Mode 7
    {
        CMatrix4f View = CGraphics::sMVPBlock.ViewMatrix;

        TexMtx = (ViewMtx.Inverse().Transpose() * ModelMtx);
        TexMtx[0][3] = TexMtx[1][3] = TexMtx[2][3] = 0.f;

        float XY = (View[3][0] + View[3][1]) * 0.025f * mAnimParams[1];
        XY = (XY - (int) XY);

        float Z = View[3][2] * 0.05f * mAnimParams[1];
        Z = (Z - (int) Z);

        float HalfA = mAnimParams[0] * 0.5f;

        PostMtx = CMatrix4f(HalfA, 0.0f,  0.0f,   XY,
                             0.0f, 0.0f, HalfA,    Z,
                             0.0f, 0.0f,  0.0f, 1.0f,
                             0.0f, 0.0f,  0.0f, 1.0f);
        break;
    }

    case EUVAnimMode::ConvolutedModeB: // Mode 8 (MP3/DKCR only)
    {
        float t = mAnimParams[2] + (Seconds * mAnimParams[3]);
        float sin = std::sin(t);
        float cos = std::cos(t);
        /* TODO Properly implement the mode type */
        if (mAnimConvolutedModeBType == EUVConvolutedModeBType::Zero)
        {
        } else if (mAnimConvolutedModeBType == EUVConvolutedModeBType::One)
        {
        } else if (mAnimConvolutedModeBType == EUVConvolutedModeBType::Two)
        {

        } else if (mAnimConvolutedModeBType == EUVConvolutedModeBType::Three)
        {

        } else if (mAnimConvolutedModeBType == EUVConvolutedModeBType::Four)
        {

        }

        float X = (mAnimParams[0] * (mAnimParams[5] + (mAnimParams[6] * Seconds))) + (0.5f - (0.5f * (cos - sin)));
        float Y = (mAnimParams[1] * (mAnimParams[5] + (mAnimParams[7] * Seconds))) + (0.5f - (0.5f * (sin + cos)));
        TexMtx = CMatrix4f(mAnimParams[0] * cos, mAnimParams[1] * -sin, 0.f,
                           X,
                           mAnimParams[0] * sin, mAnimParams[1] * cos, 0.f,
                           Y,
                           0.f, 0.f, 0.f, 1.f,
                           0.f, 0.f, 0.f, 1.f);
        break;
    }

    default:
        break;
    }

    CGraphics::sVertexBlock.TexMatrices[PassIndex] = TexMtx;
    CGraphics::sVertexBlock.PostMatrices[PassIndex] = PostMtx;
}

// ************ SETTERS ************
void CMaterialPass::SetType(CFourCC Type)
{
    mPassType = Type;
    mpParentMat->Update();
}

void CMaterialPass::SetColorInputs(ETevColorInput InputA, ETevColorInput InputB, ETevColorInput InputC, ETevColorInput InputD)
{
    mColorInputs[0] = InputA;
    mColorInputs[1] = InputB;
    mColorInputs[2] = InputC;
    mColorInputs[3] = InputD;
    mpParentMat->Update();
}

void CMaterialPass::SetAlphaInputs(ETevAlphaInput InputA, ETevAlphaInput InputB, ETevAlphaInput InputC, ETevAlphaInput InputD)
{
    mAlphaInputs[0] = InputA;
    mAlphaInputs[1] = InputB;
    mAlphaInputs[2] = InputC;
    mAlphaInputs[3] = InputD;
    mpParentMat->Update();
}

void CMaterialPass::SetColorOutput(ETevOutput OutputReg)
{
    mColorOutput = OutputReg;
    mpParentMat->Update();
}

void CMaterialPass::SetAlphaOutput(ETevOutput OutputReg)
{
    mAlphaOutput = OutputReg;
    mpParentMat->Update();
}

void CMaterialPass::SetKColorSel(ETevKSel Sel)
{
    mKColorSel = Sel;
    mpParentMat->Update();
}

void CMaterialPass::SetKAlphaSel(ETevKSel Sel)
{
    // Konst RGB is invalid for alpha, so reset to One if that's the selection
    if ((Sel >= kKonst0_RGB) && (Sel <= kKonst3_RGB))
        Sel = kKonstOne;

    mKAlphaSel = Sel;
    mpParentMat->Update();
}

void CMaterialPass::SetRasSel(ETevRasSel Sel)
{
    mRasSel = Sel;
    mpParentMat->Update();
}

void CMaterialPass::SetTevColorScale(float Scale)
{
    mTevColorScale = Scale;
    mpParentMat->Update();
}

void CMaterialPass::SetTevAlphaScale(float Scale)
{
    mTevAlphaScale = Scale;
    mpParentMat->Update();
}

void CMaterialPass::SetTexCoordSource(uint32 Source)
{
    mTexCoordSource = Source;
    mpParentMat->Update();
}

void CMaterialPass::SetTexture(CTexture *pTex)
{
    mpTexture = pTex;
}

void CMaterialPass::SetAnimMode(EUVAnimMode Mode)
{
    mAnimMode = Mode;
    mpParentMat->Update();
}

void CMaterialPass::SetAnimParam(size_t ParamIndex, float Value)
{
    mAnimParams[ParamIndex] = Value;
}

void CMaterialPass::SetTexSwapComp(size_t Comp, char Value)
{
    mTexSwapComps[Comp] = Value;
}

void CMaterialPass::SetEnabled(bool Enabled)
{
    mEnabled = Enabled;
    mpParentMat->Update();
}

// ************ STATIC ************
TString CMaterialPass::PassTypeName(CFourCC Type)
{
    if (Type == "CUST") return "Custom";
    if (Type == "DIFF") return "Light";
    if (Type == "RIML") return "Rim Light";
    if (Type == "BLOL") return "Bloom Light";
    // BLOD
    if (Type == "CLR ") return "Diffuse";
    if (Type == "TRAN") return "Opacity";
    if (Type == "INCA") return "Emissive";
    if (Type == "RFLV") return "Specular";
    if (Type == "RFLD") return "Reflection";
    // LRLD
    // LURD
    if (Type == "BLOI") return "Bloom Diffuse";
    if (Type == "XRAY") return "X-Ray";
    // TOON
    return Type.ToString();
}
