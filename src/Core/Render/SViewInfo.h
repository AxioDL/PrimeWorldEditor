#ifndef SVIEWINFO
#define SVIEWINFO

#include "Core/Scene/FShowFlags.h"
#include <Math/CFrustumPlanes.h>
#include <Math/CMatrix4f.h>
#include <Math/CRay.h>

struct SCollisionRenderSettings
{
    u64 HighlightMask;
    u64 HideMask;
    bool DrawWireframe;
    bool DrawBackfaces;
    bool DrawAreaCollisionBounds;
    bool TintWithSurfaceColor;
    bool TintUnwalkableTris;

    SCollisionRenderSettings()
        : HighlightMask(0)
        , HideMask(0)
        , DrawWireframe(true)
        , DrawBackfaces(false)
        , DrawAreaCollisionBounds(true)
        , TintWithSurfaceColor(true)
        , TintUnwalkableTris(false) {}
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

