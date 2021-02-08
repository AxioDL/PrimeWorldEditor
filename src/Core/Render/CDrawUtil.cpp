#include "CDrawUtil.h"
#include "CGraphics.h"
#include "Core/GameProject/CResourceStore.h"
#include <Common/Log.h>
#include <Common/Math/CTransform4f.h>

// ************ PUBLIC ************
void CDrawUtil::DrawGrid(CColor LineColor, CColor BoldLineColor)
{
    Init();

    mGridVertices->Bind();

    CGraphics::sMVPBlock.ModelMatrix = CMatrix4f::skIdentity;
    CGraphics::UpdateMVPBlock();

    glBlendFunc(GL_ONE, GL_ZERO);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);

    glLineWidth(1.0f);
    LineColor.A = 0.f;
    UseColorShader(LineColor);
    mGridIndices.DrawElements(0, mGridIndices.GetSize() - 4);

    glLineWidth(1.5f);
    BoldLineColor.A = 0.f;
    UseColorShader(BoldLineColor);
    mGridIndices.DrawElements(mGridIndices.GetSize() - 4, 4);
}

void CDrawUtil::DrawSquare()
{
    // Overload with default tex coords
    CVector2f TexCoords[4] = { CVector2f(0.f, 1.f), CVector2f(1.f, 1.f), CVector2f(1.f, 0.f), CVector2f(0.f, 0.f) };
    DrawSquare(&TexCoords[0].X);
}

void CDrawUtil::DrawSquare(const CVector2f& TexUL, const CVector2f& TexUR, const CVector2f& TexBR, const CVector2f& TexBL)
{
    // Overload with tex coords specified via parameters
    // I don't think that parameters are guaranteed to be contiguous in memory, so:
    CVector2f TexCoords[4] = { TexUL, TexUR, TexBR, TexBL };
    DrawSquare(&TexCoords[0].X);
}

void CDrawUtil::DrawSquare(const float *pTexCoords)
{
    Init();

    // Set tex coords
    for (uint32 iTex = 0; iTex < 8; iTex++)
    {
        EVertexAttribute TexAttrib = (EVertexAttribute) ((uint) (EVertexAttribute::Tex0) << iTex);
        mSquareVertices->BufferAttrib(TexAttrib, pTexCoords);
    }

    // Draw
    mSquareVertices->Bind();
    mSquareIndices.DrawElements();
    mSquareVertices->Unbind();
}

void CDrawUtil::DrawLine(const CVector3f& PointA, const CVector3f& PointB)
{
    DrawLine(PointA, PointB, CColor::White());
}

void CDrawUtil::DrawLine(const CVector2f& PointA, const CVector2f& PointB)
{
    // Overload for 2D lines
    DrawLine(CVector3f(PointA.X, PointA.Y, 0.f), CVector3f(PointB.X, PointB.Y, 0.f), CColor::White());
}

void CDrawUtil::DrawLine(const CVector3f& PointA, const CVector3f& PointB, const CColor& LineColor)
{
    Init();

    // Copy vec3s into an array to ensure they are adjacent in memory
    CVector3f Points[2] = { PointA, PointB };
    mLineVertices->BufferAttrib(EVertexAttribute::Position, Points);

    // Draw
    UseColorShader(LineColor);
    mLineVertices->Bind();
    mLineIndices.DrawElements();
    mLineVertices->Unbind();
}

void CDrawUtil::DrawLine(const CVector2f& PointA, const CVector2f& PointB, const CColor& LineColor)
{
    // Overload for 2D lines
    DrawLine(CVector3f(PointA.X, PointA.Y, 0.f), CVector3f(PointB.X, PointB.Y, 0.f), LineColor);
}

void CDrawUtil::DrawCube()
{
    Init();
    mpCubeModel->Draw(ERenderOption::NoMaterialSetup, 0);
}

void CDrawUtil::DrawCube(const CColor& Color)
{
    Init();
    UseColorShader(Color);
    DrawCube();
}

void CDrawUtil::DrawCube(const CVector3f& Position, const CColor& Color)
{
    CGraphics::sMVPBlock.ModelMatrix = CTransform4f::TranslationMatrix(Position);
    CGraphics::UpdateMVPBlock();
    UseColorShader(Color);
    DrawCube();
}

