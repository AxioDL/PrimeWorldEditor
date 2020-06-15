#ifndef CMATERIALPASS_H
#define CMATERIALPASS_H

#include "TResPtr.h"
#include "CTexture.h"
#include "ETevEnums.h"
#include "Core/Render/FRenderOptions.h"
#include <Common/CFourCC.h>
#include <Common/Hash/CFNV1A.h>
#include <array>

class CMaterial;

enum class EPassSettings
{
    None                    = 0x0,
    BloomContribution       = 0x4,
    ModulateIncandecenceMap = 0x8,
    InvertOpacityMap        = 0x10
};
DECLARE_FLAGS_ENUMCLASS(EPassSettings, FPassSettings)

class CMaterialPass
{
    friend class CMaterialLoader;
    friend class CMaterialCooker;

    CMaterial *mpParentMat = nullptr;
    CFourCC mPassType{"CUST"};
    FPassSettings mSettings{EPassSettings::None};

    std::array<ETevColorInput, 4> mColorInputs{kZeroRGB, kZeroRGB, kZeroRGB, kZeroRGB};
    std::array<ETevAlphaInput, 4> mAlphaInputs{kZeroAlpha, kZeroAlpha, kZeroAlpha, kZeroAlpha};
    ETevOutput mColorOutput{kPrevReg};
    ETevOutput mAlphaOutput{kPrevReg};
    ETevKSel mKColorSel{kKonstOne};
    ETevKSel mKAlphaSel{kKonstOne};
    ETevRasSel mRasSel{kRasColorNull};
    float mTevColorScale = 1.0f;
    float mTevAlphaScale = 1.0f;
    uint32 mTexCoordSource = 0xFF; // Should maybe be an enum but worried about conflicts with EVertexDescriptionn
    TResPtr<CTexture> mpTexture{nullptr};
    EUVAnimMode mAnimMode{EUVAnimMode::NoUVAnim};
    EUVConvolutedModeBType mAnimConvolutedModeBType{};
    std::array<float, 8> mAnimParams{};
    std::array<char, 4> mTexSwapComps{'r', 'g', 'b', 'a'};
    bool mEnabled = true;

public:
    explicit CMaterialPass(CMaterial *pParent);
    ~CMaterialPass();
    std::unique_ptr<CMaterialPass> Clone(CMaterial *pParent) const;
    void HashParameters(CFNV1A& rHash);
    void LoadTexture(uint32 PassIndex);
    void SetAnimCurrent(FRenderOptions Options, size_t PassIndex);

    // Setters
    void SetType(CFourCC Type);
    void SetColorInputs(ETevColorInput InputA, ETevColorInput InputB, ETevColorInput InputC, ETevColorInput InputD);
    void SetAlphaInputs(ETevAlphaInput InputA, ETevAlphaInput InputB, ETevAlphaInput InputC, ETevAlphaInput InputD);
    void SetColorOutput(ETevOutput OutputReg);
    void SetAlphaOutput(ETevOutput OutputReg);
    void SetKColorSel(ETevKSel Sel);
    void SetKAlphaSel(ETevKSel Sel);
    void SetRasSel(ETevRasSel Sel);
    void SetTevColorScale(float Scale);
    void SetTevAlphaScale(float Scale);
    void SetTexCoordSource(uint32 Source);
    void SetTexture(CTexture *pTex);
    void SetAnimMode(EUVAnimMode Mode);
    void SetAnimParam(size_t ParamIndex, float Value);
    void SetTexSwapComp(size_t Comp, char Value);
    void SetEnabled(bool Enabled);

    // Getters
    CFourCC Type() const                             { return mPassType; }
    TString NamedType() const                        { return PassTypeName(mPassType); }
    ETevColorInput ColorInput(size_t Input) const    { return mColorInputs[Input]; }
    ETevAlphaInput AlphaInput(size_t Input) const    { return mAlphaInputs[Input]; }
    ETevOutput ColorOutput() const                   { return mColorOutput; }
    ETevOutput AlphaOutput() const                   { return mAlphaOutput; }
    ETevKSel KColorSel() const                       { return mKColorSel; }
    ETevKSel KAlphaSel() const                       { return mKAlphaSel; }
    ETevRasSel RasSel() const                        { return mRasSel; }
    float TevColorScale() const                      { return mTevColorScale; }
    float TevAlphaScale() const                      { return mTevAlphaScale; }
    uint32 TexCoordSource() const                    { return mTexCoordSource; }
    CTexture* Texture() const                        { return mpTexture; }
    EUVAnimMode AnimMode() const                     { return mAnimMode; }
    float AnimParam(size_t ParamIndex) const         { return mAnimParams[ParamIndex]; }
    char TexSwapComp(size_t Comp) const              { return mTexSwapComps[Comp]; }
    bool IsEnabled() const                           { return mEnabled; }

    // Static
    static TString PassTypeName(CFourCC Type);
};

#endif // CMATERIALPASS_H
