#include "CShaderGenerator.h"
#include <Common/Macros.h>
#include <array>
#include <fstream>
#include <sstream>
#include <string_view>
#include <GL/glew.h>

using namespace std::string_view_literals;

constexpr std::array gkCoordSrc{
    "ModelSpacePos.xyz"sv,
    "ModelSpaceNormal.xyz"sv,
    "0.0, 0.0, 0.0"sv,
    "0.0, 0.0, 0.0"sv,
    "RawTex0.xy, 1.0"sv,
    "RawTex1.xy, 1.0"sv,
    "RawTex2.xy, 1.0"sv,
    "RawTex3.xy, 1.0"sv,
    "RawTex4.xy, 1.0"sv,
    "RawTex5.xy, 1.0"sv,
    "RawTex6.xy, 1.0"sv,
    "RawTex7.xy, 1.0"sv,
};

constexpr std::array gkRasSel{
    "vec4(COLOR0A0.rgb, 1.0)"sv,
    "vec4(COLOR1A1.rgb, 1.0)"sv,
    "vec4(0.0, 0.0, 0.0, COLOR0A0.a)"sv,
    "vec4(0.0, 0.0, 0.0, COLOR1A1.a)"sv,
    "COLOR0A0"sv,
    "COLOR1A1"sv,
    "vec4(0.0, 0.0, 0.0, 0.0)"sv,
};

constexpr std::array gkKonstColor{
    "1.0, 1.0, 1.0"sv,
    "0.875, 0.875, 0.875"sv,
    "0.75, 0.75, 0.75"sv,
    "0.625, 0.625, 0.625"sv,
    "0.5, 0.5, 0.5"sv,
    "0.375, 0.375, 0.375"sv,
    "0.25, 0.25, 0.25"sv,
    "0.125, 0.125, 0.125"sv,
    ""sv,
    ""sv,
    ""sv,
    ""sv,
    "KonstColors[0].rgb"sv,
    "KonstColors[1].rgb"sv,
    "KonstColors[2].rgb"sv,
    "KonstColors[3].rgb"sv,
    "KonstColors[0].rrr"sv,
    "KonstColors[1].rrr"sv,
    "KonstColors[2].rrr"sv,
    "KonstColors[3].rrr"sv,
    "KonstColors[0].ggg"sv,
    "KonstColors[1].ggg"sv,
    "KonstColors[2].ggg"sv,
    "KonstColors[3].ggg"sv,
    "KonstColors[0].bbb"sv,
    "KonstColors[1].bbb"sv,
    "KonstColors[2].bbb"sv,
    "KonstColors[3].bbb"sv,
    "KonstColors[0].aaa"sv,
    "KonstColors[1].aaa"sv,
    "KonstColors[2].aaa"sv,
    "KonstColors[3].aaa"sv,
};

constexpr std::array gkKonstAlpha{
    "1.0"sv,
    "0.875"sv,
    "0.75"sv,
    "0.625"sv,
    "0.5"sv,
    "0.375"sv,
    "0.25"sv,
    "0.125"sv,
    ""sv,
    ""sv,
    ""sv,
    ""sv,
    ""sv,
    ""sv,
    ""sv,
    ""sv,
    "KonstColors[0].r"sv,
    "KonstColors[1].r"sv,
    "KonstColors[2].r"sv,
    "KonstColors[3].r"sv,
    "KonstColors[0].g"sv,
    "KonstColors[1].g"sv,
    "KonstColors[2].g"sv,
    "KonstColors[3].g"sv,
    "KonstColors[0].b"sv,
    "KonstColors[1].b"sv,
    "KonstColors[2].b"sv,
    "KonstColors[3].b"sv,
    "KonstColors[0].a"sv,
    "KonstColors[1].a"sv,
    "KonstColors[2].a"sv,
    "KonstColors[3].a"sv,
};

