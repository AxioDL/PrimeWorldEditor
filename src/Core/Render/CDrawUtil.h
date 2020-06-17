#ifndef CDRAWUTIL
#define CDRAWUTIL

#include "Core/OpenGL/CVertexBuffer.h"
#include "Core/OpenGL/CDynamicVertexBuffer.h"
#include "Core/OpenGL/CIndexBuffer.h"
#include "Core/Resource/Model/CModel.h"
#include "Core/Resource/CLight.h"

#include <array>
#include <optional>

/**
 * @todo there are a LOT of problems with how this is implemented; trying to
 * use CDrawUtil in a lot of places in the codebase just plain doesn't work
 * because it goes outside CRenderer to draw stuff, and also it's slow as heck
 * because it issues tons of draw calls instead of batching items together
 * which is a cause of significant performance problems
 */
class CDrawUtil
{
    // 7x7 Grid
    static inline std::optional<CVertexBuffer> mGridVertices;
    static inline CIndexBuffer mGridIndices;

    // Square
    static inline std::optional<CDynamicVertexBuffer> mSquareVertices;
    static inline CIndexBuffer mSquareIndices;

    // Line
    static inline std::optional<CDynamicVertexBuffer> mLineVertices;
    static inline CIndexBuffer mLineIndices;

    // Cube
    static inline TResPtr<CModel> mpCubeModel;

    // Wire Cube
    static inline std::optional<CVertexBuffer> mWireCubeVertices;
    static inline CIndexBuffer mWireCubeIndices;

    // Sphere
    static inline TResPtr<CModel> mpSphereModel;
    static inline TResPtr<CModel> mpDoubleSidedSphereModel;

    // Wire Sphere
    static inline TResPtr<CModel> mpWireSphereModel;

    // Shaders
    static inline std::unique_ptr<CShader> mpColorShader;
    static inline std::unique_ptr<CShader> mpColorShaderLighting;
    static inline std::unique_ptr<CShader> mpBillboardShader;
    static inline std::unique_ptr<CShader> mpLightBillboardShader;
    static inline std::unique_ptr<CShader> mpTextureShader;
    static inline std::unique_ptr<CShader> mpCollisionShader;
    static inline std::unique_ptr<CShader> mpTextShader;

    // Textures
    static inline TResPtr<CTexture> mpCheckerTexture;

    static inline std::array<TResPtr<CTexture>, 4> mpLightTextures;
    static inline std::array<TResPtr<CTexture>, 4> mpLightMasks;

    // Have all the above members been initialized?
    static inline bool mDrawUtilInitialized = false;

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

    static void DrawWireSphere(const CVector3f& Position, float Radius, const CColor& Color = CColor::White());

    static void DrawBillboard(CTexture* pTexture, const CVector3f& Position, const CVector2f& Scale = CVector2f::One(), const CColor& Tint = CColor::White());

    static void DrawLightBillboard(ELightType Type, const CColor& LightColor, const CVector3f& Position, const CVector2f& Scale = CVector2f::One(), const CColor& Tint = CColor::White());

    static void UseColorShader(const CColor& Color);
    static void UseColorShaderLighting(const CColor& Color);
    static void UseTextureShader();
    static void UseTextureShader(const CColor& TintColor);
    static void UseCollisionShader(bool IsFloor, bool IsUnstandable, const CColor& TintColor = CColor::White());

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