void CDrawUtil::DrawShadedCube(const CColor& Color)
{
    Init();
    UseColorShaderLighting(Color);
    DrawCube();
}

void CDrawUtil::DrawWireCube()
{
    Init();
    glLineWidth(1.f);
    mWireCubeVertices->Bind();
    mWireCubeIndices.DrawElements();
    mWireCubeVertices->Unbind();
}

void CDrawUtil::DrawWireCube(const CAABox& kAABox, const CColor& kColor)
{
    Init();

    // Calculate model matrix
    CTransform4f Transform;
    Transform.Scale(kAABox.Size());
    Transform.Translate(kAABox.Center());
    CGraphics::sMVPBlock.ModelMatrix = Transform;
    CGraphics::UpdateMVPBlock();

    UseColorShader(kColor);
    DrawWireCube();
}

void CDrawUtil::DrawSphere(bool DoubleSided)
{
    Init();

    if (!DoubleSided)
        mpSphereModel->Draw(ERenderOption::NoMaterialSetup, 0);
    else
        mpDoubleSidedSphereModel->Draw(ERenderOption::NoMaterialSetup, 0);
}

void CDrawUtil::DrawSphere(const CColor &kColor)
{
    Init();
    UseColorShader(kColor);
    DrawSphere(false);
}

void CDrawUtil::DrawWireSphere(const CVector3f& Position, float Radius, const CColor& Color /*= CColor::skWhite*/)
{
    Init();

    // Create model matrix
    CTransform4f Transform;
    Transform.Scale(Radius);
    Transform.Translate(Position);
    CGraphics::sMVPBlock.ModelMatrix = Transform;
    CGraphics::UpdateMVPBlock();

    // Set other render params
    UseColorShader(Color);
    CMaterial::KillCachedMaterial();
    glBlendFunc(GL_ONE, GL_ZERO);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);

    // Draw
    mpWireSphereModel->Draw(ERenderOption::NoMaterialSetup, 0);
}

void CDrawUtil::DrawBillboard(CTexture* pTexture, const CVector3f& Position, const CVector2f& Scale /*= CVector2f::skOne*/, const CColor& Tint /*= CColor::skWhite*/)
{
    Init();

    // Create translation-only model matrix
    CGraphics::sMVPBlock.ModelMatrix = CTransform4f::TranslationMatrix(Position);
    CGraphics::UpdateMVPBlock();

    // Set uniforms
    mpBillboardShader->SetCurrent();

    static GLuint ScaleLoc = mpBillboardShader->GetUniformLocation("BillboardScale");
    glUniform2f(ScaleLoc, Scale.X, Scale.Y);

    static GLuint TintLoc = mpBillboardShader->GetUniformLocation("TintColor");
    glUniform4f(TintLoc, Tint.R, Tint.G, Tint.B, Tint.A);

    pTexture->Bind(0);

    // Set other properties
    CMaterial::KillCachedMaterial();
    glBlendFunc(GL_ONE, GL_ZERO);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);

    // Draw
    DrawSquare();
}

void CDrawUtil::DrawLightBillboard(ELightType Type, const CColor& LightColor, const CVector3f& Position, const CVector2f& Scale /*= CVector2f::skOne*/, const CColor& Tint /*= CColor::skWhite*/)
{
    Init();

    // Create translation-only model matrix
    CGraphics::sMVPBlock.ModelMatrix = CTransform4f::TranslationMatrix(Position);
    CGraphics::UpdateMVPBlock();

    // Set uniforms
    mpLightBillboardShader->SetCurrent();

    static GLuint ScaleLoc = mpLightBillboardShader->GetUniformLocation("BillboardScale");
    glUniform2f(ScaleLoc, Scale.X, Scale.Y);

    static GLuint ColorLoc = mpLightBillboardShader->GetUniformLocation("LightColor");
    glUniform4f(ColorLoc, LightColor.R, LightColor.G, LightColor.B, LightColor.A);

    static GLuint TintLoc = mpLightBillboardShader->GetUniformLocation("TintColor");
    glUniform4f(TintLoc, Tint.R, Tint.G, Tint.B, Tint.A);

    CTexture *pTexA = GetLightTexture(Type);
    CTexture *pTexB = GetLightMask(Type);
    pTexA->Bind(0);
    pTexB->Bind(1);

    static GLuint TextureLoc = mpLightBillboardShader->GetUniformLocation("Texture");
    static GLuint MaskLoc    = mpLightBillboardShader->GetUniformLocation("LightMask");
    glUniform1i(TextureLoc, 0);
    glUniform1i(MaskLoc, 1);

    // Set other properties
    CMaterial::KillCachedMaterial();
    glBlendFunc(GL_ONE, GL_ZERO);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);

    // Draw
    DrawSquare();

}

