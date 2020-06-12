#ifndef MATERIAL_H
#define MATERIAL_H

#include "CMaterialPass.h"
#include "CTexture.h"
#include "TResPtr.h"
#include "Core/Resource/Model/EVertexAttribute.h"
#include "Core/Render/FRenderOptions.h"
#include "Core/OpenGL/CShader.h"

#include <Common/BasicTypes.h>
#include <Common/CColor.h>
#include <Common/EGame.h>
#include <Common/Flags.h>
#include <Common/FileIO/IInputStream.h>

class CMaterialSet;

// Enums
enum class EMaterialOption
{
    None                    = 0,
    Konst                   = 0x8,
    Transparent             = 0x10,
    Masked                  = 0x20,
    Reflection              = 0x40,
    DepthWrite              = 0x80,
    SurfaceReflection       = 0x100,
    Occluder                = 0x200,
    IndStage                = 0x400,
    Lightmap                = 0x800,
    ShortTexCoord           = 0x2000,
    AllMP1Settings          = 0x2FF8,
    ColorWrite              = 0x10000,
    AlphaWrite              = 0x20000,
    ZeroDestAlpha           = 0x40000,
    DrawWhiteAmbientDKCR    = 0x80000
};
DECLARE_FLAGS_ENUMCLASS(EMaterialOption, FMaterialOptions)

enum class EMP3MaterialOption
{
    None                        = 0,
    Bloom                       = 0x1,
    ForceLightingStage          = 0x4,
    PreIncandecenceTransparency = 0x8,
    Masked                      = 0x10,
    AdditiveIncandecence        = 0x20,
    Occluder                    = 0x100,
    SolidWhiteOnly              = 0x200,
    ReflectionAlphaTarget       = 0x400,
    SolidColorOnly              = 0x800,
    ExcludeFromScanVisor        = 0x4000,
    XRayOpaque                  = 0x8000,
    XRayAlphaTarget             = 0x10000,
    DrawWhiteAmbientDKCR        = 0x80000,
};
DECLARE_FLAGS_ENUMCLASS(EMP3MaterialOption, FMP3MaterialOptions)

class CMaterial
{
    friend class CMaterialLoader;
    friend class CMaterialCooker;

public:
    enum class EShaderStatus
    {
        NoShader, ShaderExists, ShaderFailed
    };

private:
    // Statics
    static uint64 sCurrentMaterial; // The hash for the currently bound material
    static CColor sCurrentTint;  // The tint for the currently bound material

    // Members
    TString mName;                  // Name of the material
    CShader *mpShader;              // This material's generated shader. Created with GenerateShader().
    EShaderStatus mShaderStatus;    // A status variable so that PWE won't crash if a shader fails to compile.
    uint64 mParametersHash;         // A hash of all the parameters that can identify this TEV setup.
    bool mRecalcHash;               // Indicates the hash needs to be recalculated. Set true when parameters are changed.

    EGame mVersion;
    FMaterialOptions mOptions;           // See the EMaterialOption enum above
    FVertexDescription mVtxDesc;         // Descriptor of vertex attributes used by this material
    std::array<CColor, 4> mKonstColors;  // Konst color values for TEV
    std::array<CColor, 4> mTevColors;    // Initial TEV color register values (for MP3 materials only)
    GLenum mBlendSrcFac;                 // Source blend factor
    GLenum mBlendDstFac;                 // Dest blend factor
    bool mLightingEnabled;               // Color channel control flags; indicate whether lighting is enabled
    uint32 mEchoesUnknownA;              // First unknown value introduced in Echoes. Included for cooking.
    uint32 mEchoesUnknownB;              // Second unknown value introduced in Echoes. Included for cooking.
    TResPtr<CTexture> mpIndirectTexture; // Optional texture used for the indirect stage for reflections

    std::vector<std::unique_ptr<CMaterialPass>> mPasses;

    // Transparent materials in MP3/DKCR may require multiple draw passes to achieve hybrid
    // blending modes. This serves as a linked list of materials to be drawn successively
    // for each surface.
    std::unique_ptr<CMaterial> mpNextDrawPassMaterial;

    // Bloom in MP3 changes the CMaterialPass layout significantly. This is an alternate
    // material head that may be conditionally used when the user wants to view bloom.
    // (only set in the head non-bloom CMaterial).
    std::unique_ptr<CMaterial> mpBloomMaterial;

    // Reuse shaders between materials that have identical TEV setups
    struct SMaterialShader
    {
        int NumReferences;
        CShader *pShader;
    };
    static std::map<uint64, SMaterialShader> smShaderMap;

public:
    CMaterial();
    CMaterial(EGame Version, FVertexDescription VtxDesc);
    ~CMaterial();

    std::unique_ptr<CMaterial> Clone();
    void GenerateShader(bool AllowRegen = true);
    void ClearShader();
    bool SetCurrent(FRenderOptions Options);
    uint64 HashParameters();
    void Update();
    void SetNumPasses(uint32 NumPasses);

    // Accessors
    TString Name() const                         { return mName; }
    EGame Version() const                        { return mVersion; }
    FMaterialOptions Options() const             { return mOptions; }
    FVertexDescription VtxDesc() const           { return mVtxDesc; }
    GLenum BlendSrcFac() const                   { return mBlendSrcFac; }
    GLenum BlendDstFac() const                   { return mBlendDstFac; }
    CColor Konst(uint32 KIndex) const            { return mKonstColors[KIndex]; }
    CColor TevColor(ETevOutput Out) const        { return mTevColors[int(Out)]; }
    CTexture* IndTexture() const                 { return mpIndirectTexture; }
    bool IsLightingEnabled() const               { return mLightingEnabled; }
    uint32 EchoesUnknownA() const                { return mEchoesUnknownA; }
    uint32 EchoesUnknownB() const                { return mEchoesUnknownB; }
    uint32 PassCount() const                     { return mPasses.size(); }
    CMaterialPass* Pass(uint32 PassIndex) const  { return mPasses[PassIndex].get(); }
    CMaterial* GetNextDrawPass() const           { return mpNextDrawPassMaterial.get(); }
    CMaterial* GetBloomVersion() const           { return mpBloomMaterial.get(); }

    void SetName(const TString& rkName)                 { mName = rkName; }
    void SetOptions(FMaterialOptions Options)           { mOptions = Options; Update(); }
    void SetVertexDescription(FVertexDescription Desc)  { mVtxDesc = Desc; Update(); }
    void SetBlendMode(GLenum SrcFac, GLenum DstFac)     { mBlendSrcFac = SrcFac; mBlendDstFac = DstFac; mRecalcHash = true; }
    void SetKonst(const CColor& Konst, uint32 KIndex)   { mKonstColors[KIndex] = Konst; Update(); }
    void SetTevColor(const CColor& Color, ETevOutput Out) { mTevColors[int(Out)] = Color; }
    void SetIndTexture(CTexture *pTex)                  { mpIndirectTexture = pTex; }
    void SetLightingEnabled(bool Enabled)               { mLightingEnabled = Enabled; Update(); }

    // Static
    static void KillCachedMaterial() { sCurrentMaterial = 0; }
};

#endif // MATERIAL_H
