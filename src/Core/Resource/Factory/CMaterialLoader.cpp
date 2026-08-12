#include "Core/Resource/Factory/CMaterialLoader.h"

#include "Core/NRangeUtils.h"
#include "Core/GameProject/CResourceStore.h"
#include "Core/OpenGL/GLCommon.h"
#include "Core/Resource/CMaterialSet.h"
#include <Common/Log.h>
#include <Common/FileIO/IInputStream.h>

#include <assimp/scene.h>
#include <fmt/format.h>

CMaterialLoader::CMaterialLoader() = default;

CMaterialLoader::~CMaterialLoader() = default;

FVertexDescription CMaterialLoader::ConvertToVertexDescription(uint32_t VertexFlags)
{
    FVertexDescription Desc;
    if ((VertexFlags & 0x00000003) != 0) Desc |= EVertexAttribute::Position;
    if ((VertexFlags & 0x0000000C) != 0) Desc |= EVertexAttribute::Normal;
    if ((VertexFlags & 0x00000030) != 0) Desc |= EVertexAttribute::Color0;
    if ((VertexFlags & 0x000000C0) != 0) Desc |= EVertexAttribute::Color1;
    if ((VertexFlags & 0x00000300) != 0) Desc |= EVertexAttribute::Tex0;
    if ((VertexFlags & 0x00000C00) != 0) Desc |= EVertexAttribute::Tex1;
    if ((VertexFlags & 0x00003000) != 0) Desc |= EVertexAttribute::Tex2;
    if ((VertexFlags & 0x0000C000) != 0) Desc |= EVertexAttribute::Tex3;
    if ((VertexFlags & 0x00030000) != 0) Desc |= EVertexAttribute::Tex4;
    if ((VertexFlags & 0x000C0000) != 0) Desc |= EVertexAttribute::Tex5;
    if ((VertexFlags & 0x00300000) != 0) Desc |= EVertexAttribute::Tex6;
    if ((VertexFlags & 0x00C00000) != 0) Desc |= EVertexAttribute::Tex7;
    if ((VertexFlags & 0x01000000) != 0) Desc |= EVertexAttribute::PosMtx;
    if ((VertexFlags & 0x02000000) != 0) Desc |= EVertexAttribute::Tex0Mtx;
    if ((VertexFlags & 0x04000000) != 0) Desc |= EVertexAttribute::Tex1Mtx;
    if ((VertexFlags & 0x08000000) != 0) Desc |= EVertexAttribute::Tex2Mtx;
    if ((VertexFlags & 0x10000000) != 0) Desc |= EVertexAttribute::Tex3Mtx;
    if ((VertexFlags & 0x20000000) != 0) Desc |= EVertexAttribute::Tex4Mtx;
    if ((VertexFlags & 0x40000000) != 0) Desc |= EVertexAttribute::Tex5Mtx;
    if ((VertexFlags & 0x80000000) != 0) Desc |= EVertexAttribute::Tex6Mtx;
    return Desc;
}

void CMaterialLoader::ReadPrimeMatSet(CResourceStore* resourceStore)
{
    // Textures
    const auto NumTextures = mpFile->ReadU32();
    mTextures.resize(NumTextures);

    for (auto& texture : mTextures)
    {
        const auto TextureID = mpFile->ReadU32();
        texture = resourceStore->LoadResource<CTexture>(TextureID);
    }

    // Materials
    const auto NumMats = mpFile->ReadU32();
    std::vector<uint32_t> Offsets(NumMats);
    for (auto& offset : Offsets)
        offset = mpFile->ReadU32();

    const auto MatsStart = mpFile->Tell();
    mpSet->mMaterials.resize(NumMats);
    for (auto&& [idx, material] : Utils::enumerate(mpSet->mMaterials))
    {
        material = ReadPrimeMaterial();
        material->mVersion = mVersion;
        material->mName = fmt::format("Material #{}", idx + 1);
        mpFile->Seek(MatsStart + Offsets[idx], SEEK_SET);
    }
}

std::unique_ptr<CMaterial> CMaterialLoader::ReadPrimeMaterial()
{
    auto pMat = std::make_unique<CMaterial>(mVersion, FVertexDescription{});

    // Flags
    pMat->mOptions = FMaterialOptions(mpFile->ReadU32() & static_cast<uint32_t>(EMaterialOption::AllMP1Settings));
    pMat->mOptions.SetFlag(EMaterialOption::ColorWrite);

    // Textures
    const auto NumTextures = mpFile->ReadU32();
    std::vector<uint32_t> TextureIndices(NumTextures);

    for (auto& index : TextureIndices)
    {
        index = mpFile->ReadU32();
    }

    // Vertex description
    pMat->mVtxDesc = ConvertToVertexDescription(mpFile->ReadU32());

    // Unknowns
    if (mVersion >= EGame::EchoesDemo)
    {
        pMat->mEchoesUnknownA = mpFile->ReadU32();
        pMat->mEchoesUnknownB = mpFile->ReadU32();
    }
    mpFile->Seek(0x4, SEEK_CUR); // Skipping group index

    // Konst
    if (pMat->mOptions.HasFlag(EMaterialOption::Konst))
    {
        const auto KonstCount = mpFile->ReadU32();

        for (size_t iKonst = 0; iKonst < KonstCount; iKonst++)
        {
            if (iKonst >= 4)
                break;

            pMat->mKonstColors[iKonst] = CColor(*mpFile, true);
        }

        if (KonstCount > 4)
            mpFile->Seek(0x4 * (KonstCount - 4), SEEK_CUR);
    }

    // Blend mode
    pMat->mBlendDstFac = GetBlendFactor(mpFile->ReadU16());
    pMat->mBlendSrcFac = GetBlendFactor(mpFile->ReadU16());

    // Indirect texture
    if (pMat->mOptions.HasFlag(EMaterialOption::IndStage))
    {
        const auto IndTexIndex = mpFile->ReadU32();
        pMat->mpIndirectTexture = mTextures[TextureIndices[IndTexIndex]];
    }

    // Color channels
    const auto ChanCount = mpFile->ReadU32();
    pMat->mLightingEnabled = (mpFile->ReadU32() & 1) == 1;
    mpFile->Seek((4 * ChanCount) - 4, SEEK_CUR);

    // TEV
    const auto TevCount = mpFile->ReadU32();
    pMat->mPasses.resize(TevCount);

    for (auto& matPass : pMat->mPasses)
    {
        auto pPass = std::make_unique<CMaterialPass>(pMat.get());

        const auto ColorIn = mpFile->ReadU32();
        const auto AlphaIn = mpFile->ReadU32();
        pPass->mColorOutput = static_cast<ETevOutput>((mpFile->ReadU32() & 0x600) >> 9);
        pPass->mAlphaOutput = static_cast<ETevOutput>((mpFile->ReadU32() & 0x600) >> 9);
        mpFile->Seek(0x1, SEEK_CUR); // Padding byte
        pPass->mKAlphaSel = static_cast<ETevKSel>(mpFile->ReadU8());
        pPass->mKColorSel = static_cast<ETevKSel>(mpFile->ReadU8());
        pPass->mRasSel = static_cast<ETevRasSel>(mpFile->ReadU8());

        for (size_t iInput = 0; iInput < 4; iInput++)
        {
            pPass->mColorInputs[iInput] = static_cast<ETevColorInput>((ColorIn >> (iInput * 5)) & 0xF);
            pPass->mAlphaInputs[iInput] = static_cast<ETevAlphaInput>((AlphaIn >> (iInput * 5)) & 0x7);
        }

        matPass = std::move(pPass);
    }

    std::vector<uint8_t> TevCoordIndices;
    TevCoordIndices.reserve(TevCount);
    for (auto* pass : pMat->Passes())
    {
        mpFile->Seek(0x2, SEEK_CUR);
        const auto TexSel = mpFile->ReadU8();

        if (TexSel == 0xFF || TexSel >= TextureIndices.size())
            pass->mpTexture = nullptr;
        else
            pass->mpTexture = mTextures[TextureIndices[TexSel]];

        TevCoordIndices.push_back(mpFile->ReadU8());
    }

    // TexGens
    const auto TexGenCount = mpFile->ReadU32();
    std::vector<uint32_t> TexGens(TexGenCount);

    for (auto& texGen : TexGens)
        texGen = mpFile->ReadU32();

    // UV animations
    mpFile->Seek(0x4, SEEK_CUR); // Skipping UV anims size
    const auto NumAnims = mpFile->ReadU32();

    struct SUVAnim {
        int32_t Mode{};
        std::array<float, 4> Params{};
    };
    std::vector <SUVAnim> Anims(NumAnims);

    for (auto& Anim : Anims)
    {
        Anim.Mode = mpFile->ReadS32();

        switch (Anim.Mode)
        {
        case 3: // Rotation
        case 7: // ???
            Anim.Params[0] = mpFile->ReadF32();
            Anim.Params[1] = mpFile->ReadF32();
            break;
        case 2: // UV Scroll
        case 4: // U Scroll
        case 5: // V Scroll
            Anim.Params[0] = mpFile->ReadF32();
            Anim.Params[1] = mpFile->ReadF32();
            Anim.Params[2] = mpFile->ReadF32();
            Anim.Params[3] = mpFile->ReadF32();
            break;
        case 0: // Inverse ModelView Matrix
        case 1: // Inverse ModelView Matrix Translated
        case 6: // Model Matrix
            break;
        default:
            NLog::Error("{} [0x{:X}]: Unsupported animation mode encountered: {}", mpFile->GetSourceString(), mpFile->Tell() - 4, Anim.Mode);
            break;
        }
    }

    // Move TexGen and anims into passes
    for (auto&& [idx, pass] : Utils::enumerate(pMat->mPasses))
    {
        const auto TexCoordIdx = TevCoordIndices[idx];

        if (TexGens.empty() || TexCoordIdx == 0xFF)
        {
            pass->mTexCoordSource = 0xFF;
            pass->mAnimMode = EUVAnimMode::NoUVAnim;
        }
        else
        {
            pass->mTexCoordSource = static_cast<uint8_t>((TexGens[TexCoordIdx] & 0x1F0) >> 4);

            // Next step - find which animation is used by this pass
            // Texture matrix is a reliable way to tell, because every UV anim mode generates a texture matrix
            const uint32_t TexMtxIdx = ((TexGens[TexCoordIdx] & 0x3E00) >> 9) / 3;

            // 10 is identity matrix; indicates no UV anim for this pass
            if (TexMtxIdx == 10)
            {
                pass->mAnimMode = EUVAnimMode::NoUVAnim;
            }
            else
            {
                pass->mAnimMode = static_cast<EUVAnimMode>(Anims[TexMtxIdx].Mode);

                for (size_t iParam = 0; iParam < 4; iParam++)
                    pass->mAnimParams[iParam] = Anims[TexMtxIdx].Params[iParam];
            }
        }
    }

    return pMat;
}

