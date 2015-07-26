#ifndef CCAMERA_H
#define CCAMERA_H

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
    float mAspectRatio;

    float mYaw;
    float mPitch;
    CVector3f mOrbitTarget;
    float mMoveSpeed;
    float mLookSpeed;

    CMatrix4f mCachedViewMatrix;
    CMatrix4f mCachedProjectionMatrix;
    bool mViewOutdated;
    bool mProjectionOutdated;

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
    CVector3f GetDirection() const;
    float GetYaw() const;
    float GetPitch() const;
    const CMatrix4f& ViewMatrix();
    const CMatrix4f& ProjectionMatrix();

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
};

#endif // CCAMERA_H
