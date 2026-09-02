#include "Editor/CGizmo.h"

#include <Common/Log.h>
#include <Common/Math/MathUtil.h>
#include <Core/NRangeUtils.h>
#include <Core/GameProject/CResourceStore.h>
#include <Core/Render/CDrawUtil.h>
#include <Core/Render/CGraphics.h>
#include <Core/Render/CRenderer.h>
#include <Core/Resource/Model/CModel.h>
#include <Core/Resource/Model/SSurface.h>

#include <algorithm>
#include <cmath>
#include <QApplication>
#include <QScreen>

namespace
{
#define CGIZMO_TRANSLATE_X 0
#define CGIZMO_TRANSLATE_Y 1
#define CGIZMO_TRANSLATE_Z 2
#define CGIZMO_TRANSLATE_LINES_XY 3
#define CGIZMO_TRANSLATE_LINES_XZ 4
#define CGIZMO_TRANSLATE_LINES_YZ 5
#define CGIZMO_TRANSLATE_POLY_XY 6
#define CGIZMO_TRANSLATE_POLY_XZ 7
#define CGIZMO_TRANSLATE_POLY_YZ 8
#define CGIZMO_TRANSLATE_NUM 9
#define CGIZMO_ROTATE_OUTLINE 0
#define CGIZMO_ROTATE_X 1
#define CGIZMO_ROTATE_Y 2
#define CGIZMO_ROTATE_Z 3
#define CGIZMO_ROTATE_XYZ 4
#define CGIZMO_ROTATE_NUM 5
#define CGIZMO_SCALE_X 0
#define CGIZMO_SCALE_Y 1
#define CGIZMO_SCALE_Z 2
#define CGIZMO_SCALE_LINES_XY 3
#define CGIZMO_SCALE_LINES_XZ 4
#define CGIZMO_SCALE_LINES_YZ 5
#define CGIZMO_SCALE_POLY_XY 6
#define CGIZMO_SCALE_POLY_XZ 7
#define CGIZMO_SCALE_POLY_YZ 8
#define CGIZMO_SCALE_XYZ 9
#define CGIZMO_SCALE_NUM 10

constinit bool smModelsLoaded = false;
constinit std::array<SGizmoModelPart, CGIZMO_TRANSLATE_NUM> smTranslateModels;
constinit std::array<SGizmoModelPart, CGIZMO_ROTATE_NUM> smRotateModels;
constinit std::array<SGizmoModelPart, CGIZMO_SCALE_NUM> smScaleModels;
 }

