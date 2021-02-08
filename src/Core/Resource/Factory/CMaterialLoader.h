#ifndef CMATERIALLOADER_H
#define CMATERIALLOADER_H

#include "Core/GameProject/CResourceStore.h"
#include "Core/Resource/CMaterialSet.h"
#include <Common/EGame.h>

#include <Common/FileIO.h>
#include <assimp/scene.h>

#include <array>
#include <optional>

enum class EMP3RenderConfig
{
    NoBloomTransparent,
    NoBloomAdditiveIncandecence,
    FullRenderOpaque,
    OptimizedDiffuseLightingColorOpaque,
    OptimizedDiffuseBloomLightingColorOpaque,
    AdditiveIncandecenceOnly,
    AdditiveBlendPremultiplyModColor,
    FullRenderTransparent,
    FullRenderTransparentAdditiveIncandecence,
    MaterialAlphaCompare,
    SolidWhite,
    SolidKColor,
    StaticAlphaCompare,
    DynamicAlphaCompare,
    RandomStaticAlpha,
    SolidKColorKAlpha,
    XRayAdditiveIncandecence,
    XRayTransparent,
    XRayOpaque
};

enum class EPASS
{
    DIFF,
    RIML,
    BLOL,
    CLR,
    TRAN,
    INCA,
    RFLV,
    RFLD,
    LRLD,
    LURD,
    BLOD,
    BLOI,
    XRAY,
    TOON
};

enum class EINT
{
    OPAC,
    BLOD,
    BLOI,
    BNIF,
    XRBR
};

enum class ECLR
{
    CLR,
    DIFB
};

// Material PASSes do not directly map as individual TEV stages (CMaterialPass).
// Therefore, the material data chunks are stored in a random-access-friendly
// container for procedural TEV stage generation.
struct SMP3IntermediateMaterial
{
    FMP3MaterialOptions mOptions;

    struct PASS
    {
        CFourCC mPassType;
        TResPtr<CTexture> mpTexture = nullptr;
        FPassSettings mSettings;
        uint32 mUvSrc;
        EUVAnimUVSource mUvSource = EUVAnimUVSource::UV;
        EUVAnimMatrixConfig mMtxConfig = EUVAnimMatrixConfig::NoMtxNoPost;
        EUVAnimMode mAnimMode = EUVAnimMode::NoUVAnim;
        EUVConvolutedModeBType mAnimConvolutedModeBType;
        std::array<float, 8> mAnimParams;

        char GetSwapAlphaComp() const
        {
            switch (mSettings.ToInt32() & 0x3)
            {
            case 0: return 'r';
            case 1: return 'g';
            case 2: return 'b';
            default: return 'a';
            }
        }
    };
    std::array<std::optional<PASS>, 14> mPASSes;
    const std::optional<PASS>& GetPASS(EPASS pass) const { return mPASSes[static_cast<size_t>(pass)]; }

    std::array<uint8, 5> mINTs{255, 255, 0, 32, 255};
    uint8 GetINT(EINT eint) const { return mINTs[static_cast<size_t>(eint)]; }

    std::array<CColor, 2> mCLRs{CColor::White(), CColor::White()};
    const CColor& GetCLR(ECLR clr) const { return mCLRs[static_cast<size_t>(clr)]; }
};

struct STevTracker
{
    uint8 mCurKColor = 0;
    bool mStaticDiffuseLightingAlphaSet = false;
    bool mStaticLightingAlphaSet = false;
};

class CMaterialLoader
{
    // Material data
    CMaterialSet *mpSet = nullptr;
    IInputStream *mpFile = nullptr;
    EGame mVersion{};
    std::vector<TResPtr<CTexture>> mTextures;

    std::array<CColor, 4> mCorruptionColors;
    std::array<uint8, 5> mCorruptionInts{};
    uint32 mCorruptionFlags = 0;

    CMaterialLoader();
    ~CMaterialLoader();
    static FVertexDescription ConvertToVertexDescription(uint32 VertexFlags);

    // Load Functions
    void ReadPrimeMatSet();
    std::unique_ptr<CMaterial> ReadPrimeMaterial();

    void ReadCorruptionMatSet();
    std::unique_ptr<CMaterial> ReadCorruptionMaterial();
    void SetMP3IntermediateIntoMaterialPass(CMaterialPass* pPass, const SMP3IntermediateMaterial::PASS& Intermediate);
    void SelectBestCombinerConfig(EMP3RenderConfig& OutConfig, uint8& OutAlpha,
                                  const SMP3IntermediateMaterial& Material, bool Bloom);