void CMaterialLoader::ReadCorruptionMatSet(CResourceStore* resourceStore)
{
    const auto NumMats = mpFile->ReadU32();
    mpSet->mMaterials.resize(NumMats);

    for (auto&& [idx, material] : Utils::enumerate(mpSet->mMaterials))
    {
        const auto Size = mpFile->ReadU32();
        const auto Next = mpFile->Tell() + Size;
        material = ReadCorruptionMaterial(resourceStore);
        material->mVersion = mVersion;
        material->mName = fmt::format("Material #{}", idx + 1);
        mpFile->Seek(Next, SEEK_SET);
    }
}

// Represent passes as contiguous integers for random-access storage in a fixed array.
static EPASS PassFourCCToEnum(CFourCC fcc)
{
    if (fcc == CFourCC("DIFF"))
        return EPASS::DIFF;
    if (fcc == CFourCC("RIML"))
        return EPASS::RIML;
    if (fcc == CFourCC("BLOL"))
        return EPASS::BLOL;
    if (fcc == CFourCC("CLR "))
        return EPASS::CLR;
    if (fcc == CFourCC("TRAN"))
        return EPASS::TRAN;
    if (fcc == CFourCC("INCA"))
        return EPASS::INCA;
    if (fcc == CFourCC("RFLV"))
        return EPASS::RFLV;
    if (fcc == CFourCC("RFLD"))
        return EPASS::RFLD;
    if (fcc == CFourCC("LRLD"))
        return EPASS::LRLD;
    if (fcc == CFourCC("LURD"))
        return EPASS::LURD;
    if (fcc == CFourCC("BLOD"))
        return EPASS::BLOD;
    if (fcc == CFourCC("BLOI"))
        return EPASS::BLOI;
    if (fcc == CFourCC("XRAY"))
        return EPASS::XRAY;
    if (fcc == CFourCC("TOON"))
        return EPASS::TOON;

    return EPASS::DIFF;
}

static EINT IntFourCCToEnum(CFourCC fcc)
{
    if (fcc == CFourCC("OPAC"))
        return EINT::OPAC;
    if (fcc == CFourCC("BLOD"))
        return EINT::BLOD;
    if (fcc == CFourCC("BLOI"))
        return EINT::BLOI;
    if (fcc == CFourCC("BNIF"))
        return EINT::BNIF;
    if (fcc == CFourCC("XRBR"))
        return EINT::XRBR;

    return EINT::OPAC;
}

static ECLR ClrFourCCToEnum(CFourCC fcc)
{
    if (fcc == CFourCC("CLR "))
        return ECLR::CLR;
    if (fcc == CFourCC("DIFB"))
        return ECLR::DIFB;

    return ECLR::CLR;
}

std::unique_ptr<CMaterial> CMaterialLoader::ReadCorruptionMaterial(CResourceStore* resourceStore)
{
    // Flags
    const FMP3MaterialOptions MP3Options(mpFile->ReadS32());

    mpFile->Seek(0x8, SEEK_CUR); // Don't know what any of this is
    FVertexDescription VtxDesc = ConvertToVertexDescription(mpFile->ReadU32());
    mpFile->Seek(0xC, SEEK_CUR);

    SMP3IntermediateMaterial Intermediate;
    Intermediate.mOptions = MP3Options;
    while (true)
    {
        const CFourCC Type(mpFile->ReadU32());

        // END
        if (Type == CFourCC("END "))
            break;

        // INT
        if (Type == CFourCC("INT "))
        {
            const CFourCC IntType(mpFile->ReadU32());
            const auto IntVal = static_cast<uint8_t>(mpFile->ReadU32());
            Intermediate.mINTs[static_cast<int>(IntFourCCToEnum(IntType))] = IntVal;
        }

        // CLR
        if (Type == CFourCC("CLR "))
        {
            const CFourCC ClrType(mpFile->ReadU32());
            const CColor ClrVal(*mpFile, true);
            Intermediate.mCLRs[static_cast<int>(ClrFourCCToEnum(ClrType))] = ClrVal;
        }

        // PASS
        if (Type == CFourCC("PASS"))
        {
            const auto Size = mpFile->ReadU32();
            const auto Next = Size + mpFile->Tell();

            const CFourCC PassType(mpFile->ReadU32());
            auto& Pass = Intermediate.mPASSes[static_cast<int>(PassFourCCToEnum(PassType))].emplace(SMP3IntermediateMaterial::PASS());

            Pass.mPassType = PassType;
            Pass.mSettings = static_cast<EPassSettings>(mpFile->ReadU32());

            const auto TextureID = mpFile->ReadU64();
            if (TextureID != UINT64_MAX)
                Pass.mpTexture = resourceStore->LoadResource<CTexture>(TextureID);

            Pass.mUvSrc = mpFile->ReadU32();

            const auto AnimSize = mpFile->ReadU32();
            if (AnimSize > 0)
            {
                Pass.mUvSource = static_cast<EUVAnimUVSource>(mpFile->ReadU16());
                Pass.mMtxConfig = static_cast<EUVAnimMatrixConfig>(mpFile->ReadU16());

                Pass.mAnimMode = static_cast<EUVAnimMode>(mpFile->ReadU32());

                switch (Pass.mAnimMode)
                {
                    case EUVAnimMode::UVRotation: // Rotation
                    case EUVAnimMode::ConvolutedModeA: // ???
                        Pass.mAnimParams[0] = mpFile->ReadF32();
                        Pass.mAnimParams[1] = mpFile->ReadF32();
                        break;
                    case EUVAnimMode::UVScroll: // UV Scroll
                    case EUVAnimMode::HFilmstrip: // U Scroll
                    case EUVAnimMode::VFilmstrip: // V Scroll
                        Pass.mAnimParams[0] = mpFile->ReadF32();
                        Pass.mAnimParams[1] = mpFile->ReadF32();
                        Pass.mAnimParams[2] = mpFile->ReadF32();
                        Pass.mAnimParams[3] = mpFile->ReadF32();
                        break;
                    case EUVAnimMode::InverseMV: // Inverse ModelView Matrix
                    case EUVAnimMode::InverseMVTranslated: // Inverse ModelView Matrix Translated
                    case EUVAnimMode::ModelMatrix: // Model Matrix
                    case EUVAnimMode::SimpleMode: // Yet-to-be-named
                        break;

                        // Unknown/unsupported animation type
                    case EUVAnimMode::ConvolutedModeB:
                        Pass.mAnimConvolutedModeBType = EUVConvolutedModeBType(mpFile->ReadU32());
                        Pass.mAnimParams[0] = mpFile->ReadF32();
                        Pass.mAnimParams[1] = mpFile->ReadF32();
                        Pass.mAnimParams[2] = mpFile->ReadF32();
                        Pass.mAnimParams[3] = mpFile->ReadF32();
                        Pass.mAnimParams[4] = mpFile->ReadF32();
                        Pass.mAnimParams[5] = mpFile->ReadF32();
                        Pass.mAnimParams[6] = mpFile->ReadF32();
                        Pass.mAnimParams[7] = mpFile->ReadF32();
                        NLog::Debug("{}: UVMode8 Used with type {}",
                                    mpFile->GetSourceString(), static_cast<int>(Pass.mAnimConvolutedModeBType));
                        break;
                    case EUVAnimMode::Eleven:
                        break;
                    default:
                        NLog::Error("{} [0x{:X}]: Unsupported animation mode encountered: {}",
                                    mpFile->GetSourceString(), mpFile->Tell() - 8, int(Pass.mAnimMode));
                        break;
                }
            }
            mpFile->Seek(Next, SEEK_SET);
        }
    }

    // Create non-bloom and bloom versions
    auto pMat = std::make_unique<CMaterial>(mVersion, VtxDesc);
    CreateCorruptionPasses(pMat.get(), Intermediate, false);
    pMat->mpBloomMaterial = std::make_unique<CMaterial>(mVersion, VtxDesc);
    CreateCorruptionPasses(pMat->mpBloomMaterial.get(), Intermediate, true);
    return pMat;
}