static void LoadModels()
 {
     if (!smModelsLoaded)
     {
         NLog::Debug("Loading transform gizmo models");

         smTranslateModels[CGIZMO_TRANSLATE_X]        = SGizmoModelPart(EAxis::X, true, false, gpEditorStore->LoadResource("editor/TranslateX.CMDL"));
         smTranslateModels[CGIZMO_TRANSLATE_Y]        = SGizmoModelPart(EAxis::Y, true, false, gpEditorStore->LoadResource("editor/TranslateY.CMDL"));
         smTranslateModels[CGIZMO_TRANSLATE_Z]        = SGizmoModelPart(EAxis::Z, true, false, gpEditorStore->LoadResource("editor/TranslateZ.CMDL"));
         smTranslateModels[CGIZMO_TRANSLATE_LINES_XY] = SGizmoModelPart(EAxis::XY, true, false, gpEditorStore->LoadResource("editor/TranslateLinesXY.CMDL"));
         smTranslateModels[CGIZMO_TRANSLATE_LINES_XZ] = SGizmoModelPart(EAxis::XZ, true, false, gpEditorStore->LoadResource("editor/TranslateLinesXZ.CMDL"));
         smTranslateModels[CGIZMO_TRANSLATE_LINES_YZ] = SGizmoModelPart(EAxis::YZ, true, false, gpEditorStore->LoadResource("editor/TranslateLinesYZ.CMDL"));
         smTranslateModels[CGIZMO_TRANSLATE_POLY_XY]  = SGizmoModelPart(EAxis::XY, false, false, gpEditorStore->LoadResource("editor/TranslatePolyXY.CMDL"));
         smTranslateModels[CGIZMO_TRANSLATE_POLY_XZ]  = SGizmoModelPart(EAxis::XZ, false, false, gpEditorStore->LoadResource("editor/TranslatePolyXZ.CMDL"));
         smTranslateModels[CGIZMO_TRANSLATE_POLY_YZ]  = SGizmoModelPart(EAxis::YZ, false, false, gpEditorStore->LoadResource("editor/TranslatePolyYZ.CMDL"));

         smRotateModels[CGIZMO_ROTATE_OUTLINE] = SGizmoModelPart(EAxis::None, true, true, gpEditorStore->LoadResource("editor/RotateClipOutline.CMDL"));
         smRotateModels[CGIZMO_ROTATE_X]       = SGizmoModelPart(EAxis::X, true, false, gpEditorStore->LoadResource("editor/RotateX.CMDL"));
         smRotateModels[CGIZMO_ROTATE_Y]       = SGizmoModelPart(EAxis::Y, true, false, gpEditorStore->LoadResource("editor/RotateY.CMDL"));
         smRotateModels[CGIZMO_ROTATE_Z]       = SGizmoModelPart(EAxis::Z, true, false, gpEditorStore->LoadResource("editor/RotateZ.CMDL"));
         smRotateModels[CGIZMO_ROTATE_XYZ]     = SGizmoModelPart(EAxis::XYZ, false, false, gpEditorStore->LoadResource("editor/RotateXYZ.CMDL"));

         smScaleModels[CGIZMO_SCALE_X]        = SGizmoModelPart(EAxis::X, true, false, gpEditorStore->LoadResource("editor/ScaleX.CMDL"));
         smScaleModels[CGIZMO_SCALE_Y]        = SGizmoModelPart(EAxis::Y, true, false, gpEditorStore->LoadResource("editor/ScaleY.CMDL"));
         smScaleModels[CGIZMO_SCALE_Z]        = SGizmoModelPart(EAxis::Z, true, false, gpEditorStore->LoadResource("editor/ScaleZ.CMDL"));
         smScaleModels[CGIZMO_SCALE_LINES_XY] = SGizmoModelPart(EAxis::XY, true, false, gpEditorStore->LoadResource("editor/ScaleLinesXY.CMDL"));
         smScaleModels[CGIZMO_SCALE_LINES_XZ] = SGizmoModelPart(EAxis::XZ, true, false, gpEditorStore->LoadResource("editor/ScaleLinesXZ.CMDL"));
         smScaleModels[CGIZMO_SCALE_LINES_YZ] = SGizmoModelPart(EAxis::YZ, true, false, gpEditorStore->LoadResource("editor/ScaleLinesYZ.CMDL"));
         smScaleModels[CGIZMO_SCALE_POLY_XY]  = SGizmoModelPart(EAxis::XY, true, false, gpEditorStore->LoadResource("editor/ScalePolyXY.CMDL"));
         smScaleModels[CGIZMO_SCALE_POLY_XZ]  = SGizmoModelPart(EAxis::XZ, true, false, gpEditorStore->LoadResource("editor/ScalePolyXZ.CMDL"));
         smScaleModels[CGIZMO_SCALE_POLY_YZ]  = SGizmoModelPart(EAxis::YZ, true, false, gpEditorStore->LoadResource("editor/ScalePolyYZ.CMDL"));
         smScaleModels[CGIZMO_SCALE_XYZ]      = SGizmoModelPart(EAxis::XYZ, true, false, gpEditorStore->LoadResource("editor/ScaleXYZ.CMDL"));

         smModelsLoaded = true;
     }
 }

CGizmo::CGizmo()
{
    LoadModels();
    SetMode(EGizmoMode::Translate);
}

CGizmo::~CGizmo() = default;

void CGizmo::AddToRenderer(CRenderer *pRenderer, const SViewInfo&)
{
    // Transform is updated every frame even if the user doesn't modify the gizmo
    // in order to account for scale changes based on camera distance
    UpdateTransform();

    // Add all parts to renderer
    for (auto&& [idx, part] : Utils::enumerate(mpCurrentParts))
    {
        const CModel* pModel = part.pModel;

        // Determine whether to use the mat set for regular (0) or highlight (1)
        const FAxes PartAxes = part.ModelAxes;
        const bool IsHighlighted = (PartAxes != EAxis::None) && ((mSelectedAxes & PartAxes) == part.ModelAxes);
        const auto SetID = (IsHighlighted ? 1U : 0U);

        // Add to renderer...
        pRenderer->AddMesh(this, int(idx), pModel->AABox().Transformed(mTransform), pModel->HasTransparency(SetID), ERenderCommand::DrawMesh, EDepthGroup::Foreground);
    }
}

