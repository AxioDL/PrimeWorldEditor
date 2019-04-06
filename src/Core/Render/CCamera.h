#ifndef CCAMERA_H
#define CCAMERA_H

#include <Common/BasicTypes.h>
#include <Common/EKeyInputs.h>
#include <Common/EMouseInputs.h>
#include <Common/Math/CAABox.h>
#include <Common/Math/CFrustumPlanes.h>
#include <Common/Math/CMatrix4f.h>
#include <Common/Math/CRay.h>
#include <Common/Math/CVector2i.h>
#include <Common/Math/CVector3f.h>

enum class ECameraMoveMode
{
    Free, Orbit
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
    void ProcessKeyInput(FKeyInputs KeyFlags, double DeltaTime);
    void ProcessMouseInput(FKeyInputs KeyFlags, FMouseInputs MouseFlags, float XMovement, float YMovement);
    CRay CastRay(CVector2f DeviceCoords) const;
    void LoadMatrices() const;
    CTransform4f GetCameraTransform() const;

    void SetMoveMode(ECameraMoveMode Mode);
    void SetOrbit(const CVector3f& rkOrbitTarget, float Distance);
    void SetOrbit(const CAABox& rkOrbitTarget, float DistScale = 1.75f);
    void SetOrbitTarget(const CVector3f& rkOrbitTarget);
    void SetOrbitDistance(float Distance);

    // Inline Accessors
    inline CVector3f Position() const                       { UpdateTransform(); return mPosition; }
    inline CVector3f Direction() const                      { UpdateTransform(); return mDirection; }
    inline CVector3f UpVector() const                       { UpdateTransform(); return mUpVector; }
    inline CVector3f RightVector() const                    { UpdateTransform(); return mRightVector; }
    inline float Yaw() const                                { return mYaw; }
    inline float Pitch() const                              { return mPitch; }
    inline float FieldOfView() const                        { return 55.f; }
    inline ECameraMoveMode MoveMode() const                 { return mMode; }
    inline const CMatrix4f& ViewMatrix() const              { UpdateView(); return mViewMatrix; }
    inline const CMatrix4f& ProjectionMatrix() const        { UpdateProjection(); return mProjectionMatrix; }
    inline const CFrustumPlanes& FrustumPlanes() const      { UpdateFrustum(); return mFrustumPlanes; }

    inline void SetYaw(float Yaw)                   { mYaw = Yaw; mTransformDirty = true; }
    inline void SetPitch(float Pitch)               { mPitch = Pitch; ValidatePitch(); mTransformDirty = true; }
    inline void SetMoveSpeed(float MoveSpeed)       { mMoveSpeed = MoveSpeed; }
    inline void SetLookSpeed(float LookSpeed)       { mLookSpeed = LookSpeed; }
    inline void SetAspectRatio(float AspectRatio)   { mAspectRatio = AspectRatio; mProjectionDirty = true; mFrustumPlanesDirty = true; }

    inline void ResetOrbit()                        { SetOrbit(CVector3f::skZero, 5.f); }

    // Private
private:
    void ValidatePitch();
    void UpdateTransform() const;
    void UpdateView() const;
    void UpdateProjection() const;
    void UpdateFrustum() const;
};

#endif // CCAMERA_H
