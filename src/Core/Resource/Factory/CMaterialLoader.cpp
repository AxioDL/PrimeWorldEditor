#include "CMaterialLoader.h"
#include "Core/GameProject/CResourceStore.h"
#include "Core/OpenGL/GLCommon.h"
#include <Common/Log.h>
#include <iostream>
#include <iomanip>

CMaterialLoader::CMaterialLoader()
    : mCorruptionFlags(0)
    , mHasOPAC(false)
{
}

CMaterialLoader::~CMaterialLoader()
{
}

FVertexDescription CMaterialLoader::ConvertToVertexDescription(uint32 VertexFlags)
{
    FVertexDescription Desc;
    if (VertexFlags & 0x00000003) Desc |= EVertexAttribute::Position;
    if (VertexFlags & 0x0000000C) Desc |= EVertexAttribute::Normal;
    if (VertexFlags & 0x00000030) Desc |= EVertexAttribute::Color0;
    if (VertexFlags & 0x000000C0) Desc |= EVertexAttribute::Color1;
    if (VertexFlags & 0x00000300) Desc |= EVertexAttribute::Tex0;
    if (VertexFlags & 0x00000C00) Desc |= EVertexAttribute::Tex1;
    if (VertexFlags & 0x00003000) Desc |= EVertexAttribute::Tex2;
    if (VertexFlags & 0x0000C000) Desc |= EVertexAttribute::Tex3;
    if (VertexFlags & 0x00030000) Desc |= EVertexAttribute::Tex4;
    if (VertexFlags & 0x000C0000) Desc |= EVertexAttribute::Tex5;
    if (VertexFlags & 0x00300000) Desc |= EVertexAttribute::Tex6;
    if (VertexFlags & 0x00C00000) Desc |= EVertexAttribute::Tex7;
    if (VertexFlags & 0x01000000) Desc |= EVertexAttribute::PosMtx;
    if (VertexFlags & 0x02000000) Desc |= EVertexAttribute::Tex0Mtx;
    if (VertexFlags & 0x04000000) Desc |= EVertexAttribute::Tex1Mtx;
    if (VertexFlags & 0x08000000) Desc |= EVertexAttribute::Tex2Mtx;
    if (VertexFlags & 0x10000000) Desc |= EVertexAttribute::Tex3Mtx;
    if (VertexFlags & 0x20000000) Desc |= EVertexAttribute::Tex4Mtx;
    if (VertexFlags & 0x40000000) Desc |= EVertexAttribute::Tex5Mtx;
    if (VertexFlags & 0x80000000) Desc |= EVertexAttribute::Tex6Mtx;
    return Desc;
}

void CMaterialLoader::ReadPrimeMatSet()
{
    // Textures
    uint32 NumTextures = mpFile->ReadLong();
    mTextures.resize(NumTextures);

    for (uint32 iTex = 0; iTex < NumTextures; iTex++)
    {
        uint32 TextureID = mpFile->ReadLong();
        mTextures[iTex] = gpResourceStore->LoadResource<CTexture>(TextureID);
    }

    // Materials
    uint32 NumMats = mpFile->ReadLong();
    std::vector<uint32> Offsets(NumMats);
    for (uint32 iMat = 0; iMat < NumMats; iMat++)
        Offsets[iMat] = mpFile->ReadLong();

    uint32 MatsStart = mpFile->Tell();
    mpSet->mMaterials.resize(NumMats);
    for (uint32 iMat = 0; iMat < NumMats; iMat++)
    {
        mpSet->mMaterials[iMat] = ReadPrimeMaterial();
        mpSet->mMaterials[iMat]->mVersion = mVersion;
        mpSet->mMaterials[iMat]->mName = TString("Material #") + TString::FromInt32(iMat + 1, 0, 10);
        mpFile->Seek(MatsStart + Offsets[iMat], SEEK_SET);
    }
}

