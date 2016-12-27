#ifndef SVIEWINFO
#define SVIEWINFO

#include "Core/Scene/FShowFlags.h"
#include <Math/CFrustumPlanes.h>
#include <Math/CMatrix4f.h>
#include <Math/CRay.h>

enum ECollisionDrawMode
{
    eCDM_Default,
    eCDM_TintSurfaceType
};

struct SCollisionRenderSettings
{
    ECollisionDrawMode DrawMode;
    u64 HighlightMask;
    u64 HideMask;
    bool DrawWireframe;

    SCollisionRenderSettings()
        : DrawMode(eCDM_TintSurfaceType), HighlightMask(0), HideMask(0), DrawWireframe(false) {}
};

struct SViewInfo
{
    class CScene *pScene;
    class CRenderer *pRenderer;
    class CCamera *pCamera;

    bool GameMode;
    FShowFlags ShowFlags;
    SCollisionRenderSettings CollisionSettings;
    CFrustumPlanes ViewFrustum;
    CMatrix4f RotationOnlyViewMatrix;
};

#endif // SVIEWINFO

