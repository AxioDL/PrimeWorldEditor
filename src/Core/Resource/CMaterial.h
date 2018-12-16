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
    DrawWhiteAmbientDKCR    = 0x80000
};
DECLARE_FLAGS_ENUMCLASS(EMaterialOption, FMaterialOptions)

class CMaterial
{
    friend class CMaterialLoader;
    friend class CMaterialCooker;

    enum class EShaderStatus
    {
        NoShader, ShaderExists, ShaderFailed
    };

    // Statics
    static uint64 sCurrentMaterial; // The hash for the currently bound material
    static CColor sCurrentTint;  // The tint for the currently bound material

    // Members
    TString mName;                  // Name of the material
    CShader *mpShader;              // This material's generated shader. Created with GenerateShader().
    EShaderStatus mShaderStatus;    // A status variable so that PWE won't crash if a shader fails to compile.
    uint64 mParametersHash;         // A hash of all the parameters that can identify this TEV setup.
    bool mRecalcHash;               // Indicates the hash needs to be recalculated. Set true when parameters are changed.
    bool mEnableBloom;              // Bool that toggles bloom on or off. On by default on MP3 materials, off by default on MP1 materials.

    EGame mVersion;
    FMaterialOptions mOptions;           // See the EMaterialOptions enum above
    FVertexDescription mVtxDesc;         // Descriptor of vertex attributes used by this material
    CColor mKonstColors[4];              // Konst color values for TEV
    GLenum mBlendSrcFac;                 // Source blend factor
    GLenum mBlendDstFac;                 // Dest blend factor
    bool mLightingEnabled;               // Color channel control flags; indicate whether lighting is enabled
    uint32 mEchoesUnknownA;              // First unknown value introduced in Echoes. Included for cooking.
    uint32 mEchoesUnknownB;              // Second unknown value introduced in Echoes. Included for cooking.
    TResPtr<CTexture> mpIndirectTexture; // Optional texture used for the indirect stage for reflections

    std::vector<CMaterialPass*> mPasses;

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

    CMaterial* Clone();
    void GenerateShader(bool AllowRegen = true);
    void ClearShader();
    bool SetCurrent(FRenderOptions Options);
    uint64 HashParameters();
    void Update();
    void SetNumPasses(uint32 NumPasses);

    // Accessors
    inline TString Name() const                         { return mName; }
    inline EGame Version() const                        { return mVersion; }
    inline FMaterialOptions Options() const             { return mOptions; }
    inline FVertexDescription VtxDesc() const           { return mVtxDesc; }
    inline GLenum BlendSrcFac() const                   { return mBlendSrcFac; }
    inline GLenum BlendDstFac() const                   { return mBlendDstFac; }
    inline CColor Konst(uint32 KIndex) const            { return mKonstColors[KIndex]; }
    inline CTexture* IndTexture() const                 { return mpIndirectTexture; }
    inline bool IsLightingEnabled() const               { return mLightingEnabled; }
    inline uint32 EchoesUnknownA() const                { return mEchoesUnknownA; }
    inline uint32 EchoesUnknownB() const                { return mEchoesUnknownB; }
    inline uint32 PassCount() const                     { return mPasses.size(); }
    inline CMaterialPass* Pass(uint32 PassIndex) const  { return mPasses[PassIndex]; }

    inline void SetName(const TString& rkName)                 { mName = rkName; }
    inline void SetOptions(FMaterialOptions Options)           { mOptions = Options; Update(); }
    inline void SetVertexDescription(FVertexDescription Desc)  { mVtxDesc = Desc; Update(); }
    inline void SetBlendMode(GLenum SrcFac, GLenum DstFac)     { mBlendSrcFac = SrcFac; mBlendDstFac = DstFac; mRecalcHash = true; }
    inline void SetKonst(const CColor& Konst, uint32 KIndex)   { mKonstColors[KIndex] = Konst; Update(); }
    inline void SetIndTexture(CTexture *pTex)                  { mpIndirectTexture = pTex; }
    inline void SetLightingEnabled(bool Enabled)               { mLightingEnabled = Enabled; Update(); }

    // Static
    inline static void KillCachedMaterial() { sCurrentMaterial = 0; }
};

#endif // MATERIAL_H
