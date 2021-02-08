#ifndef SVIEWINFO
#define SVIEWINFO

#include "Core/Resource/Collision/CCollisionMaterial.h"
#include "Core/Scene/FShowFlags.h"
#include <Common/Math/CFrustumPlanes.h>
#include <Common/Math/CMatrix4f.h>
#include <Common/Math/CRay.h>

struct SCollisionRenderSettings
{
    uint64 HighlightMask = 0;
    uint64 HideMask = 0;
    int BoundingHierarchyRenderDepth = 0;

    CCollisionMaterial HideMaterial;
    bool DrawWireframe = true;
    bool DrawBackfaces = false;
    bool DrawAreaCollisionBounds = true;
    bool DrawBoundingHierarchy = false;
    bool TintWithSurfaceColor = true;
    bool TintUnwalkableTris = true;

    SCollisionRenderSettings() = default;
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