void CGizmo::Draw(FRenderOptions /*Options*/, int ComponentIndex, ERenderCommand /*Command*/, const SViewInfo& /*rkViewInfo*/)
{
    // Determine which SGizmoModelPart array to use
    if (ComponentIndex >= std::ssize(mpCurrentParts))
        return;

    // Set model matrix
    if (mpCurrentParts[ComponentIndex].IsBillboard)
        CGraphics::sMVPBlock.ModelMatrix = mBillboardTransform;
    else if (mMode == EGizmoMode::Scale && (mSelectedAxes & mpCurrentParts[ComponentIndex].ModelAxes))
        CGraphics::sMVPBlock.ModelMatrix = mScaledTransform;
    else
        CGraphics::sMVPBlock.ModelMatrix = mTransform;

    CGraphics::UpdateMVPBlock();

    // Clear tint color
    CGraphics::sPixelBlock.TintColor = CColor::White();
    CGraphics::UpdatePixelBlock();

    // Choose material set
    const FAxes PartAxes = mpCurrentParts[ComponentIndex].ModelAxes;
    const bool IsHighlighted = (PartAxes != EAxis::None) && ((mSelectedAxes & PartAxes) == mpCurrentParts[ComponentIndex].ModelAxes);
    const auto SetID = (IsHighlighted ? 1U : 0U);

    // Draw model
    mpCurrentParts[ComponentIndex].pModel->Draw(FRenderOptions(0), SetID);
}

void CGizmo::IncrementSize()
{
    static const float skIncAmount = 1.3f;
    static const float skMaxSize = powf(skIncAmount, 4);

    mGizmoSize *= skIncAmount;
    mGizmoSize = std::min(mGizmoSize, skMaxSize);
}

void CGizmo::DecrementSize()
{
    static const float skDecAmount = (1.f / 1.3f);
    static const float skMinSize = powf(skDecAmount, 4);

    mGizmoSize *= skDecAmount;
    mGizmoSize = std::max(mGizmoSize, skMinSize);
}

void CGizmo::UpdateForCamera(const CCamera& rkCamera)
{
    const CVector3f CamPos = rkCamera.Position();
    const CVector3f CameraToGizmo = (mPosition - CamPos).Normalized();
    mFlipScaleX = (mRotation.XAxis().Dot(CameraToGizmo) >= 0.f);
    mFlipScaleY = (mRotation.YAxis().Dot(CameraToGizmo) >= 0.f);
    mFlipScaleZ = (mRotation.ZAxis().Dot(CameraToGizmo) >= 0.f);

    if (!mIsTransforming || mMode != EGizmoMode::Translate)
        mCameraDist = mPosition.Distance(CamPos);

    // todo: make this cleaner...
    const CVector3f BillDir = (CamPos - mPosition).Normalized();
    const CVector3f Axis = CVector3f::Forward().Cross(BillDir);
    const float Angle = acosf(CVector3f::Forward().Dot(BillDir));
    mBillboardRotation = CQuaternion::FromAxisAngle(Angle, Axis);
}

