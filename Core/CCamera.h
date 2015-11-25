#ifndef CCAMERA_H
#define CCAMERA_H

#include "CFrustumPlanes.h"
#include <Common/CAABox.h>
#include <Common/CMatrix4f.h>
#include <Common/CRay.h>
#include <Common/CVector2i.h>
#include <Common/CVector3f.h>
#include <Common/types.h>
#include <Common/EKeyInputs.h>
#include <Common/EMouseInputs.h>

enum ECameraMoveMode
{
    eFreeCamera, eOrbitCamera
};

class CCamera
{
    ECameraMoveMode mMode;
    CVector3f mPosition;
    CVector3f mDirection;
    CVector3f mRightVector;
    CVector3f mUpVector;
    float mAspectRatio;

    float mYaw;
    float mPitch;
    CVector3f mOrbitTarget;
    float mMoveSpeed;
    float mLookSpeed;

    CMatrix4f mCachedViewMatrix;
    CMatrix4f mCachedProjectionMatrix;
    CFrustumPlanes mCachedFrustumPlanes;
    bool mViewOutdated;
    bool mProjectionOutdated;
    bool mFrustumPlanesOutdated;

public:
    CCamera();
    CCamera(CVector3f Position, CVector3f Target);

    void Pan(float XAmount, float YAmount);
    void Rotate(float XAmount, float YAmount);
    void Zoom(float Amount);
    void Snap(CVector3f Position);
    void ProcessKeyInput(EKeyInputs KeyFlags, double DeltaTime);
    void ProcessMouseInput(EKeyInputs KeyFlags, EMouseInputs MouseFlags, float XMovement, float YMovement);
    CRay CastRay(CVector2f DeviceCoords);
    void LoadMatrices();

    // Getters
    CVector3f Position() const;
    CVector3f Direction() const;
    CVector3f UpVector() const;
    CVector3f RightVector() const;
    float Yaw() const;
    float Pitch() const;
    float FieldOfView() const;
    const CMatrix4f& ViewMatrix();
    const CMatrix4f& ProjectionMatrix();
    const CFrustumPlanes& FrustumPlanes();

    // Setters
    void SetPosition(CVector3f Position);
    void SetDirection(CVector3f Direction);
    void SetYaw(float Yaw);
    void SetPitch(float Pitch);
    void SetMoveSpeed(float MoveSpeed);
    void SetLookSpeed(float LookSpeed);
    void SetFree();
    void SetOrbit(const CVector3f& OrbitTarget);
    void SetOrbit(const CAABox& OrbitTarget);
    void SetAspectRatio(float AspectRatio);

    // Private
private:
    void CalculateDirection();
    void CalculateView();
    void CalculateProjection();
    void CalculateFrustumPlanes();
};

#endif // CCAMERA_H
