#include "CGizmo.h"
#include <Common/Math/MathUtil.h>
#include <Core/GameProject/CResourceStore.h>
#include <Core/Render/CDrawUtil.h>
#include <Core/Render/CRenderer.h>
#include <Common/Log.h>

#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>

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
    SModelPart *pPart = mpCurrentParts;

    // Add all parts to renderer
    for (uint32 iPart = 0; iPart < mNumCurrentParts; iPart++)
    {
        CModel *pModel = pPart->pModel;

        // Determine whether to use the mat set for regular (0) or highlight (1)
        const FAxes PartAxes = pPart->ModelAxes;
        const bool IsHighlighted = (PartAxes != EAxis::None) && ((mSelectedAxes & PartAxes) == pPart->ModelAxes);
        const size_t SetID = (IsHighlighted ? 1 : 0);

        // Add to renderer...
        pRenderer->AddMesh(this, iPart, pModel->AABox().Transformed(mTransform), pModel->HasTransparency(SetID), ERenderCommand::DrawMesh, EDepthGroup::Foreground);
        pPart++;
    }
}

void CGizmo::Draw(FRenderOptions /*Options*/, int ComponentIndex, ERenderCommand /*Command*/, const SViewInfo& /*rkViewInfo*/)
{
    // Determine which SModelPart array to use
    if (ComponentIndex >= (int) mNumCurrentParts) return;
    SModelPart *pPart = mpCurrentParts;

    // Set model matrix
    if (pPart[ComponentIndex].IsBillboard)
        CGraphics::sMVPBlock.ModelMatrix = mBillboardTransform;
    else if ((mMode == EGizmoMode::Scale) && ((mSelectedAxes & pPart[ComponentIndex].ModelAxes) != 0))
        CGraphics::sMVPBlock.ModelMatrix = mScaledTransform;
    else
        CGraphics::sMVPBlock.ModelMatrix = mTransform;

    CGraphics::UpdateMVPBlock();

    // Clear tint color
    CGraphics::sPixelBlock.TintColor = CColor::White();
    CGraphics::UpdatePixelBlock();

    // Choose material set
    FAxes PartAxes = pPart[ComponentIndex].ModelAxes;
    bool IsHighlighted = (PartAxes != EAxis::None) && ((mSelectedAxes & PartAxes) == pPart[ComponentIndex].ModelAxes);
    uint32 SetID = (IsHighlighted ? 1 : 0);

    // Draw model
    pPart[ComponentIndex].pModel->Draw((FRenderOptions) 0, SetID);
}

void CGizmo::IncrementSize()
{
    static const float skIncAmount = 1.3f;
    static const float skMaxSize = powf(skIncAmount, 4);

    mGizmoSize *= skIncAmount;
    if (mGizmoSize > skMaxSize)
        mGizmoSize = skMaxSize;
}

void CGizmo::DecrementSize()
{
    static const float skDecAmount = (1.f / 1.3f);
    static const float skMinSize = powf(skDecAmount, 4);

    mGizmoSize *= skDecAmount;
    if (mGizmoSize < skMinSize)
        mGizmoSize = skMinSize;
}

void CGizmo::UpdateForCamera(const CCamera& rkCamera)
{
    CVector3f CamPos = rkCamera.Position();
    CVector3f CameraToGizmo = (mPosition - CamPos).Normalized();
    mFlipScaleX = (mRotation.XAxis().Dot(CameraToGizmo) >= 0.f);
    mFlipScaleY = (mRotation.YAxis().Dot(CameraToGizmo) >= 0.f);
    mFlipScaleZ = (mRotation.ZAxis().Dot(CameraToGizmo) >= 0.f);

    if (!mIsTransforming || mMode != EGizmoMode::Translate)
        mCameraDist = mPosition.Distance(CamPos);

    // todo: make this cleaner...
    CVector3f BillDir = (CamPos - mPosition).Normalized();
    CVector3f Axis = CVector3f::Forward().Cross(BillDir);
    float Angle = acosf(CVector3f::Forward().Dot(BillDir));
    mBillboardRotation = CQuaternion::FromAxisAngle(Angle, Axis);
}

