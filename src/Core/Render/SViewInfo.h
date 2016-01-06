#ifndef SVIEWINFO
#define SVIEWINFO

#include "Core/Scene/FShowFlags.h"
#include <Math/CFrustumPlanes.h>
#include <Math/CMatrix4f.h>
#include <Math/CRay.h>

struct SViewInfo
{
    class CScene *pScene;
    class CRenderer *pRenderer;
    class CCamera *pCamera;

    bool GameMode;
    FShowFlags ShowFlags;
    CFrustumPlanes ViewFrustum;
    CMatrix4f RotationOnlyViewMatrix;
};

#endif // SVIEWINFO

