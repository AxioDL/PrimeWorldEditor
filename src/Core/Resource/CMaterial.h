#ifndef MATERIAL_H
#define MATERIAL_H

#include "CMaterialPass.h"
#include "CTexture.h"
#include "EGame.h"
#include "TResPtr.h"
#include "Core/Resource/Model/EVertexAttribute.h"
#include "Core/Render/FRenderOptions.h"
#include "Core/OpenGL/CShader.h"

#include <Common/CColor.h>
#include <Common/Flags.h>
#include <Common/types.h>
#include <FileIO/IInputStream.h>

class CMaterialSet;

class CMaterial
{
public:
    friend class CMaterialLoader;
    friend class CMaterialCooker;

    // Enums
    enum EMaterialOption
    {
        eNoSettings        = 0,
        eKonst             = 0x8,
        eTransparent       = 0x10,
        ePunchthrough      = 0x20,
        eReflection        = 0x40,
        eDepthWrite        = 0x80,
        eSurfaceReflection = 0x100,
        eOccluder          = 0x200,
        eIndStage          = 0x400,
        eLightmap          = 0x800,
        eShortTexCoord     = 0x2000,
        eAllSettings       = 0x2FF8
    };
    DECLARE_FLAGS(EMaterialOption, FMaterialOptions)

private:
    enum EShaderStatus
    {
        eNoShader, eShaderExists, eShaderFailed
    };

    // Statics
    static u64 sCurrentMaterial; // The hash for the currently bound material
    static CColor sCurrentTint;  // The tint for the currently bound material

    // Members
    TString mName;                  // Name of the material
    CShader *mpShader;              // This material's generated shader. Created with GenerateShader().
    EShaderStatus mShaderStatus;    // A status variable so that PWE won't crash if a shader fails to compile.
    u64 mParametersHash;            // A hash of all the parameters that can identify this TEV setup.
    bool mRecalcHash;               // Indicates the hash needs to be recalculated. Set true when parameters are changed.
    bool mEnableBloom;              // Bool that toggles bloom on or off. On by default on MP3 materials, off by default on MP1 materials.

    EGame mVersion;
    FMaterialOptions mOptions;           // See the EMaterialOptions enum above
    FVertexDescription mVtxDesc;         // Descriptor of vertex attributes used by this material
    CColor mKonstColors[4];              // Konst color values for TEV
    GLenum mBlendSrcFac;                 // Source blend factor
    GLenum mBlendDstFac;                 // Dest blend factor
    bool mLightingEnabled;               // Color channel control flags; indicate whether lighting is enabled
    u32 mEchoesUnknownA;                 // First unknown value introduced in Echoes. Included for cooking.
    u32 mEchoesUnknownB;                 // Second unknown value introduced in Echoes. Included for cooking.
    TResPtr<CTexture> mpIndirectTexture; // Optional texture used for the indirect stage for reflections

    std::vector<CMaterialPass*> mPasses;

public:
    CMaterial();
    CMaterial(EGame version, FVertexDescription vtxDesc);
    ~CMaterial();
    CMaterial* Clone();
    void GenerateShader(bool AllowRegen = true);
    bool SetCurrent(FRenderOptions Options);
    u64 HashParameters();
    void Update();

    // Getters
    TString Name() const;
    EGame Version() const;
    FMaterialOptions Options() const;
    FVertexDescription VtxDesc() const;
    GLenum BlendSrcFac() const;
    GLenum BlendDstFac() const;
    CColor Konst(u32 KIndex) const;
    CTexture* IndTexture() const;
    bool IsLightingEnabled() const;
    u32 EchoesUnknownA() const;
    u32 EchoesUnknownB() const;
    u32 PassCount() const;
    CMaterialPass* Pass(u32 PassIndex) const;

    // Setters
    void SetName(const TString& name);
    void SetOptions(FMaterialOptions Options);
    void SetVertexDescription(FVertexDescription desc);
    void SetBlendMode(GLenum SrcFac, GLenum DstFac);
    void SetKonst(CColor& Konst, u32 KIndex);
    void SetIndTexture(CTexture *pTex);
    void SetLightingEnabled(bool Enabled);
    void SetNumPasses(u32 NumPasses);

    // Static
    static void KillCachedMaterial();
};

#endif // MATERIAL_H