    bool SetupStaticDiffuseLightingStage(STevTracker& Tracker, CMaterial* pMat,
                                         const SMP3IntermediateMaterial& Intermediate, bool FullAlpha);
    void SetupStaticDiffuseLightingNoBloomStage(STevTracker& Tracker, CMaterial* pMat,
                                                const SMP3IntermediateMaterial& Intermediate);
    void SetupStaticDiffuseLightingNoBLOLStage(STevTracker& Tracker, CMaterial* pMat,
                                               const SMP3IntermediateMaterial& Intermediate);
    void SetupColorTextureStage(STevTracker& Tracker, CMaterial* pMat, const SMP3IntermediateMaterial& Intermediate,
                                bool useStageAlpha, uint8 Alpha, bool StaticLighting);
    void SetupColorTextureAlwaysStaticLightingStage(STevTracker& Tracker, CMaterial* pMat,
                                                    const SMP3IntermediateMaterial& Intermediate);
    void SetupColorKColorStage(STevTracker& Tracker, CMaterial* pMat, const SMP3IntermediateMaterial& Intermediate,
                               bool useStageAlpha, uint8 Alpha, bool StaticLighting);
    bool SetupTransparencyStage(STevTracker& Tracker, CMaterial* pMat, const SMP3IntermediateMaterial& Intermediate);
    void SetupTransparencyKAlphaMultiplyStage(STevTracker& Tracker, CMaterial* pMat,
                                              const SMP3IntermediateMaterial& Intermediate, bool multiplyPrevAlpha,
                                              uint8 Alpha);
    bool SetupReflectionAlphaStage(STevTracker& Tracker, CMaterial* pMat, const SMP3IntermediateMaterial& Intermediate);
    bool SetupReflectionStages(STevTracker& Tracker, CMaterial* pMat, const SMP3IntermediateMaterial& Intermediate,
                               ETevColorInput argD, bool StaticLighting);
    bool SetupQuantizedKAlphaAdd(STevTracker& Tracker, CMaterial* pMat, uint8 Value);
    bool SetupIncandecenceStage(STevTracker& Tracker, CMaterial* pMat, const SMP3IntermediateMaterial& Intermediate);
    bool SetupIncandecenceStageNoBloom(STevTracker& Tracker, CMaterial* pMat,
                                       const SMP3IntermediateMaterial& Intermediate);
    bool SetupStartingIncandecenceStage(STevTracker& Tracker, CMaterial* pMat,
                                        const SMP3IntermediateMaterial& Intermediate);
    void SetupStartingIncandecenceDynamicKColorStage(STevTracker& Tracker, CMaterial* pMat,
                                                     const SMP3IntermediateMaterial& Intermediate);
    bool SetupStaticBloomLightingStage(STevTracker& Tracker, CMaterial* pMat,
                                       const SMP3IntermediateMaterial& Intermediate, bool StaticLighting);
    bool SetupStaticBloomLightingA1Stages(STevTracker& Tracker, CMaterial* pMat,
                                          const SMP3IntermediateMaterial& Intermediate);
    bool SetupStaticBloomDiffuseLightingStages(STevTracker& Tracker, CMaterial* pMat,
                                               const SMP3IntermediateMaterial& Intermediate, bool StaticLighting);
    bool SetupStaticBloomIncandecenceLightingStage(STevTracker& Tracker, CMaterial* pMat,
                                                   const SMP3IntermediateMaterial& Intermediate);
    void SetupNoBloomTransparent(CMaterial* pMat, const SMP3IntermediateMaterial& Intermediate, uint8 Alpha);
    void SetupNoBloomAdditiveIncandecence(CMaterial* pMat, const SMP3IntermediateMaterial& Intermediate, uint8 Alpha);
    void SetupFullRenderOpaque(CMaterial* pMat, const SMP3IntermediateMaterial& Intermediate);
    void SetupOptimizedDiffuseLightingColorOpaque(CMaterial* pMat, const SMP3IntermediateMaterial& Intermediate);
    void SetupOptimizedDiffuseBloomLightingColorOpaque(CMaterial* pMat, const SMP3IntermediateMaterial& Intermediate);
    void SetupAdditiveIncandecenceOnly(CMaterial* pMat, const SMP3IntermediateMaterial& Intermediate);
    CMaterial* SetupFullRenderTransparent(CMaterial* pMat, const SMP3IntermediateMaterial& Intermediate, uint8 Alpha);
    void SetupFullRenderTransparentAdditiveIncandecence(CMaterial* pMat, const SMP3IntermediateMaterial& Intermediate,
                                                        uint8 Alpha);
    void SetupMaterialAlphaCompare(CMaterial* pMat, const SMP3IntermediateMaterial& Intermediate);
    void SetupSolidWhite(CMaterial* pMat, const SMP3IntermediateMaterial& Intermediate);
    void SetupSolidKColorKAlpha(CMaterial* pMat, const SMP3IntermediateMaterial& Intermediate);

    void CreateCorruptionPasses(CMaterial *pMat, const SMP3IntermediateMaterial& Intermediate, bool Bloom);

    std::unique_ptr<CMaterial> LoadAssimpMaterial(const aiMaterial* pAiMat);

    // Static
public:
    static CMaterialSet* LoadMaterialSet(IInputStream& rMat, EGame Version);
    static CMaterialSet* ImportAssimpMaterials(const aiScene *pScene, EGame TargetVersion);
};

#endif // CMATERIALLOADER_H
