#ifndef CGRAPHICS_H
#define CGRAPHICS_H

#include "Core/OpenGL/CUniformBuffer.h"
#include "Core/OpenGL/CVertexArrayManager.h"
#include "Core/Resource/CLight.h"
#include <Common/CColor.h>
#include <Common/Math/CMatrix4f.h>
#include <Common/Math/CVector3f.h>
#include <Common/Math/CVector4f.h>
#include <GL/glew.h>

/**
 * todo: should probably be replaced with a CGraphicsState class which
 * can be instantiated and is probably more safe/functional than global access.
 * also,
 */
class CGraphics
{
    static CUniformBuffer *mpMVPBlockBuffer;
    static CUniformBuffer *mpVertexBlockBuffer;
    static CUniformBuffer *mpPixelBlockBuffer;
    static CUniformBuffer *mpLightBlockBuffer;
    static u32 mContextIndices;
    static u32 mActiveContext;
    static bool mInitialized;
    static std::vector<CVertexArrayManager*> mVAMs;

public:
    // SMVPBlock
    struct SMVPBlock
    {
        CMatrix4f ModelMatrix;
        CMatrix4f ViewMatrix;
        CMatrix4f ProjectionMatrix;
    };
    static SMVPBlock sMVPBlock;

    // SVertexBlock
    struct SVertexBlock
    {
        CMatrix4f TexMatrices[10];
        CMatrix4f PostMatrices[20];
        CVector4f COLOR0_Amb;
        CVector4f COLOR0_Mat;
        CVector4f COLOR1_Amb;
        CVector4f COLOR1_Mat;
    };
    static SVertexBlock sVertexBlock;

    // SPixelBlock
    struct SPixelBlock
    {
        CVector4f Konst[4];
        CVector4f TevColor;
        CVector4f TintColor;
    };
    static SPixelBlock sPixelBlock;

    // SLightBlock
    struct SLightBlock
    {
        struct SGXLight
        {
            CVector4f Position;
            CVector4f Direction;
            CVector4f Color;
            CVector4f DistAtten;
            CVector4f AngleAtten;
        };
        SGXLight Lights[8];
    };
    static SLightBlock sLightBlock;

    // Lighting-related
    enum ELightingMode { eNoLighting, eBasicLighting, eWorldLighting };
    static ELightingMode sLightMode;
    static u32 sNumLights;
    static const CColor skDefaultAmbientColor;
    static CColor sAreaAmbientColor;
    static float sWorldLightMultiplier;
    static CLight sDefaultDirectionalLights[3];

    // Functions
    static void Initialize();
    static void Shutdown();
    static void UpdateMVPBlock();
    static void UpdateVertexBlock();
    static void UpdatePixelBlock();
    static void UpdateLightBlock();
    static GLuint MVPBlockBindingPoint();
    static GLuint VertexBlockBindingPoint();
    static GLuint PixelBlockBindingPoint();
    static GLuint LightBlockBindingPoint();
    static u32 GetContextIndex();
    static u32 GetActiveContext();
    static void ReleaseContext(u32 Index);
    static void SetActiveContext(u32 Index);
    static void SetDefaultLighting();
    static void SetupAmbientColor();
    static void SetIdentityMVP();
};

#endif // CGRAPHICS_H
