#ifndef MATERIAL_H
#define MATERIAL_H

#include "CMaterialPass.h"
#include "CTexture.h"
#include "EFormatVersion.h"
#include "model/EVertexDescription.h"
#include <Common/CColor.h>
#include <Common/EnumUtil.h>
#include <Common/types.h>
#include <Core/TResPtr.h>
#include <Core/ERenderOptions.h>
#include <FileIO/CInputStream.h>
#include <OpenGL/CShader.h>
#include <Resource/CTexture.h>

class CMaterialSet;

class CMaterial
{
public:
    friend class CMaterialLoader;
    friend class CMaterialCooker;

    // Enums
    enum EMaterialOptions
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
    EMaterialOptions mOptions;           // See the EMaterialOptions enum above
    EVertexDescription mVtxDesc;         // Descriptor of vertex attributes used by this material
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
    CMaterial(EGame version, EVertexDescription vtxDesc);
    ~CMaterial();
    CMaterial* Clone();
    void GenerateShader();
    bool SetCurrent(ERenderOptions Options);
    u64 HashParameters();
    void Update();

    // Getters
    TString Name() const;
    EGame Version() const;
    EMaterialOptions Options() const;
    EVertexDescription VtxDesc() const;
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
    void SetOptions(EMaterialOptions Options);
    void SetVertexDescription(EVertexDescription desc);
    void SetBlendMode(GLenum SrcFac, GLenum DstFac);
    void SetKonst(CColor& Konst, u32 KIndex);
    void SetIndTexture(CTexture *pTex);
    void SetLightingEnabled(bool Enabled);
    void SetNumPasses(u32 NumPasses);

    // Static
    static void KillCachedMaterial();
};
DEFINE_ENUM_FLAGS(CMaterial::EMaterialOptions)

#endif // MATERIAL_H
