#include "CShaderGenerator.h"
#include <Common/Macros.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>

const TString gkCoordSrc[] = {
    "ModelSpacePos.xyz",
    "ModelSpaceNormal.xyz",
    "0.0, 0.0, 0.0",
    "0.0, 0.0, 0.0",
    "RawTex0.xy, 1.0",
    "RawTex1.xy, 1.0",
    "RawTex2.xy, 1.0",
    "RawTex3.xy, 1.0",
    "RawTex4.xy, 1.0",
    "RawTex5.xy, 1.0",
    "RawTex6.xy, 1.0",
    "RawTex7.xy, 1.0"
};

const TString gkRasSel[] = {
    "vec4(COLOR0A0.rgb, 1.0)",
    "vec4(COLOR1A1.rgb, 1.0)",
    "vec4(0.0, 0.0, 0.0, COLOR0A0.a)",
    "vec4(0.0, 0.0, 0.0, COLOR1A1.a)",
    "COLOR0A0",
    "COLOR1A1",
    "vec4(0.0, 0.0, 0.0, 0.0)"
};

const TString gkKonstColor[] = {
    "1.0, 1.0, 1.0",
    "0.875, 0.875, 0.875",
    "0.75, 0.75, 0.75",
    "0.625, 0.625, 0.625",
    "0.5, 0.5, 0.5",
    "0.375, 0.375, 0.375",
    "0.25, 0.25, 0.25",
    "0.125, 0.125, 0.125",
    "",
    "",
    "",
    "",
    "KonstColors[0].rgb",
    "KonstColors[1].rgb",
    "KonstColors[2].rgb",
    "KonstColors[3].rgb",
    "KonstColors[0].rrr",
    "KonstColors[1].rrr",
    "KonstColors[2].rrr",
    "KonstColors[3].rrr",
    "KonstColors[0].ggg",
    "KonstColors[1].ggg",
    "KonstColors[2].ggg",
    "KonstColors[3].ggg",
    "KonstColors[0].bbb",
    "KonstColors[1].bbb",
    "KonstColors[2].bbb",
    "KonstColors[3].bbb",
    "KonstColors[0].aaa",
    "KonstColors[1].aaa",
    "KonstColors[2].aaa",
    "KonstColors[3].aaa"
};

const TString gkKonstAlpha[] = {
    "1.0",
    "0.875",
    "0.75",
    "0.625",
    "0.5",
    "0.375",
    "0.25",
    "0.125",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "KonstColors[0].r",
    "KonstColors[1].r",
    "KonstColors[2].r",
    "KonstColors[3].r",
    "KonstColors[0].g",
    "KonstColors[1].g",
    "KonstColors[2].g",
    "KonstColors[3].g",
    "KonstColors[0].b",
    "KonstColors[1].b",
    "KonstColors[2].b",
    "KonstColors[3].b",
    "KonstColors[0].a",
    "KonstColors[1].a",
    "KonstColors[2].a",
    "KonstColors[3].a"
};

const TString gkTevColor[] = {
    "Prev.rgb",
    "Prev.aaa",
    "C0.rgb",
    "C0.aaa",
    "C1.rgb",
    "C1.aaa",
    "C2.rgb",
    "C2.aaa",
    "Tex.rgb",
    "Tex.aaa",
    "Ras.rgb",
    "Ras.aaa",
    "1.0, 1.0, 1.0",
    "0.5, 0.5, 0.5",
    "Konst.rgb",
    "0.0, 0.0, 0.0"
};

const TString gkTevAlpha[] = {
    "Prev.a",
    "C0.a",
    "C1.a",
    "C2.a",
    "Tex.a",
    "Ras.a",
    "Konst.a",
    "0.0"
};

const TString gkTevRigid[] = {
    "Prev",
    "C0",
    "C1",
    "C2"
};

CShaderGenerator::CShaderGenerator()
{
}

CShaderGenerator::~CShaderGenerator()
{
}