CMaterial* CMaterialLoader::ReadPrimeMaterial()
{
    CMaterial *pMat = new CMaterial();
    pMat->mEnableBloom = false;

    // Flags
    pMat->mOptions = (mpFile->ReadLong() & (uint) EMaterialOption::AllMP1Settings);

    // Textures
    uint32 NumTextures = mpFile->ReadLong();
    std::vector<uint32> TextureIndices(NumTextures);

    for (uint32 iTex = 0; iTex < NumTextures; iTex++)
    {
        uint32 Index = mpFile->ReadLong();
        TextureIndices[iTex] = Index;
    }

    // Vertex description
    pMat->mVtxDesc = ConvertToVertexDescription( mpFile->ReadLong() );

    // Unknowns
    if (mVersion >= EGame::EchoesDemo)
    {
        pMat->mEchoesUnknownA = mpFile->ReadLong();
        pMat->mEchoesUnknownB = mpFile->ReadLong();
    }
    mpFile->Seek(0x4, SEEK_CUR); // Skipping group index

    // Konst
    if (pMat->mOptions & EMaterialOption::Konst)
    {
        uint32 KonstCount = mpFile->ReadLong();

        for (uint32 iKonst = 0; iKonst < KonstCount; iKonst++)
        {
            if (iKonst >= 4) break;
            pMat->mKonstColors[iKonst] = CColor(*mpFile, true);
        }
        if (KonstCount > 4) mpFile->Seek(0x4 * (KonstCount - 4), SEEK_CUR);
    }

    // Blend mode
    pMat->mBlendDstFac = gBlendFactor[mpFile->ReadShort()];
    pMat->mBlendSrcFac = gBlendFactor[mpFile->ReadShort()];

    // Indirect texture
    if (pMat->mOptions & EMaterialOption::IndStage)
    {
        uint32 IndTexIndex = mpFile->ReadLong();
        pMat->mpIndirectTexture = mTextures[TextureIndices[IndTexIndex]];
    }

    // Color channels
    uint32 ChanCount = mpFile->ReadLong();
    pMat->mLightingEnabled = ((mpFile->ReadLong() & 0x1) == 1);
    mpFile->Seek((4 * ChanCount) - 4, SEEK_CUR);

    // TEV
    uint32 TevCount = mpFile->ReadLong();
    pMat->mPasses.resize(TevCount);

    for (uint32 iTev = 0; iTev < TevCount; iTev++)
    {
        CMaterialPass *pPass = new CMaterialPass(pMat);

        uint32 ColorIn = mpFile->ReadLong();
        uint32 AlphaIn = mpFile->ReadLong();
        pPass->mColorOutput = (ETevOutput) ((mpFile->ReadLong() & 0x600) >> 9);
        pPass->mAlphaOutput = (ETevOutput) ((mpFile->ReadLong() & 0x600) >> 9);
        mpFile->Seek(0x1, SEEK_CUR); // Padding byte
        pPass->mKAlphaSel = (ETevKSel) mpFile->ReadByte();
        pPass->mKColorSel = (ETevKSel) mpFile->ReadByte();
        pPass->mRasSel = (ETevRasSel) (uint8) mpFile->ReadByte();

        for (uint32 iInput = 0; iInput < 4; iInput++)
        {
            pPass->mColorInputs[iInput] = (ETevColorInput) ((ColorIn >> (iInput * 5)) & 0xF);
            pPass->mAlphaInputs[iInput] = (ETevAlphaInput) ((AlphaIn >> (iInput * 5)) & 0x7);
        }

        pMat->mPasses[iTev] = pPass;
    }

    std::vector<uint8> TevCoordIndices(TevCount);
    for (uint32 iTev = 0; iTev < TevCount; iTev++)
    {
        mpFile->Seek(0x2, SEEK_CUR);
        CMaterialPass *pPass = pMat->Pass(iTev);

        uint8 TexSel = mpFile->ReadByte();

        if ((TexSel == 0xFF) || (TexSel >= TextureIndices.size()))
            pPass->mpTexture = nullptr;
        else
            pPass->mpTexture = mTextures[TextureIndices[TexSel]];

        TevCoordIndices[iTev] = mpFile->ReadByte();
    }

    // TexGens
    uint32 TexGenCount = mpFile->ReadLong();
    std::vector<uint32> TexGens(TexGenCount);

    for (uint32 iTex = 0; iTex < TexGenCount; iTex++)
        TexGens[iTex] = mpFile->ReadLong();

    // UV animations
    mpFile->Seek(0x4, SEEK_CUR); // Skipping UV anims size
    uint32 NumAnims = mpFile->ReadLong();

    struct SUVAnim {
        int32 Mode; float Params[4];
    };
    std::vector <SUVAnim> Anims(NumAnims);

    for (uint32 iAnim = 0; iAnim < NumAnims; iAnim++)
    {
        Anims[iAnim].Mode = mpFile->ReadLong();

        switch (Anims[iAnim].Mode)
        {
        case 3: // Rotation
        case 7: // ???
            Anims[iAnim].Params[0] = mpFile->ReadFloat();
            Anims[iAnim].Params[1] = mpFile->ReadFloat();
            break;
        case 2: // UV Scroll
        case 4: // U Scroll
        case 5: // V Scroll
            Anims[iAnim].Params[0] = mpFile->ReadFloat();
            Anims[iAnim].Params[1] = mpFile->ReadFloat();
            Anims[iAnim].Params[2] = mpFile->ReadFloat();
            Anims[iAnim].Params[3] = mpFile->ReadFloat();
            break;
        case 0: // Inverse ModelView Matrix
        case 1: // Inverse ModelView Matrix Translated
        case 6: // Model Matrix
            break;
        default:
            errorf("%s [0x%X]: Unsupported animation mode encountered: %d", *mpFile->GetSourceString(), mpFile->Tell() - 4, Anims[iAnim].Mode);
            break;
        }
    }

    // Move TexGen and anims into passes
    for (uint32 iPass = 0; iPass < pMat->mPasses.size(); iPass++)
    {
        CMaterialPass *pPass = pMat->mPasses[iPass];
        uint8 TexCoordIdx = TevCoordIndices[iPass];

        if ((TexGens.size() == 0) || (TexCoordIdx == 0xFF))
        {
            pPass->mTexCoordSource = 0xFF;
            pPass->mAnimMode = EUVAnimMode::NoUVAnim;
        }

        else
        {
            pPass->mTexCoordSource = (uint8) ((TexGens[TexCoordIdx] & 0x1F0) >> 4);

            // Next step - find which animation is used by this pass
            // Texture matrix is a reliable way to tell, because every UV anim mode generates a texture matrix
            uint32 TexMtxIdx = ((TexGens[TexCoordIdx] & 0x3E00) >> 9) / 3;

            if (TexMtxIdx == 10) pPass->mAnimMode = EUVAnimMode::NoUVAnim; // 10 is identity matrix; indicates no UV anim for this pass

            else
            {
                pPass->mAnimMode = (EUVAnimMode) Anims[TexMtxIdx].Mode;

                for (uint32 iParam = 0; iParam < 4; iParam++)
                    pPass->mAnimParams[iParam] = Anims[TexMtxIdx].Params[iParam];
            }
        }
    }

    return pMat;
}

