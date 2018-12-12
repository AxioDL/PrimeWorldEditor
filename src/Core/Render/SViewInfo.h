#ifndef SVIEWINFO
#define SVIEWINFO

#include "Core/Resource/CCollisionMaterial.h"
#include "Core/Scene/FShowFlags.h"
#include <Common/Math/CFrustumPlanes.h>
#include <Common/Math/CMatrix4f.h>
#include <Common/Math/CRay.h>

struct SCollisionRenderSettings
{
    uint64 HighlightMask;
    uint64 HideMask;

    CCollisionMaterial HideMaterial;
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
        , TintUnwalkableTris(true) {}
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