bool CGizmo::CheckSelectedAxes(const CRay& rkRay)
{
    const CRay LocalRay = rkRay.Transformed(mTransform.Inverse());
    const CRay BillRay = rkRay.Transformed(mBillboardTransform.Inverse());

    // Do raycast on each model
    const auto* pPart = mpCurrentParts.data();

    struct SResult {
        const SGizmoModelPart* pPart;
        float Dist;

        static bool DistanceLessThan(const SResult& left, const SResult& right)
        {
            return left.Dist < right.Dist;
        }
    };
    std::list<SResult> Results;

    for (size_t iPart = 0; iPart < mpCurrentParts.size(); iPart++)
    {
        if (!pPart->EnableRayCast)
        {
            pPart++;
            continue;
        }

        const CModel* pModel = pPart->pModel;
        const CRay& rPartRay = (pPart->IsBillboard ? BillRay : LocalRay);

        // Ray/Model AABox test - allow buffer room because lines are small
        CAABox AABox = pModel->AABox();
        AABox.ExpandBy(CVector3f::One());
        const bool ModelBoxCheck = Math::RayBoxIntersection(rPartRay, AABox).first;

        if (ModelBoxCheck)
        {
            bool Hit = false;
            float Dist = 0.0f;

            for (size_t iSurf = 0; iSurf < pModel->GetSurfaceCount(); iSurf++)
            {
                // Skip surface/box check - since we use lines the boxes might be too small
                const SSurface* pSurf = pModel->GetSurface(iSurf);
                const auto [intersects, distance] = pSurf->IntersectsRay(rPartRay, false, 0.05f);

                if (intersects)
                {
                    if (!Hit || distance < Dist)
                        Dist = distance;

                    Hit = true;
                }
            }

            if (Hit)
                Results.emplace_back(pPart, Dist);
        }

        pPart++;
    }

    // Results list empty = no hits
    if (Results.empty())
    {
        mSelectedAxes = EAxis::None;
        return false;
    }

    // Otherwise, we have at least one hit - sort results and set selected axes
    Results.sort(SResult::DistanceLessThan);

    const CRay& rPartRay = (pPart->IsBillboard ? BillRay : LocalRay);
    mSelectedAxes = Results.front().pPart->ModelAxes;
    mHitPoint = mTransform * rPartRay.PointOnRay(Results.front().Dist);

    return mSelectedAxes != EAxis::None;
}

uint32_t CGizmo::NumSelectedAxes() const
{
    uint32_t Out = 0;

    for (uint32_t iAxis = 1; iAxis < 8; iAxis <<= 1)
    {
        if (mSelectedAxes & FAxes(iAxis))
            Out++;
    }

    return Out;
}

void CGizmo::ResetSelectedAxes()
{
    mSelectedAxes = EAxis::None;
}

void CGizmo::StartTransform()
{
    mIsTransforming = true;
    mHasTransformed = false;
    mWrapOffset = CVector2f::Zero();
    mSetOffset = false;
    mTotalTranslation = CVector3f::Zero();
    mTotalRotation = CVector3f::Zero();
    mCurrentRotation = CQuaternion::Identity();
    mTotalScale = CVector3f::One();

    // Set rotation direction
    if (mMode == EGizmoMode::Rotate)
    {
        CVector3f Axis;
        if (mSelectedAxes & EAxis::X)
            Axis = mRotation.XAxis();
        else if (mSelectedAxes & EAxis::Y)
            Axis = mRotation.YAxis();
        else
            Axis = mRotation.ZAxis();

        const CVector3f GizmoToHit = (mHitPoint - mPosition).Normalized();
        mMoveDir = Axis.Cross(GizmoToHit);
    }
    // Set scale direction
    else if (mMode == EGizmoMode::Scale)
    {
        // Only need to set scale direction if < 3 axes selected
        if (NumSelectedAxes() != 3)
        {
            // One axis; direction = selected axis
            if (NumSelectedAxes() == 1)
            {
                if (mSelectedAxes & EAxis::X)
                    mMoveDir = mRotation.XAxis();
                else if (mSelectedAxes & EAxis::Y)
                    mMoveDir = mRotation.YAxis();
                else
                    mMoveDir = mRotation.ZAxis();
            }
            // Two axes; interpolate between the two selected axes
            else if (NumSelectedAxes() == 2)
            {
                const CVector3f AxisA = (mSelectedAxes & EAxis::X ? mRotation.XAxis() : mRotation.YAxis());
                const CVector3f AxisB = (mSelectedAxes & EAxis::Z ? mRotation.ZAxis() : mRotation.YAxis());
                mMoveDir = (AxisA + AxisB) / 2.f;
            }
        }
    }
}