void CMaterialLoader::ReadCorruptionMatSet()
{
    uint32 NumMats = mpFile->ReadLong();
    mpSet->mMaterials.resize(NumMats);

    for (uint32 iMat = 0; iMat < NumMats; iMat++)
    {
        uint32 Size = mpFile->ReadLong();
        uint32 Next = mpFile->Tell() + Size;
        mpSet->mMaterials[iMat] = ReadCorruptionMaterial();
        mpSet->mMaterials[iMat]->mVersion = mVersion;
        mpSet->mMaterials[iMat]->mName = TString("Material #") + TString::FromInt32(iMat + 1, 0, 10);
        mpFile->Seek(Next, SEEK_SET);
    }
}

CMaterial* CMaterialLoader::ReadCorruptionMaterial()
{
    CMaterial *pMat = new CMaterial();
    pMat->mOptions = EMaterialOption::DepthWrite;
    pMat->mEnableBloom = true;

    // Flags
    uint32 Flags = mpFile->ReadLong();
    if (Flags & 0x8)
    {
        pMat->mBlendSrcFac = GL_SRC_ALPHA;
        pMat->mBlendDstFac = GL_ONE_MINUS_SRC_ALPHA;
        pMat->mOptions |= EMaterialOption::Transparent;
    }
    else if (Flags & 0x20)
    {
        pMat->mBlendSrcFac = GL_ONE;
        pMat->mBlendDstFac = GL_ONE;
        pMat->mOptions |= EMaterialOption::Transparent;
    }

    if (Flags & 0x10)       pMat->mOptions |= EMaterialOption::Masked;
    if (Flags & 0x100)      pMat->mOptions |= EMaterialOption::Occluder;
    if (Flags & 0x80000)    pMat->mOptions |= EMaterialOption::DrawWhiteAmbientDKCR;
    mHas0x400 = ((Flags & 0x400) != 0);

    mpFile->Seek(0x8, SEEK_CUR); // Don't know what any of this is
    pMat->mVtxDesc = ConvertToVertexDescription( mpFile->ReadLong() );
    mpFile->Seek(0xC, SEEK_CUR);

    // Initialize all KColors to white
    pMat->mKonstColors[0] = CColor::skWhite;
    pMat->mKonstColors[1] = CColor::skWhite;
    pMat->mKonstColors[2] = CColor::skWhite;
    // Current usage of KColors:
    // 0 - INT OPAC (transparency)
    // 1 - CLR DIFB (lightmap multiplier)
    // 2 - CLR CLR  (additive color)

    while (true)
    {
        CFourCC Type = mpFile->ReadLong();

        // END
        if (Type == "END ")
            break;

        // INT
        if (Type == "INT ")
        {
            CFourCC IntType = mpFile->ReadLong();
            uint8 IntVal = (uint8) mpFile->ReadLong();

            if (IntType == "OPAC")
            {
                pMat->mKonstColors[0] = CColor(1.f, 1.f, 1.f, (float) IntVal / 255);
                mHasOPAC = true;
            }
        }

        // CLR
        if (Type == "CLR ")
        {
            CFourCC ClrType = mpFile->ReadLong();
            CColor ClrVal(*mpFile, true);

            if (ClrType == "DIFB")
            {
                ClrVal.A = 0xFF;
                pMat->mKonstColors[1] = ClrVal;
            }

            if (ClrType == "CLR ")
            {
                // I'm not sure what this does. It has a clear and obvious ingame effect
                // but I need to test it further to tell specifically what it's doing.
                // All attempts at implementing this just break things.
            }
        }

        // PASS
        if (Type == "PASS")
        {
            CMaterialPass *pPass = new CMaterialPass(pMat);
            mPassOffsets.push_back(mpFile->Tell() - 4);

            uint32 Size = mpFile->ReadLong();
            uint32 Next = Size + mpFile->Tell();

            pPass->mPassType = mpFile->ReadLong();
            pPass->mSettings = (EPassSettings) mpFile->ReadLong();

            // Skip passes that don't have a texture. Honestly don't really know what to do with these right now
            uint64 TextureID = mpFile->ReadLongLong();
            if (TextureID == 0xFFFFFFFFFFFFFFFF)
            {
                //Log::FileWarning(mpFile->GetSourceString(), mPassOffsets.back(), "Skipping " + pPass->mPassType.ToString() + " pass with no texture");
                delete pPass;
                continue;
            }

            pPass->mpTexture = gpResourceStore->LoadResource<CTexture>(TextureID);

            pPass->mTexCoordSource = 4 + (uint8) mpFile->ReadLong();
            uint32 AnimSize = mpFile->ReadLong();

            if (AnimSize > 0)
            {
                mpFile->Seek(0x4, SEEK_CUR);
                pPass->mAnimMode = (EUVAnimMode) mpFile->ReadLong();

                switch (pPass->mAnimMode)
                {
                case 3: // Rotation
                case 7: // ???
                    pPass->mAnimParams[0] = mpFile->ReadFloat();
                    pPass->mAnimParams[1] = mpFile->ReadFloat();
                    break;
                case 2: // UV Scroll
                case 4: // U Scroll
                case 5: // V Scroll
                    pPass->mAnimParams[0] = mpFile->ReadFloat();
                    pPass->mAnimParams[1] = mpFile->ReadFloat();
                    pPass->mAnimParams[2] = mpFile->ReadFloat();
                    pPass->mAnimParams[3] = mpFile->ReadFloat();
                    break;
                case 0: // Inverse ModelView Matrix
                case 1: // Inverse ModelView Matrix Translated
                case 6: // Model Matrix
                case 10: // Yet-to-be-named
                    break;

                // Unknown/unsupported animation type
                case 8:
                case 11:
                    break;
                default:
                    errorf("%s [0x%X]: Unsupported animation mode encountered: %d", *mpFile->GetSourceString(), mpFile->Tell() - 8, pPass->mAnimMode);
                    break;
                }

                // Hack until the correct way to determine tex coord source is figured out
                if ((pPass->mAnimMode < EUVAnimMode::UVScroll) ||
                    (pPass->mAnimMode == EUVAnimMode::ModelMatrix) ||
                    (pPass->mAnimMode == EUVAnimMode::ConvolutedModeA) ||
                    (pPass->mAnimMode == EUVAnimMode::SimpleMode))
                {
                    pPass->mTexCoordSource = 1;
                }
            }

            else pPass->mAnimMode = EUVAnimMode::NoUVAnim;

            pMat->mPasses.push_back(pPass);
            mpFile->Seek(Next, SEEK_SET);
        }
    }

    CreateCorruptionPasses(pMat);
    mHasOPAC = false;
    return pMat;
}

