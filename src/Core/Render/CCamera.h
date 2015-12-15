#ifndef CCAMERA_H
#define CCAMERA_H

#include <Common/Math/CAABox.h>
#include <Common/Math/CFrustumPlanes.h>
#include <Common/Math/CMatrix4f.h>
#include <Common/Math/CRay.h>
#include <Common/Math/CVector2i.h>
#include <Common/Math/CVector3f.h>
#include <Common/types.h>
#include <Common/EKeyInputs.h>
#include <Common/EMouseInputs.h>

enum ECameraMoveMode
{
    eFreeCamera, eOrbitCamera
};

/* This class uses a lot of mutable members as an optimization so that they can
 * be updated as infrequently as possible (eg only when the values are requested
 * the next time after changes are made) while still always returning the correct
 * value via the const get functions. They are not modified in const functions
 * beyond ensuring that all data is valid and synced with everything else (eg
 * mPosition is only modified to ensure it's correct in orbit mode given the
 * target/distance/pitch/yaw; it won't be snapped to a different location in a
 * const function). */
class CCamera
{
    ECameraMoveMode mMode;
    mutable CVector3f mPosition;
    mutable CVector3f mDirection;
    mutable CVector3f mRightVector;
    mutable CVector3f mUpVector;
    float mAspectRatio;

    float mYaw;
    float mPitch;
    CVector3f mOrbitTarget;
    mutable float mOrbitDistance;
    float mMoveSpeed;
    float mLookSpeed;

    mutable CMatrix4f mViewMatrix;
    mutable CMatrix4f mProjectionMatrix;
    mutable CFrustumPlanes mFrustumPlanes;

    mutable bool mTransformDirty;
    mutable bool mViewDirty;
    mutable bool mProjectionDirty;
    mutable bool mFrustumPlanesDirty;

public:
    CCamera();
    CCamera(CVector3f Position, CVector3f Target);

    void Pan(float XAmount, float YAmount);
    void Rotate(float XAmount, float YAmount);
    void Zoom(float Amount);
    void Snap(CVector3f Position);
    void ProcessKeyInput(EKeyInputs KeyFlags, double DeltaTime);
    void ProcessMouseInput(EKeyInputs KeyFlags, EMouseInputs MouseFlags, float XMovement, float YMovement);
    CRay CastRay(CVector2f DeviceCoords) const;
    void LoadMatrices() const;

    void SetMoveMode(ECameraMoveMode Mode);
    void SetOrbit(const CVector3f& OrbitTarget, float Distance);
    void SetOrbit(const CAABox& OrbitTarget, float DistScale = 4.f);
    void SetOrbitDistance(float Distance);

    // Getters
    CVector3f Position() const;
    CVector3f Direction() const;
    CVector3f UpVector() const;
    CVector3f RightVector() const;
    float Yaw() const;
    float Pitch() const;
    float FieldOfView() const;
    ECameraMoveMode MoveMode() const;
    const CMatrix4f& ViewMatrix() const;
    const CMatrix4f& ProjectionMatrix() const;
    const CFrustumPlanes& FrustumPlanes() const;

    // Setters
    void SetYaw(float Yaw);
    void SetPitch(float Pitch);
    void SetMoveSpeed(float MoveSpeed);
    void SetLookSpeed(float LookSpeed);
    void SetAspectRatio(float AspectRatio);

    // Private
private:
    void ValidatePitch();
    void UpdateTransform() const;
    void UpdateView() const;
    void UpdateProjection() const;
    void UpdateFrustum() const;
};

#endif // CCAMERA_H
