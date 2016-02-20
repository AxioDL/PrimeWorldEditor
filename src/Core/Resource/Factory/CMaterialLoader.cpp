#include "CMaterialLoader.h"
#include "Core/Resource/CResCache.h"
#include "Core/OpenGL/GLCommon.h"
#include <Common/Log.h>
#include <iostream>
#include <iomanip>

CMaterialLoader::CMaterialLoader()
{
    mCorruptionFlags = 0;
    mHasOPAC = false;
}

CMaterialLoader::~CMaterialLoader()
{
}

void CMaterialLoader::ReadPrimeMatSet()
{
    // Textures
    u32 numTextures = mpFile->ReadLong();
    mTextures.resize(numTextures);

    for (u32 iTex = 0; iTex < numTextures; iTex++)
    {
        u32 TextureID = mpFile->ReadLong();
        mTextures[iTex] = gResCache.GetResource(TextureID, "TXTR");
    }

    // Materials
    u32 numMats = mpFile->ReadLong();
    std::vector<u32> offsets(numMats);
    for (u32 iMat = 0; iMat < numMats; iMat++)
        offsets[iMat] = mpFile->ReadLong();

    u32 matsStart = mpFile->Tell();
    mpSet->mMaterials.resize(numMats);
    for (u32 iMat = 0; iMat < numMats; iMat++)
    {
        mpSet->mMaterials[iMat] = ReadPrimeMaterial();
        mpSet->mMaterials[iMat]->mVersion = mVersion;
        mpSet->mMaterials[iMat]->mName = TString("Material #") + std::to_string(iMat + 1);
        mpFile->Seek(matsStart + offsets[iMat], SEEK_SET);
    }
}

