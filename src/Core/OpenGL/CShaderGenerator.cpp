#include "Core/OpenGL/CShaderGenerator.h"

#include <Common/Macros.h>
#include "Core/NRangeUtils.h"
#include "Core/OpenGL/CShader.h"
#include "Core/Resource/CMaterial.h"
#include <array>
#include <string_view>

#include <fmt/format.h>
#include <GL/glew.h>

using namespace std::string_view_literals;

namespace
{
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

// Encapsulates general formatting facilities
struct FormatBuffer
{
    std::string buffer;

    FormatBuffer() { buffer.reserve(2048); }

    template <typename... Args>
    void Append(fmt::format_string<Args...> fmt, Args&&... args)
    {
        fmt::format_to(std::back_inserter(buffer), fmt, std::forward<Args>(args)...);
    }
    void Append(char c)
    {
        buffer += c;
    }
};
} // Anonymous namespace

CShaderGenerator::CShaderGenerator() = default;

CShaderGenerator::~CShaderGenerator() = default;

bool CShaderGenerator::CreateVertexShader(const CMaterial& rkMat)
{
    FormatBuffer ShaderCode;

    ShaderCode.Append("#version 330 core\n\n");

    // Input
    ShaderCode.Append("// Input\n");
    const FVertexDescription VtxDesc = rkMat.VtxDesc();
    ASSERT(VtxDesc.HasFlag(EVertexAttribute::Position));

    ShaderCode                                                    .Append("layout(location = 0) in vec3 RawPosition;\n");
    if (VtxDesc.HasFlag(EVertexAttribute::Normal))      ShaderCode.Append("layout(location = 1) in vec3 RawNormal;\n");
    if (VtxDesc.HasFlag(EVertexAttribute::Color0))      ShaderCode.Append("layout(location = 2) in vec4 RawColor0;\n");
    if (VtxDesc.HasFlag(EVertexAttribute::Color1))      ShaderCode.Append("layout(location = 3) in vec4 RawColor1;\n");
    if (VtxDesc.HasFlag(EVertexAttribute::Tex0))        ShaderCode.Append("layout(location = 4) in vec2 RawTex0;\n");
    if (VtxDesc.HasFlag(EVertexAttribute::Tex1))        ShaderCode.Append("layout(location = 5) in vec2 RawTex1;\n");
    if (VtxDesc.HasFlag(EVertexAttribute::Tex2))        ShaderCode.Append("layout(location = 6) in vec2 RawTex2;\n");
    if (VtxDesc.HasFlag(EVertexAttribute::Tex3))        ShaderCode.Append("layout(location = 7) in vec2 RawTex3;\n");
    if (VtxDesc.HasFlag(EVertexAttribute::Tex4))        ShaderCode.Append("layout(location = 8) in vec2 RawTex4;\n");
    if (VtxDesc.HasFlag(EVertexAttribute::Tex5))        ShaderCode.Append("layout(location = 9) in vec2 RawTex5;\n");
    if (VtxDesc.HasFlag(EVertexAttribute::Tex6))        ShaderCode.Append("layout(location = 10) in vec2 RawTex6;\n");
    if (VtxDesc.HasFlag(EVertexAttribute::Tex7))        ShaderCode.Append("layout(location = 11) in vec2 RawTex7;\n");
    if (VtxDesc.HasFlag(EVertexAttribute::BoneIndices)) ShaderCode.Append("layout(location = 12) in int BoneIndices;\n");
    if (VtxDesc.HasFlag(EVertexAttribute::BoneWeights)) ShaderCode.Append("layout(location = 13) in vec4 BoneWeights;\n");
    ShaderCode.Append('\n');

    // Output
    ShaderCode.Append("// Output\n");
    if (VtxDesc.HasFlag(EVertexAttribute::Normal)) ShaderCode.Append("out vec3 Normal;\n");
    if (VtxDesc.HasFlag(EVertexAttribute::Color0)) ShaderCode.Append("out vec4 Color0;\n");
    if (VtxDesc.HasFlag(EVertexAttribute::Color1)) ShaderCode.Append("out vec4 Color1;\n");

    for (const auto [idx, pass] : Utils::enumerate(rkMat.Passes()))
    {
        if (pass->TexCoordSource() != 0xFF)
            ShaderCode.Append("out vec3 Tex{};\n", idx);
    }

    ShaderCode.Append("out vec4 COLOR0A0;\n"
                      "out vec4 COLOR1A1;\n"
                      "\n");

    // Uniforms
    ShaderCode.Append("// Uniforms\n"
                      "layout(std140) uniform MVPBlock\n"
                      "{{\n"
                      "    mat4 ModelMtx;\n"
                      "    mat4 ViewMtx;\n"
                      "    mat4 ProjMtx;\n"
                      "}};\n"
                      "\n"
                      "layout(std140) uniform VertexBlock\n"
                      "{{\n"
                      "    mat4 TexMtx[10];\n"
                      "    mat4 PostMtx[20];\n"
                      "    vec4 COLOR0_Amb;\n"
                      "    vec4 COLOR0_Mat;\n"
                      "    vec4 COLOR1_Amb;\n"
                      "    vec4 COLOR1_Mat;\n"
                      "}};\n"
                      "\n"
                      "struct GXLight\n"
                      "{{\n"
                      "    vec4 Position;\n"
                      "    vec4 Direction;\n"
                      "    vec4 Color;\n"
                      "    vec4 DistAtten;\n"
                      "    vec4 AngleAtten;\n"
                      "}};\n"
                      "\n"
                      "layout(std140) uniform LightBlock\n"
                      "{{\n"
                      "    GXLight Lights[8];\n"
                      "}};\n"
                      "uniform int NumLights;\n"
                      "\n");

    const bool HasSkinning = (rkMat.VtxDesc().HasAnyFlags(EVertexAttribute::BoneIndices | EVertexAttribute::BoneWeights));
    if (HasSkinning)
    {
        ShaderCode.Append("layout(std140) uniform BoneTransformBlock\n"
                          "{{\n"
                          "    mat4 BoneTransforms[100];\n"
                          "}};\n"
                          "\n");
    }

    // Main
    ShaderCode.Append("// Main\n"
                      "void main()\n"
                      "{{\n"
                      "    mat4 MV = ModelMtx * ViewMtx;\n"
                      "    mat4 MVP = MV * ProjMtx;\n");

    if (VtxDesc.HasFlag(EVertexAttribute::Color0)) ShaderCode.Append("    Color0 = RawColor0;\n");
    if (VtxDesc.HasFlag(EVertexAttribute::Color1)) ShaderCode.Append("    Color1 = RawColor1;\n");
    ShaderCode.Append('\n');

    // Skinning
    if (HasSkinning)
    {
        ShaderCode.Append("    // Skinning\n"
                          "    vec3 ModelSpacePos = vec3(0,0,0);\n");

        if (VtxDesc.HasFlag(EVertexAttribute::Normal))
            ShaderCode.Append("    vec3 ModelSpaceNormal = vec3(0,0,0);\n");

        ShaderCode.Append("    \n"
                          "    for (int iBone = 0; iBone < 4; iBone++)\n"
                          "    {{\n"
                          "        int Shift = (8 * iBone);\n"
                          "        int BoneIdx = (BoneIndices >> Shift) & 0xFF;\n"
                          "        float Weight = BoneWeights[iBone];\n"
                          "        \n"
                          "        if (BoneIdx > 0)\n"
                          "        {{\n"
                          "            ModelSpacePos += vec3(vec4(RawPosition, 1) * BoneTransforms[BoneIdx] * Weight);\n");

        if (VtxDesc.HasFlag(EVertexAttribute::Normal))
            ShaderCode.Append("            ModelSpaceNormal += RawNormal.xyz * inverse(transpose(mat3(BoneTransforms[BoneIdx]))) * Weight;\n");

        ShaderCode.Append("        }}\n"
                          "    }}\n"
                          "    \n");

        if (VtxDesc.HasFlag(EVertexAttribute::Normal))
            ShaderCode.Append("    ModelSpaceNormal = normalize(ModelSpaceNormal);\n"
                              "    \n");
    }
    else
    {
        ShaderCode.Append("    vec3 ModelSpacePos = RawPosition;\n");

        if (VtxDesc.HasFlag(EVertexAttribute::Normal))
            ShaderCode.Append("    vec3 ModelSpaceNormal = RawNormal.xyz;\n");

        ShaderCode.Append('\n');
    }

    ShaderCode.Append("    gl_Position = vec4(ModelSpacePos, 1) * MVP;\n");

    if (VtxDesc.HasFlag(EVertexAttribute::Normal))
        ShaderCode.Append("    Normal = normalize(ModelSpaceNormal * inverse(transpose(mat3(MV))));\n");

    // Per-vertex lighting
    ShaderCode.Append("\n"
                      "    // Dynamic Lighting\n");

    // This bit could do with some cleaning up
    // It took a lot of experimentation to get dynamic lights working and I never went back and cleaned it up after
    if (rkMat.IsLightingEnabled())
    {
        ShaderCode.Append("    vec4 Illum = vec4(0.0);\n"
                          "    vec3 PositionMV = vec3(vec4(ModelSpacePos, 1.0) * MV);\n"
                          "    \n"
                          "    for (int iLight = 0; iLight < NumLights; iLight++)\n"
                          "    {{\n"
                          "        vec3 LightPosMV = vec3(Lights[iLight].Position * ViewMtx);\n"
                          "        vec3 LightDirMV = normalize(Lights[iLight].Direction.xyz * inverse(transpose(mat3(ViewMtx))));\n"
                          "        vec3 LightDist = LightPosMV.xyz - PositionMV.xyz;\n"
                          "        float DistSquared = dot(LightDist, LightDist);\n"
                          "        float Dist = sqrt(DistSquared);\n"
                          "        LightDist /= Dist;\n"
                          "        vec3 AngleAtten = Lights[iLight].AngleAtten.xyz;\n"
                          "        AngleAtten = vec3(AngleAtten.x, AngleAtten.y, AngleAtten.z);\n"
                          "        float Atten = max(0, dot(LightDist, LightDirMV.xyz));\n"
                          "        Atten = max(0, dot(AngleAtten, vec3(1.0, Atten, Atten * Atten))) / dot(Lights[iLight].DistAtten.xyz, vec3(1.0, Dist, DistSquared));\n"
                          "        float DiffuseAtten = max(0, dot(Normal, LightDist));\n"
                          "        Illum += (Atten * DiffuseAtten * Lights[iLight].Color);\n"
                          "    }}\n"
                          "    COLOR0A0 = COLOR0_Mat * (Illum + COLOR0_Amb);\n"
                          "    COLOR1A1 = COLOR1_Mat * (Illum + COLOR1_Amb);\n"
                          "    \n");
    }
    else
    {
        ShaderCode.Append("    COLOR0A0 = COLOR0_Mat;\n"
                          "    COLOR1A1 = COLOR1_Mat;\n"
                          "\n");
    }

    // Texture coordinate generation
    ShaderCode.Append("    \n"
                      "    // TexGen\n");

    for (const auto [idx, pass] : Utils::enumerate(rkMat.Passes()))
    {
        if (pass->TexCoordSource() == 0xFF)
            continue;

        const auto AnimMode = pass->AnimMode();
        if (AnimMode == EUVAnimMode::NoUVAnim) // No animation
        {
            ShaderCode.Append("    Tex{} = vec3({});\n", idx, gkCoordSrc[pass->TexCoordSource()]);
        }
        else // Animation used - texture matrix at least, possibly normalize/post-transform
        {
            // Texture Matrix
            ShaderCode.Append("    Tex{} = vec3(vec4({}, 1.0) * TexMtx[{}]).xyz;\n",
                              idx, gkCoordSrc[pass->TexCoordSource()], idx);

            if ((AnimMode < EUVAnimMode::UVScroll) || (AnimMode > EUVAnimMode::VFilmstrip))
            {
                // Normalization + Post-Transform
                ShaderCode.Append("    Tex{0} = normalize(Tex{0});\n"
                                  "    Tex{0} = vec3(vec4(Tex{0}, 1.0) * PostMtx[{0}]).xyz;\n", idx);
            }
        }

        ShaderCode.Append('\n');
    }
    ShaderCode.Append("}}\n\n");


    // Done!
    return mpShader->CompileVertexSource(ShaderCode.buffer);
}