constexpr std::array gkTevColor{
    "Prev.rgb"sv,
    "Prev.aaa"sv,
    "C0.rgb"sv,
    "C0.aaa"sv,
    "C1.rgb"sv,
    "C1.aaa"sv,
    "C2.rgb"sv,
    "C2.aaa"sv,
    "Tex.rgb"sv,
    "Tex.aaa"sv,
    "Ras.rgb"sv,
    "Ras.aaa"sv,
    "1.0, 1.0, 1.0"sv,
    "0.5, 0.5, 0.5"sv,
    "Konst.rgb"sv,
    "0.0, 0.0, 0.0"sv,
};

constexpr std::array gkTevAlpha{
    "Prev.a"sv,
    "C0.a"sv,
    "C1.a"sv,
    "C2.a"sv,
    "Tex.a"sv,
    "Ras.a"sv,
    "Konst.a"sv,
    "0.0"sv,
};

constexpr std::array gkTevRigid{
    "Prev"sv,
    "C0"sv,
    "C1"sv,
    "C2"sv,
};

CShaderGenerator::CShaderGenerator() = default;

CShaderGenerator::~CShaderGenerator() = default;

bool CShaderGenerator::CreateVertexShader(const CMaterial& rkMat)
{
    std::stringstream ShaderCode;

    ShaderCode << "#version 330 core\n"
               << "\n";

    // Input
    ShaderCode << "// Input\n";
    FVertexDescription VtxDesc = rkMat.VtxDesc();
    ASSERT(VtxDesc & EVertexAttribute::Position);

    ShaderCode                                               << "layout(location = 0) in vec3 RawPosition;\n";
    if (VtxDesc & EVertexAttribute::Normal)      ShaderCode  << "layout(location = 1) in vec3 RawNormal;\n";
    if (VtxDesc & EVertexAttribute::Color0)      ShaderCode  << "layout(location = 2) in vec4 RawColor0;\n";
    if (VtxDesc & EVertexAttribute::Color1)      ShaderCode  << "layout(location = 3) in vec4 RawColor1;\n";
    if (VtxDesc & EVertexAttribute::Tex0)        ShaderCode  << "layout(location = 4) in vec2 RawTex0;\n";
    if (VtxDesc & EVertexAttribute::Tex1)        ShaderCode  << "layout(location = 5) in vec2 RawTex1;\n";
    if (VtxDesc & EVertexAttribute::Tex2)        ShaderCode  << "layout(location = 6) in vec2 RawTex2;\n";
    if (VtxDesc & EVertexAttribute::Tex3)        ShaderCode  << "layout(location = 7) in vec2 RawTex3;\n";
    if (VtxDesc & EVertexAttribute::Tex4)        ShaderCode  << "layout(location = 8) in vec2 RawTex4;\n";
    if (VtxDesc & EVertexAttribute::Tex5)        ShaderCode  << "layout(location = 9) in vec2 RawTex5;\n";
    if (VtxDesc & EVertexAttribute::Tex6)        ShaderCode  << "layout(location = 10) in vec2 RawTex6;\n";
    if (VtxDesc & EVertexAttribute::Tex7)        ShaderCode  << "layout(location = 11) in vec2 RawTex7;\n";
    if (VtxDesc & EVertexAttribute::BoneIndices) ShaderCode  << "layout(location = 12) in int BoneIndices;\n";
    if (VtxDesc & EVertexAttribute::BoneWeights) ShaderCode  << "layout(location = 13) in vec4 BoneWeights;\n";
    ShaderCode << "\n";

    // Output
    ShaderCode << "// Output\n";
    if (VtxDesc & EVertexAttribute::Normal)  ShaderCode  << "out vec3 Normal;\n";
    if (VtxDesc & EVertexAttribute::Color0)  ShaderCode  << "out vec4 Color0;\n";
    if (VtxDesc & EVertexAttribute::Color1)  ShaderCode  << "out vec4 Color1;\n";

    for (uint32 iPass = 0; iPass < rkMat.PassCount(); iPass++)
        if (rkMat.Pass(iPass)->TexCoordSource() != 0xFF)
            ShaderCode << "out vec3 Tex" << iPass << ";\n";

    ShaderCode  << "out vec4 COLOR0A0;\n"
                << "out vec4 COLOR1A1;\n";
    ShaderCode  << "\n";

    // Uniforms
    ShaderCode  << "// Uniforms\n"
                << "layout(std140) uniform MVPBlock\n"
                << "{\n"
                << "    mat4 ModelMtx;\n"
                << "    mat4 ViewMtx;\n"
                << "    mat4 ProjMtx;\n"
                << "};\n"
                << "\n"
                << "layout(std140) uniform VertexBlock\n"
                << "{\n"
                << "    mat4 TexMtx[10];\n"
                << "    mat4 PostMtx[20];\n"
                << "    vec4 COLOR0_Amb;\n"
                << "    vec4 COLOR0_Mat;\n"
                << "    vec4 COLOR1_Amb;\n"
                << "    vec4 COLOR1_Mat;\n"
                << "};\n"
                << "\n"
                << "struct GXLight\n"
                << "{\n"
                << "    vec4 Position;\n"
                << "    vec4 Direction;\n"
                << "    vec4 Color;\n"
                << "    vec4 DistAtten;\n"
                << "    vec4 AngleAtten;\n"
                << "};\n"
                << "\n"
                << "layout(std140) uniform LightBlock\n"
                << "{\n"
                << "    GXLight Lights[8];\n"
                << "};\n"
                << "uniform int NumLights;\n"
                << "\n";

    bool HasSkinning = (rkMat.VtxDesc().HasAnyFlags(EVertexAttribute::BoneIndices | EVertexAttribute::BoneWeights));

    if (HasSkinning)
    {
        ShaderCode  << "layout(std140) uniform BoneTransformBlock\n"
                    << "{\n"
                    << "    mat4 BoneTransforms[100];\n"
                    << "};\n"
                    << "\n";
    }

    // Main
    ShaderCode  << "// Main\n"
                << "void main()\n"
                << "{\n"
                << "    mat4 MV = ModelMtx * ViewMtx;\n"
                << "    mat4 MVP = MV * ProjMtx;\n";

    if (VtxDesc & EVertexAttribute::Color0)   ShaderCode << "    Color0 = RawColor0;\n";
    if (VtxDesc & EVertexAttribute::Color1)   ShaderCode << "    Color1 = RawColor1;\n";
    ShaderCode << "\n";

    // Skinning
    if (HasSkinning)
    {
        ShaderCode  << "    // Skinning\n"
                    << "    vec3 ModelSpacePos = vec3(0,0,0);\n";

        if (VtxDesc & EVertexAttribute::Normal)
            ShaderCode  << "    vec3 ModelSpaceNormal = vec3(0,0,0);\n";

        ShaderCode  << "    \n"
                    << "    for (int iBone = 0; iBone < 4; iBone++)\n"
                    << "    {\n"
                    << "        int Shift = (8 * iBone);\n"
                    << "        int BoneIdx = (BoneIndices >> Shift) & 0xFF;\n"
                    << "        float Weight = BoneWeights[iBone];\n"
                    << "        \n"
                    << "        if (BoneIdx > 0)\n"
                    << "        {\n"
                    << "            ModelSpacePos += vec3(vec4(RawPosition, 1) * BoneTransforms[BoneIdx] * Weight);\n";

        if (VtxDesc & EVertexAttribute::Normal)
            ShaderCode  << "            ModelSpaceNormal += RawNormal.xyz * inverse(transpose(mat3(BoneTransforms[BoneIdx]))) * Weight;\n";

        ShaderCode  << "        }\n"
                    << "    }\n"
                    << "    \n";

        if (VtxDesc & EVertexAttribute::Normal)
            ShaderCode  << "    ModelSpaceNormal = normalize(ModelSpaceNormal);\n"
                        << "    \n";
    }
    else
    {
        ShaderCode  << "    vec3 ModelSpacePos = RawPosition;\n";

        if (VtxDesc & EVertexAttribute::Normal)
            ShaderCode  << "    vec3 ModelSpaceNormal = RawNormal.xyz;\n";

        ShaderCode << "\n";
    }

    ShaderCode  << "    gl_Position = vec4(ModelSpacePos, 1) * MVP;\n";

    if (VtxDesc & EVertexAttribute::Normal)
        ShaderCode  << "    Normal = normalize(ModelSpaceNormal * inverse(transpose(mat3(MV))));\n";

    // Per-vertex lighting
    ShaderCode  << "\n"
                << "    // Dynamic Lighting\n";

    // This bit could do with some cleaning up
    // It took a lot of experimentation to get dynamic lights working and I never went back and cleaned it up after
    if (rkMat.IsLightingEnabled())
    {
        ShaderCode  << "    vec4 Illum = vec4(0.0);\n"
                    << "    vec3 PositionMV = vec3(vec4(ModelSpacePos, 1.0) * MV);\n"
                    << "    \n"
                    << "    for (int iLight = 0; iLight < NumLights; iLight++)\n"
                    << "    {\n"
                    << "        vec3 LightPosMV = vec3(Lights[iLight].Position * ViewMtx);\n"
                    << "        vec3 LightDirMV = normalize(Lights[iLight].Direction.xyz * inverse(transpose(mat3(ViewMtx))));\n"
                    << "        vec3 LightDist = LightPosMV.xyz - PositionMV.xyz;\n"
                    << "        float DistSquared = dot(LightDist, LightDist);\n"
                    << "        float Dist = sqrt(DistSquared);\n"
                    << "        LightDist /= Dist;\n"
                    << "        vec3 AngleAtten = Lights[iLight].AngleAtten.xyz;\n"
                    << "        AngleAtten = vec3(AngleAtten.x, AngleAtten.y, AngleAtten.z);\n"
                    << "        float Atten = max(0, dot(LightDist, LightDirMV.xyz));\n"
                    << "        Atten = max(0, dot(AngleAtten, vec3(1.0, Atten, Atten * Atten))) / dot(Lights[iLight].DistAtten.xyz, vec3(1.0, Dist, DistSquared));\n"
                    << "        float DiffuseAtten = max(0, dot(Normal, LightDist));\n"
                    << "        Illum += (Atten * DiffuseAtten * Lights[iLight].Color);\n"
                    << "    }\n"
                    << "    COLOR0A0 = COLOR0_Mat * (Illum + COLOR0_Amb);\n"
                    << "    COLOR1A1 = COLOR1_Mat * (Illum + COLOR1_Amb);\n"
                    << "    \n";
    }

    else
    {
        ShaderCode  << "    COLOR0A0 = COLOR0_Mat;\n"
                    << "    COLOR1A1 = COLOR1_Mat;\n"
                    << "\n";
    }

    // Texture coordinate generation
    ShaderCode  << "    \n"
                << "    // TexGen\n";

    uint32 PassCount = rkMat.PassCount();

    for (uint32 iPass = 0; iPass < PassCount; iPass++)
    {
        CMaterialPass *pPass = rkMat.Pass(iPass);
        if (pPass->TexCoordSource() == 0xFF) continue;

        EUVAnimMode AnimMode = pPass->AnimMode();

        if (AnimMode == EUVAnimMode::NoUVAnim) // No animation
            ShaderCode << "    Tex" << iPass << " = vec3(" << gkCoordSrc[pPass->TexCoordSource()] << ");\n";

        else // Animation used - texture matrix at least, possibly normalize/post-transform
        {
            // Texture Matrix
            ShaderCode << "    Tex" << iPass << " = vec3(vec4(" << gkCoordSrc[pPass->TexCoordSource()] << ", 1.0) * TexMtx[" << iPass << "]).xyz;\n";

            if ((AnimMode < EUVAnimMode::UVScroll) || (AnimMode > EUVAnimMode::VFilmstrip))
            {
                // Normalization + Post-Transform
                ShaderCode  << "    Tex" << iPass << " = normalize(Tex" << iPass << ");\n"
                            << "    Tex" << iPass << " = vec3(vec4(Tex" << iPass << ", 1.0) * PostMtx[" << iPass << "]).xyz;\n";
            }
        }

        ShaderCode << "\n";
    }
    ShaderCode << "}\n\n";


    // Done!
    return mpShader->CompileVertexSource(ShaderCode.str().c_str());
}