void CMaterialLoader::SetMP3IntermediateIntoMaterialPass(CMaterialPass* pPass,
                                                         const SMP3IntermediateMaterial::PASS& Intermediate)
{
    pPass->mPassType = Intermediate.mPassType;
    pPass->mSettings = Intermediate.mSettings;
    if (Intermediate.mUvSource == EUVAnimUVSource::UV)
        pPass->mTexCoordSource = 4 + Intermediate.mUvSrc; // TEX
    else if (Intermediate.mUvSource == EUVAnimUVSource::Position)
        pPass->mTexCoordSource = 0; // POS
    else if (Intermediate.mUvSource == EUVAnimUVSource::Normal)
        pPass->mTexCoordSource = 1; // NORM
    pPass->mpTexture = Intermediate.mpTexture;
    pPass->mAnimMode = Intermediate.mAnimMode;
    pPass->mAnimConvolutedModeBType = Intermediate.mAnimConvolutedModeBType;
    pPass->mAnimParams = Intermediate.mAnimParams;
}

void CMaterialLoader::SelectBestCombinerConfig(EMP3RenderConfig& OutConfig, uint8_t& OutAlpha,
                                               const SMP3IntermediateMaterial& Material, bool Bloom)
{
    const uint8_t UseAlpha = Material.GetINT(EINT::OPAC);

    EMP3RenderConfig UseConfig = EMP3RenderConfig::FullRenderOpaque;
    if (Material.mOptions.HasFlag(EMP3MaterialOption::SolidWhiteOnly))
    {
        // Just white
        UseConfig = EMP3RenderConfig::SolidWhite;
    }
    else if (Material.mOptions.HasFlag(EMP3MaterialOption::SolidColorOnly))
    {
        // Just KColor/KAlpha
        UseConfig = EMP3RenderConfig::SolidKColorKAlpha;
    }
    else
    {
        const bool AdditiveIncandecence = Material.mOptions.HasFlag(EMP3MaterialOption::AdditiveIncandecence);
        // Essentially skips optimized variants for special rendering cases even if opaque
        // Config 6 being an important case here
        const bool ForceAlphaBlendingConfig = Material.mOptions.HasFlag(EMP3MaterialOption::PreIncandecenceTransparency);
        if (AdditiveIncandecence && !ForceAlphaBlendingConfig)
        {
            // Incandecence/Reflect only
            UseConfig = EMP3RenderConfig::AdditiveIncandecenceOnly;
        }
        else
        {
            bool AlphaCompare = false;
            if (Material.mOptions.HasFlag(EMP3MaterialOption::Masked) && UseAlpha == 255)
            {
                AlphaCompare = true;
            }

            bool ConsiderBloom = false;
            if ((ForceAlphaBlendingConfig || Material.GetINT(EINT::OPAC) < 255) && !AlphaCompare)
            {
                ConsiderBloom = true;
            }

            bool ForceNoBloom = true;
            if (!(ConsiderBloom && Material.mOptions.HasFlag(EMP3MaterialOption::Bloom)))
                ForceNoBloom = false;

            if (AdditiveIncandecence)
            {
                if (ForceNoBloom || !Bloom)
                {
                    if (Material.GetPASS(EPASS::INCA))
                    {
                        UseConfig = EMP3RenderConfig::NoBloomAdditiveIncandecence;
                    }
                    else
                    {
                        UseConfig = EMP3RenderConfig::NoBloomTransparent;
                    }
                }
                else
                {
                    UseConfig = EMP3RenderConfig::FullRenderTransparentAdditiveIncandecence;
                }
            }
            else if (AlphaCompare)
            {
                UseConfig = EMP3RenderConfig::MaterialAlphaCompare;
            }
            else if (ForceAlphaBlendingConfig || UseAlpha < 255)
            {
                const bool WithIncandecence = Material.GetPASS(EPASS::INCA) || Material.GetPASS(EPASS::BLOI);
                if (ForceNoBloom || !Bloom)
                {
                    if (WithIncandecence)
                    {
                        UseConfig = EMP3RenderConfig::NoBloomAdditiveIncandecence;
                    }
                    else
                    {
                        UseConfig = EMP3RenderConfig::NoBloomTransparent;
                    }
                }
                else
                {
                    UseConfig = EMP3RenderConfig::FullRenderTransparent;
                    if (WithIncandecence)
                    {
                        UseConfig = EMP3RenderConfig::FullRenderTransparentAdditiveIncandecence;
                    }
                }
            }
        }
    }

    if (UseConfig == EMP3RenderConfig::FullRenderOpaque)
    {
        if (Material.GetPASS(EPASS::DIFF) && Material.GetPASS(EPASS::CLR))
        {
            UseConfig = EMP3RenderConfig::OptimizedDiffuseLightingColorOpaque;
        }
        else if (Material.GetPASS(EPASS::DIFF) && Material.GetPASS(EPASS::BLOL) && Material.GetPASS(EPASS::CLR))
        {
            UseConfig = EMP3RenderConfig::OptimizedDiffuseBloomLightingColorOpaque;
        }
    }

    OutConfig = UseConfig;
    OutAlpha = UseAlpha;
}

constexpr std::array KColorEighths
{
    kKonstOneEighth,
    kKonstOneFourth,
    kKonstThreeEighths,
    kKonstOneHalf,
    kKonstFiveEighths,
    kKonstThreeFourths,
    kKonstSevenEighths,
    kKonstOne,
};

bool CMaterialLoader::SetupStaticDiffuseLightingStage(STevTracker& Tracker, CMaterial* pMat,
                                                      const SMP3IntermediateMaterial& Intermediate, bool FullAlpha)
{
    const bool hasDIFFTexture = Intermediate.GetPASS(EPASS::DIFF).has_value();
    constexpr auto LightTest = EMP3MaterialOption(EMP3MaterialOption::AdditiveIncandecence |
                                                  EMP3MaterialOption::PreIncandecenceTransparency |
                                                  EMP3MaterialOption::ForceLightingStage);

    if (!hasDIFFTexture && (Intermediate.mOptions & LightTest) != EMP3MaterialOption::ForceLightingStage)
        return false;

    pMat->SetTevColor(Intermediate.GetCLR(ECLR::DIFB), kColor1Reg);
    auto pPass = std::make_unique<CMaterialPass>(pMat);
    if (hasDIFFTexture)
    {
        SetMP3IntermediateIntoMaterialPass(pPass.get(), *Intermediate.GetPASS(EPASS::DIFF));
        pPass->SetColorInputs(kZeroRGB, kTextureRGB, kColor1RGB, kRasRGB);
        pPass->SetColorOutput(kColor0Reg);
        if (FullAlpha)
        {
            /* Full Alpha */
            pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kKonstAlpha);
            pPass->SetKAlphaSel(kKonstOne);
            pPass->SetAlphaOutput(kPrevReg);
        }
        else if (!Intermediate.GetPASS(EPASS::BLOL))
        {
            Tracker.mStaticDiffuseLightingAlphaSet = true;
            Tracker.mStaticLightingAlphaSet = true;
            pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kZeroAlpha);
            pPass->SetAlphaOutput(kPrevReg);
        }
        else
        {
            Tracker.mStaticDiffuseLightingAlphaSet = true;
            pPass->SetAlphaOutput(kPrevReg);
            if (Intermediate.GetINT(EINT::BLOD) == 0)
            {
                pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kRasAlpha);
            }
            else if (Intermediate.GetINT(EINT::BLOD) == 255)
            {
                pPass->SetAlphaInputs(kZeroAlpha, kTextureAlpha, kColor1Alpha, kRasAlpha);
            }
            else
            {
                pMat->SetKonst(CColor::Integral(0, 0, 0, Intermediate.GetINT(EINT::BLOD)), Tracker.mCurKColor);
                pPass->SetKAlphaSel(ETevKSel(kKonst0_A + Tracker.mCurKColor));
                Tracker.mCurKColor += 1;
                pPass->SetAlphaInputs(kZeroAlpha, kTextureAlpha, kKonstAlpha, kRasAlpha);
            }
            Tracker.mStaticLightingAlphaSet = true;
        }
        pPass->SetRasSel(kRasColor0A0);
    }
    else
    {
        pPass->mPassType = CFourCC("DIFF");
        pPass->SetColorOutput(kColor0Reg);
        pPass->SetAlphaOutput(kPrevReg);
        pPass->SetColorInputs(kZeroRGB, kColor1RGB, kKonstRGB, kRasRGB);
        pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, FullAlpha ? kKonstAlpha : kZeroAlpha);
        pPass->SetKColorSel(kKonstOne);
        pPass->SetKAlphaSel(kKonstOne);
        pPass->SetRasSel(kRasColor0A0);
        Tracker.mStaticLightingAlphaSet = true;
    }
    pMat->mPasses.push_back(std::move(pPass));
    return true;
}

void CMaterialLoader::SetupStaticDiffuseLightingNoBloomStage(STevTracker& Tracker, CMaterial* pMat,
                                                             const SMP3IntermediateMaterial& Intermediate)
{
    auto pPass = std::make_unique<CMaterialPass>(pMat);
    SetMP3IntermediateIntoMaterialPass(pPass.get(), *Intermediate.GetPASS(EPASS::DIFF));

    pMat->SetTevColor(Intermediate.GetCLR(ECLR::DIFB), kColor1Reg);
    pPass->SetColorInputs(kZeroRGB, kTextureRGB, kColor1RGB, kRasRGB);
    pPass->SetColorOutput(kColor0Reg);
    Tracker.mStaticDiffuseLightingAlphaSet = true;
    Tracker.mStaticLightingAlphaSet = true;
    pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kZeroAlpha);
    pPass->SetAlphaOutput(kPrevReg);
    pPass->SetRasSel(kRasColor0A0);

    pMat->mPasses.push_back(std::move(pPass));
}