void CMaterialLoader::CreateCorruptionPasses(CMaterial *pMat)
{
    uint32 NumPass = pMat->PassCount();
    bool Lightmap = false;
    bool AlphaBlended = ((pMat->mBlendSrcFac == GL_SRC_ALPHA) && (pMat->mBlendDstFac == GL_ONE_MINUS_SRC_ALPHA));

    for (uint32 iPass = 0; iPass < NumPass; iPass++)
    {
        CMaterialPass *pPass = pMat->Pass(iPass);
        CFourCC Type = pPass->Type();

        // Color Map (Diffuse)
        if (Type == "CLR ")
        {
            pPass->SetRasSel(kRasColor0A0);

            if (Lightmap)
            {
                pPass->SetColorInputs(kZeroRGB, kColor0RGB, kTextureRGB, kZeroRGB);
            }

            else
            {
                pPass->SetColorInputs(kZeroRGB, kRasRGB, kTextureRGB, kZeroRGB);
            }


            if (pMat->mOptions & EMaterialOption::Masked)
            {
                pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kTextureAlpha);
            }
            else if (mHasOPAC)
            {
                pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kKonstAlpha);
                pPass->SetKColorSel(kKonst0_RGB);
                pPass->SetKAlphaSel(kKonst0_A);
            }
            else
            {
                pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kPrevAlpha);
            }

            pPass->SetColorOutput(kPrevReg);
            pPass->SetAlphaOutput(kPrevReg);
        }

        // Lightmap
        else if (Type == "DIFF")
        {
            pPass->SetColorInputs(kZeroRGB, kKonstRGB, kTextureRGB, kRasRGB);
            pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kKonstAlpha);
            pPass->SetColorOutput(kColor0Reg);
            pPass->SetAlphaOutput(kColor0Reg);
            pPass->SetKColorSel(kKonst1_RGB);
            pPass->SetKAlphaSel(kKonst1_A);
            pPass->SetRasSel(kRasColor0A0);
            Lightmap = true;
        }

        // Bloom Lightmap
        else if (Type == "BLOL")
        {
            // Bloom maps work by writing to framebuffer alpha. Can't do this on alpha-blended mats.
            pPass->SetColorInputs(kZeroRGB, kZeroRGB, kZeroRGB, kPrevRGB);

            if ((AlphaBlended) || (pMat->mOptions & EMaterialOption::Masked))
                pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kPrevAlpha);
            else
                pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kTextureAlpha);

            pPass->SetColorOutput(kPrevReg);
            pPass->SetAlphaOutput(kPrevReg);
        }

        // Rim Light Map
        else if (Type == "RIML")
        {
            pPass->SetColorInputs(kZeroRGB, kOneRGB, kPrevRGB, kTextureRGB);
            pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kPrevAlpha);
            pPass->SetColorOutput(kPrevReg);
            pPass->SetAlphaOutput(kPrevReg);
        }

        // Emissive Map
        else if (Type == "INCA")
        {
            pPass->SetColorInputs(kZeroRGB, kTextureRGB, kOneRGB, kPrevRGB);

            if ((pPass->mSettings & EPassSettings::EmissiveBloom) && (!AlphaBlended))
            {
                pPass->SetAlphaInputs(kZeroAlpha, kTextureAlpha, kKonstAlpha, kPrevAlpha);
                pPass->SetKAlphaSel(kKonstOneFourth);
            }
            else
            {
                pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kPrevAlpha);
            }

            pPass->SetColorOutput(kPrevReg);
            pPass->SetAlphaOutput(kPrevReg);
        }

        // Opacity Map
        else if (Type == "TRAN")
        {
            pPass->SetColorInputs(kZeroRGB, kZeroRGB, kZeroRGB, kPrevRGB);

            if (pPass->mSettings & EPassSettings::InvertOpacityMap)
                pPass->SetAlphaInputs(kKonstAlpha, kZeroAlpha, kTextureAlpha, kZeroAlpha);
            else
                pPass->SetAlphaInputs(kZeroAlpha, kKonstAlpha, kTextureAlpha, kZeroAlpha);

            pPass->SetColorOutput(kPrevReg);
            pPass->SetAlphaOutput(kPrevReg);
        }

        // Specular Map
        else if (Type == "RFLV")
        {
            pPass->SetColorInputs(kZeroRGB, kZeroRGB, kZeroRGB, kTextureRGB);
            pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kPrevAlpha);
            pPass->SetColorOutput(kColor2Reg);
            pPass->SetAlphaOutput(kColor2Reg);
        }

        // Reflection Map
        else if (Type == "RFLD")
        {
            pPass->SetColorInputs(kZeroRGB, kColor2RGB, kTextureRGB, kPrevRGB);
            pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kPrevAlpha);
            pPass->SetColorOutput(kPrevReg);
            pPass->SetAlphaOutput(kPrevReg);
            if (mHas0x400) pPass->SetEnabled(false);
        }

        // Bloom Incandescence
        else if (Type == "BLOI")
        {
            pPass->SetColorInputs(kZeroRGB, kZeroRGB, kZeroRGB, kPrevRGB);

            // Comes out wrong every time even though this is exactly how the Dolphin shaders say this is done.
            if (AlphaBlended)
                pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kPrevAlpha);
            else
                pPass->SetAlphaInputs(kTextureAlpha, kZeroAlpha, kZeroAlpha, kPrevAlpha);

            pPass->SetColorOutput(kPrevReg);
            pPass->SetAlphaOutput(kPrevReg);
        }

        // Bloom Diffuse
        else if (Type == "BLOD")
        {
            // Not supported yet
        }

        // X-Ray - since we don't support X-Ray previews, no effect
        else if (Type == "XRAY")
        {
            pPass->SetColorInputs(kZeroRGB, kZeroRGB, kZeroRGB, kPrevRGB);
            pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kPrevAlpha);
            pPass->SetColorOutput(kPrevReg);
            pPass->SetAlphaOutput(kPrevReg);
        }

        // Toon? Don't know what it's for but got TEV setup from shader dumps
        else if (Type == "TOON")
        {
            pPass->SetColorInputs(kZeroRGB, kPrevRGB, kTextureRGB, kZeroRGB);
            pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kTextureAlpha);
            pPass->SetColorOutput(kPrevReg);
            pPass->SetAlphaOutput(kPrevReg);
        }

        // LURD and LRLD are unknown and don't seem to do anything
        else if ((Type == "LURD") || (Type == "LRLD"))
        {
            pPass->SetColorInputs(kZeroRGB, kZeroRGB, kZeroRGB, kPrevRGB);
            pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kPrevAlpha);
            pPass->SetColorOutput(kPrevReg);
            pPass->SetAlphaOutput(kPrevReg);
        }

        else if (Type == "CUST") {}

        else
        {
            errorf("%s [0x%X]: Unsupported material pass type: %s", *mpFile->GetSourceString(), mPassOffsets[iPass], *Type.ToString());
            pPass->mEnabled = false;
        }
    }
}