static std::string GetColorInputExpression(const CMaterialPass* pPass, ETevColorInput iInput)
{
    if (iInput == ETevColorInput::kTextureRGB)
    {
        std::string Ret("Tex.");
        for (uint32 i = 0; i < 3; ++i)
            Ret += pPass->TexSwapComp(i);
        return Ret;
    }
    else if (iInput == ETevColorInput::kTextureAAA)
    {
        std::string Ret("Tex.");
        for (uint32 i = 0; i < 3; ++i)
            Ret += pPass->TexSwapComp(3);
        return Ret;
    }
    return std::string(gkTevColor[iInput]);
}

static std::string GetAlphaInputExpression(const CMaterialPass* pPass, ETevAlphaInput iInput)
{
    if (iInput == ETevAlphaInput::kTextureAlpha)
    {
        std::string Ret("Tex.");
        Ret += pPass->TexSwapComp(3);
        return Ret;
    }
    return std::string(gkTevAlpha[iInput]);
}

bool CShaderGenerator::CreatePixelShader(const CMaterial& rkMat)
{
    std::stringstream ShaderCode;
    ShaderCode << "#version 330 core\n"
               << "\n";

    FVertexDescription VtxDesc = rkMat.VtxDesc();
    if (VtxDesc & EVertexAttribute::Position) ShaderCode << "in vec3 Position;\n";
    if (VtxDesc & EVertexAttribute::Normal)   ShaderCode << "in vec3 Normal;\n";
    if (VtxDesc & EVertexAttribute::Color0)   ShaderCode << "in vec4 Color0;\n";
    if (VtxDesc & EVertexAttribute::Color1)   ShaderCode << "in vec4 Color1;\n";

    uint32 PassCount = rkMat.PassCount();

    for (uint32 iPass = 0; iPass < PassCount; iPass++)
        if (rkMat.Pass(iPass)->TexCoordSource() != 0xFF)
            ShaderCode << "in vec3 Tex" << iPass << ";\n";

    ShaderCode << "in vec4 COLOR0A0;\n"
               << "in vec4 COLOR1A1;\n"
               << "\n"
               << "out vec4 PixelColor;\n"
               << "\n"
               << "layout(std140) uniform PixelBlock {\n"
               << "    vec4 KonstColors[4];\n"
               << "    vec4 TevColor[4];\n"
               << "    vec4 TintColor;\n"
               << "    float LightmapMultiplier;\n"
               << "};\n\n";

    for (uint32 iPass = 0; iPass < PassCount; iPass++)
        if (rkMat.Pass(iPass)->Texture() != nullptr)
            ShaderCode << "uniform sampler2D Texture" << iPass << ";\n";

    ShaderCode <<"\n";

    ShaderCode << "void main()\n"
               << "{\n"
               << "    vec4 TevInA = vec4(0, 0, 0, 0), TevInB = vec4(0, 0, 0, 0), TevInC = vec4(0, 0, 0, 0), TevInD = vec4(0, 0, 0, 0);\n"
               << "    vec4 Prev = TevColor[0], C0 = TevColor[1], C1 = TevColor[2], C2 = TevColor[3];\n"
               << "    vec4 Ras = vec4(0, 0, 0, 1), Tex = vec4(0, 0, 0, 0);\n"
               << "    vec4 Konst = vec4(1, 1, 1, 1);\n";

    ShaderCode << "    vec2 TevCoord = vec2(0, 0);\n"
               << "    \n";

    bool Lightmap = false;
    for (uint32 iPass = 0; iPass < PassCount; iPass++)
    {
        const CMaterialPass *pPass = rkMat.Pass(iPass);
        CFourCC PassType = pPass->Type();

        ShaderCode << "    // TEV Stage " << iPass << " - " << PassType.ToString() << "\n";
        if (pPass->Type() == "DIFF") Lightmap = true;

        if (!pPass->IsEnabled())
        {
            ShaderCode << "    // Pass is disabled\n\n";
            continue;
        }

        if (pPass->TexCoordSource() != 0xFF)
            ShaderCode << "    TevCoord = (Tex" << iPass << ".z == 0.0 ? Tex" << iPass << ".xy : Tex" << iPass << ".xy / Tex" << iPass << ".z);\n";

        if (pPass->Texture())
            ShaderCode << "    Tex = texture(Texture" << iPass << ", TevCoord)";

        // Apply lightmap multiplier
        bool UseLightmapMultiplier = (PassType == "DIFF") ||
                                     (PassType == "CUST" && (rkMat.Options() & EMaterialOption::Lightmap) && iPass == 0);
        if (UseLightmapMultiplier && pPass->Texture())
            ShaderCode << " * LightmapMultiplier";

        ShaderCode << ";\n";

        ShaderCode << "    Konst = vec4(" << gkKonstColor[pPass->KColorSel()] << ", " << gkKonstAlpha[pPass->KAlphaSel()] << ");\n";

        if (pPass->RasSel() != kRasColorNull)
            ShaderCode << "    Ras = " << gkRasSel[pPass->RasSel()] << ";\n";

        for (uint8 iInput = 0; iInput < 4; iInput++)
        {
            char TevChar = iInput + 0x41; // the current stage number represented as an ASCII letter; eg 0 is 'A'

            ShaderCode << "    TevIn" << TevChar << " = vec4("
                       << GetColorInputExpression(pPass, ETevColorInput(pPass->ColorInput(iInput) & 0xF))
                       << ", "
                       << GetAlphaInputExpression(pPass, ETevAlphaInput(pPass->AlphaInput(iInput) & 0x7))
                       << ")";
            if (UseLightmapMultiplier && !pPass->Texture() && iInput == 1)
                ShaderCode << " * LightmapMultiplier";
            ShaderCode << ";\n";
        }

        ShaderCode << "    // RGB Combine\n"
                   << "    "
                   << gkTevRigid[pPass->ColorOutput()]
                   << ".rgb = ";

        ShaderCode << "clamp(vec3(TevInD.rgb + ((1.0 - TevInC.rgb) * TevInA.rgb + TevInC.rgb * TevInB.rgb)) * " << pPass->TevColorScale();
        ShaderCode << ", vec3(0, 0, 0), vec3(1.0, 1.0, 1.0));\n";

        ShaderCode << "    // Alpha Combine\n"
                   << "    "
                   << gkTevRigid[pPass->AlphaOutput()]
                   << ".a = ";

        ShaderCode << "clamp((TevInD.a + ((1.0 - TevInC.a) * TevInA.a + TevInC.a * TevInB.a)) * " << pPass->TevAlphaScale() << ", 0.0, 1.0);\n\n";
    }

    if (rkMat.Options() & EMaterialOption::Masked)
    {
        if (rkMat.Version() < EGame::CorruptionProto)
        {
            ShaderCode << "    if (Prev.a <= 0.25) discard;\n"
                       << "    else Prev.a = 1.0;\n\n";
        }
        else
        {
            ShaderCode << "    if (Prev.a <= 0.75) discard;\n"
                          "    else Prev.a = 0.0;\n\n";
        }
    }

    ShaderCode << "    PixelColor = Prev.rgba * TintColor;\n"
               << "}\n\n";

    // Done!
    return mpShader->CompilePixelSource(ShaderCode.str().c_str());
}

CShader* CShaderGenerator::GenerateShader(const CMaterial& rkMat)
{
    CShaderGenerator Generator;
    Generator.mpShader = new CShader();

    bool Success = Generator.CreateVertexShader(rkMat);
    if (Success) Success = Generator.CreatePixelShader(rkMat);

    Generator.mpShader->LinkShaders();
    return Generator.mpShader;
}
