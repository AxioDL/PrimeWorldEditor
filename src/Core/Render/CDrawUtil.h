#ifndef CDRAWUTIL
#define CDRAWUTIL

#include "Core/OpenGL/CVertexBuffer.h"
#include "Core/OpenGL/CDynamicVertexBuffer.h"
#include "Core/OpenGL/CIndexBuffer.h"
#include "Core/Resource/model/CModel.h"
#include "Core/Resource/CLight.h"

/* todo: CDrawUtil should work with CRenderer to queue primitives for rendering
 * rather than trying to draw them straight away, so that CDrawUtil functions can
 * be called from anywhere in the codebase and still function correctly     */
class CDrawUtil
{
    // 7x7 Grid
    static CVertexBuffer mGridVertices;
    static CIndexBuffer mGridIndices;

    // Square
    static CDynamicVertexBuffer mSquareVertices;
    static CIndexBuffer mSquareIndices;

    // Line
    static CDynamicVertexBuffer mLineVertices;
    static CIndexBuffer mLineIndices;

    // Cube
    static TResPtr<CModel> mpCubeModel;

    // Wire Cube
    static CVertexBuffer mWireCubeVertices;
    static CIndexBuffer mWireCubeIndices;

    // Sphere
    static TResPtr<CModel> mpSphereModel;
    static TResPtr<CModel> mpDoubleSidedSphereModel;

    // Wire Sphere
    static TResPtr<CModel> mpWireSphereModel;

    // Shaders
    static CShader *mpColorShader;
    static CShader *mpColorShaderLighting;
    static CShader *mpBillboardShader;
    static CShader *mpLightBillboardShader;
    static CShader *mpTextureShader;
    static CShader *mpCollisionShader;
    static CShader *mpTextShader;

    // Textures
    static TResPtr<CTexture> mpCheckerTexture;

    static TResPtr<CTexture> mpLightTextures[4];
    static TResPtr<CTexture> mpLightMasks[4];

    // Have all the above members been initialized?
    static bool mDrawUtilInitialized;

public:
    static void DrawGrid(CColor LineColor, CColor BoldLineColor);

    static void DrawSquare();
    static void DrawSquare(const CVector2f& TexUL, const CVector2f& TexUR, const CVector2f& TexBR, const CVector2f& TexBL);
    static void DrawSquare(const float *pTexCoords);

    static void DrawLine(const CVector3f& PointA, const CVector3f& PointB);
    static void DrawLine(const CVector2f& PointA, const CVector2f& PointB);
    static void DrawLine(const CVector3f& PointA, const CVector3f& PointB, const CColor& LineColor);
    static void DrawLine(const CVector2f& PointA, const CVector2f& PointB, const CColor& LineColor);

    static void DrawCube();
    static void DrawCube(const CColor& Color);
    static void DrawCube(const CVector3f& Position, const CColor& Color);
    static void DrawShadedCube(const CColor& Color);

    static void DrawWireCube();
    static void DrawWireCube(const CAABox& AABox, const CColor& Color);

    static void DrawSphere(bool DoubleSided = false);
    static void DrawSphere(const CColor& Color);

    static void DrawWireSphere(const CVector3f& Position, float Radius, const CColor& Color = CColor::skWhite);

    static void DrawBillboard(CTexture* pTexture, const CVector3f& Position, const CVector2f& Scale = CVector2f::skOne, const CColor& Tint = CColor::skWhite);

    static void DrawLightBillboard(ELightType Type, const CColor& LightColor, const CVector3f& Position, const CVector2f& Scale = CVector2f::skOne, const CColor& Tint = CColor::skWhite);

    static void UseColorShader(const CColor& Color);
    static void UseColorShaderLighting(const CColor& Color);
    static void UseTextureShader();
    static void UseTextureShader(const CColor& TintColor);
    static void UseCollisionShader(bool IsFloor, bool IsUnstandable, const CColor& TintColor = CColor::skWhite);

    static CShader* GetTextShader();
    static void LoadCheckerboardTexture(uint32 GLTextureUnit);
    static CTexture* GetLightTexture(ELightType Type);
    static CTexture* GetLightMask(ELightType Type);
    static CModel* GetCubeModel();

private:
    CDrawUtil(); // Private constructor to prevent class from being instantiated
    static void Init();
    static void InitGrid();
    static void InitSquare();
    static void InitLine();
    static void InitCube();
    static void InitWireCube();
    static void InitSphere();
    static void InitWireSphere();
    static void InitShaders();
    static void InitTextures();

public:
    static void Shutdown();
};

#endif // CDRAWUTIL

