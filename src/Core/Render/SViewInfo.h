#ifndef SVIEWINFO
#define SVIEWINFO

#include "CFrustumPlanes.h"
#include <Common/CMatrix4f.h>
#include <Common/CRay.h>

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