void CMaterialLoader::SetupStaticDiffuseLightingNoBLOLStage(STevTracker& Tracker, CMaterial* pMat,
                                                            const SMP3IntermediateMaterial& Intermediate)
{
    auto pPass = std::make_unique<CMaterialPass>(pMat);
    SetMP3IntermediateIntoMaterialPass(pPass.get(), *Intermediate.GetPASS(EPASS::DIFF));

    pMat->SetTevColor(Intermediate.GetCLR(ECLR::DIFB), kColor1Reg);
    pPass->SetColorInputs(kZeroRGB, kTextureRGB, kColor1RGB, kRasRGB);
    pPass->SetColorOutput(kColor0Reg);
    Tracker.mStaticDiffuseLightingAlphaSet = true;
    pPass->SetAlphaOutput(kPrevReg);
    if (Intermediate.GetINT(EINT::BLOD) == 0)
    {
        pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kRasAlpha);
    }
    else if (Intermediate.GetINT(EINT::BLOD) == 255)
    {
        pPass->SetAlphaInputs(kZeroAlpha, kTextureAlpha, kColor1Alpha, kRasAlpha);
    }
    else
    {
        pMat->SetKonst(CColor::Integral(0, 0, 0, Intermediate.GetINT(EINT::BLOD)), Tracker.mCurKColor);
        pPass->SetKAlphaSel(ETevKSel(kKonst0_A + Tracker.mCurKColor));
        Tracker.mCurKColor += 1;
        pPass->SetAlphaInputs(kZeroAlpha, kTextureAlpha, kKonstAlpha, kRasAlpha);
    }
    Tracker.mStaticLightingAlphaSet = true;
    pPass->SetRasSel(kRasColor0A0);

    pMat->mPasses.push_back(std::move(pPass));
}

void CMaterialLoader::SetupColorTextureStage(STevTracker& Tracker, CMaterial* pMat,
                                             const SMP3IntermediateMaterial& Intermediate,
                                             bool useStageAlpha, uint8_t Alpha, bool StaticLighting)
{
    auto pPass = std::make_unique<CMaterialPass>(pMat);
    SetMP3IntermediateIntoMaterialPass(pPass.get(), *Intermediate.GetPASS(EPASS::CLR));

    bool useDynamicLightingAlpha = false;
    pPass->SetColorInputs(kZeroRGB, StaticLighting ? kColor0RGB : kRasRGB, kTextureRGB, kZeroRGB);
    pPass->SetTevColorScale(StaticLighting ? 2.f : 1.f);
    pPass->SetColorOutput(kPrevReg);

    if (useStageAlpha)
    {
        if (Alpha == 255)
        {
            pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kTextureAlpha);
        }
        else
        {
            pMat->SetKonst(CColor::Integral(0, 0, 0, Alpha), Tracker.mCurKColor);
            pPass->SetKAlphaSel(ETevKSel(kKonst0_A + Tracker.mCurKColor));
            Tracker.mCurKColor += 1;
            pPass->SetAlphaInputs(kZeroAlpha, kTextureAlpha, kKonstAlpha, kZeroAlpha);
        }
    }
    else
    {
        pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha,
                              Tracker.mStaticLightingAlphaSet ? kPrevAlpha : kRasAlpha);
        useDynamicLightingAlpha = !Tracker.mStaticLightingAlphaSet;
        Tracker.mStaticLightingAlphaSet = true;
    }
    pPass->SetAlphaOutput(kPrevReg);
    pPass->SetRasSel((useDynamicLightingAlpha || !StaticLighting) ? kRasColor0A0 : kRasColorNull);

    pMat->mPasses.push_back(std::move(pPass));
}

void CMaterialLoader::SetupColorTextureAlwaysStaticLightingStage(STevTracker& Tracker, CMaterial* pMat,
                                                                 const SMP3IntermediateMaterial& Intermediate)
{
    auto pPass = std::make_unique<CMaterialPass>(pMat);
    SetMP3IntermediateIntoMaterialPass(pPass.get(), *Intermediate.GetPASS(EPASS::CLR));

    pPass->SetColorInputs(kZeroRGB, kColor0RGB, kTextureRGB, kZeroRGB);
    pPass->SetColorOutput(kPrevReg);
    pPass->SetTevColorScale(2.f);
    pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kPrevAlpha);
    pPass->SetAlphaOutput(kPrevReg);
    pPass->SetRasSel(kRasColorNull);

    pMat->mPasses.push_back(std::move(pPass));
}

void CMaterialLoader::SetupColorKColorStage(STevTracker& Tracker, CMaterial* pMat,
                                            const SMP3IntermediateMaterial& Intermediate,
                                            bool useStageAlpha, uint8_t Alpha, bool StaticLighting)
{
    auto pPass = std::make_unique<CMaterialPass>(pMat);
    pPass->mPassType = CFourCC("CLR ");

    bool useDynamicLightingAlpha = false;
    CColor col = Intermediate.GetCLR(ECLR::CLR);
    col.A = static_cast<float>(Alpha) / 255.f;
    pMat->SetKonst(col, Tracker.mCurKColor);
    pPass->SetKColorSel(ETevKSel(kKonst0_RGB + Tracker.mCurKColor));
    pPass->SetColorInputs(kZeroRGB, StaticLighting ? kColor0RGB : kRasRGB, kKonstRGB, kZeroRGB);
    pPass->SetTevColorScale(2.f);
    pPass->SetColorOutput(kPrevReg);
    if (useStageAlpha)
    {
        pPass->SetKAlphaSel(ETevKSel(kKonst0_A + Tracker.mCurKColor));
        pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kKonstAlpha);
    }
    else
    {
        pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha,
                              Tracker.mStaticLightingAlphaSet ? kPrevAlpha : kRasAlpha);
        useDynamicLightingAlpha = !Tracker.mStaticLightingAlphaSet;
        Tracker.mStaticLightingAlphaSet = true;
    }
    pPass->SetAlphaOutput(kPrevReg);
    pPass->SetRasSel((useDynamicLightingAlpha || !StaticLighting) ? kRasColor0A0 : kRasColorNull);
    Tracker.mCurKColor += 1;

    pMat->mPasses.push_back(std::move(pPass));
}

bool CMaterialLoader::SetupTransparencyStage(STevTracker& Tracker, CMaterial* pMat,
                                             const SMP3IntermediateMaterial& Intermediate)
{
    if (const auto& IntermediateTran = Intermediate.GetPASS(EPASS::TRAN)) {
        auto pPass = std::make_unique<CMaterialPass>(pMat);
        SetMP3IntermediateIntoMaterialPass(pPass.get(), *IntermediateTran);

        if (IntermediateTran->mSettings.HasFlag(EPassSettings::InvertOpacityMap))
        {
            pPass->SetAlphaInputs(kPrevAlpha, kZeroAlpha, kTextureAlpha, kZeroAlpha);
        }
        else
        {
            pPass->SetAlphaInputs(kZeroAlpha, kPrevAlpha, kTextureAlpha, kZeroAlpha);
        }
        pPass->SetColorInputs(kZeroRGB, kZeroRGB, kZeroRGB, kPrevRGB);
        pPass->SetColorOutput(kPrevReg);
        pPass->SetAlphaOutput(kPrevReg);
        pPass->SetRasSel(kRasColorNull);
        pPass->SetTexSwapComp(3, IntermediateTran->GetSwapAlphaComp());

        pMat->mPasses.push_back(std::move(pPass));
        return true;
    }
    return false;
}

void CMaterialLoader::SetupTransparencyKAlphaMultiplyStage(STevTracker& Tracker, CMaterial* pMat,
                                                           const SMP3IntermediateMaterial& Intermediate,
                                                           bool multiplyPrevAlpha, uint8_t Alpha)
{
    if (Intermediate.GetPASS(EPASS::CLR) || Intermediate.GetPASS(EPASS::TRAN))
    {
        auto pPass = std::make_unique<CMaterialPass>(pMat);
        ETevAlphaInput argA;
        if (Alpha < 255)
        {
            pMat->SetKonst(CColor::Integral(255, 255, 255, Alpha), Tracker.mCurKColor);
            pPass->SetKAlphaSel(ETevKSel(kKonst0_A + Tracker.mCurKColor));
            if (multiplyPrevAlpha)
            {
                pPass->SetAlphaInputs(kZeroAlpha, kPrevAlpha, kKonstAlpha, kZeroAlpha);
                pPass->SetAlphaOutput(kPrevReg);
                pPass->SetRasSel(kRasColorNull);
                pMat->mPasses.push_back(std::move(pPass));
                pPass = std::make_unique<CMaterialPass>(pMat);
                argA = kPrevAlpha;
            }
            else
            {
                argA = kKonstAlpha;
            }
        }
        else
        {
            if (multiplyPrevAlpha)
            {
                argA = kPrevAlpha;
            }
            else
            {
                pPass->SetKAlphaSel(kKonstOne);
                argA = kKonstAlpha;
            }
        }
        const auto& IntermediateTran = Intermediate.GetPASS(EPASS::TRAN);
        if (IntermediateTran && IntermediateTran->mSettings.HasFlag(EPassSettings::InvertOpacityMap))
        {
            pPass->SetAlphaInputs(argA, kZeroAlpha, kTextureAlpha, kZeroAlpha);
        }
        else
        {
            pPass->SetAlphaInputs(kZeroAlpha, argA, kTextureAlpha, kZeroAlpha);
        }
        pPass->SetAlphaOutput(kPrevReg);
        if (IntermediateTran)
        {
            SetMP3IntermediateIntoMaterialPass(pPass.get(), *IntermediateTran);
            pPass->SetRasSel(kRasColorNull);
            pPass->SetTexSwapComp(3, IntermediateTran->GetSwapAlphaComp());
        }
        else
        {
            SetMP3IntermediateIntoMaterialPass(pPass.get(), *Intermediate.GetPASS(EPASS::CLR));
            pPass->SetRasSel(kRasColorNull);
        }
        Tracker.mCurKColor += 1;
        pMat->mPasses.push_back(std::move(pPass));
    }
    else
    {
        auto pPass = std::make_unique<CMaterialPass>(pMat);
        pMat->SetKonst(CColor::Integral(255, 255, 255, Alpha), Tracker.mCurKColor);
        pPass->SetKAlphaSel(ETevKSel(kKonst0_A + Tracker.mCurKColor));
        if (multiplyPrevAlpha)
        {
            pPass->SetAlphaInputs(kZeroAlpha, kKonstAlpha, kPrevAlpha, kZeroAlpha);
        }
        else
        {
            pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kKonstAlpha);
        }
        pPass->SetAlphaOutput(kPrevReg);
        pPass->SetRasSel(kRasColorNull);
        Tracker.mCurKColor += 1;
        pMat->mPasses.push_back(std::move(pPass));
    }
}