CMaterial* CMaterialLoader::ReadPrimeMaterial()
{
    CMaterial *pMat = new CMaterial();
    pMat->mEnableBloom = false;

    // Flags
    pMat->mOptions = (mpFile->ReadLong() & CMaterial::eAllMP1Settings);

    // Textures
    u32 NumTextures = mpFile->ReadLong();
    std::vector<u32> TextureIndices(NumTextures);

    for (u32 iTex = 0; iTex < NumTextures; iTex++)
    {
        u32 Index = mpFile->ReadLong();
        TextureIndices[iTex] = Index;
    }

    // Vertex description
    pMat->mVtxDesc = (FVertexDescription) mpFile->ReadLong();

    // Unknowns
    if (mVersion >= eEchoesDemo)
    {
        pMat->mEchoesUnknownA = mpFile->ReadLong();
        pMat->mEchoesUnknownB = mpFile->ReadLong();
    }
    mpFile->Seek(0x4, SEEK_CUR); // Skipping group index

    // Konst
    if (pMat->mOptions & CMaterial::eKonst)
    {
        u32 KonstCount = mpFile->ReadLong();

        for (u32 iKonst = 0; iKonst < KonstCount; iKonst++)
        {
            if (iKonst >= 4) break;
            pMat->mKonstColors[iKonst] = CColor(*mpFile, true);
        }
        if (KonstCount > 4) mpFile->Seek(0x4 * (KonstCount - 4), SEEK_CUR);
    }

    // Blend mode
    pMat->mBlendDstFac = glBlendFactor[mpFile->ReadShort()];
    pMat->mBlendSrcFac = glBlendFactor[mpFile->ReadShort()];

    // Indirect texture
    if (pMat->mOptions & CMaterial::eIndStage)
    {
        u32 IndTexIndex = mpFile->ReadLong();
        pMat->mpIndirectTexture = mTextures[IndTexIndex];
    }

    // Color channels
    u32 ChanCount = mpFile->ReadLong();
    pMat->mLightingEnabled = ((mpFile->ReadLong() & 0x1) == 1);
    mpFile->Seek((4 * ChanCount) - 4, SEEK_CUR);

    // TEV
    u32 TevCount = mpFile->ReadLong();
    pMat->mPasses.resize(TevCount);

    for (u32 iTev = 0; iTev < TevCount; iTev++)
    {
        CMaterialPass *pPass = new CMaterialPass(pMat);

        u32 ColorIn = mpFile->ReadLong();
        u32 AlphaIn = mpFile->ReadLong();
        pPass->mColorOutput = (ETevOutput) ((mpFile->ReadLong() & 0x600) >> 9);
        pPass->mAlphaOutput = (ETevOutput) ((mpFile->ReadLong() & 0x600) >> 9);
        mpFile->Seek(0x1, SEEK_CUR); // Padding byte
        pPass->mKAlphaSel = (ETevKSel) mpFile->ReadByte();
        pPass->mKColorSel = (ETevKSel) mpFile->ReadByte();
        pPass->mRasSel = (ETevRasSel) (u8) mpFile->ReadByte();

        for (u32 iInput = 0; iInput < 4; iInput++)
        {
            pPass->mColorInputs[iInput] = (ETevColorInput) ((ColorIn >> (iInput * 5)) & 0xF);
            pPass->mAlphaInputs[iInput] = (ETevAlphaInput) ((AlphaIn >> (iInput * 5)) & 0x7);
        }

        pMat->mPasses[iTev] = pPass;
    }

    std::vector<u8> TevCoordIndices(TevCount);
    for (u32 iTev = 0; iTev < TevCount; iTev++)
    {
        mpFile->Seek(0x2, SEEK_CUR);
        CMaterialPass *pPass = pMat->Pass(iTev);

        u8 TexSel = mpFile->ReadByte();

        if ((TexSel == 0xFF) || (TexSel >= mTextures.size()))
            pPass->mpTexture = nullptr;
        else
            pPass->mpTexture = mTextures[TextureIndices[TexSel]];

        TevCoordIndices[iTev] = mpFile->ReadByte();
    }

    // TexGens
    u32 TexGenCount = mpFile->ReadLong();
    std::vector<u32> TexGens(TexGenCount);

    for (u32 iTex = 0; iTex < TexGenCount; iTex++)
        TexGens[iTex] = mpFile->ReadLong();

    // UV animations
    mpFile->Seek(0x4, SEEK_CUR); // Skipping UV anims size
    u32 NumAnims = mpFile->ReadLong();

    struct SUVAnim {
        s32 Mode; float Params[4];
    };
    std::vector <SUVAnim> Anims(NumAnims);

    for (u32 iAnim = 0; iAnim < NumAnims; iAnim++)
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
            Log::FileError(mpFile->GetSourceString(), mpFile->Tell() - 4, "Unsupported animation mode encountered: " + TString::HexString((u32) Anims[iAnim].Mode));
            break;
        }
    }

    // Move TexGen and anims into passes
    for (u32 iPass = 0; iPass < pMat->mPasses.size(); iPass++)
    {
        CMaterialPass *pPass = pMat->mPasses[iPass];
        u8 TexCoordIdx = TevCoordIndices[iPass];

        if ((TexGens.size() == 0) || (TexCoordIdx == 0xFF))
        {
            pPass->mTexCoordSource = 0xFF;
            pPass->mAnimMode = eNoUVAnim;
        }

        else
        {
            pPass->mTexCoordSource = (u8) ((TexGens[TexCoordIdx] & 0x1F0) >> 4);

            // Next step - find which animation is used by this pass
            // Texture matrix is a reliable way to tell, because every UV anim mode generates a texture matrix
            u32 TexMtxIdx = ((TexGens[TexCoordIdx] & 0x3E00) >> 9) / 3;

            if (TexMtxIdx == 10) pPass->mAnimMode = eNoUVAnim; // 10 is identity matrix; indicates no UV anim for this pass

            else
            {
                pPass->mAnimMode = (EUVAnimMode) Anims[TexMtxIdx].Mode;

                for (u32 iParam = 0; iParam < 4; iParam++)
                    pPass->mAnimParams[iParam] = Anims[TexMtxIdx].Params[iParam];
            }
        }
    }

    return pMat;
}

void CMaterialLoader::ReadCorruptionMatSet()
{
    u32 NumMats = mpFile->ReadLong();
    mpSet->mMaterials.resize(NumMats);

    for (u32 iMat = 0; iMat < NumMats; iMat++)
    {
        u32 Size = mpFile->ReadLong();
        u32 Next = mpFile->Tell() + Size;
        mpSet->mMaterials[iMat] = ReadCorruptionMaterial();
        mpSet->mMaterials[iMat]->mVersion = mVersion;
        mpSet->mMaterials[iMat]->mName = TString("Material #") + std::to_string(iMat + 1);
        mpFile->Seek(Next, SEEK_SET);
    }
}