void CDrawUtil::UseColorShader(const CColor& kColor)
{
    Init();
    mpColorShader->SetCurrent();

    static GLuint ColorLoc = mpColorShader->GetUniformLocation("ColorIn");
    glUniform4f(ColorLoc, kColor.R, kColor.G, kColor.B, kColor.A);

    CMaterial::KillCachedMaterial();
}

void CDrawUtil::UseColorShaderLighting(const CColor& kColor)
{
    Init();
    mpColorShaderLighting->SetCurrent();

    static GLuint NumLightsLoc = mpColorShaderLighting->GetUniformLocation("NumLights");
    glUniform1i(NumLightsLoc, CGraphics::sNumLights);

    static GLuint ColorLoc = mpColorShaderLighting->GetUniformLocation("ColorIn");
    glUniform4f(ColorLoc, kColor.R, kColor.G, kColor.B, kColor.A);

    CMaterial::KillCachedMaterial();
}

void CDrawUtil::UseTextureShader()
{
    UseTextureShader(CColor::White());
}

void CDrawUtil::UseTextureShader(const CColor& TintColor)
{
    Init();
    mpTextureShader->SetCurrent();

    static GLuint TintColorLoc = mpTextureShader->GetUniformLocation("TintColor");
    glUniform4f(TintColorLoc, TintColor.R, TintColor.G, TintColor.B, TintColor.A);

    CMaterial::KillCachedMaterial();
}

void CDrawUtil::UseCollisionShader(bool IsFloor, bool IsUnstandable, const CColor& TintColor /*= CColor::skWhite*/)
{
    Init();
    mpCollisionShader->SetCurrent();

    // Force blend mode to opaque + set alpha to 0 to ensure collision geometry isn't bloomed
    glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ZERO, GL_ZERO);

    static GLuint TintColorLoc = mpCollisionShader->GetUniformLocation("TintColor");
    glUniform4f(TintColorLoc, TintColor.R, TintColor.G, TintColor.B, TintColor.A);

    static GLuint IsFloorLoc = mpCollisionShader->GetUniformLocation("IsFloor");
    glUniform1i(IsFloorLoc, IsFloor ? 1 : 0);

    static GLuint IsUnstandableLoc = mpCollisionShader->GetUniformLocation("IsUnstandable");
    glUniform1i(IsUnstandableLoc, IsUnstandable ? 1 : 0);

    CMaterial::KillCachedMaterial();
}

CShader* CDrawUtil::GetTextShader()
{
    Init();
    return mpTextShader.get();
}

void CDrawUtil::LoadCheckerboardTexture(uint32 GLTextureUnit)
{
    Init();
    mpCheckerTexture->Bind(GLTextureUnit);
}

CTexture* CDrawUtil::GetLightTexture(ELightType Type)
{
    Init();
    return mpLightTextures[static_cast<size_t>(Type)];
}

CTexture* CDrawUtil::GetLightMask(ELightType Type)
{
    Init();
    return mpLightMasks[static_cast<size_t>(Type)];
}

CModel* CDrawUtil::GetCubeModel()
{
    Init();
    return mpCubeModel;
}

// ************ PRIVATE ************
CDrawUtil::CDrawUtil()
{
}

void CDrawUtil::Init()
{
    if (!mDrawUtilInitialized)
    {
        debugf("Initializing CDrawUtil");
        InitGrid();
        InitSquare();
        InitLine();
        InitCube();
        InitWireCube();
        InitSphere();
        InitWireSphere();
        InitShaders();
        InitTextures();
        mDrawUtilInitialized = true;
    }
}