bool CGizmo::TransformFromInput(const CRay& rkRay, const CCamera& rCamera)
{
    // Wrap cursor (this has no effect until the next time this function is called)
    if (mEnableCursorWrap && (mMode != EGizmoMode::Translate))
        WrapCursor();

    // Calculate normalized cursor position
    const QPoint CursorPos = QCursor::pos();
    const QRect Geom = QApplication::screenAt(CursorPos)->geometry();
    const CVector2f MouseCoords(
        (((2.f * CursorPos.x()) / Geom.width()) - 1.f),
        (1.f - ((2.f * CursorPos.y()) / Geom.height()))
    );

    // Translate
    if (mMode == EGizmoMode::Translate)
    {
        // Create translate plane
        CVector3f AxisA, AxisB;
        const uint32_t NumAxes = NumSelectedAxes();

        if (NumAxes == 1)
        {
            if (mSelectedAxes & EAxis::X)
                AxisB = mRotation.XAxis();
            else if (mSelectedAxes & EAxis::Y)
                AxisB = mRotation.YAxis();
            else
                AxisB = mRotation.ZAxis();

            const CVector3f GizmoToCamera = (mPosition - rCamera.Position()).Normalized();
            AxisA = AxisB.Cross(GizmoToCamera);
        }
        else if (NumAxes == 2)
        {
            AxisA = mSelectedAxes & EAxis::X ? mRotation.XAxis() : mRotation.YAxis();
            AxisB = mSelectedAxes & EAxis::Z ? mRotation.ZAxis() : mRotation.YAxis();
        }

        const CVector3f PlaneNormal = AxisA.Cross(AxisB);
        mTranslatePlane.Redefine(PlaneNormal, mPosition);

        // Do translate
        const auto [Intersects, Distance] = Math::RayPlaneIntersection(rkRay, mTranslatePlane);
        if (Intersects)
        {
            const CVector3f Hit = rkRay.PointOnRay(Distance);
            const CVector3f LocalDelta = mRotation.Inverse() * (Hit - mPosition);

            // Calculate new position
            CVector3f NewPos = mPosition;
            if (mSelectedAxes & EAxis::X)
                NewPos += mRotation.XAxis() * LocalDelta.X;
            if (mSelectedAxes & EAxis::Y)
                NewPos += mRotation.YAxis() * LocalDelta.Y;
            if (mSelectedAxes & EAxis::Z)
                NewPos += mRotation.ZAxis() * LocalDelta.Z;

            // Check relativity of new pos to camera to reduce issue where the gizmo might
            // go flying off into the distance if newPosToCamera is parallel to the plane
            const CVector3f NewPosToCamera = (NewPos - rCamera.Position()).Normalized();
            const float Dot = std::abs(PlaneNormal.Dot(NewPosToCamera));
            if (Dot < 0.02f)
                return false;

            // Set offset
            if (!mSetOffset)
            {
                mTranslateOffset = mPosition - NewPos;
                mDeltaTranslation = CVector3f::Zero();
                mSetOffset = true;
                return false;
            }
            else // Apply translation
            {
                mDeltaTranslation = mRotation.Inverse() * (NewPos - mPosition + mTranslateOffset);
                if (!(mSelectedAxes & EAxis::X))
                    mDeltaTranslation.X = 0.f;
                if (!(mSelectedAxes & EAxis::Y))
                    mDeltaTranslation.Y = 0.f;
                if (!(mSelectedAxes & EAxis::Z))
                    mDeltaTranslation.Z = 0.f;

                mTotalTranslation += mDeltaTranslation;
                mPosition += mRotation * mDeltaTranslation;

                if (!mHasTransformed && (mDeltaTranslation != CVector3f::Zero()))
                    mHasTransformed = true;

                return mHasTransformed;
            }
        }
        else
        {
            mDeltaTranslation = CVector3f::Zero();
            return false;
        }
    }

    // Rotate
    if (mMode == EGizmoMode::Rotate)
    {
        // Choose rotation axis
        CVector3f Axis;
        if (mSelectedAxes & EAxis::X)
            Axis = CVector3f::UnitX();
        else if (mSelectedAxes & EAxis::Y)
            Axis = CVector3f::UnitY();
        else
            Axis = CVector3f::UnitZ();

        // Convert hit point + move direction into a line in screen space
        // Clockwise direction is set in StartTransform(). Is there a cleaner way to calculate the direction?
        const CMatrix4f VP = rCamera.ViewMatrix().Transpose() * rCamera.ProjectionMatrix().Transpose();
        const CVector2f LineOrigin = (mHitPoint * VP).XY();
        const CVector2f LineDir = (((mHitPoint + mMoveDir) * VP).XY() - LineOrigin).Normalized();
        float RotAmount = LineDir.Dot(MouseCoords + mWrapOffset - LineOrigin) * 180.f;

        // Set offset
        if (!mSetOffset)
        {
            mRotateOffset = -RotAmount;
            mDeltaRotation = CQuaternion::Identity();
            mSetOffset = true;
            return false;
        }

        // Apply rotation
        RotAmount += mRotateOffset;
        const CQuaternion OldRot = mCurrentRotation;
        mCurrentRotation = CQuaternion::FromAxisAngle(Math::DegreesToRadians(RotAmount), Axis);
        mDeltaRotation = mCurrentRotation * OldRot.Inverse();

        if (mTransformSpace == ETransformSpace::Local)
            mRotation *= mDeltaRotation;

        // Add to total
        if (mSelectedAxes & EAxis::X)
            mTotalRotation.X = RotAmount;
        else if (mSelectedAxes & EAxis::Y)
            mTotalRotation.Y = RotAmount;
        else
            mTotalRotation.Z = RotAmount;

        if (!mHasTransformed && RotAmount != 0.f)
            mHasTransformed = true;

        return mHasTransformed;
    }

    // Scale
    if (mMode == EGizmoMode::Scale)
    {
        // Create a line in screen space. First step: line origin
        const CMatrix4f VP = rCamera.ViewMatrix().Transpose() * rCamera.ProjectionMatrix().Transpose();
        const CVector2f LineOrigin = (mPosition * VP).XY();

        // Next step: determine the appropriate world space direction using the selected axes and then convert to screen space
        // Since the axes can be flipped while the gizmo is transforming, this has to be done every frame rather than
        // pre-saving the world space direction like the rotate gizmo does.
        const CVector3f DirX = (mFlipScaleX ? -mRotation.XAxis() : mRotation.XAxis());
        const CVector3f DirY = (mFlipScaleY ? -mRotation.YAxis() : mRotation.YAxis());
        const CVector3f DirZ = (mFlipScaleZ ? -mRotation.ZAxis() : mRotation.ZAxis());
        CVector2f LineDir;

        // One axis - world space direction is just the selected axis
        if (NumSelectedAxes() == 1)
        {
            CVector3f WorldDir;
            if (mSelectedAxes & EAxis::X)
                WorldDir = DirX;
            else if (mSelectedAxes & EAxis::Y)
                WorldDir = DirY;
            else
                WorldDir = DirZ;
            LineDir = (((mPosition + WorldDir) * VP).XY() - LineOrigin).Normalized();
        }
        // Two axes - take the two selected axes and convert them to world space, then average them for the line direction
        else if (NumSelectedAxes() == 2)
        {
            const CVector3f AxisA = (mSelectedAxes & EAxis::X ? DirX : DirY);
            const CVector3f AxisB = (mSelectedAxes & EAxis::Z ? DirZ : DirY);
            const CVector2f ScreenA = (((mPosition + AxisA) * VP).XY() - LineOrigin).Normalized();
            const CVector2f ScreenB = (((mPosition + AxisB) * VP).XY() - LineOrigin).Normalized();
            LineDir = ((ScreenA + ScreenB) / 2.f).Normalized();
        }
        else // Three axes - use straight up
        {
            LineDir = CVector2f::Up();
        }
        float ScaleAmount = LineDir.Dot(MouseCoords + mWrapOffset - LineOrigin) * 5.f;

        // Set offset
        if (!mSetOffset)
        {
            mScaleOffset = -ScaleAmount;
            mDeltaScale = CVector3f::One();
            mSetOffset = true;
            return false;
        }

        // Apply scale
        ScaleAmount = ScaleAmount + mScaleOffset + 1.f;

        // A multiplier is applied to the scale amount of it's less than 1 to prevent it from going negative
        if (ScaleAmount < 1.f)
            ScaleAmount = 1.f / (-(ScaleAmount - 1.f) + 1.f);

        const CVector3f OldScale = mTotalScale;

        mTotalScale = CVector3f::One();
        if (mSelectedAxes & EAxis::X)
            mTotalScale.X = ScaleAmount;
        if (mSelectedAxes & EAxis::Y)
            mTotalScale.Y = ScaleAmount;
        if (mSelectedAxes & EAxis::Z)
            mTotalScale.Z = ScaleAmount;

        mDeltaScale = mTotalScale / OldScale;

        if (!mHasTransformed && (ScaleAmount != 1.f))
            mHasTransformed = true;

        return mHasTransformed;
    }

    return false;
}