CMaterial* CMaterialLoader::ReadCorruptionMaterial()
{
    CMaterial *pMat = new CMaterial();
    pMat->mOptions = CMaterial::eDepthWrite;
    pMat->mEnableBloom = true;

    // Flags
    u32 Flags = mpFile->ReadLong();
    if (Flags & 0x8)
    {
        pMat->mBlendSrcFac = GL_SRC_ALPHA;
        pMat->mBlendDstFac = GL_ONE_MINUS_SRC_ALPHA;
        pMat->mOptions |= CMaterial::eTransparent;
    }
    else if (Flags & 0x20)
    {
        pMat->mBlendSrcFac = GL_ONE;
        pMat->mBlendDstFac = GL_ONE;
        pMat->mOptions |= CMaterial::eTransparent;
    }

    if (Flags & 0x10)       pMat->mOptions |= CMaterial::ePunchthrough;
    if (Flags & 0x100)      pMat->mOptions |= CMaterial::eOccluder;
    if (Flags & 0x80000)    pMat->mOptions |= CMaterial::eDrawWhiteAmbientDKCR;
    mHas0x400 = ((Flags & 0x400) != 0);

    mpFile->Seek(0x8, SEEK_CUR); // Don't know what any of this is
    pMat->mVtxDesc = (FVertexDescription) mpFile->ReadLong();
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
            u8 IntVal = (u8) mpFile->ReadLong();

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
                ClrVal.a = 0xFF;
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

            u32 Size = mpFile->ReadLong();
            u32 Next = Size + mpFile->Tell();

            pPass->mPassType = mpFile->ReadLong();
            pPass->mSettings = (CMaterialPass::EPassSettings) mpFile->ReadLong();

            u64 TextureID = mpFile->ReadLongLong();
            if (TextureID == 0xFFFFFFFFFFFFFFFF)
            {
                Log::FileWarning(mpFile->GetSourceString(), mPassOffsets.back(), "Skipping " + pPass->mPassType.ToString() + " pass with no texture");
                delete pPass;
                continue;
            }

            pPass->mpTexture = gResCache.GetResource(TextureID, "TXTR");

            pPass->mTexCoordSource = 4 + (u8) mpFile->ReadLong();
            u32 AnimSize = mpFile->ReadLong();

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
                default:
                    Log::FileError(mpFile->GetSourceString(), mpFile->Tell() - 8, "Unsupported animation mode encountered: " + TString::HexString((u32) pPass->mAnimMode));
                    break;
                }

                // Hack until the correct way to determine tex coord source is figured out
                if ((pPass->mAnimMode < 2) || (pPass->mAnimMode == 6) || (pPass->mAnimMode == 7) || (pPass->mAnimMode == 10))
                    pPass->mTexCoordSource = 1;
            }

            else pPass->mAnimMode = eNoUVAnim;

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
    u32 NumPass = pMat->PassCount();
    bool Lightmap = false;
    bool AlphaBlended = ((pMat->mBlendSrcFac == GL_SRC_ALPHA) && (pMat->mBlendDstFac == GL_ONE_MINUS_SRC_ALPHA));

    for (u32 iPass = 0; iPass < NumPass; iPass++)
    {
        CMaterialPass *pPass = pMat->Pass(iPass);
        CFourCC Type = pPass->Type();

        // Color Map (Diffuse)
        if (Type == "CLR ")
        {
            pPass->SetRasSel(eRasColor0A0);

            if (Lightmap)
            {
                pPass->SetColorInputs(eZeroRGB, eColor0RGB, eTextureRGB, eZeroRGB);
            }

            else
            {
                pPass->SetColorInputs(eZeroRGB, eRasRGB, eTextureRGB, eZeroRGB);
            }


            if (pMat->mOptions & CMaterial::ePunchthrough)
            {
                pPass->SetAlphaInputs(eZeroAlpha, eZeroAlpha, eZeroAlpha, eTextureAlpha);
            }
            else if (mHasOPAC)
            {
                pPass->SetAlphaInputs(eZeroAlpha, eZeroAlpha, eZeroAlpha, eKonstAlpha);
                pPass->SetKColorSel(eKonst0_RGB);
                pPass->SetKAlphaSel(eKonst0_A);
            }
            else
            {
                pPass->SetAlphaInputs(eZeroAlpha, eZeroAlpha, eZeroAlpha, ePrevAlpha);
            }

            pPass->SetColorOutput(ePrevReg);
            pPass->SetAlphaOutput(ePrevReg);
        }

        // Lightmap
        else if (Type == "DIFF")
        {
            pPass->SetColorInputs(eZeroRGB, eKonstRGB, eTextureRGB, eRasRGB);
            pPass->SetAlphaInputs(eZeroAlpha, eZeroAlpha, eZeroAlpha, eKonstAlpha);
            pPass->SetColorOutput(eColor0Reg);
            pPass->SetAlphaOutput(eColor0Reg);
            pPass->SetKColorSel(eKonst1_RGB);
            pPass->SetKAlphaSel(eKonst1_A);
            pPass->SetRasSel(eRasColor0A0);
            Lightmap = true;
        }

        // Bloom Lightmap
        else if (Type == "BLOL")
        {
            // Bloom maps work by writing to framebuffer alpha. Can't do this on alpha-blended mats.
            pPass->SetColorInputs(eZeroRGB, eZeroRGB, eZeroRGB, ePrevRGB);

            if ((AlphaBlended) || (pMat->mOptions & CMaterial::ePunchthrough))
                pPass->SetAlphaInputs(eZeroAlpha, eZeroAlpha, eZeroAlpha, ePrevAlpha);
            else
                pPass->SetAlphaInputs(eZeroAlpha, eZeroAlpha, eZeroAlpha, eTextureAlpha);

            pPass->SetColorOutput(ePrevReg);
            pPass->SetAlphaOutput(ePrevReg);
        }

        // Rim Light Map
        else if (Type == "RIML")
        {
            pPass->SetColorInputs(eZeroRGB, eOneRGB, ePrevRGB, eTextureRGB);
            pPass->SetAlphaInputs(eZeroAlpha, eZeroAlpha, eZeroAlpha, ePrevAlpha);
            pPass->SetColorOutput(ePrevReg);
            pPass->SetAlphaOutput(ePrevReg);
        }

        // Emissive Map
        else if (Type == "INCA")
        {
            pPass->SetColorInputs(eZeroRGB, eTextureRGB, eOneRGB, ePrevRGB);

            if ((pPass->mSettings & CMaterialPass::eEmissiveBloom) && (!AlphaBlended))
            {
                pPass->SetAlphaInputs(eZeroAlpha, eTextureAlpha, eKonstAlpha, ePrevAlpha);
                pPass->SetKAlphaSel(eKonstOneFourth);
            }
            else
            {
                pPass->SetAlphaInputs(eZeroAlpha, eZeroAlpha, eZeroAlpha, ePrevAlpha);
            }

            pPass->SetColorOutput(ePrevReg);
            pPass->SetAlphaOutput(ePrevReg);
        }

        // Opacity Map
        else if (Type == "TRAN")
        {
            pPass->SetColorInputs(eZeroRGB, eZeroRGB, eZeroRGB, ePrevRGB);

            if (pPass->mSettings & CMaterialPass::eInvertOpacityMap)
                pPass->SetAlphaInputs(eKonstAlpha, eZeroAlpha, eTextureAlpha, eZeroAlpha);
            else
                pPass->SetAlphaInputs(eZeroAlpha, eKonstAlpha, eTextureAlpha, eZeroAlpha);

            pPass->SetColorOutput(ePrevReg);
            pPass->SetAlphaOutput(ePrevReg);
        }

        // Specular Map
        else if (Type == "RFLV")
        {
            pPass->SetColorInputs(eZeroRGB, eZeroRGB, eZeroRGB, eTextureRGB);
            pPass->SetAlphaInputs(eZeroAlpha, eZeroAlpha, eZeroAlpha, ePrevAlpha);
            pPass->SetColorOutput(eColor2Reg);
            pPass->SetAlphaOutput(eColor2Reg);
        }

        // Reflection Map
        else if (Type == "RFLD")
        {
            pPass->SetColorInputs(eZeroRGB, eColor2RGB, eTextureRGB, ePrevRGB);
            pPass->SetAlphaInputs(eZeroAlpha, eZeroAlpha, eZeroAlpha, ePrevAlpha);
            pPass->SetColorOutput(ePrevReg);
            pPass->SetAlphaOutput(ePrevReg);
            if (mHas0x400) pPass->SetEnabled(false);
        }

        // Bloom
        else if (Type == "BLOI")
        {
            pPass->SetColorInputs(eZeroRGB, eZeroRGB, eZeroRGB, ePrevRGB);

            // Comes out wrong every time even though this is exactly how the Dolphin shaders say this is done.
            if (AlphaBlended)
                pPass->SetAlphaInputs(eZeroAlpha, eZeroAlpha, eZeroAlpha, ePrevAlpha);
            else
                pPass->SetAlphaInputs(eTextureAlpha, eZeroAlpha, eZeroAlpha, ePrevAlpha);

            pPass->SetColorOutput(ePrevReg);
            pPass->SetAlphaOutput(ePrevReg);
        }

        // X-Ray - since we don't support X-Ray previews, no effect
        else if (Type == "XRAY")
        {
            pPass->SetColorInputs(eZeroRGB, eZeroRGB, eZeroRGB, ePrevRGB);
            pPass->SetAlphaInputs(eZeroAlpha, eZeroAlpha, eZeroAlpha, ePrevAlpha);
            pPass->SetColorOutput(ePrevReg);
            pPass->SetAlphaOutput(ePrevReg);
        }

        // Toon? Don't know what it's for but got TEV setup from shader dumps
        else if (Type == "TOON")
        {
            pPass->SetColorInputs(eZeroRGB, ePrevRGB, eTextureRGB, eZeroRGB);
            pPass->SetAlphaInputs(eZeroAlpha, eZeroAlpha, eZeroAlpha, eTextureAlpha);
            pPass->SetColorOutput(ePrevReg);
            pPass->SetAlphaOutput(ePrevReg);
        }

        // LURD and LRLD are unknown and don't seem to do anything
        else if ((Type == "LURD") || (Type == "LRLD"))
        {
            pPass->SetColorInputs(eZeroRGB, eZeroRGB, eZeroRGB, ePrevRGB);
            pPass->SetAlphaInputs(eZeroAlpha, eZeroAlpha, eZeroAlpha, ePrevAlpha);
            pPass->SetColorOutput(ePrevReg);
            pPass->SetAlphaOutput(ePrevReg);
        }

        else if (Type == "CUST") {}

        else
        {
            Log::FileError(mpFile->GetSourceString(), mPassOffsets[iPass], "Unsupported material pass type: " + Type.ToString());
            pPass->mEnabled = false;
        }
    }
}