CMaterial* CMaterialLoader::LoadAssimpMaterial(const aiMaterial *pAiMat)
{
    // todo: generate new material using import values.
    CMaterial *pMat = new CMaterial(mVersion, EVertexAttribute::None);

    aiString Name;
    pAiMat->Get(AI_MATKEY_NAME, Name);
    pMat->SetName(Name.C_Str());

    // Create generic custom pass that uses Konst color
    CMaterialPass *pPass = new CMaterialPass(pMat);
    pPass->SetColorInputs(kZeroRGB, kRasRGB, kKonstRGB, kZeroRGB);
    pPass->SetAlphaInputs(kZeroAlpha, kZeroAlpha, kZeroAlpha, kKonstAlpha);
    pPass->SetKColorSel(kKonst0_RGB);
    pPass->SetKAlphaSel(kKonstOne);
    pPass->SetRasSel(kRasColor0A0);
    pMat->mKonstColors[0] = CColor::RandomLightColor(false);
    pMat->mPasses.push_back(pPass);

    return pMat;
}

// ************ STATIC ************
CMaterialSet* CMaterialLoader::LoadMaterialSet(IInputStream& rMat, EGame Version)
{
    CMaterialLoader Loader;
    Loader.mpSet = new CMaterialSet();
    Loader.mpFile = &rMat;
    Loader.mVersion = Version;

    if ((Version >= EGame::PrimeDemo) && (Version <= EGame::Echoes))
        Loader.ReadPrimeMatSet();
    else
        Loader.ReadCorruptionMatSet();

    return Loader.mpSet;
}

CMaterialSet* CMaterialLoader::ImportAssimpMaterials(const aiScene *pScene, EGame TargetVersion)
{
    CMaterialLoader Loader;
    Loader.mVersion = TargetVersion;

    CMaterialSet *pOut = new CMaterialSet();
    pOut->mMaterials.reserve(pScene->mNumMaterials);

    for (uint32 iMat = 0; iMat < pScene->mNumMaterials; iMat++)
    {
        CMaterial *pMat = Loader.LoadAssimpMaterial(pScene->mMaterials[iMat]);
        pOut->mMaterials.push_back(pMat);
    }

    return pOut;
}