bool CGizmo::CheckSelectedAxes(const CRay& rkRay)
{
    CRay LocalRay = rkRay.Transformed(mTransform.Inverse());
    CRay BillRay = rkRay.Transformed(mBillboardTransform.Inverse());

    // Do raycast on each model
    SModelPart *pPart = mpCurrentParts;

    struct SResult {
        SModelPart *pPart;
        float Dist;
    };
    std::list<SResult> Results;

    for (uint32 iPart = 0; iPart < mNumCurrentParts; iPart++)
    {
        if (!pPart->EnableRayCast)
        {
            pPart++;
            continue;
        }

        CModel *pModel = pPart->pModel;
        CRay& rPartRay = (pPart->IsBillboard ? BillRay : LocalRay);

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
            {
                SResult Result;
                Result.pPart = pPart;
                Result.Dist = Dist;
                Results.push_back(Result);
            }
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
    Results.sort([](const SResult& rkLeft, SResult& rkRight) {
        return rkLeft.Dist < rkRight.Dist;
    });

    CRay& rPartRay = (pPart->IsBillboard ? BillRay : LocalRay);
    mSelectedAxes = Results.front().pPart->ModelAxes;
    mHitPoint = mTransform * rPartRay.PointOnRay(Results.front().Dist);

    return mSelectedAxes != EAxis::None;
}

uint32 CGizmo::NumSelectedAxes() const
{
    uint32 Out = 0;

    for (uint32 iAxis = 1; iAxis < 8; iAxis <<= 1)
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

bool CGizmo::TransformFromInput(const CRay& rkRay, CCamera& rCamera)
{
    // Wrap cursor (this has no effect until the next time this function is called)
    if (mEnableCursorWrap && (mMode != EGizmoMode::Translate))
        WrapCursor();

    // Calculate normalized cursor position
    QPoint CursorPos = QCursor::pos();
    QRect Geom = QApplication::primaryScreen()->geometry();
    CVector2f MouseCoords(
                (((2.f * CursorPos.x()) / Geom.width()) - 1.f),
                (1.f - ((2.f * CursorPos.y()) / Geom.height()))
            );

    // Translate
    if (mMode == EGizmoMode::Translate)
    {
        // Create translate plane
        CVector3f AxisA, AxisB;
        uint32 NumAxes = NumSelectedAxes();

        if (NumAxes == 1)
        {
            if (mSelectedAxes & EAxis::X)
                AxisB = mRotation.XAxis();
            else if (mSelectedAxes & EAxis::Y)
                AxisB = mRotation.YAxis();
            else
                AxisB = mRotation.ZAxis();

            CVector3f GizmoToCamera = (mPosition - rCamera.Position()).Normalized();
            AxisA = AxisB.Cross(GizmoToCamera);
        }
        else if (NumAxes == 2)
        {
            AxisA = mSelectedAxes & EAxis::X ? mRotation.XAxis() : mRotation.YAxis();
            AxisB = mSelectedAxes & EAxis::Z ? mRotation.ZAxis() : mRotation.YAxis();
        }

        CVector3f PlaneNormal = AxisA.Cross(AxisB);
        mTranslatePlane.Redefine(PlaneNormal, mPosition);

        // Do translate
        std::pair<bool,float> Result = Math::RayPlaneIntersection(rkRay, mTranslatePlane);

        if (Result.first)
        {
            CVector3f Hit = rkRay.PointOnRay(Result.second);
            CVector3f LocalDelta = mRotation.Inverse() * (Hit - mPosition);

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
            CVector3f NewPosToCamera = (NewPos - rCamera.Position()).Normalized();
            float Dot = Math::Abs(PlaneNormal.Dot(NewPosToCamera));
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
        CMatrix4f VP = rCamera.ViewMatrix().Transpose() * rCamera.ProjectionMatrix().Transpose();
        CVector2f LineOrigin = (mHitPoint * VP).XY();
        CVector2f LineDir = (((mHitPoint + mMoveDir) * VP).XY() - LineOrigin).Normalized();
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
        CQuaternion OldRot = mCurrentRotation;
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
        CMatrix4f VP = rCamera.ViewMatrix().Transpose() * rCamera.ProjectionMatrix().Transpose();
        CVector2f LineOrigin = (mPosition * VP).XY();

        // Next step: determine the appropriate world space direction using the selected axes and then convert to screen space
        // Since the axes can be flipped while the gizmo is transforming, this has to be done every frame rather than
        // pre-saving the world space direction like the rotate gizmo does.
        CVector3f DirX = (mFlipScaleX ? -mRotation.XAxis() : mRotation.XAxis());
        CVector3f DirY = (mFlipScaleY ? -mRotation.YAxis() : mRotation.YAxis());
        CVector3f DirZ = (mFlipScaleZ ? -mRotation.ZAxis() : mRotation.ZAxis());
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
            CVector3f AxisA = (mSelectedAxes & EAxis::X ? DirX : DirY);
            CVector3f AxisB = (mSelectedAxes & EAxis::Z ? DirZ : DirY);
            CVector2f ScreenA = (((mPosition + AxisA) * VP).XY() - LineOrigin).Normalized();
            CVector2f ScreenB = (((mPosition + AxisB) * VP).XY() - LineOrigin).Normalized();
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

        CVector3f OldScale = mTotalScale;

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
        mpCurrentParts = smTranslateModels.data();
        mNumCurrentParts = smTranslateModels.size();
        mDeltaRotation = CQuaternion::Identity();
        mDeltaScale = CVector3f::One();
        break;

    case EGizmoMode::Rotate:
        mpCurrentParts = smRotateModels.data();
        mNumCurrentParts = smRotateModels.size();
        mDeltaTranslation = CVector3f::Zero();
        mDeltaScale = CVector3f::One();
        break;

    case EGizmoMode::Scale:
        mpCurrentParts = smScaleModels.data();
        mNumCurrentParts = smScaleModels.size();
        mDeltaTranslation = CVector3f::Zero();
        mDeltaRotation = CQuaternion::Identity();
        break;
    default: break;
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

// ************ PRIVATE STATIC ************
void CGizmo::LoadModels()
{
    if (!smModelsLoaded)
    {
        debugf("Loading transform gizmo models");

        smTranslateModels[CGIZMO_TRANSLATE_X]        = SModelPart(EAxis::X,  true,  false, gpEditorStore->LoadResource("editor/TranslateX.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_Y]        = SModelPart(EAxis::Y,  true,  false, gpEditorStore->LoadResource("editor/TranslateY.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_Z]        = SModelPart(EAxis::Z,  true,  false, gpEditorStore->LoadResource("editor/TranslateZ.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_LINES_XY] = SModelPart(EAxis::XY, true,  false, gpEditorStore->LoadResource("editor/TranslateLinesXY.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_LINES_XZ] = SModelPart(EAxis::XZ, true,  false, gpEditorStore->LoadResource("editor/TranslateLinesXZ.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_LINES_YZ] = SModelPart(EAxis::YZ, true,  false, gpEditorStore->LoadResource("editor/TranslateLinesYZ.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_POLY_XY]  = SModelPart(EAxis::XY, false, false, gpEditorStore->LoadResource("editor/TranslatePolyXY.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_POLY_XZ]  = SModelPart(EAxis::XZ, false, false, gpEditorStore->LoadResource("editor/TranslatePolyXZ.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_POLY_YZ]  = SModelPart(EAxis::YZ, false, false, gpEditorStore->LoadResource("editor/TranslatePolyYZ.CMDL"));

        smRotateModels[CGIZMO_ROTATE_OUTLINE] = SModelPart(EAxis::None, true,  true,  gpEditorStore->LoadResource("editor/RotateClipOutline.CMDL"));
        smRotateModels[CGIZMO_ROTATE_X]       = SModelPart(EAxis::X,    true,  false, gpEditorStore->LoadResource("editor/RotateX.CMDL"));
        smRotateModels[CGIZMO_ROTATE_Y]       = SModelPart(EAxis::Y,    true,  false, gpEditorStore->LoadResource("editor/RotateY.CMDL"));
        smRotateModels[CGIZMO_ROTATE_Z]       = SModelPart(EAxis::Z,    true,  false, gpEditorStore->LoadResource("editor/RotateZ.CMDL"));
        smRotateModels[CGIZMO_ROTATE_XYZ]     = SModelPart(EAxis::XYZ,  false, false, gpEditorStore->LoadResource("editor/RotateXYZ.CMDL"));

        smScaleModels[CGIZMO_SCALE_X]         = SModelPart(EAxis::X,   true,  false, gpEditorStore->LoadResource("editor/ScaleX.CMDL"));
        smScaleModels[CGIZMO_SCALE_Y]         = SModelPart(EAxis::Y,   true,  false, gpEditorStore->LoadResource("editor/ScaleY.CMDL"));
        smScaleModels[CGIZMO_SCALE_Z]         = SModelPart(EAxis::Z,   true,  false, gpEditorStore->LoadResource("editor/ScaleZ.CMDL"));
        smScaleModels[CGIZMO_SCALE_LINES_XY]  = SModelPart(EAxis::XY,  true,  false, gpEditorStore->LoadResource("editor/ScaleLinesXY.CMDL"));
        smScaleModels[CGIZMO_SCALE_LINES_XZ]  = SModelPart(EAxis::XZ,  true,  false, gpEditorStore->LoadResource("editor/ScaleLinesXZ.CMDL"));
        smScaleModels[CGIZMO_SCALE_LINES_YZ]  = SModelPart(EAxis::YZ,  true,  false, gpEditorStore->LoadResource("editor/ScaleLinesYZ.CMDL"));
        smScaleModels[CGIZMO_SCALE_POLY_XY]   = SModelPart(EAxis::XY,  true,  false, gpEditorStore->LoadResource("editor/ScalePolyXY.CMDL"));
        smScaleModels[CGIZMO_SCALE_POLY_XZ]   = SModelPart(EAxis::XZ,  true,  false, gpEditorStore->LoadResource("editor/ScalePolyXZ.CMDL"));
        smScaleModels[CGIZMO_SCALE_POLY_YZ]   = SModelPart(EAxis::YZ,  true,  false, gpEditorStore->LoadResource("editor/ScalePolyYZ.CMDL"));
        smScaleModels[CGIZMO_SCALE_XYZ]       = SModelPart(EAxis::XYZ, true,  false, gpEditorStore->LoadResource("editor/ScaleXYZ.CMDL"));

        smModelsLoaded = true;
    }
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
    QRect Geom = QApplication::primaryScreen()->geometry();
    QPoint CursorPos = QCursor::pos();

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