void CGizmo::EndTransform()
{
    mTotalScale = CVector3f::One();
    mIsTransforming = false;
}

void CGizmo::SetMode(EGizmoMode Mode)
{
    mMode = Mode;

    switch (Mode)
    {
    case EGizmoMode::Translate:
        mpCurrentParts = smTranslateModels;
        mDeltaRotation = CQuaternion::Identity();
        mDeltaScale = CVector3f::One();
        break;

    case EGizmoMode::Rotate:
        mpCurrentParts = smRotateModels;
        mDeltaTranslation = CVector3f::Zero();
        mDeltaScale = CVector3f::One();
        break;

    case EGizmoMode::Scale:
        mpCurrentParts = smScaleModels;
        mDeltaTranslation = CVector3f::Zero();
        mDeltaRotation = CQuaternion::Identity();
        break;

    case EGizmoMode::Off:
        break;
    }
}

void CGizmo::SetTransformSpace(ETransformSpace Space)
{
    mTransformSpace = Space;

    if (Space == ETransformSpace::World)
        mRotation = CQuaternion::Identity();
    else
        mRotation = mLocalRotation;
}

void CGizmo::SetLocalRotation(const CQuaternion& rkOrientation)
{
    mLocalRotation = rkOrientation;

    if (mTransformSpace == ETransformSpace::Local)
        mRotation = rkOrientation;
}