bool CMaterialLoader::SetupReflectionAlphaStage(STevTracker& Tracker, CMaterial* pMat,
                                                const SMP3IntermediateMaterial& Intermediate)
{
    if (Intermediate.mOptions.HasFlag(EMP3MaterialOption::ReflectionAlphaTarget))
    {
        if (const auto& IntermediateRfld = Intermediate.GetPASS(EPASS::RFLD))
        {
            auto pPass = std::make_unique<CMaterialPass>(pMat);
            SetMP3IntermediateIntoMaterialPass(pPass.get(), *IntermediateRfld);

            pPass->SetColorInputs(kZeroRGB, kZeroRGB, kZeroRGB, kPrevRGB);
            pPass->SetAlphaInputs(kZeroAlpha, kPrevAlpha, kTextureAlpha, kZeroAlpha);
            pPass->SetColorOutput(kPrevReg);
            pPass->SetAlphaOutput(kPrevReg);
            pPass->SetRasSel(kRasColorNull);
            pPass->SetTexSwapComp(3, 'r');

            pMat->mPasses.push_back(std::move(pPass));
            return true;
        }
    }
    return false;
}

bool CMaterialLoader::SetupReflectionStages(STevTracker& Tracker, CMaterial* pMat,
                                            const SMP3IntermediateMaterial& Intermediate,
                                            ETevColorInput argD, bool StaticLighting)
{
    if (!Intermediate.GetPASS(EPASS::RFLD) || Intermediate.mOptions.HasFlag(EMP3MaterialOption::ReflectionAlphaTarget))
        return false;

    ETevColorInput argC = kOneRGB;
    if (Intermediate.GetPASS(EPASS::RFLV) || Intermediate.GetPASS(EPASS::LRLD) || Intermediate.GetPASS(EPASS::LURD)) {
        auto pPass = std::make_unique<CMaterialPass>(pMat);
        pPass->SetColorOutput(kColor2Reg);
        pPass->SetAlphaOutput(kColor2Reg);
        if (const auto& IntermediateRflv = Intermediate.GetPASS(EPASS::RFLV))
        {
            pPass->SetColorInputs(kZeroRGB, kZeroRGB, kZeroRGB, kTextureRGB);
            pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kZeroAlpha);
            pPass->SetRasSel(kRasColorNull);
            SetMP3IntermediateIntoMaterialPass(pPass.get(), *IntermediateRflv);
        }
        else if (const auto& IntermediateLrld = Intermediate.GetPASS(EPASS::LRLD))
        {
            pPass->SetColorInputs(kZeroRGB, StaticLighting ? kColor0RGB : kRasRGB, kTextureRGB, kZeroRGB);
            pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kZeroAlpha);
            pPass->SetRasSel(StaticLighting ? kRasColorNull : kRasColor0A0);
            SetMP3IntermediateIntoMaterialPass(pPass.get(), *IntermediateLrld);
        }
        else if (const auto& IntermediateLurd = Intermediate.GetPASS(EPASS::LURD))
        {
            pPass->SetColorInputs(kZeroRGB, StaticLighting ? kColor0RGB : kRasRGB, kTextureAAA, kTextureRGB);
            pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kZeroAlpha);
            pPass->SetRasSel(StaticLighting ? kRasColorNull : kRasColor0A0);
            SetMP3IntermediateIntoMaterialPass(pPass.get(), *IntermediateLurd);
        }
        argC = kColor2RGB;
        pMat->mPasses.push_back(std::move(pPass));
    }

    if (const auto& IntermediateRfld = Intermediate.GetPASS(EPASS::RFLD))
    {
        auto pPass = std::make_unique<CMaterialPass>(pMat);
        pPass->SetColorInputs(kZeroRGB, kTextureRGB, argC, argD);
        pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kPrevAlpha);
        pPass->SetColorOutput(kPrevReg);
        pPass->SetAlphaOutput(kPrevReg);
        pPass->SetRasSel(kRasColorNull);
        SetMP3IntermediateIntoMaterialPass(pPass.get(), *IntermediateRfld);
        pMat->mPasses.push_back(std::move(pPass));
    }

    return true;
}

bool CMaterialLoader::SetupQuantizedKAlphaAdd(STevTracker& Tracker, CMaterial* pMat, uint8_t Value)
{
    auto pPass = std::make_unique<CMaterialPass>(pMat);
    pPass->SetColorInputs(kZeroRGB, kZeroRGB, kZeroRGB, kPrevRGB);
    pPass->SetKAlphaSel(KColorEighths[Value / 32]);
    pPass->SetAlphaInputs(kKonstAlpha, kZeroAlpha, kZeroAlpha, kPrevAlpha);
    pPass->SetColorOutput(kPrevReg);
    pPass->SetAlphaOutput(kPrevReg);
    pPass->SetRasSel(kRasColorNull);
    pMat->mPasses.push_back(std::move(pPass));
    return true;
}

bool CMaterialLoader::SetupIncandecenceStage(STevTracker& Tracker, CMaterial* pMat,
                                             const SMP3IntermediateMaterial& Intermediate)
{
    const uint8_t bloi = Intermediate.GetINT(EINT::BLOI);
    const auto& IntermediateInca = Intermediate.GetPASS(EPASS::INCA);
    if (!IntermediateInca)
    {
        if (bloi != 0)
            return SetupQuantizedKAlphaAdd(Tracker, pMat, bloi);

        return false;
    }

    auto pPass = std::make_unique<CMaterialPass>(pMat);
    SetMP3IntermediateIntoMaterialPass(pPass.get(), *IntermediateInca);

    /* KColor is set as the INCA mod color in game */
    pPass->SetKColorSel(kKonstOne);
    pPass->SetColorInputs(kZeroRGB, kKonstRGB, kTextureRGB, kPrevRGB);
    pPass->SetColorOutput(kPrevReg);
    pPass->SetAlphaOutput(kPrevReg);
    pPass->SetRasSel(kRasColorNull);
    if (IntermediateInca->mSettings.HasFlag(EPassSettings::BloomContribution))
    {
        pPass->SetTexSwapComp(3, IntermediateInca->GetSwapAlphaComp());
        pPass->SetKAlphaSel(KColorEighths[Intermediate.GetINT(EINT::BNIF) / 32]);
        pPass->SetAlphaInputs(kZeroAlpha, kTextureAlpha, kKonstAlpha, kPrevAlpha);
        pMat->mPasses.push_back(std::move(pPass));
        if (bloi != 0)
            SetupQuantizedKAlphaAdd(Tracker, pMat, bloi);
    }
    else
    {
        if (bloi != 0)
        {
            pPass->SetKAlphaSel(KColorEighths[bloi / 32]);
            pPass->SetAlphaInputs(kKonstAlpha, kZeroAlpha, kZeroAlpha, kPrevAlpha);
        }
        else
        {
            pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kPrevAlpha);
        }
        pMat->mPasses.push_back(std::move(pPass));
    }
    return true;
}

bool CMaterialLoader::SetupIncandecenceStageNoBloom(STevTracker& Tracker, CMaterial* pMat,
                                                    const SMP3IntermediateMaterial& Intermediate)
{
    const auto& IntermediateInca = Intermediate.GetPASS(EPASS::INCA);
    if (!IntermediateInca)
        return false;

    auto pPass = std::make_unique<CMaterialPass>(pMat);
    SetMP3IntermediateIntoMaterialPass(pPass.get(), *IntermediateInca);

    /* KColor is set as the INCA mod color in game */
    pPass->SetKColorSel(kKonstOne);
    pPass->SetColorInputs(kZeroRGB, kKonstRGB, kTextureRGB, kPrevRGB);
    pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kPrevAlpha);
    pPass->SetColorOutput(kPrevReg);
    pPass->SetAlphaOutput(kPrevReg);
    pPass->SetRasSel(kRasColorNull);
    pMat->mPasses.push_back(std::move(pPass));

    return true;
}