void CDrawUtil::InitGrid()
{
    debugf("Creating grid");

    const int kGridSize = 501; // must be odd
    const float kGridSpacing = 1.f;
    const int MinIdx = (kGridSize - 1) / -2;
    const int MaxIdx = (kGridSize - 1) / 2;

    mGridVertices.emplace();
    mGridVertices->SetVertexDesc(EVertexAttribute::Position);
    mGridVertices->Reserve(static_cast<size_t>(kGridSize * 4));

     for (int32 i = MinIdx; i <= MaxIdx; i++)
     {
         if (i == 0)
             continue;

         mGridVertices->AddVertex(CVector3f(MinIdx * kGridSpacing, i * kGridSpacing, 0.0f));
         mGridVertices->AddVertex(CVector3f(MaxIdx * kGridSpacing, i * kGridSpacing, 0.0f));
         mGridVertices->AddVertex(CVector3f(i * kGridSpacing, MinIdx * kGridSpacing, 0.0f));
         mGridVertices->AddVertex(CVector3f(i * kGridSpacing, MaxIdx * kGridSpacing, 0.0f));
     }

     mGridVertices->AddVertex(CVector3f(MinIdx * kGridSpacing, 0, 0.0f));
     mGridVertices->AddVertex(CVector3f(MaxIdx * kGridSpacing, 0, 0.0f));
     mGridVertices->AddVertex(CVector3f(0, MinIdx * kGridSpacing, 0.0f));
     mGridVertices->AddVertex(CVector3f(0, MaxIdx * kGridSpacing, 0.0f));

     const auto NumIndices = static_cast<size_t>(kGridSize * 4);
     mGridIndices.Reserve(NumIndices);
     for (uint16 i = 0; i < NumIndices; i++)
         mGridIndices.AddIndex(i);
     mGridIndices.SetPrimitiveType(GL_LINES);
}

void CDrawUtil::InitSquare()
{
    debugf("Creating square");
    mSquareVertices.emplace();
    mSquareVertices->SetActiveAttribs(EVertexAttribute::Position |
                                      EVertexAttribute::Normal |
                                      EVertexAttribute::Tex0 |
                                      EVertexAttribute::Tex1 |
                                      EVertexAttribute::Tex2 |
                                      EVertexAttribute::Tex3 |
                                      EVertexAttribute::Tex4 |
                                      EVertexAttribute::Tex5 |
                                      EVertexAttribute::Tex6 |
                                      EVertexAttribute::Tex7);
    mSquareVertices->SetVertexCount(4);

    static constexpr std::array SquareVertices{
        CVector3f(-1.f,  1.f, 0.f),
        CVector3f( 1.f,  1.f, 0.f),
        CVector3f( 1.f, -1.f, 0.f),
        CVector3f(-1.f, -1.f, 0.f)
    };

    static constexpr std::array SquareNormals{
        CVector3f(0.f, 0.f, 1.f),
        CVector3f(0.f, 0.f, 1.f),
        CVector3f(0.f, 0.f, 1.f),
        CVector3f(0.f, 0.f, 1.f)
    };

    static constexpr std::array SquareTexCoords{
        CVector2f(0.f, 1.f),
        CVector2f(1.f, 1.f),
        CVector2f(1.f, 0.f),
        CVector2f(0.f, 0.f)
    };

    mSquareVertices->BufferAttrib(EVertexAttribute::Position, SquareVertices.data());
    mSquareVertices->BufferAttrib(EVertexAttribute::Normal, SquareNormals.data());

    for (uint32 iTex = 0; iTex < 8; iTex++)
    {
        const auto Attrib = static_cast<EVertexAttribute>(EVertexAttribute::Tex0 << iTex);
        mSquareVertices->BufferAttrib(Attrib, SquareTexCoords.data());
    }

    mSquareIndices.Reserve(4);
    mSquareIndices.SetPrimitiveType(GL_TRIANGLE_STRIP);
    mSquareIndices.AddIndex(3);
    mSquareIndices.AddIndex(2);
    mSquareIndices.AddIndex(0);
    mSquareIndices.AddIndex(1);
}

void CDrawUtil::InitLine()
{
    debugf("Creating line");
    mLineVertices.emplace();
    mLineVertices->SetActiveAttribs(EVertexAttribute::Position);
    mLineVertices->SetVertexCount(2);

    mLineIndices.Reserve(2);
    mLineIndices.SetPrimitiveType(GL_LINES);
    mLineIndices.AddIndex(0);
    mLineIndices.AddIndex(1);
}