bool CShaderGenerator::CreateVertexShader(const CMaterial& rkMat)
{
    std::stringstream ShaderCode;

    ShaderCode << "#version 330 core\n"
               << "\n";

    // Input
    ShaderCode << "// Input\n";
    FVertexDescription VtxDesc = rkMat.VtxDesc();
    ASSERT(VtxDesc & ePosition);

    ShaderCode                              << "layout(location = 0) in vec3 RawPosition;\n";
    if (VtxDesc & eNormal)      ShaderCode  << "layout(location = 1) in vec3 RawNormal;\n";
    if (VtxDesc & eColor0)      ShaderCode  << "layout(location = 2) in vec4 RawColor0;\n";
    if (VtxDesc & eColor1)      ShaderCode  << "layout(location = 3) in vec4 RawColor1;\n";
    if (VtxDesc & eTex0)        ShaderCode  << "layout(location = 4) in vec2 RawTex0;\n";
    if (VtxDesc & eTex1)        ShaderCode  << "layout(location = 5) in vec2 RawTex1;\n";
    if (VtxDesc & eTex2)        ShaderCode  << "layout(location = 6) in vec2 RawTex2;\n";
    if (VtxDesc & eTex3)        ShaderCode  << "layout(location = 7) in vec2 RawTex3;\n";
    if (VtxDesc & eTex4)        ShaderCode  << "layout(location = 8) in vec2 RawTex4;\n";
    if (VtxDesc & eTex5)        ShaderCode  << "layout(location = 9) in vec2 RawTex5;\n";
    if (VtxDesc & eTex6)        ShaderCode  << "layout(location = 10) in vec2 RawTex6;\n";
    if (VtxDesc & eTex7)        ShaderCode  << "layout(location = 11) in vec2 RawTex7;\n";
    if (VtxDesc & eBoneIndices) ShaderCode  << "layout(location = 12) in int BoneIndices;\n";
    if (VtxDesc & eBoneWeights) ShaderCode  << "layout(location = 13) in vec4 BoneWeights;\n";
    ShaderCode << "\n";

    // Output
    ShaderCode << "// Output\n";
    if (VtxDesc & eNormal)  ShaderCode  << "out vec3 Normal;\n";
    if (VtxDesc & eColor0)  ShaderCode  << "out vec4 Color0;\n";
    if (VtxDesc & eColor1)  ShaderCode  << "out vec4 Color1;\n";

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

    bool HasSkinning = (rkMat.VtxDesc().HasAnyFlags(eBoneIndices | eBoneWeights));

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

    if (VtxDesc & eColor0)   ShaderCode << "    Color0 = RawColor0;\n";
    if (VtxDesc & eColor1)   ShaderCode << "    Color1 = RawColor1;\n";
    ShaderCode << "\n";

    // Skinning
    if (HasSkinning)
    {
        ShaderCode  << "    // Skinning\n"
                    << "    vec3 ModelSpacePos = vec3(0,0,0);\n";

        if (VtxDesc & eNormal)
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

        if (VtxDesc & eNormal)
            ShaderCode  << "            ModelSpaceNormal += RawNormal.xyz * inverse(transpose(mat3(BoneTransforms[BoneIdx]))) * Weight;\n";

        ShaderCode  << "        }\n"
                    << "    }\n"
                    << "    \n";

        if (VtxDesc & eNormal)
            ShaderCode  << "    ModelSpaceNormal = normalize(ModelSpaceNormal);\n"
                        << "    \n";
    }
    else
    {
        ShaderCode  << "    vec3 ModelSpacePos = RawPosition;\n";

        if (VtxDesc & eNormal)
            ShaderCode  << "    vec3 ModelSpaceNormal = RawNormal.xyz;\n";

        ShaderCode << "\n";
    }

    ShaderCode  << "    gl_Position = vec4(ModelSpacePos, 1) * MVP;\n";

    if (VtxDesc & eNormal)
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

        if (AnimMode == eNoUVAnim) // No animation
            ShaderCode << "    Tex" << iPass << " = vec3(" << gkCoordSrc[pPass->TexCoordSource()] << ");\n";

        else // Animation used - texture matrix at least, possibly normalize/post-transform
        {
            // Texture Matrix
            ShaderCode << "    Tex" << iPass << " = vec3(vec4(" << gkCoordSrc[pPass->TexCoordSource()] << ", 1.0) * TexMtx[" << iPass << "]).xyz;\n";

            if ((AnimMode < 2) || (AnimMode > 5))
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

bool CShaderGenerator::CreatePixelShader(const CMaterial& rkMat)
{
    std::stringstream ShaderCode;
    ShaderCode << "#version 330 core\n"
               << "\n";

    FVertexDescription VtxDesc = rkMat.VtxDesc();
    if (VtxDesc & ePosition) ShaderCode << "in vec3 Position;\n";
    if (VtxDesc & eNormal)   ShaderCode << "in vec3 Normal;\n";
    if (VtxDesc & eColor0)   ShaderCode << "in vec4 Color0;\n";
    if (VtxDesc & eColor1)   ShaderCode << "in vec4 Color1;\n";

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
               << "    vec4 TevColor;\n"
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
               << "    vec4 Prev = vec4(0, 0, 0, 0), C0 = TevColor, C1 = C0, C2 = C0;\n"
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

        if (pPass->Texture() != nullptr)
            ShaderCode << "    Tex = texture(Texture" << iPass << ", TevCoord)";

        // A couple pass types require special swizzles to access different texture color channels as alpha
        if ((PassType == "TRAN") || (PassType == "INCA") || (PassType == "BLOI"))
            ShaderCode << ".rgbr";
        else if (PassType == "BLOL")
            ShaderCode << ".rgbg";

        // Apply lightmap multiplier
        if ( (PassType == "DIFF") ||
             (PassType == "CUST" && (rkMat.Options() & CMaterial::eLightmap) && iPass == 0) )
            ShaderCode << " * LightmapMultiplier";

        ShaderCode << ";\n";

        ShaderCode << "    Konst = vec4(" << gkKonstColor[pPass->KColorSel()] << ", " << gkKonstAlpha[pPass->KAlphaSel()] << ");\n";

        if (pPass->RasSel() != eRasColorNull)
            ShaderCode << "    Ras = " << gkRasSel[pPass->RasSel()] << ";\n";

        for (uint8 iInput = 0; iInput < 4; iInput++)
        {
            char TevChar = iInput + 0x41; // the current stage number represented as an ASCII letter; eg 0 is 'A'

            ShaderCode << "    TevIn" << TevChar << " = vec4("
                       << gkTevColor[pPass->ColorInput(iInput) & 0xF]
                       << ", "
                       << gkTevAlpha[pPass->AlphaInput(iInput) & 0x7]
                       << ");\n";
        }

        ShaderCode << "    // RGB Combine\n"
                   << "    "
                   << gkTevRigid[pPass->ColorOutput()]
                   << ".rgb = ";

        ShaderCode << "clamp(vec3(TevInD.rgb + ((1.0 - TevInC.rgb) * TevInA.rgb + TevInC.rgb * TevInB.rgb))";
        if ((PassType == "CLR ") && (Lightmap)) ShaderCode << "* (2.0 - (1.0 - LightmapMultiplier))"; // Apply tevscale 2.0 on the color pass if lightmap is present. Scale so we don't apply if lightmaps are off.
        ShaderCode << ", vec3(0, 0, 0), vec3(1.0, 1.0, 1.0));\n";

        ShaderCode << "    // Alpha Combine\n"
                   << "    "
                   << gkTevRigid[pPass->AlphaOutput()]
                   << ".a = ";

        ShaderCode << "clamp(TevInD.a + ((1.0 - TevInC.a) * TevInA.a + TevInC.a * TevInB.a), 0.0, 1.0);\n\n";
    }

    if (rkMat.Options() & CMaterial::ePunchthrough)
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