CMaterial* CMaterialLoader::LoadAssimpMaterial(const aiMaterial *pAiMat)
{
    // todo: generate new material using import values.
    CMaterial *pMat = new CMaterial(mVersion, eNoAttributes);

    aiString name;
    pAiMat->Get(AI_MATKEY_NAME, name);
    pMat->SetName(name.C_Str());

    // Create generic custom pass that uses Konst color
    CMaterialPass *pPass = new CMaterialPass(pMat);
    pPass->SetColorInputs(eZeroRGB, eRasRGB, eKonstRGB, eZeroRGB);
    pPass->SetAlphaInputs(eZeroAlpha, eZeroAlpha, eZeroAlpha, eKonstAlpha);
    pPass->SetKColorSel(eKonst0_RGB);
    pPass->SetKAlphaSel(eKonstOne);
    pPass->SetRasSel(eRasColor0A0);
    pMat->mKonstColors[0] = CColor::RandomLightColor(false);
    pMat->mPasses.push_back(pPass);

    return pMat;
}

// ************ STATIC ************
CMaterialSet* CMaterialLoader::LoadMaterialSet(IInputStream& Mat, EGame Version)
{
    CMaterialLoader Loader;
    Loader.mpSet = new CMaterialSet();
    Loader.mpFile = &Mat;
    Loader.mVersion = Version;

    if ((Version >= ePrimeDemo) && (Version <= eEchoes))
        Loader.ReadPrimeMatSet();
    else
        Loader.ReadCorruptionMatSet();

    return Loader.mpSet;
}

CMaterialSet* CMaterialLoader::ImportAssimpMaterials(const aiScene *pScene, EGame targetVersion)
{
    CMaterialLoader loader;
    loader.mVersion = targetVersion;

    CMaterialSet *pOut = new CMaterialSet();
    pOut->mMaterials.reserve(pScene->mNumMaterials);

    for (u32 iMat = 0; iMat < pScene->mNumMaterials; iMat++)
    {
        CMaterial *pMat = loader.LoadAssimpMaterial(pScene->mMaterials[iMat]);
        pOut->mMaterials.push_back(pMat);
    }

    return pOut;
}