static std::string GetColorInputExpression(const CMaterialPass* pPass, ETevColorInput iInput)
{
    if (iInput == ETevColorInput::kTextureRGB)
    {
        std::string Ret("Tex.");
        for (uint32_t i = 0; i < 3; ++i)
            Ret += pPass->TexSwapComp(i);
        return Ret;
    }

    if (iInput == ETevColorInput::kTextureAAA)
        return std::string("Tex.").append(3, pPass->TexSwapComp(3));

    return std::string(gkTevColor[iInput]);
}

static std::string GetAlphaInputExpression(const CMaterialPass* pPass, ETevAlphaInput iInput)
{
    if (iInput == ETevAlphaInput::kTextureAlpha)
        return std::string("Tex.").append(1, pPass->TexSwapComp(3));

    return std::string(gkTevAlpha[iInput]);
}

bool CShaderGenerator::CreatePixelShader(const CMaterial& rkMat)
{
    FormatBuffer ShaderCode;
    ShaderCode.Append("#version 330 core\n\n");

    const FVertexDescription VtxDesc = rkMat.VtxDesc();
    if (VtxDesc.HasFlag(EVertexAttribute::Position)) ShaderCode.Append("in vec3 Position;\n");
    if (VtxDesc.HasFlag(EVertexAttribute::Normal))   ShaderCode.Append("in vec3 Normal;\n");
    if (VtxDesc.HasFlag(EVertexAttribute::Color0))   ShaderCode.Append("in vec4 Color0;\n");
    if (VtxDesc.HasFlag(EVertexAttribute::Color1))   ShaderCode.Append("in vec4 Color1;\n");

    for (const auto [idx, pass] : Utils::enumerate(rkMat.Passes()))
    {
        if (pass->TexCoordSource() != 0xFF)
            ShaderCode.Append("in vec3 Tex{};\n", idx);
    }

    ShaderCode.Append("in vec4 COLOR0A0;\n"
                      "in vec4 COLOR1A1;\n"
                      "\n"
                      "out vec4 PixelColor;\n"
                      "\n"
                      "layout(std140) uniform PixelBlock {{\n"
                      "    vec4 KonstColors[4];\n"
                      "    vec4 TevColor[4];\n"
                      "    vec4 TintColor;\n"
                      "    float LightmapMultiplier;\n"
                      "}};\n\n");

    for (const auto [idx, pass] : Utils::enumerate(rkMat.Passes()))
    {
        if (pass->Texture() != nullptr)
            ShaderCode.Append("uniform sampler2D Texture{};\n", idx);
    }

    ShaderCode.Append('\n');

    ShaderCode.Append("void main()\n"
                      "{{\n"
                      "    vec4 TevInA = vec4(0, 0, 0, 0), TevInB = vec4(0, 0, 0, 0), TevInC = vec4(0, 0, 0, 0), TevInD = vec4(0, 0, 0, 0);\n"
                      "    vec4 Prev = TevColor[0], C0 = TevColor[1], C1 = TevColor[2], C2 = TevColor[3];\n"
                      "    vec4 Ras = vec4(0, 0, 0, 1), Tex = vec4(0, 0, 0, 0);\n"
                      "    vec4 Konst = vec4(1, 1, 1, 1);\n");

    ShaderCode.Append("    vec2 TevCoord = vec2(0, 0);\n"
                      "    \n");

    for (const auto [idx, pass] : Utils::enumerate(rkMat.Passes()))
    {
        const CFourCC PassType = pass->Type();
        ShaderCode.Append("    // TEV Stage {} - {}\n", idx, PassType.ToString());

        if (!pass->IsEnabled())
        {
            ShaderCode.Append("    // Pass is disabled\n\n");
            continue;
        }

        if (pass->TexCoordSource() != 0xFF)
            ShaderCode.Append("    TevCoord = (Tex{0}.z == 0.0 ? Tex{0}.xy : Tex{0}.xy / Tex{0}.z);\n", idx);

        if (pass->Texture())
            ShaderCode.Append("    Tex = texture(Texture{}, TevCoord)", idx);

        // Apply lightmap multiplier
        const bool UseLightmapMultiplier = (PassType == CFourCC("DIFF")) ||
                                           (PassType == CFourCC("CUST") && rkMat.Options().HasFlag(EMaterialOption::Lightmap) && idx == 0);
        if (UseLightmapMultiplier && pass->Texture())
            ShaderCode.Append(" * LightmapMultiplier");

        ShaderCode.Append(";\n");

        ShaderCode.Append("    Konst = vec4({}, {});\n", gkKonstColor[pass->KColorSel()], gkKonstAlpha[pass->KAlphaSel()]);

        if (pass->RasSel() != kRasColorNull)
            ShaderCode.Append("    Ras = {};\n", gkRasSel[pass->RasSel()]);

        for (uint8_t iInput = 0; iInput < 4; iInput++)
        {
            // The current stage number represented as an ASCII letter; eg 0 is 'A'
            const char TevChar = iInput + 0x41;

            ShaderCode.Append("    TevIn{} = vec4({}, {})", TevChar,
                              GetColorInputExpression(pass, ETevColorInput(pass->ColorInput(iInput) & 0xF)),
                              GetAlphaInputExpression(pass, ETevAlphaInput(pass->AlphaInput(iInput) & 0x7)));
            if (UseLightmapMultiplier && !pass->Texture() && iInput == 1)
                ShaderCode.Append(" * LightmapMultiplier");
            ShaderCode.Append(";\n");
        }

        ShaderCode.Append("    // RGB Combine\n"
                          "    {}.rgb = ", gkTevRigid[pass->ColorOutput()]);

        ShaderCode.Append("clamp(vec3(TevInD.rgb + ((1.0 - TevInC.rgb) * TevInA.rgb + TevInC.rgb * TevInB.rgb)) * {}", pass->TevColorScale());
        ShaderCode.Append(", vec3(0, 0, 0), vec3(1.0, 1.0, 1.0));\n");

        ShaderCode.Append("    // Alpha Combine\n"
                          "    {}.a = ", gkTevRigid[pass->AlphaOutput()]);

        ShaderCode.Append("clamp((TevInD.a + ((1.0 - TevInC.a) * TevInA.a + TevInC.a * TevInB.a)) * {}, 0.0, 1.0);\n\n", pass->TevAlphaScale());
    }

    if (rkMat.Options().HasFlag(EMaterialOption::Masked))
    {
        if (rkMat.Version() < EGame::CorruptionProto)
        {
            ShaderCode.Append("    if (Prev.a <= 0.25) discard;\n"
                              "    else Prev.a = 1.0;\n\n");
        }
        else
        {
            ShaderCode.Append("    if (Prev.a <= 0.75) discard;\n"
                              "    else Prev.a = 0.0;\n\n");
        }
    }

    ShaderCode.Append("    PixelColor = Prev.rgba * TintColor;\n"
                      "}}\n\n");

    // Done!
    return mpShader->CompilePixelSource(ShaderCode.buffer);
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
