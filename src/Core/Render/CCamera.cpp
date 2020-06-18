#include "CCamera.h"
#include "CGraphics.h"
#include <Common/Math/CQuaternion.h>
#include <Common/Math/MathUtil.h>

CCamera::CCamera()
    : mYaw(-Math::skHalfPi)
{
    ResetOrbit();
}

// todo: make it actually look at the target!
// don't actually use this constructor, it's unfinished and won't work properly
CCamera::CCamera(CVector3f Position, CVector3f /*Target*/)
    : mPosition(Position)
    , mYaw(-Math::skHalfPi)
{
}

void CCamera::Pan(float XAmount, float YAmount)
{
    if (mMode == ECameraMoveMode::Free)
    {
        mPosition += mRightVector * (XAmount * mMoveSpeed);
        mPosition += mUpVector * (YAmount * mMoveSpeed);
        mTransformDirty = true;
        mViewDirty = true;
        mFrustumPlanesDirty = true;
    }

    else
        Rotate(-XAmount * 0.3f, YAmount * 0.3f);
}

void CCamera::Rotate(float XAmount, float YAmount)
{
    mYaw -= (XAmount * mLookSpeed * 0.3f);
    mPitch -= (YAmount * mLookSpeed * 0.3f);
    ValidatePitch();

    mTransformDirty = true;
    mViewDirty = true;
    mFrustumPlanesDirty = true;
}

void CCamera::Zoom(float Amount)
{
    if (mMode == ECameraMoveMode::Free)
        mPosition += mDirection * (Amount * mMoveSpeed);

    else
    {
        mOrbitDistance -= Amount * mMoveSpeed;
        mTransformDirty = true;
    }

    mViewDirty = true;
    mFrustumPlanesDirty = true;
}

void CCamera::Snap(CVector3f Position)
{
    mPosition = Position;
    mYaw = -Math::skHalfPi;
    mPitch = 0.0f;
    mTransformDirty = true;
    mViewDirty = true;
    mFrustumPlanesDirty = true;
}

void CCamera::ProcessKeyInput(FKeyInputs KeyFlags, double DeltaTime)
{
    float FDeltaTime = (float) DeltaTime;

    if (KeyFlags & EKeyInput::W) Zoom(FDeltaTime * 25.f);
    if (KeyFlags & EKeyInput::S) Zoom(-FDeltaTime * 25.f);
    if (KeyFlags & EKeyInput::Q) Pan(0, -FDeltaTime * 25.f);
    if (KeyFlags & EKeyInput::E) Pan(0, FDeltaTime * 25.f);
    if (KeyFlags & EKeyInput::A) Pan(-FDeltaTime * 25.f, 0);
    if (KeyFlags & EKeyInput::D) Pan(FDeltaTime * 25.f, 0);
}

void CCamera::ProcessMouseInput(FKeyInputs KeyFlags, FMouseInputs MouseFlags, float XMovement, float YMovement)
{
    // Free Camera
    if (mMode == ECameraMoveMode::Free)
    {
        if (MouseFlags & EMouseInput::MiddleButton)
        {
            if (KeyFlags & EKeyInput::Ctrl) Zoom(-YMovement * 0.2f);
            else                            Pan(-XMovement, YMovement);
        }

        else if (MouseFlags & EMouseInput::RightButton) Rotate(XMovement, YMovement);
    }

    // Orbit Camera
    else if (mMode == ECameraMoveMode::Orbit)
    {
        if ((MouseFlags & EMouseInput::MiddleButton) || (MouseFlags & EMouseInput::RightButton))
            Pan(-XMovement, YMovement);
    }
}

CRay CCamera::CastRay(CVector2f DeviceCoords) const
{
    CMatrix4f InverseVP = (ViewMatrix().Transpose() * ProjectionMatrix().Transpose()).Inverse();

    CVector3f RayOrigin = CVector3f(DeviceCoords.X, DeviceCoords.Y, -1.f) * InverseVP;
    CVector3f RayTarget = CVector3f(DeviceCoords.X, DeviceCoords.Y,  0.f) * InverseVP;
    CVector3f RayDir = (RayTarget - RayOrigin).Normalized();

    CRay Ray;
    Ray.SetOrigin(RayOrigin);
    Ray.SetDirection(RayDir);
    return Ray;
}

void CCamera::SetMoveMode(ECameraMoveMode Mode)
{
    mMode = Mode;
    mViewDirty = true;
    mFrustumPlanesDirty = true;

    if (mMode == ECameraMoveMode::Orbit)
        mTransformDirty = true;
}

