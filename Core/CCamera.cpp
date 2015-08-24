#include "CCamera.h"
#include "CGraphics.h"
#include <Common/CQuaternion.h>

#include <gtc/matrix_transform.hpp>

#define HALF_PI 1.570796371f

CCamera::CCamera()
{
    mMode = eFreeCamera;
    mPosition = CVector3f(0);
    mAspectRatio = 1.7777777f;

    mYaw = -HALF_PI;
    mPitch = 0.0f;
    CalculateDirection();

    mMoveSpeed = 1.f; // Old: 0.01f
    mLookSpeed = 1.f; // Old: 0.003f
    mViewOutdated = true;
    mProjectionOutdated = true;
}

CCamera::CCamera(CVector3f Position, CVector3f)
{
    // todo: make it actually look at the target!
    // Not using parameter 2 (CVector3f - Target)
    mMode = eFreeCamera;
    mMoveSpeed = 1.f; // Old: 0.01f
    mLookSpeed = 1.f; // Old: 0.003f
    mPosition = Position;
    mYaw = -HALF_PI;
    mPitch = 0.0f;
    CalculateDirection();
}

void CCamera::Pan(float XAmount, float YAmount)
{
    switch (mMode)
    {

    case eFreeCamera:
    {
        CVector3f Right(
            cos(mYaw - HALF_PI),
            sin(mYaw - HALF_PI),
            0
        );
        CVector3f Up = Right.Cross(mDirection);

        mPosition += Right * (XAmount * mMoveSpeed);
        mPosition += Up * (YAmount * mMoveSpeed);
        mViewOutdated = true;
        break;
    }

    // Unfinished
    case eOrbitCamera:
    {
        CVector3f Right(
            cos(mYaw - HALF_PI),
            sin(mYaw - HALF_PI),
            0
        );
        CVector3f Up = Right.Cross(mDirection);

        CVector3f TargetDirection = mPosition - mOrbitTarget;
        //CMatrix4f YawRotation = CQuaternion::F
        }
    }
}

void CCamera::Rotate(float XAmount, float YAmount)
{
    switch (mMode)
    {
    case eFreeCamera:
        mYaw -= (XAmount * mLookSpeed * 0.3f);
        mPitch -= (YAmount * mLookSpeed * 0.3f);
        mViewOutdated = true;
        break;
    }
}

void CCamera::Zoom(float Amount)
{
    mPosition += (mDirection * Amount) * (mMoveSpeed * 25.f);
    mViewOutdated = true;
}

void CCamera::Snap(CVector3f Position)
{
    mPosition = Position;
    mYaw = -1.570796371f;
    mPitch = 0.0f;
    mViewOutdated = true;
}

void CCamera::ProcessKeyInput(EKeyInputs KeyFlags, double DeltaTime)
{
    float FDeltaTime = (float) DeltaTime;
    if (mMode == eFreeCamera)
    {
        if (KeyFlags & eWKey) Zoom(FDeltaTime);
        if (KeyFlags & eSKey) Zoom(-FDeltaTime);
        if (KeyFlags & eQKey) Pan(0, -FDeltaTime * 25.f);
        if (KeyFlags & eEKey) Pan(0, FDeltaTime * 25.f);
        if (KeyFlags & eAKey) Pan(-FDeltaTime * 25.f, 0);
        if (KeyFlags & eDKey) Pan(FDeltaTime * 25.f, 0);
    }
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
        if ((MouseFlags & eMiddleButton) && (KeyFlags & eCtrlKey))
            Zoom(-YMovement * 0.2f);

        else if ((MouseFlags & eMiddleButton) || (MouseFlags & eRightButton))
            Pan(XMovement, YMovement);
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

void CCamera::LoadMatrices()
{
    CGraphics::sMVPBlock.ViewMatrix = ViewMatrix();
    CGraphics::sMVPBlock.ProjectionMatrix = ProjectionMatrix();
    CGraphics::UpdateMVPBlock();
}

void CCamera::LoadRotationOnlyMatrices()
{
    CGraphics::sMVPBlock.ViewMatrix = RotationOnlyViewMatrix();
    CGraphics::sMVPBlock.ProjectionMatrix = ProjectionMatrix();
    CGraphics::UpdateMVPBlock();
}

// ************ GETTERS ************
CVector3f CCamera::Position() const
{
    return mPosition;
}

CVector3f CCamera::GetDirection() const
{
    return mDirection;
}

float CCamera::GetYaw() const
{
    return mYaw;
}

float CCamera::GetPitch() const
{
    return mPitch;
}


const CMatrix4f& CCamera::ViewMatrix()
{
    if (mViewOutdated)
        CalculateView();

    return mCachedViewMatrix;
}

const CMatrix4f& CCamera::RotationOnlyViewMatrix()
{
    if (mViewOutdated)
        CalculateView();

    return CMatrix4f(mCachedViewMatrix[0][0], mCachedViewMatrix[0][1], mCachedViewMatrix[0][2], 0.f,
                     mCachedViewMatrix[1][0], mCachedViewMatrix[1][1], mCachedViewMatrix[1][2], 0.f,
                     mCachedViewMatrix[2][0], mCachedViewMatrix[2][1], mCachedViewMatrix[2][2], 0.f,
                     mCachedViewMatrix[3][0], mCachedViewMatrix[3][1], mCachedViewMatrix[3][2], 1.f);
}

const CMatrix4f& CCamera::ProjectionMatrix()
{
    if (mProjectionOutdated)
        CalculateProjection();

    return mCachedProjectionMatrix;
}

// ************ SETTERS ************
void CCamera::SetPosition(CVector3f Position)
{
    mPosition = Position;
    mViewOutdated = true;
}

void CCamera::SetDirection(CVector3f Direction)
{
    mDirection = Direction;
    mViewOutdated = true;
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

void CCamera::SetFree()
{
    mMode = eFreeCamera;
}

void CCamera::SetOrbit(const CVector3f& OrbitTarget)
{
    mMode = eOrbitCamera;
    mOrbitTarget = OrbitTarget;
}

void CCamera::SetAspectRatio(float AspectRatio)
{
    mAspectRatio = AspectRatio;
    mProjectionOutdated = true;
}

// ************ PRIVATE ************
void CCamera::CalculateDirection()
{
    if (mPitch > HALF_PI)  mPitch = HALF_PI;
    if (mPitch < -HALF_PI) mPitch = -HALF_PI;

    mDirection = CVector3f(
                 cos(mPitch) * cos(mYaw),
                 cos(mPitch) * sin(mYaw),
                 sin(mPitch)
                 );
}

void CCamera::CalculateView()
{
    // todo: don't use glm
    CalculateDirection();

    CVector3f Right(
        cos(mYaw - HALF_PI),
        sin(mYaw - HALF_PI),
        0
    );

    CVector3f Up = Right.Cross(mDirection);

    glm::vec3 glmpos(mPosition.x, mPosition.y, mPosition.z);
    glm::vec3 glmdir(mDirection.x, mDirection.y, mDirection.z);
    glm::vec3 glmup(Up.x, Up.y, Up.z);
    mCachedViewMatrix = CMatrix4f::FromGlmMat4(glm::lookAt(glmpos, glmpos + glmdir, glmup)).Transpose();
    mViewOutdated = false;
}

void CCamera::CalculateProjection()
{
    // todo: don't use glm
    mCachedProjectionMatrix = CMatrix4f::FromGlmMat4(glm::perspective(55.f, mAspectRatio, 0.1f, 4096.f)).Transpose();
    mProjectionOutdated = false;
}