bool CMaterialLoader::SetupStartingIncandecenceStage(STevTracker& Tracker, CMaterial* pMat,
                                                     const SMP3IntermediateMaterial& Intermediate)
{
    bool needsBloiAdd = false;
    const uint8_t bloi = Intermediate.GetINT(EINT::BLOI);
    const auto& IntermediateInca = Intermediate.GetPASS(EPASS::INCA);
    if (!IntermediateInca)
    {
        if (bloi != 0)
        {
            auto pPass = std::make_unique<CMaterialPass>(pMat);
            pPass->SetColorInputs(kZeroRGB, kZeroRGB, kZeroRGB, kZeroRGB);
            pPass->SetKAlphaSel(KColorEighths[bloi / 32]);
            pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kKonstAlpha);
            pPass->SetColorOutput(kPrevReg);
            pPass->SetAlphaOutput(kPrevReg);
            pPass->SetRasSel(kRasColorNull);
            pMat->mPasses.push_back(std::move(pPass));
            return true;
        }
        return false;
    }

    auto pPass = std::make_unique<CMaterialPass>(pMat);
    SetMP3IntermediateIntoMaterialPass(pPass.get(), *IntermediateInca);

    /* KColor is set as the INCA mod color in game */
    pPass->SetKColorSel(kKonstOne);
    pPass->SetColorInputs(kZeroRGB, kTextureRGB, kKonstRGB, kZeroRGB);
    if (IntermediateInca->mSettings.HasFlag(EPassSettings::BloomContribution))
    {
        pPass->SetTexSwapComp(3, IntermediateInca->GetSwapAlphaComp());
        const uint8_t bnif = Intermediate.GetINT(EINT::BNIF);
        if (bloi != 0 && !bnif)
        {
            pPass->SetKAlphaSel(KColorEighths[bloi / 32]);
            pPass->SetAlphaInputs(kKonstAlpha, kZeroAlpha, kZeroAlpha, kTextureAlpha);
        }
        else
        {
            pPass->SetKAlphaSel(KColorEighths[bnif / 32]);
            pPass->SetAlphaInputs(kZeroAlpha, kTextureAlpha, kKonstAlpha, kZeroAlpha);
            needsBloiAdd = bloi != 0;
        }
    }
    else
    {
        if (bloi != 0)
        {
            pPass->SetKAlphaSel(KColorEighths[bloi / 32]);
            pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kKonstAlpha);
        }
        else
        {
            pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kZeroAlpha);
        }
    }
    pPass->SetColorOutput(kPrevReg);
    pPass->SetAlphaOutput(kPrevReg);
    pPass->SetRasSel(kRasColorNull);
    pMat->mPasses.push_back(std::move(pPass));
    if (needsBloiAdd)
        SetupQuantizedKAlphaAdd(Tracker, pMat, bloi);
    return true;
}

void CMaterialLoader::SetupStartingIncandecenceDynamicKColorStage(STevTracker& Tracker, CMaterial* pMat,
                                                                  const SMP3IntermediateMaterial& Intermediate)
{
    auto pPass = std::make_unique<CMaterialPass>(pMat);
    SetMP3IntermediateIntoMaterialPass(pPass.get(), *Intermediate.GetPASS(EPASS::INCA));

    pPass->SetKColorSel(kKonstOne);
    pPass->SetKAlphaSel(kKonstOne);
    pPass->SetColorInputs(kZeroRGB, kKonstRGB, kTextureRGB, kZeroRGB);
    pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kKonstAlpha);
    pPass->SetColorOutput(kPrevReg);
    pPass->SetAlphaOutput(kPrevReg);
    pPass->SetRasSel(kRasColorNull);

    pMat->mPasses.push_back(std::move(pPass));
}

bool CMaterialLoader::SetupStaticBloomLightingStage(STevTracker& Tracker, CMaterial* pMat,
                                                    const SMP3IntermediateMaterial& Intermediate, bool StaticLighting)
{
    const auto& IntermediateBlol = Intermediate.GetPASS(EPASS::BLOL);
    if (!StaticLighting || !IntermediateBlol)
        return false;
    auto pPass = std::make_unique<CMaterialPass>(pMat);
    SetMP3IntermediateIntoMaterialPass(pPass.get(), *IntermediateBlol);

    pPass->SetTexSwapComp(3, IntermediateBlol->GetSwapAlphaComp());
    pPass->SetColorOutput(kPrevReg);
    pPass->SetAlphaOutput(kPrevReg);
    pPass->SetColorInputs(kZeroRGB, kZeroRGB, kZeroRGB, kPrevRGB);
    pPass->SetAlphaInputs(kZeroAlpha, kPrevAlpha, kTextureAlpha, kZeroAlpha);
    pPass->SetRasSel(kRasColorNull);

    pMat->mPasses.push_back(std::move(pPass));
    return true;
}

bool CMaterialLoader::SetupStaticBloomLightingA1Stages(STevTracker& Tracker, CMaterial* pMat,
                                                       const SMP3IntermediateMaterial& Intermediate)
{
    const auto& IntermediateBlol = Intermediate.GetPASS(EPASS::BLOL);
    auto pPass = std::make_unique<CMaterialPass>(pMat);

    const ETevAlphaInput argC = IntermediateBlol ? kTextureAlpha : kZeroAlpha;
    pPass->SetAlphaInputs(kZeroAlpha, kColor1Alpha, argC, kRasAlpha);
    pPass->SetAlphaOutput(kPrevReg);
    if (IntermediateBlol)
    {
        pPass->SetTexSwapComp(3, IntermediateBlol->GetSwapAlphaComp());
        SetMP3IntermediateIntoMaterialPass(pPass.get(), *IntermediateBlol);
        pPass->SetRasSel(kRasColor0A0);
    }
    else
    {
        pPass->SetRasSel(kRasColor0A0);
    }
    Tracker.mStaticLightingAlphaSet = true;
    pMat->mPasses.push_back(std::move(pPass));
    SetupStaticBloomDiffuseLightingStages(Tracker, pMat, Intermediate, true);
    return true;
}

bool CMaterialLoader::SetupStaticBloomDiffuseLightingStages(STevTracker& Tracker, CMaterial* pMat,
                                                            const SMP3IntermediateMaterial& Intermediate,
                                                            bool StaticLighting)
{
    if (!StaticLighting)
    {
        Tracker.mStaticDiffuseLightingAlphaSet = true;
        return false;
    }
    bool ret = false;
    bool useDynamicAlpha = false;
    const auto& IntermediateBlod = Intermediate.GetPASS(EPASS::BLOD);
    if (!IntermediateBlod)
    {
        useDynamicAlpha = !Tracker.mStaticDiffuseLightingAlphaSet;
    }
    else
    {
        auto pPass = std::make_unique<CMaterialPass>(pMat);
        SetMP3IntermediateIntoMaterialPass(pPass.get(), *IntermediateBlod);

        pPass->SetTexSwapComp(3, IntermediateBlod->GetSwapAlphaComp());
        pPass->SetColorOutput(kPrevReg);
        pPass->SetAlphaOutput(kPrevReg);
        pPass->SetColorInputs(kZeroRGB, kZeroRGB, kZeroRGB, kPrevRGB);
        const ETevAlphaInput argC = Tracker.mStaticLightingAlphaSet ? kPrevAlpha : kRasAlpha;
        if (!StaticLighting)
        {
            if (Intermediate.GetINT(EINT::BLOD) == 255 || Tracker.mStaticDiffuseLightingAlphaSet)
            {
                pPass->SetAlphaInputs(kZeroAlpha, kTextureAlpha, argC, kZeroAlpha);
                Tracker.mStaticDiffuseLightingAlphaSet = true;
            }
            else if (Intermediate.GetINT(EINT::BLOD) == 0)
            {
                pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kZeroAlpha);
                Tracker.mStaticDiffuseLightingAlphaSet = true;
            }
            else
            {
                pPass->SetAlphaInputs(kZeroAlpha, kTextureAlpha, argC, kZeroAlpha);
                useDynamicAlpha = !Tracker.mStaticDiffuseLightingAlphaSet;
            }
            Tracker.mStaticLightingAlphaSet = true;
        }
        else
        {
            useDynamicAlpha = !Tracker.mStaticDiffuseLightingAlphaSet;
            pPass->SetAlphaInputs(kZeroAlpha, kPrevAlpha, kTextureAlpha, kZeroAlpha);
        }
        pPass->SetRasSel(argC == kRasAlpha ? kRasColor0A0 : kRasColorNull);
        pMat->mPasses.push_back(std::move(pPass));
        ret = true;
    }
    if (useDynamicAlpha)
    {
        if (Intermediate.GetINT(EINT::BLOD) != 255)
        {
            auto pPass = std::make_unique<CMaterialPass>(pMat);

            pPass->SetColorOutput(kPrevReg);
            pPass->SetAlphaOutput(kPrevReg);
            pPass->SetColorInputs(kZeroRGB, kZeroRGB, kZeroRGB, kPrevRGB);
            if (Intermediate.GetINT(EINT::BLOD) == 0)
            {
                pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kZeroAlpha);
                pPass->SetRasSel(kRasColorNull);
            }
            else
            {
                const ETevAlphaInput argB = Tracker.mStaticLightingAlphaSet ? kPrevAlpha : kRasAlpha;
                pPass->SetKAlphaSel(KColorEighths[Intermediate.GetINT(EINT::BLOD) / 32]);
                pPass->SetAlphaInputs(kZeroAlpha, argB, kKonstAlpha, kZeroAlpha);
                pPass->SetRasSel(argB == kPrevAlpha ? kRasColor0A0 : kRasColorNull);
                Tracker.mStaticLightingAlphaSet = true;
            }
            pMat->mPasses.push_back(std::move(pPass));
            ret = true;
        }
        Tracker.mStaticDiffuseLightingAlphaSet = true;
    }
    return ret;
}

