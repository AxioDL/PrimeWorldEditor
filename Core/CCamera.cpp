#include "CCamera.h"
#include "CGraphics.h"
#include <Common/CQuaternion.h>
#include <Common/Math.h>
#include <gtc/matrix_transform.hpp>

CCamera::CCamera()
{
    mMode = eFreeCamera;
    mPosition = CVector3f(0);
    mAspectRatio = 1.7777777f;

    mYaw = -Math::skHalfPi;
    mPitch = 0.0f;
    SetOrbit(CVector3f(0), 5.f);
    Update();

    mMoveSpeed = 1.f;
    mLookSpeed = 1.f;
    mViewOutdated = true;
    mProjectionOutdated = true;
    mFrustumPlanesOutdated = true;
}

CCamera::CCamera(CVector3f Position, CVector3f /*Target*/)
{
    // todo: make it actually look at the target!
    mMode = eFreeCamera;
    mMoveSpeed = 1.f;
    mLookSpeed = 1.f;
    mPosition = Position;
    mYaw = -Math::skHalfPi;
    mPitch = 0.0f;
    Update();
}

void CCamera::Pan(float XAmount, float YAmount)
{
    if (mMode == eFreeCamera)
    {
        Update();
        mPosition += mRightVector * (XAmount * mMoveSpeed);
        mPosition += mUpVector * (YAmount * mMoveSpeed);
        mViewOutdated = true;
        mFrustumPlanesOutdated = true;
    }

    else
        Rotate(-XAmount * 0.3f, YAmount * 0.3f);
}

void CCamera::Rotate(float XAmount, float YAmount)
{
    mYaw -= (XAmount * mLookSpeed * 0.3f);
    mPitch -= (YAmount * mLookSpeed * 0.3f);

    mViewOutdated = true;
    mFrustumPlanesOutdated = true;
}

void CCamera::Zoom(float Amount)
{
    if (mMode == eFreeCamera)
    {
        Update();
        mPosition += mDirection * (Amount * mMoveSpeed);
    }

    else
        mOrbitDistance -= Amount * mMoveSpeed;

    mViewOutdated = true;
    mFrustumPlanesOutdated = true;
}

void CCamera::Snap(CVector3f Position)
{
    mPosition = Position;
    mYaw = -Math::skHalfPi;
    mPitch = 0.0f;
    mViewOutdated = true;
    mFrustumPlanesOutdated = true;
    Update();
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

CRay CCamera::CastRay(CVector2f DeviceCoords)
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
    mViewOutdated = true;
    mFrustumPlanesOutdated = true;
}

void CCamera::SetOrbit(const CVector3f& OrbitTarget, float Distance)
{
    mOrbitTarget = OrbitTarget;
    mOrbitDistance = Distance;

    if (mMode == eOrbitCamera)
    {
        mViewOutdated = true;
        mFrustumPlanesOutdated = true;
    }
}

void CCamera::SetOrbit(const CAABox& OrbitTarget, float DistScale /*= 2.5f*/)
{
    CVector3f Min = OrbitTarget.Min();
    CVector3f Max = OrbitTarget.Max();

    mOrbitTarget = OrbitTarget.Center();
    mOrbitDistance = ((Max.x - Min.x) + (Max.y - Min.y) + (Max.z - Min.z)) / 3.f;
    mOrbitDistance *= DistScale;

    if (mMode == eOrbitCamera)
    {
        mViewOutdated = true;
        mFrustumPlanesOutdated = true;
    }
}

void CCamera::SetOrbitDistance(float Distance)
{
    mOrbitDistance = Distance;

    if (mMode == eOrbitCamera)
    {
        mViewOutdated = true;
        mFrustumPlanesOutdated = true;
    }
}

void CCamera::LoadMatrices()
{
    CGraphics::sMVPBlock.ViewMatrix = ViewMatrix();
    CGraphics::sMVPBlock.ProjectionMatrix = ProjectionMatrix();
    CGraphics::UpdateMVPBlock();
}

// ************ GETTERS ************
CVector3f CCamera::Position() const
{
    return mPosition;
}

CVector3f CCamera::Direction() const
{
    return mDirection;
}

CVector3f CCamera::UpVector() const
{
    return mUpVector;
}

CVector3f CCamera::RightVector() const
{
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

const CMatrix4f& CCamera::ViewMatrix()
{
    if (mViewOutdated)
        UpdateView();

    return mCachedViewMatrix;
}

const CMatrix4f& CCamera::ProjectionMatrix()
{
    if (mProjectionOutdated)
        UpdateProjection();

    return mCachedProjectionMatrix;
}

const CFrustumPlanes& CCamera::FrustumPlanes()
{
    if (mFrustumPlanesOutdated)
        UpdateFrustum();

    return mCachedFrustumPlanes;
}

// ************ SETTERS ************
void CCamera::SetPosition(CVector3f Position)
{
    mPosition = Position;
    mViewOutdated = true;
    mFrustumPlanesOutdated = true;
}

void CCamera::SetDirection(CVector3f Direction)
{
    mDirection = Direction;
    mViewOutdated = true;
    mFrustumPlanesOutdated = true;
}

void CCamera::SetYaw(float Yaw)
{
    mYaw = Yaw;
}

void CCamera::SetPitch(float Pitch)
{
    mPitch = Pitch;
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
    mProjectionOutdated = true;
    mFrustumPlanesOutdated = true;
}

// ************ PRIVATE ************
void CCamera::Update()
{
    // Update direction
    if (mPitch > Math::skHalfPi)  mPitch = Math::skHalfPi;
    if (mPitch < -Math::skHalfPi) mPitch = -Math::skHalfPi;

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

    mViewOutdated = true;
    mFrustumPlanesOutdated = true;
}

void CCamera::UpdateView()
{
    // todo: don't use glm
    Update();

    glm::vec3 glmpos(mPosition.x, mPosition.y, mPosition.z);
    glm::vec3 glmdir(mDirection.x, mDirection.y, mDirection.z);
    glm::vec3 glmup(mUpVector.x, mUpVector.y, mUpVector.z);
    mCachedViewMatrix = CMatrix4f::FromGlmMat4(glm::lookAt(glmpos, glmpos + glmdir, glmup)).Transpose();
    mViewOutdated = false;
}

void CCamera::UpdateProjection()
{
    mCachedProjectionMatrix = Math::PerspectiveMatrix(55.f, mAspectRatio, 0.1f, 4096.f);
    mProjectionOutdated = false;
}

void CCamera::UpdateFrustum()
{
    mCachedFrustumPlanes.SetPlanes(mPosition, mDirection, 55.f, mAspectRatio, 0.1f, 4096.f);
    mFrustumPlanesOutdated = false;
}
