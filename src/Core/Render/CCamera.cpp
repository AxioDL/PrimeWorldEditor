#include "CCamera.h"
#include "CGraphics.h"
#include <Math/CQuaternion.h>
#include <Math/MathUtil.h>
#include <gtc/matrix_transform.hpp>

CCamera::CCamera()
{
    mMode = eFreeCamera;
    mPosition = CVector3f(0);
    mAspectRatio = 1.7777777f;

    mYaw = -Math::skHalfPi;
    mPitch = 0.0f;
    SetOrbit(CVector3f(0), 5.f);

    mMoveSpeed = 1.f;
    mLookSpeed = 1.f;
    mTransformDirty = true;
    mViewDirty = true;
    mProjectionDirty = true;
    mFrustumPlanesDirty = true;
}

CCamera::CCamera(CVector3f Position, CVector3f /*Target*/)
{
    // todo: make it actually look at the target!
    // don't actually use this constructor, it's unfinished and won't work properly
    mMode = eFreeCamera;
    mMoveSpeed = 1.f;
    mLookSpeed = 1.f;
    mPosition = Position;
    mYaw = -Math::skHalfPi;
    mPitch = 0.0f;
}

void CCamera::Pan(float XAmount, float YAmount)
{
    if (mMode == eFreeCamera)
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
    if (mMode == eFreeCamera)
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

void CCamera::ProcessKeyInput(EKeyInputs KeyFlags, double DeltaTime)
{
    float FDeltaTime = (float) DeltaTime;

    if (KeyFlags & eWKey) Zoom(FDeltaTime * 25.f);
    if (KeyFlags & eSKey) Zoom(-FDeltaTime * 25.f);
    if (KeyFlags & eQKey) Pan(0, -FDeltaTime * 25.f);
    if (KeyFlags & eEKey) Pan(0, FDeltaTime * 25.f);
    if (KeyFlags & eAKey) Pan(-FDeltaTime * 25.f, 0);
    if (KeyFlags & eDKey) Pan(FDeltaTime * 25.f, 0);
}

void CCamera::ProcessMouseInput(EKeyInputs KeyFlags, EMouseInputs MouseFlags, float XMovement, float YMovement)
{
    // Free Camera
    if (mMode == eFreeCamera)
    {
        if (MouseFlags & eMiddleButton)
        {
            if (KeyFlags & eCtrlKey) Zoom(-YMovement * 0.2f);
            else                     Pan(-XMovement, YMovement);
        }

        else if (MouseFlags & eRightButton) Rotate(XMovement, YMovement);
    }

    // Orbit Camera
    else if (mMode == eOrbitCamera)
    {
        if ((MouseFlags & eMiddleButton) || (MouseFlags & eRightButton))
            Pan(-XMovement, YMovement);
    }
}

CRay CCamera::CastRay(CVector2f DeviceCoords) const
{
    CMatrix4f InverseVP = (ViewMatrix().Transpose() * ProjectionMatrix().Transpose()).Inverse();

    CVector3f RayOrigin = CVector3f(DeviceCoords.x, DeviceCoords.y, -1.f) * InverseVP;
    CVector3f RayTarget = CVector3f(DeviceCoords.x, DeviceCoords.y,  0.f) * InverseVP;
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

    if (mMode == eOrbitCamera)
        mTransformDirty = true;
}

void CCamera::SetOrbit(const CVector3f& OrbitTarget, float Distance)
{
    mOrbitTarget = OrbitTarget;
    mOrbitDistance = Distance;

    if (mMode == eOrbitCamera)
    {
        mTransformDirty = true;
        mViewDirty = true;
        mFrustumPlanesDirty = true;
    }
}

void CCamera::SetOrbit(const CAABox& OrbitTarget, float DistScale /*= 4.f*/)
{
    CVector3f Min = OrbitTarget.Min();
    CVector3f Max = OrbitTarget.Max();

    mOrbitTarget = OrbitTarget.Center();

    // Find largest extent
    CVector3f Extent = (Max - Min) / 2.f;
    float Dist = 0.f;

    if (Extent.x >= Extent.y && Extent.x >= Extent.z) Dist = Extent.x;
    else if (Extent.y >= Extent.x && Extent.y >= Extent.z) Dist = Extent.y;
    else Dist = Extent.z;

    mOrbitDistance = Dist * DistScale;

    if (mMode == eOrbitCamera)
    {
        mTransformDirty = true;
        mViewDirty = true;
        mFrustumPlanesDirty = true;
    }
}

void CCamera::SetOrbitDistance(float Distance)
{
    mOrbitDistance = Distance;

    if (mMode == eOrbitCamera)
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

// ************ GETTERS ************
CVector3f CCamera::Position() const
{
    UpdateTransform();
    return mPosition;
}

CVector3f CCamera::Direction() const
{
    UpdateTransform();
    return mDirection;
}

CVector3f CCamera::UpVector() const
{
    UpdateTransform();
    return mUpVector;
}

CVector3f CCamera::RightVector() const
{
    UpdateTransform();
    return mRightVector;
}

float CCamera::Yaw() const
{
    return mYaw;
}

float CCamera::Pitch() const
{
    return mPitch;
}

float CCamera::FieldOfView() const
{
    return 55.f;
}

ECameraMoveMode CCamera::MoveMode() const
{
    return mMode;
}

const CMatrix4f& CCamera::ViewMatrix() const
{
    UpdateView();
    return mViewMatrix;
}

const CMatrix4f& CCamera::ProjectionMatrix() const
{
    UpdateProjection();
    return mProjectionMatrix;
}

const CFrustumPlanes& CCamera::FrustumPlanes() const
{
    UpdateFrustum();
    return mFrustumPlanes;
}

// ************ SETTERS ************
void CCamera::SetYaw(float Yaw)
{
    mYaw = Yaw;
    mTransformDirty = true;
}

void CCamera::SetPitch(float Pitch)
{
    mPitch = Pitch;
    ValidatePitch();
    mTransformDirty = true;
}

void CCamera::SetMoveSpeed(float MoveSpeed)
{
    mMoveSpeed = MoveSpeed;
}

void CCamera::SetLookSpeed(float LookSpeed)
{
    mLookSpeed = LookSpeed;
}

void CCamera::SetAspectRatio(float AspectRatio)
{
    mAspectRatio = AspectRatio;
    mProjectionDirty = true;
    mFrustumPlanesDirty = true;
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
        if (mMode == eOrbitCamera)
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
    // todo: don't use glm
    UpdateTransform();

    if (mViewDirty)
    {
        glm::vec3 glmpos(mPosition.x, mPosition.y, mPosition.z);
        glm::vec3 glmdir(mDirection.x, mDirection.y, mDirection.z);
        glm::vec3 glmup(mUpVector.x, mUpVector.y, mUpVector.z);
        mViewMatrix = CMatrix4f::FromGlmMat4(glm::lookAt(glmpos, glmpos + glmdir, glmup)).Transpose();
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