bool CMaterialLoader::SetupStaticBloomIncandecenceLightingStage(STevTracker& Tracker, CMaterial* pMat,
                                                                const SMP3IntermediateMaterial& Intermediate)
{
    const auto& IntermediateBloi = Intermediate.GetPASS(EPASS::BLOI);
    if (!IntermediateBloi)
        return false;
    auto pPass = std::make_unique<CMaterialPass>(pMat);
    SetMP3IntermediateIntoMaterialPass(pPass.get(), *IntermediateBloi);

    pPass->SetColorInputs(kZeroRGB, kZeroRGB, kZeroRGB, kPrevRGB);
    pPass->SetColorOutput(kPrevReg);
    pPass->SetAlphaOutput(kPrevReg);
    pPass->SetRasSel(kRasColorNull);
    pPass->SetTexSwapComp(3, IntermediateBloi->GetSwapAlphaComp());
    pPass->SetAlphaInputs(kTextureAlpha, kZeroAlpha, kZeroAlpha, kPrevAlpha);

    pMat->mPasses.push_back(std::move(pPass));
    return true;
}

void CMaterialLoader::SetupNoBloomTransparent(CMaterial* pMat, const SMP3IntermediateMaterial& Intermediate,
                                              uint8_t Alpha)
{
    STevTracker Tracker;
    const bool StaticLighting = SetupStaticDiffuseLightingStage(Tracker, pMat, Intermediate, true);
    if (Intermediate.GetPASS(EPASS::CLR))
    {
        SetupColorTextureStage(Tracker, pMat, Intermediate, true, Alpha, StaticLighting);
    }
    else
    {
        SetupColorKColorStage(Tracker, pMat, Intermediate, true, Alpha, StaticLighting);
    }
    SetupTransparencyStage(Tracker, pMat, Intermediate);
    SetupReflectionAlphaStage(Tracker, pMat, Intermediate);
    SetupReflectionStages(Tracker, pMat, Intermediate, kPrevRGB, StaticLighting);
    pMat->mOptions.AssignFlag(EMaterialOption::DepthWrite, false);
    pMat->mOptions.AssignFlag(EMaterialOption::Masked, false);
    pMat->SetBlendMode(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    pMat->mOptions.AssignFlag(EMaterialOption::Transparent, true);
    pMat->mOptions.AssignFlag(EMaterialOption::AlphaWrite, false);
}

void CMaterialLoader::SetupNoBloomAdditiveIncandecence(CMaterial* pMat, const SMP3IntermediateMaterial& Intermediate,
                                                       uint8_t Alpha)
{
    SetupNoBloomTransparent(pMat, Intermediate, Alpha);
    pMat->mpNextDrawPassMaterial = std::make_unique<CMaterial>(pMat->Version(), pMat->VtxDesc());
    pMat = pMat->mpNextDrawPassMaterial.get();
    STevTracker Tracker;
    SetupStartingIncandecenceDynamicKColorStage(Tracker, pMat, Intermediate);
    pMat->mOptions.AssignFlag(EMaterialOption::DepthWrite, false);
    pMat->mOptions.AssignFlag(EMaterialOption::Masked, false);
    pMat->SetBlendMode(GL_SRC_ALPHA, GL_ONE);
    pMat->mOptions.AssignFlag(EMaterialOption::Transparent, true);
    pMat->mOptions.AssignFlag(EMaterialOption::AlphaWrite, false);
}

void CMaterialLoader::SetupFullRenderOpaque(CMaterial* pMat, const SMP3IntermediateMaterial& Intermediate)
{
    pMat->mOptions.AssignFlag(EMaterialOption::AlphaWrite, true);
    STevTracker Tracker;
    const bool StaticLighting = SetupStaticDiffuseLightingStage(Tracker, pMat, Intermediate, false);
    SetupStaticBloomLightingStage(Tracker, pMat, Intermediate, StaticLighting);
    SetupStaticBloomDiffuseLightingStages(Tracker, pMat, Intermediate, StaticLighting);
    if (Intermediate.GetPASS(EPASS::CLR))
    {
        SetupColorTextureStage(Tracker, pMat, Intermediate, false, 255, StaticLighting);
    }
    else
    {
        SetupColorKColorStage(Tracker, pMat, Intermediate, false, 255, StaticLighting);
    }
    SetupReflectionStages(Tracker, pMat, Intermediate, kPrevRGB, StaticLighting);
    SetupIncandecenceStage(Tracker, pMat, Intermediate);
    SetupStaticBloomIncandecenceLightingStage(Tracker, pMat, Intermediate);
    pMat->mOptions.AssignFlag(EMaterialOption::DepthWrite, true);
    pMat->mOptions.AssignFlag(EMaterialOption::Masked, false);
    pMat->SetBlendMode(GL_ONE, GL_ZERO);
    pMat->mOptions.AssignFlag(EMaterialOption::Transparent, false);
}

void CMaterialLoader::SetupOptimizedDiffuseLightingColorOpaque(CMaterial* pMat,
                                                               const SMP3IntermediateMaterial& Intermediate)
{
    pMat->mOptions.AssignFlag(EMaterialOption::AlphaWrite, true);
    STevTracker Tracker;
    SetupStaticDiffuseLightingNoBloomStage(Tracker, pMat, Intermediate);
    SetupColorTextureAlwaysStaticLightingStage(Tracker, pMat, Intermediate);
    pMat->mOptions.AssignFlag(EMaterialOption::DepthWrite, true);
    pMat->mOptions.AssignFlag(EMaterialOption::Masked, false);
    pMat->SetBlendMode(GL_ONE, GL_ZERO);
    pMat->mOptions.AssignFlag(EMaterialOption::Transparent, false);
}

void CMaterialLoader::SetupOptimizedDiffuseBloomLightingColorOpaque(CMaterial* pMat,
                                                                    const SMP3IntermediateMaterial& Intermediate)
{
    pMat->mOptions.AssignFlag(EMaterialOption::AlphaWrite, true);
    STevTracker Tracker;
    SetupStaticDiffuseLightingNoBLOLStage(Tracker, pMat, Intermediate);
    SetupStaticBloomLightingStage(Tracker, pMat, Intermediate, true);
    SetupColorTextureAlwaysStaticLightingStage(Tracker, pMat, Intermediate);
    pMat->mOptions.AssignFlag(EMaterialOption::DepthWrite, true);
    pMat->mOptions.AssignFlag(EMaterialOption::Masked, false);
    pMat->SetBlendMode(GL_ONE, GL_ZERO);
    pMat->mOptions.AssignFlag(EMaterialOption::Transparent, false);
}

void CMaterialLoader::SetupAdditiveIncandecenceOnly(CMaterial* pMat,
                                                    const SMP3IntermediateMaterial& Intermediate)
{
    pMat->mOptions.AssignFlag(EMaterialOption::AlphaWrite, true);
    STevTracker Tracker;
    SetupStartingIncandecenceStage(Tracker, pMat, Intermediate);
    SetupStaticBloomIncandecenceLightingStage(Tracker, pMat, Intermediate);
    SetupReflectionStages(Tracker, pMat, Intermediate, kPrevRGB, false);
    pMat->mOptions.AssignFlag(EMaterialOption::DepthWrite, false);
    pMat->mOptions.AssignFlag(EMaterialOption::Masked, false);
    pMat->SetBlendMode(GL_ONE, GL_ONE);
    pMat->mOptions.AssignFlag(EMaterialOption::Transparent, true);
}

CMaterial* CMaterialLoader::SetupFullRenderTransparent(
    CMaterial* pMat, const SMP3IntermediateMaterial& Intermediate, uint8_t Alpha)
{
    STevTracker Tracker;
    const bool StaticLighting = SetupStaticDiffuseLightingStage(Tracker, pMat, Intermediate, true);
    if (Intermediate.GetPASS(EPASS::CLR))
    {
        SetupColorTextureStage(Tracker, pMat, Intermediate, true, Alpha, StaticLighting);
    }
    else
    {
        SetupColorKColorStage(Tracker, pMat, Intermediate, true, Alpha, StaticLighting);
    }
    SetupTransparencyStage(Tracker, pMat, Intermediate);
    SetupReflectionAlphaStage(Tracker, pMat, Intermediate);
    SetupReflectionStages(Tracker, pMat, Intermediate, kPrevRGB, StaticLighting);
    pMat->mOptions.AssignFlag(EMaterialOption::DepthWrite, false);
    pMat->mOptions.AssignFlag(EMaterialOption::Masked, false);
    pMat->SetBlendMode(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    pMat->mOptions.AssignFlag(EMaterialOption::Transparent, true);
    pMat->mOptions.AssignFlag(EMaterialOption::AlphaWrite, false);

    pMat->mpNextDrawPassMaterial = std::make_unique<CMaterial>(pMat->Version(), pMat->VtxDesc());
    pMat = pMat->mpNextDrawPassMaterial.get();
    Tracker = STevTracker();
    pMat->mOptions.AssignFlag(EMaterialOption::AlphaWrite, true);
    SetupTransparencyKAlphaMultiplyStage(Tracker, pMat, Intermediate, false, Alpha);
    pMat->mOptions.AssignFlag(EMaterialOption::DepthWrite, false);
    pMat->mOptions.AssignFlag(EMaterialOption::Masked, false);
    pMat->SetBlendMode(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
    pMat->mOptions.AssignFlag(EMaterialOption::Transparent, true);
    pMat->mOptions.AssignFlag(EMaterialOption::ColorWrite, false);

    pMat->mpNextDrawPassMaterial = std::make_unique<CMaterial>(pMat->Version(), pMat->VtxDesc());
    pMat = pMat->mpNextDrawPassMaterial.get();
    Tracker = STevTracker();
    if (SetupStaticBloomLightingA1Stages(Tracker, pMat, Intermediate))
    {
        SetupTransparencyKAlphaMultiplyStage(Tracker, pMat, Intermediate, true, Alpha);
        pMat->mOptions.AssignFlag(EMaterialOption::DepthWrite, false);
        pMat->mOptions.AssignFlag(EMaterialOption::Masked, false);
        pMat->SetBlendMode(GL_ONE, GL_ONE);
        pMat->mOptions.AssignFlag(EMaterialOption::Transparent, true);
        pMat->mOptions.AssignFlag(EMaterialOption::ColorWrite, true);
    }

    return pMat;
}

void CMaterialLoader::SetupFullRenderTransparentAdditiveIncandecence(CMaterial* pMat,
                                                                     const SMP3IntermediateMaterial& Intermediate,
                                                                     uint8_t Alpha)
{
    pMat = SetupFullRenderTransparent(pMat, Intermediate, Alpha);
    pMat->mpNextDrawPassMaterial = std::make_unique<CMaterial>(pMat->Version(), pMat->VtxDesc());
    pMat = pMat->mpNextDrawPassMaterial.get();

    STevTracker Tracker;
    SetupStartingIncandecenceStage(Tracker, pMat, Intermediate);
    SetupStaticBloomIncandecenceLightingStage(Tracker, pMat, Intermediate);
    pMat->mOptions.AssignFlag(EMaterialOption::DepthWrite, false);
    pMat->mOptions.AssignFlag(EMaterialOption::Masked, false);
    pMat->SetBlendMode(GL_ONE, GL_ONE);
    pMat->mOptions.AssignFlag(EMaterialOption::Transparent, true);
}

void CMaterialLoader::SetupMaterialAlphaCompare(CMaterial* pMat, const SMP3IntermediateMaterial& Intermediate)
{
    STevTracker Tracker;
    const bool StaticLighting = SetupStaticDiffuseLightingStage(Tracker, pMat, Intermediate, true);
    if (Intermediate.GetPASS(EPASS::CLR))
    {
        SetupColorTextureStage(Tracker, pMat, Intermediate, false, 255, StaticLighting);
    }
    else
    {
        SetupColorKColorStage(Tracker, pMat, Intermediate, false, 255, StaticLighting);
    }
    SetupTransparencyStage(Tracker, pMat, Intermediate);
    SetupReflectionAlphaStage(Tracker, pMat, Intermediate);
    SetupReflectionStages(Tracker, pMat, Intermediate, kPrevRGB, StaticLighting);
    SetupIncandecenceStageNoBloom(Tracker, pMat, Intermediate);
    pMat->mOptions.AssignFlag(EMaterialOption::DepthWrite, true);
    pMat->mOptions.AssignFlag(EMaterialOption::Masked, true);
    pMat->SetBlendMode(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    pMat->mOptions.AssignFlag(EMaterialOption::Transparent, true);
    pMat->mOptions.AssignFlag(EMaterialOption::ZeroDestAlpha, true);
}

void CMaterialLoader::SetupSolidWhite(CMaterial* pMat, const SMP3IntermediateMaterial& Intermediate)
{
    auto pPass = std::make_unique<CMaterialPass>(pMat);
    pPass->SetColorInputs(kZeroRGB, kZeroRGB, kZeroRGB, kOneRGB);
    pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kZeroAlpha);
    pPass->SetColorOutput(kPrevReg);
    pPass->SetAlphaOutput(kPrevReg);
    pPass->SetRasSel(kRasColorNull);
    pMat->mPasses.push_back(std::move(pPass));
    pMat->mOptions.AssignFlag(EMaterialOption::DepthWrite, false);
    pMat->SetBlendMode(GL_ONE, GL_ZERO);
    pMat->mOptions.AssignFlag(EMaterialOption::Transparent, false);
    pMat->mOptions.AssignFlag(EMaterialOption::Masked, false);
}

void CMaterialLoader::SetupSolidKColorKAlpha(CMaterial* pMat, const SMP3IntermediateMaterial& Intermediate)
{
    auto pPass = std::make_unique<CMaterialPass>(pMat);
    pMat->SetKonst(Intermediate.GetCLR(ECLR::CLR), 0);
    pPass->SetKColorSel(kKonst0_RGB);
    pPass->SetKAlphaSel(kKonst0_A);
    pPass->SetColorInputs(kZeroRGB, kZeroRGB, kZeroRGB, kKonstRGB);
    pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kKonstAlpha);
    pPass->SetColorOutput(kPrevReg);
    pPass->SetAlphaOutput(kPrevReg);
    pPass->SetRasSel(kRasColorNull);
    pMat->mPasses.push_back(std::move(pPass));
    pMat->mOptions.AssignFlag(EMaterialOption::DepthWrite, false);
    pMat->mOptions.AssignFlag(EMaterialOption::Masked, false);
    pMat->SetBlendMode(GL_SRC_ALPHA, GL_ONE);
    pMat->mOptions.AssignFlag(EMaterialOption::Transparent, true);
}

void CMaterialLoader::CreateCorruptionPasses(CMaterial* pMat, const SMP3IntermediateMaterial& Intermediate, bool Bloom)
{
    pMat->mOptions.AssignFlag(EMaterialOption::Occluder,
                              Intermediate.mOptions.HasFlag(EMP3MaterialOption::Occluder));
    pMat->mOptions.AssignFlag(EMaterialOption::DrawWhiteAmbientDKCR,
                              Intermediate.mOptions.HasFlag(EMP3MaterialOption::DrawWhiteAmbientDKCR));

    EMP3RenderConfig Config;
    uint8_t Alpha;
    SelectBestCombinerConfig(Config, Alpha, Intermediate, Bloom);

    switch (Config)
    {
    case EMP3RenderConfig::NoBloomTransparent:
        SetupNoBloomTransparent(pMat, Intermediate, Alpha); break;
    case EMP3RenderConfig::NoBloomAdditiveIncandecence:
        SetupNoBloomAdditiveIncandecence(pMat, Intermediate, Alpha); break;
    case EMP3RenderConfig::FullRenderOpaque:
        SetupFullRenderOpaque(pMat, Intermediate); break;
    case EMP3RenderConfig::OptimizedDiffuseLightingColorOpaque:
        SetupOptimizedDiffuseLightingColorOpaque(pMat, Intermediate); break;
    case EMP3RenderConfig::OptimizedDiffuseBloomLightingColorOpaque:
        SetupOptimizedDiffuseBloomLightingColorOpaque(pMat, Intermediate); break;
    case EMP3RenderConfig::AdditiveIncandecenceOnly:
        SetupAdditiveIncandecenceOnly(pMat, Intermediate); break;
    case EMP3RenderConfig::FullRenderTransparent:
    default:
        SetupFullRenderTransparent(pMat, Intermediate, Alpha); break;
    case EMP3RenderConfig::FullRenderTransparentAdditiveIncandecence:
        SetupFullRenderTransparentAdditiveIncandecence(pMat, Intermediate, Alpha); break;
    case EMP3RenderConfig::MaterialAlphaCompare:
        SetupMaterialAlphaCompare(pMat, Intermediate); break;
    case EMP3RenderConfig::SolidWhite:
        SetupSolidWhite(pMat, Intermediate); break;
    case EMP3RenderConfig::SolidKColorKAlpha:
        SetupSolidKColorKAlpha(pMat, Intermediate); break;
    }
}

std::unique_ptr<CMaterial> CMaterialLoader::LoadAssimpMaterial(const aiMaterial *pAiMat)
{
    // todo: generate new material using import values.
    auto pMat = std::make_unique<CMaterial>(mVersion, EVertexAttribute::None);

    aiString Name;
    pAiMat->Get(AI_MATKEY_NAME, Name);
    pMat->SetName(Name.C_Str());

    // Create generic custom pass that uses Konst color
    auto pPass = std::make_unique<CMaterialPass>(pMat.get());
    pPass->SetColorInputs(kZeroRGB, kRasRGB, kKonstRGB, kZeroRGB);
    pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kKonstAlpha);
    pPass->SetKColorSel(kKonst0_RGB);
    pPass->SetKAlphaSel(kKonstOne);
    pPass->SetRasSel(kRasColor0A0);
    pMat->mKonstColors[0] = CColor::RandomLightColor(false);
    pMat->mPasses.push_back(std::move(pPass));

    return pMat;
}

// ************ STATIC ************
CMaterialSet* CMaterialLoader::LoadMaterialSet(IInputStream& rMat, EGame Version, CResourceStore* resourceStore)
{
    CMaterialLoader Loader;
    Loader.mpSet = new CMaterialSet();
    Loader.mpFile = &rMat;
    Loader.mVersion = Version;

    if ((Version >= EGame::PrimeDemo) && (Version <= EGame::Echoes))
        Loader.ReadPrimeMatSet(resourceStore);
    else
        Loader.ReadCorruptionMatSet(resourceStore);

    return Loader.mpSet;
}

CMaterialSet* CMaterialLoader::ImportAssimpMaterials(const aiScene *pScene, EGame TargetVersion)
{
    CMaterialLoader Loader;
    Loader.mVersion = TargetVersion;

    CMaterialSet *pOut = new CMaterialSet();
    pOut->mMaterials.reserve(pScene->mNumMaterials);

    for (uint32_t iMat = 0; iMat < pScene->mNumMaterials; iMat++)
    {
        pOut->mMaterials.push_back(Loader.LoadAssimpMaterial(pScene->mMaterials[iMat]));
    }

    return pOut;
}
