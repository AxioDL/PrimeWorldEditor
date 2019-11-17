#ifndef CMATERIALPASS_H
#define CMATERIALPASS_H

#include "TResPtr.h"
#include "CTexture.h"
#include "ETevEnums.h"
#include "Core/Render/FRenderOptions.h"
#include <Common/CFourCC.h>
#include <Common/Hash/CFNV1A.h>

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

    CMaterial *mpParentMat;
    CFourCC mPassType;
    FPassSettings mSettings;

    ETevColorInput mColorInputs[4];
    ETevAlphaInput mAlphaInputs[4];
    ETevOutput mColorOutput;
    ETevOutput mAlphaOutput;
    ETevKSel mKColorSel;
    ETevKSel mKAlphaSel;
    ETevRasSel mRasSel;
    float mTevColorScale;
    float mTevAlphaScale;
    uint32 mTexCoordSource; // Should maybe be an enum but worried about conflicts with EVertexDescriptionn
    TResPtr<CTexture> mpTexture;
    EUVAnimMode mAnimMode;
    EUVConvolutedModeBType mAnimConvolutedModeBType;
    float mAnimParams[8];
    char mTexSwapComps[4];
    bool mEnabled;

public:
    CMaterialPass(CMaterial *pParent);
    ~CMaterialPass();
    CMaterialPass* Clone(CMaterial *pParent);
    void HashParameters(CFNV1A& rHash);
    void LoadTexture(uint32 PassIndex);
    void SetAnimCurrent(FRenderOptions Options, uint32 PassIndex);

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
    void SetAnimParam(uint32 ParamIndex, float Value);
    void SetTexSwapComp(uint32 Comp, char Value);
    void SetEnabled(bool Enabled);

    // Getters
    inline CFourCC Type() const                             { return mPassType; }
    inline TString NamedType() const                        { return PassTypeName(mPassType); }
    inline ETevColorInput ColorInput(uint32 Input) const    { return mColorInputs[Input]; }
    inline ETevAlphaInput AlphaInput(uint32 Input) const    { return mAlphaInputs[Input]; }
    inline ETevOutput ColorOutput() const                   { return mColorOutput; }
    inline ETevOutput AlphaOutput() const                   { return mAlphaOutput; }
    inline ETevKSel KColorSel() const                       { return mKColorSel; }
    inline ETevKSel KAlphaSel() const                       { return mKAlphaSel; }
    inline ETevRasSel RasSel() const                        { return mRasSel; }
    inline float TevColorScale() const                      { return mTevColorScale; }
    inline float TevAlphaScale() const                      { return mTevAlphaScale; }
    inline uint32 TexCoordSource() const                    { return mTexCoordSource; }
    inline CTexture* Texture() const                        { return mpTexture; }
    inline EUVAnimMode AnimMode() const                     { return mAnimMode; }
    inline float AnimParam(uint32 ParamIndex) const         { return mAnimParams[ParamIndex]; }
    inline char TexSwapComp(uint32 Comp) const              { return mTexSwapComps[Comp]; }
    inline bool IsEnabled() const                           { return mEnabled; }

    // Static
    static TString PassTypeName(CFourCC Type);
};

#endif // CMATERIALPASS_H