void CCamera::SetOrbit(const CVector3f& OrbitTarget, float Distance)
{
    mOrbitTarget = OrbitTarget;
    mOrbitDistance = Distance;

    if (mMode == ECameraMoveMode::Orbit)
    {
        mTransformDirty = true;
        mViewDirty = true;
        mFrustumPlanesDirty = true;
    }
}

void CCamera::SetOrbit(const CAABox& OrbitTarget, float DistScale /*= 1.75f*/)
{
    mOrbitTarget = OrbitTarget.Center();

    // Determine orbit radius, which should be enough to cover the entire box with some buffer room
    float Dist = OrbitTarget.Center().Distance(OrbitTarget.Max());
    mOrbitDistance = Dist * DistScale;

    if (mMode == ECameraMoveMode::Orbit)
    {
        mTransformDirty = true;
        mViewDirty = true;
        mFrustumPlanesDirty = true;
    }
}

void CCamera::SetOrbitTarget(const CVector3f& rkOrbitTarget)
{
    mOrbitTarget = rkOrbitTarget;

    if (mMode == ECameraMoveMode::Orbit)
    {
        mTransformDirty = true;
        mViewDirty = true;
        mFrustumPlanesDirty = true;
    }
}

void CCamera::SetOrbitDistance(float Distance)
{
    mOrbitDistance = Distance;

    if (mMode == ECameraMoveMode::Orbit)
    {
        mTransformDirty = true;
        mViewDirty = true;
        mFrustumPlanesDirty = true;
    }
}

void CCamera::LoadMatrices() const
{
    CGraphics::sMVPBlock.ViewMatrix = ViewMatrix();
    CGraphics::sMVPBlock.ProjectionMatrix = ProjectionMatrix();
    CGraphics::UpdateMVPBlock();
}

CTransform4f CCamera::GetCameraTransform() const
{
    CTransform4f Out = CTransform4f::skIdentity;
    Out.SetRotationFromAxes(mRightVector, mDirection, mUpVector);
    Out.SetTranslation(mPosition);
    return Out;
}

// ************ PRIVATE ************
void CCamera::ValidatePitch()
{
    // This function mainly just exists to ensure the camera doesn't flip upside down
    if (mPitch > Math::skHalfPi)  mPitch = Math::skHalfPi;
    if (mPitch < -Math::skHalfPi) mPitch = -Math::skHalfPi;
}

void CCamera::UpdateTransform() const
{
    // Transform should be marked dirty when pitch, yaw, or orbit target/distance are changed
    if (mTransformDirty)
    {
        mDirection = CVector3f(
                     cos(mPitch) * cos(mYaw),
                     cos(mPitch) * sin(mYaw),
                     sin(mPitch)
                     );

        mRightVector = CVector3f(
            cos(mYaw - Math::skHalfPi),
            sin(mYaw - Math::skHalfPi),
            0
        );

        mUpVector = mRightVector.Cross(mDirection);

        // Update position
        if (mMode == ECameraMoveMode::Orbit)
        {
            if (mOrbitDistance < 1.f) mOrbitDistance = 1.f;
            mPosition = mOrbitTarget + (mDirection * -mOrbitDistance);
        }

        mViewDirty = true;
        mFrustumPlanesDirty = true;
        mTransformDirty = false;
    }
}

void CCamera::UpdateView() const
{
    UpdateTransform();

    if (mViewDirty)
    {
        mViewMatrix = CMatrix4f(
            mRightVector.X, mRightVector.Y, mRightVector.Z, -mRightVector.Dot(mPosition),
               mUpVector.X,    mUpVector.Y,    mUpVector.Z,    -mUpVector.Dot(mPosition),
             -mDirection.X,  -mDirection.Y,  -mDirection.Z,    mDirection.Dot(mPosition),
                       0.f,            0.f,            0.f,                          1.f
            );

        mViewDirty = false;
    }
}

void CCamera::UpdateProjection() const
{
    if (mProjectionDirty)
    {
        mProjectionMatrix = Math::PerspectiveMatrix(55.f, mAspectRatio, 0.1f, 4096.f);
        mProjectionDirty = false;
    }
}

void CCamera::UpdateFrustum() const
{
    UpdateTransform();

    if (mFrustumPlanesDirty)
    {
        mFrustumPlanes.SetPlanes(mPosition, mDirection, 55.f, mAspectRatio, 0.1f, 4096.f);
        mFrustumPlanesDirty = false;
    }
}