// ************ PROTECTED ************
void CGizmo::UpdateTransform()
{
    // Scale is recalculated every frame because it changes frequently, based on camera distance
    // Rotation and position values are just saved directly
    mScale = mGizmoSize * (mCameraDist / 10.f);

    // Scale also factors in axis flip if mode is Scale.
    if (mMode == EGizmoMode::Scale)
    {
        if (mFlipScaleX) mScale.X = -mScale.X;
        if (mFlipScaleY) mScale.Y = -mScale.Y;
        if (mFlipScaleZ) mScale.Z = -mScale.Z;
    }

    // Create transform
    mTransform.SetIdentity();
    mTransform.Scale(mScale);
    mTransform.Rotate(mRotation);
    mTransform.Translate(mPosition);

    // Create billboard transform for rotation gizmo
    if (mMode == EGizmoMode::Rotate)
    {
        mBillboardTransform.SetIdentity();
        mBillboardTransform.Scale(mScale);
        mBillboardTransform.Rotate(mBillboardRotation);
        mBillboardTransform.Translate(mPosition);
    }

    // Create scaled transform for scale gizmo
    else if (mMode == EGizmoMode::Scale)
    {
        mScaledTransform.SetIdentity();
        mScaledTransform.Scale(mScale * mTotalScale);
        mScaledTransform.Rotate(mRotation);
        mScaledTransform.Translate(mPosition);
    }
}

void CGizmo::WrapCursor()
{
    QPoint CursorPos = QCursor::pos();
    const QRect Geom = QApplication::screenAt(CursorPos)->geometry();

    // Horizontal
    if (CursorPos.x() == Geom.width() - 1)
    {
        QCursor::setPos(1, CursorPos.y());
        mWrapOffset.X += 2.f;
    }
    else if (CursorPos.x() == 0)
    {
        QCursor::setPos(Geom.width() - 2, CursorPos.y());
        mWrapOffset.X -= 2.f;
    }

    // Vertical
    CursorPos = QCursor::pos(); // Grab again to account for changes on horizontal wrap

    if (CursorPos.y() == Geom.height() - 1)
    {
        QCursor::setPos(CursorPos.x(), 1);
        mWrapOffset.Y -= 2.f;
    }
    else if (CursorPos.y() == 0)
    {
        QCursor::setPos(CursorPos.x(), Geom.height() - 2);
        mWrapOffset.Y += 2.f;
    }
}
