#ifndef SVIEWINFO
#define SVIEWINFO

#include <Math/CFrustumPlanes.h>
#include <Math/CMatrix4f.h>
#include <Math/CRay.h>

struct SViewInfo
{
    class CScene *pScene;
    class CRenderer *pRenderer;
    class CCamera *pCamera;

    bool GameMode;
    CFrustumPlanes ViewFrustum;
    CMatrix4f RotationOnlyViewMatrix;
};

#endif // SVIEWINFO

