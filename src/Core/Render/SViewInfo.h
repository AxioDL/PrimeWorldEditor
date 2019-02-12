#ifndef SVIEWINFO
#define SVIEWINFO

#include "Core/Resource/Collision/CCollisionMaterial.h"
#include "Core/Scene/FShowFlags.h"
#include <Common/Math/CFrustumPlanes.h>
#include <Common/Math/CMatrix4f.h>
#include <Common/Math/CRay.h>

struct SCollisionRenderSettings
{
    uint64 HighlightMask;
    uint64 HideMask;
    int BoundingHierarchyRenderDepth;

    CCollisionMaterial HideMaterial;
    bool DrawWireframe;
    bool DrawBackfaces;
    bool DrawAreaCollisionBounds;
    bool DrawBoundingHierarchy;
    bool TintWithSurfaceColor;
    bool TintUnwalkableTris;

    SCollisionRenderSettings()
        : HighlightMask(0)
        , HideMask(0)
        , BoundingHierarchyRenderDepth(0)
        , DrawWireframe(true)
        , DrawBackfaces(false)
        , DrawAreaCollisionBounds(true)
        , DrawBoundingHierarchy(false)
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