void CDrawUtil::InitCube()
{
    debugf("Creating cube");
    mpCubeModel = gpEditorStore->LoadResource("Cube.CMDL");
}

void CDrawUtil::InitWireCube()
{
    debugf("Creating wire cube");
    mWireCubeVertices.emplace();
    mWireCubeVertices->SetVertexDesc(EVertexAttribute::Position);
    mWireCubeVertices->Reserve(8);
    mWireCubeVertices->AddVertex(CVector3f(-0.5f, -0.5f, -0.5f));
    mWireCubeVertices->AddVertex(CVector3f(-0.5f,  0.5f, -0.5f));
    mWireCubeVertices->AddVertex(CVector3f( 0.5f,  0.5f, -0.5f));
    mWireCubeVertices->AddVertex(CVector3f( 0.5f, -0.5f, -0.5f));
    mWireCubeVertices->AddVertex(CVector3f(-0.5f, -0.5f,  0.5f));
    mWireCubeVertices->AddVertex(CVector3f( 0.5f, -0.5f,  0.5f));
    mWireCubeVertices->AddVertex(CVector3f( 0.5f,  0.5f,  0.5f));
    mWireCubeVertices->AddVertex(CVector3f(-0.5f,  0.5f,  0.5f));

    static constexpr std::array<uint16, 24> Indices{
        0, 1,
        1, 2,
        2, 3,
        3, 0,
        4, 5,
        5, 6,
        6, 7,
        7, 4,
        0, 4,
        1, 7,
        2, 6,
        3, 5
    };
    mWireCubeIndices.AddIndices(Indices.data(), Indices.size());
    mWireCubeIndices.SetPrimitiveType(GL_LINES);
}

void CDrawUtil::InitSphere()
{
    debugf("Creating sphere");
    mpSphereModel = gpEditorStore->LoadResource("Sphere.CMDL");
    mpDoubleSidedSphereModel = gpEditorStore->LoadResource("SphereDoubleSided.CMDL");
}

void CDrawUtil::InitWireSphere()
{
    debugf("Creating wire sphere");
    mpWireSphereModel = gpEditorStore->LoadResource("WireSphere.CMDL");
}

void CDrawUtil::InitShaders()
{
    debugf("Creating shaders");
    mpColorShader          = CShader::FromResourceFile("ColorShader");
    mpColorShaderLighting  = CShader::FromResourceFile("ColorShaderLighting");
    mpBillboardShader      = CShader::FromResourceFile("BillboardShader");
    mpLightBillboardShader = CShader::FromResourceFile("LightBillboardShader");
    mpTextureShader        = CShader::FromResourceFile("TextureShader");
    mpCollisionShader      = CShader::FromResourceFile("CollisionShader");
    mpTextShader           = CShader::FromResourceFile("TextShader");
}

void CDrawUtil::InitTextures()
{
    debugf("Loading textures");
    mpCheckerTexture = gpEditorStore->LoadResource("Checkerboard.TXTR");

    mpLightTextures[0] = gpEditorStore->LoadResource("LightAmbient.TXTR");
    mpLightTextures[1] = gpEditorStore->LoadResource("LightDirectional.TXTR");
    mpLightTextures[2] = gpEditorStore->LoadResource("LightCustom.TXTR");
    mpLightTextures[3] = gpEditorStore->LoadResource("LightSpot.TXTR");

    mpLightMasks[0] = gpEditorStore->LoadResource("LightAmbientMask.TXTR");
    mpLightMasks[1] = gpEditorStore->LoadResource("LightDirectionalMask.TXTR");
    mpLightMasks[2] = gpEditorStore->LoadResource("LightCustomMask.TXTR");
    mpLightMasks[3] = gpEditorStore->LoadResource("LightSpotMask.TXTR");
}

void CDrawUtil::Shutdown()
{
    if (!mDrawUtilInitialized)
        return;

    debugf("Shutting down");
    mGridVertices.reset();
    mSquareVertices.reset();
    mLineVertices.reset();
    mWireCubeVertices.reset();
    mpColorShader.reset();
    mpColorShaderLighting.reset();
    mpTextureShader.reset();
    mpCollisionShader.reset();
    mpTextShader.reset();
    mDrawUtilInitialized = false;
}
