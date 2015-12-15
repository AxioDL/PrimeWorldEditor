#ifndef SVIEWINFO
#define SVIEWINFO

#include <Common/Math/CFrustumPlanes.h>
#include <Common/Math/CMatrix4f.h>
#include <Common/Math/CRay.h>

struct SViewInfo
{
    class CSceneManager *pScene;
    class CRenderer *pRenderer;

    class CCamera *pCamera;
    bool GameMode;
    CFrustumPlanes ViewFrustum;
    CMatrix4f RotationOnlyViewMatrix;
};

#endif // SVIEWINFO

