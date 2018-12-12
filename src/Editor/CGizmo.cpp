#include "CGizmo.h"
#include <Common/Math/MathUtil.h>
#include <Core/GameProject/CResourceStore.h>
#include <Core/Render/CDrawUtil.h>
#include <Core/Render/CRenderer.h>
#include <Common/Log.h>

#include <iostream>
#include <QApplication>
#include <QDesktopWidget>

CGizmo::CGizmo()
    : mSelectedAxes(eNone)
    , mTransformSpace(eWorldTransform)
    , mGizmoSize(1.f)
    , mCameraDist(0.f)
    , mIsTransforming(false)
    , mHasTransformed(false)
    , mWrapOffset(0.f)
    , mEnableCursorWrap(true)
    , mPosition(CVector3f::skZero)
    , mRotation(CQuaternion::skIdentity)
    , mLocalRotation(CQuaternion::skIdentity)
    , mScale(CVector3f::skOne)
    , mFlipScaleX(false)
    , mFlipScaleY(false)
    , mFlipScaleZ(false)
    , mDeltaTranslation(CVector3f::skZero)
    , mDeltaRotation(CQuaternion::skIdentity)
    , mDeltaScale(CVector3f::skOne)
    , mTotalScale(CVector3f::skOne)
    , mSetOffset(false)
{
    LoadModels();
    SetMode(eTranslate);
}

CGizmo::~CGizmo()
{
}

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
        FGizmoAxes PartAxes = pPart->ModelAxes;
        bool IsHighlighted = (PartAxes != eNone) && ((mSelectedAxes & PartAxes) == pPart->ModelAxes);
        uint32 SetID = (IsHighlighted ? 1 : 0);

        // Add to renderer...
        pRenderer->AddMesh(this, iPart, pModel->AABox().Transformed(mTransform), pModel->HasTransparency(SetID), eDrawMesh, eForeground);
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
    else if ((mMode == eScale) && ((mSelectedAxes & pPart[ComponentIndex].ModelAxes) != 0))
        CGraphics::sMVPBlock.ModelMatrix = mScaledTransform;
    else
        CGraphics::sMVPBlock.ModelMatrix = mTransform;

    CGraphics::UpdateMVPBlock();

    // Clear tint color
    CGraphics::sPixelBlock.TintColor = CColor::skWhite;
    CGraphics::UpdatePixelBlock();

    // Choose material set
    FGizmoAxes PartAxes = pPart[ComponentIndex].ModelAxes;
    bool IsHighlighted = (PartAxes != eNone) && ((mSelectedAxes & PartAxes) == pPart[ComponentIndex].ModelAxes);
    uint32 SetID = (IsHighlighted ? 1 : 0);

    // Draw model
    pPart[ComponentIndex].pModel->Draw((FRenderOptions) 0, SetID);
}

void CGizmo::IncrementSize()
{
    static const float skIncAmount = 1.3f;
    static const float skMaxSize = powf(skIncAmount, 4);

    mGizmoSize *= skIncAmount;
    if (mGizmoSize > skMaxSize) mGizmoSize = skMaxSize;
}

void CGizmo::DecrementSize()
{
    static const float skDecAmount = (1.f / 1.3f);
    static const float skMinSize = powf(skDecAmount, 4);

    mGizmoSize *= skDecAmount;
    if (mGizmoSize < skMinSize) mGizmoSize = skMinSize;
}

void CGizmo::UpdateForCamera(const CCamera& rkCamera)
{
    CVector3f CamPos = rkCamera.Position();
    CVector3f CameraToGizmo = (mPosition - CamPos).Normalized();
    mFlipScaleX = (mRotation.XAxis().Dot(CameraToGizmo) >= 0.f);
    mFlipScaleY = (mRotation.YAxis().Dot(CameraToGizmo) >= 0.f);
    mFlipScaleZ = (mRotation.ZAxis().Dot(CameraToGizmo) >= 0.f);

    if ((!mIsTransforming) || (mMode != eTranslate))
        mCameraDist = mPosition.Distance(CamPos);

    // todo: make this cleaner...
    CVector3f BillDir = (CamPos - mPosition).Normalized();
    CVector3f Axis = CVector3f::skForward.Cross(BillDir);
    float Angle = acosf(CVector3f::skForward.Dot(BillDir));
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
        AABox.ExpandBy(CVector3f::skOne);
        bool ModelBoxCheck = Math::RayBoxIntersection(rPartRay, AABox).first;

        if (ModelBoxCheck)
        {
            bool Hit = false;
            float Dist;

            for (uint32 iSurf = 0; iSurf < pModel->GetSurfaceCount(); iSurf++)
            {
                // Skip surface/box check - since we use lines the boxes might be too small
                SSurface *pSurf = pModel->GetSurface(iSurf);
                std::pair<bool,float> SurfCheck = pSurf->IntersectsRay(rPartRay, false, 0.05f);

                if (SurfCheck.first)
                {
                    if ((!Hit) || (SurfCheck.second < Dist))
                        Dist = SurfCheck.second;

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
        mSelectedAxes = eNone;
        return false;
    }

    // Otherwise, we have at least one hit - sort results and set selected axes
    Results.sort([](const SResult& rkLeft, SResult& rkRight) -> bool
            {
                return (rkLeft.Dist < rkRight.Dist);
            });

    CRay& rPartRay = (pPart->IsBillboard ? BillRay : LocalRay);
    mSelectedAxes = Results.front().pPart->ModelAxes;
    mHitPoint = mTransform * rPartRay.PointOnRay(Results.front().Dist);

    return (mSelectedAxes != eNone);
}

uint32 CGizmo::NumSelectedAxes()
{
    uint32 Out = 0;

    for (uint32 iAxis = 1; iAxis < 8; iAxis <<= 1)
        if (mSelectedAxes & FGizmoAxes(iAxis)) Out++;

    return Out;
}

void CGizmo::ResetSelectedAxes()
{
    mSelectedAxes = eNone;
}

void CGizmo::StartTransform()
{
    mIsTransforming = true;
    mHasTransformed = false;
    mWrapOffset = CVector2f::skZero;
    mSetOffset = false;
    mTotalTranslation = CVector3f::skZero;
    mTotalRotation = CVector3f::skZero;
    mCurrentRotation = CQuaternion::skIdentity;
    mTotalScale = CVector3f::skOne;

    // Set rotation direction
    if (mMode == eRotate)
    {
        CVector3f Axis;
        if (mSelectedAxes & eX) Axis = mRotation.XAxis();
        else if (mSelectedAxes & eY) Axis = mRotation.YAxis();
        else Axis = mRotation.ZAxis();

        CVector3f GizmoToHit = (mHitPoint - mPosition).Normalized();
        mMoveDir = Axis.Cross(GizmoToHit);
    }

    // Set scale direction
    else if (mMode == eScale)
    {
        // Only need to set scale direction if < 3 axes selected
        if (NumSelectedAxes() != 3)
        {
            // One axis; direction = selected axis
            if (NumSelectedAxes() == 1)
            {
                if (mSelectedAxes & eX)      mMoveDir = mRotation.XAxis();
                else if (mSelectedAxes & eY) mMoveDir = mRotation.YAxis();
                else                         mMoveDir = mRotation.ZAxis();
            }

            // Two axes; interpolate between the two selected axes
            else if (NumSelectedAxes() == 2)
            {
                CVector3f AxisA = (mSelectedAxes & eX ? mRotation.XAxis() : mRotation.YAxis());
                CVector3f AxisB = (mSelectedAxes & eZ ? mRotation.ZAxis() : mRotation.YAxis());
                mMoveDir = (AxisA + AxisB) / 2.f;
            }
        }
    }
}

bool CGizmo::TransformFromInput(const CRay& rkRay, CCamera& rCamera)
{
    // Wrap cursor (this has no effect until the next time this function is called)
    if (mEnableCursorWrap && (mMode != eTranslate))
        WrapCursor();

    // Calculate normalized cursor position
    QPoint CursorPos = QCursor::pos();
    QRect Geom = QApplication::desktop()->screenGeometry();
    CVector2f MouseCoords(
                (((2.f * CursorPos.x()) / Geom.width()) - 1.f),
                (1.f - ((2.f * CursorPos.y()) / Geom.height()))
            );

    // Translate
    if (mMode == eTranslate)
    {
        // Create translate plane
        CVector3f AxisA, AxisB;
        uint32 NumAxes = NumSelectedAxes();

        if (NumAxes == 1)
        {
            if (mSelectedAxes & eX) AxisB = mRotation.XAxis();
            else if (mSelectedAxes & eY) AxisB = mRotation.YAxis();
            else AxisB = mRotation.ZAxis();

            CVector3f GizmoToCamera = (mPosition - rCamera.Position()).Normalized();
            AxisA = AxisB.Cross(GizmoToCamera);
        }

        else if (NumAxes == 2)
        {
            AxisA = (mSelectedAxes & eX ? mRotation.XAxis() : mRotation.YAxis());
            AxisB = (mSelectedAxes & eZ ? mRotation.ZAxis() : mRotation.YAxis());
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
            if (mSelectedAxes & eX) NewPos += mRotation.XAxis() * LocalDelta.X;
            if (mSelectedAxes & eY) NewPos += mRotation.YAxis() * LocalDelta.Y;
            if (mSelectedAxes & eZ) NewPos += mRotation.ZAxis() * LocalDelta.Z;

            // Check relativity of new pos to camera to reduce issue where the gizmo might
            // go flying off into the distance if newPosToCamera is parallel to the plane
            CVector3f NewPosToCamera = (NewPos - rCamera.Position()).Normalized();
            float Dot = Math::Abs(PlaneNormal.Dot(NewPosToCamera));
            if (Dot < 0.02f) return false;

            // Set offset
            if (!mSetOffset)
            {
                mTranslateOffset = mPosition - NewPos;
                mDeltaTranslation = CVector3f::skZero;
                mSetOffset = true;
                return false;
            }

            // Apply translation
            else
            {
                mDeltaTranslation = mRotation.Inverse() * (NewPos - mPosition + mTranslateOffset);
                if (!(mSelectedAxes & eX)) mDeltaTranslation.X = 0.f;
                if (!(mSelectedAxes & eY)) mDeltaTranslation.Y = 0.f;
                if (!(mSelectedAxes & eZ)) mDeltaTranslation.Z = 0.f;

                mTotalTranslation += mDeltaTranslation;
                mPosition += mRotation * mDeltaTranslation;

                if (!mHasTransformed && (mDeltaTranslation != CVector3f::skZero))
                    mHasTransformed = true;

                return mHasTransformed;
            }
        }

        else
        {
            mDeltaTranslation = CVector3f::skZero;
            return false;
        }
    }

    // Rotate
    else if (mMode == eRotate)
    {
        // Choose rotation axis
        CVector3f Axis;
        if (mSelectedAxes & eX) Axis = CVector3f::skUnitX;
        else if (mSelectedAxes & eY) Axis = CVector3f::skUnitY;
        else Axis = CVector3f::skUnitZ;

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
            mDeltaRotation = CQuaternion::skIdentity;
            mSetOffset = true;
            return false;
        }

        // Apply rotation
        RotAmount += mRotateOffset;
        CQuaternion OldRot = mCurrentRotation;
        mCurrentRotation = CQuaternion::FromAxisAngle(Math::DegreesToRadians(RotAmount), Axis);
        mDeltaRotation = mCurrentRotation * OldRot.Inverse();

        if (mTransformSpace == eLocalTransform)
            mRotation *= mDeltaRotation;

        // Add to total
        if (mSelectedAxes & eX)      mTotalRotation.X = RotAmount;
        else if (mSelectedAxes & eY) mTotalRotation.Y = RotAmount;
        else                         mTotalRotation.Z = RotAmount;

        if (!mHasTransformed && (RotAmount != 0.f))
            mHasTransformed = true;

        return mHasTransformed;
    }

    // Scale
    else if (mMode == eScale)
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
            if (mSelectedAxes & eX)      WorldDir = DirX;
            else if (mSelectedAxes & eY) WorldDir = DirY;
            else                         WorldDir = DirZ;
            LineDir = (((mPosition + WorldDir) * VP).XY() - LineOrigin).Normalized();
        }
        // Two axes - take the two selected axes and convert them to world space, then average them for the line direction
        else if (NumSelectedAxes() == 2)
        {
            CVector3f AxisA = (mSelectedAxes & eX ? DirX : DirY);
            CVector3f AxisB = (mSelectedAxes & eZ ? DirZ : DirY);
            CVector2f ScreenA = (((mPosition + AxisA) * VP).XY() - LineOrigin).Normalized();
            CVector2f ScreenB = (((mPosition + AxisB) * VP).XY() - LineOrigin).Normalized();
            LineDir = ((ScreenA + ScreenB) / 2.f).Normalized();
        }
        // Three axes - use straight up
        else LineDir = CVector2f::skUp;

        float ScaleAmount = LineDir.Dot(MouseCoords + mWrapOffset - LineOrigin) * 5.f;

        // Set offset
        if (!mSetOffset)
        {
            mScaleOffset = -ScaleAmount;
            mDeltaScale = CVector3f::skOne;
            mSetOffset = true;
            return false;
        }

        // Apply scale
        ScaleAmount = ScaleAmount + mScaleOffset + 1.f;

        // A multiplier is applied to the scale amount of it's less than 1 to prevent it from going negative
        if (ScaleAmount < 1.f)
            ScaleAmount = 1.f / (-(ScaleAmount - 1.f) + 1.f);

        CVector3f OldScale = mTotalScale;

        mTotalScale = CVector3f::skOne;
        if (mSelectedAxes & eX) mTotalScale.X = ScaleAmount;
        if (mSelectedAxes & eY) mTotalScale.Y = ScaleAmount;
        if (mSelectedAxes & eZ) mTotalScale.Z = ScaleAmount;

        mDeltaScale = mTotalScale / OldScale;

        if (!mHasTransformed && (ScaleAmount != 1.f))
            mHasTransformed = true;

        return mHasTransformed;
    }

    return false;
}

void CGizmo::EndTransform()
{
    mTotalScale = CVector3f::skOne;
    mIsTransforming = false;
}

void CGizmo::SetMode(EGizmoMode Mode)
{
    mMode = Mode;

    switch (Mode)
    {
    case eTranslate:
        mpCurrentParts = smTranslateModels;
        mNumCurrentParts = CGIZMO_TRANSLATE_NUM;
        mDeltaRotation = CQuaternion::skIdentity;
        mDeltaScale = CVector3f::skOne;
        break;

    case eRotate:
        mpCurrentParts = smRotateModels;
        mNumCurrentParts = CGIZMO_ROTATE_NUM;
        mDeltaTranslation = CVector3f::skZero;
        mDeltaScale = CVector3f::skOne;
        break;

    case eScale:
        mpCurrentParts = smScaleModels;
        mNumCurrentParts = CGIZMO_SCALE_NUM;
        mDeltaTranslation = CVector3f::skZero;
        mDeltaRotation = CQuaternion::skIdentity;
        break;
    }
}

void CGizmo::SetTransformSpace(ETransformSpace Space)
{
    mTransformSpace = Space;

    if (Space == eWorldTransform)
        mRotation = CQuaternion::skIdentity;
    else
        mRotation = mLocalRotation;
}

void CGizmo::SetLocalRotation(const CQuaternion& rkOrientation)
{
    mLocalRotation = rkOrientation;

    if (mTransformSpace == eLocalTransform)
        mRotation = rkOrientation;
}

// ************ PRIVATE STATIC ************
void CGizmo::LoadModels()
{
    if (!smModelsLoaded)
    {
        debugf("Loading transform gizmo models");

        smTranslateModels[CGIZMO_TRANSLATE_X]        = SModelPart(eX,  true,  false, gpEditorStore->LoadResource("editor/TranslateX.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_Y]        = SModelPart(eY,  true,  false, gpEditorStore->LoadResource("editor/TranslateY.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_Z]        = SModelPart(eZ,  true,  false, gpEditorStore->LoadResource("editor/TranslateZ.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_LINES_XY] = SModelPart(eXY, true,  false, gpEditorStore->LoadResource("editor/TranslateLinesXY.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_LINES_XZ] = SModelPart(eXZ, true,  false, gpEditorStore->LoadResource("editor/TranslateLinesXZ.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_LINES_YZ] = SModelPart(eYZ, true,  false, gpEditorStore->LoadResource("editor/TranslateLinesYZ.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_POLY_XY]  = SModelPart(eXY, false, false, gpEditorStore->LoadResource("editor/TranslatePolyXY.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_POLY_XZ]  = SModelPart(eXZ, false, false, gpEditorStore->LoadResource("editor/TranslatePolyXZ.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_POLY_YZ]  = SModelPart(eYZ, false, false, gpEditorStore->LoadResource("editor/TranslatePolyYZ.CMDL"));

        smRotateModels[CGIZMO_ROTATE_OUTLINE] = SModelPart(eNone, true,  true,  gpEditorStore->LoadResource("editor/RotateClipOutline.CMDL"));
        smRotateModels[CGIZMO_ROTATE_X]       = SModelPart(eX,    true,  false, gpEditorStore->LoadResource("editor/RotateX.CMDL"));
        smRotateModels[CGIZMO_ROTATE_Y]       = SModelPart(eY,    true,  false, gpEditorStore->LoadResource("editor/RotateY.CMDL"));
        smRotateModels[CGIZMO_ROTATE_Z]       = SModelPart(eZ,    true,  false, gpEditorStore->LoadResource("editor/RotateZ.CMDL"));
        smRotateModels[CGIZMO_ROTATE_XYZ]     = SModelPart(eXYZ,  false, false, gpEditorStore->LoadResource("editor/RotateXYZ.CMDL"));

        smScaleModels[CGIZMO_SCALE_X]         = SModelPart(eX,   true,  false, gpEditorStore->LoadResource("editor/ScaleX.CMDL"));
        smScaleModels[CGIZMO_SCALE_Y]         = SModelPart(eY,   true,  false, gpEditorStore->LoadResource("editor/ScaleY.CMDL"));
        smScaleModels[CGIZMO_SCALE_Z]         = SModelPart(eZ,   true,  false, gpEditorStore->LoadResource("editor/ScaleZ.CMDL"));
        smScaleModels[CGIZMO_SCALE_LINES_XY]  = SModelPart(eXY,  true,  false, gpEditorStore->LoadResource("editor/ScaleLinesXY.CMDL"));
        smScaleModels[CGIZMO_SCALE_LINES_XZ]  = SModelPart(eXZ,  true,  false, gpEditorStore->LoadResource("editor/ScaleLinesXZ.CMDL"));
        smScaleModels[CGIZMO_SCALE_LINES_YZ]  = SModelPart(eYZ,  true,  false, gpEditorStore->LoadResource("editor/ScaleLinesYZ.CMDL"));
        smScaleModels[CGIZMO_SCALE_POLY_XY]   = SModelPart(eXY,  true,  false, gpEditorStore->LoadResource("editor/ScalePolyXY.CMDL"));
        smScaleModels[CGIZMO_SCALE_POLY_XZ]   = SModelPart(eXZ,  true,  false, gpEditorStore->LoadResource("editor/ScalePolyXZ.CMDL"));
        smScaleModels[CGIZMO_SCALE_POLY_YZ]   = SModelPart(eYZ,  true,  false, gpEditorStore->LoadResource("editor/ScalePolyYZ.CMDL"));
        smScaleModels[CGIZMO_SCALE_XYZ]       = SModelPart(eXYZ, true,  false, gpEditorStore->LoadResource("editor/ScaleXYZ.CMDL"));

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
    if (mMode == eScale)
    {
        if (mFlipScaleX) mScale.X = -mScale.X;
        if (mFlipScaleY) mScale.Y = -mScale.Y;
        if (mFlipScaleZ) mScale.Z = -mScale.Z;
    }

    // Create transform
    mTransform = CTransform4f::skIdentity;
    mTransform.Scale(mScale);
    mTransform.Rotate(mRotation);
    mTransform.Translate(mPosition);

    // Create billboard transform for rotation gizmo
    if (mMode == eRotate)
    {
        mBillboardTransform = CTransform4f::skIdentity;
        mBillboardTransform.Scale(mScale);
        mBillboardTransform.Rotate(mBillboardRotation);
        mBillboardTransform.Translate(mPosition);
    }

    // Create scaled transform for scale gizmo
    else if (mMode == eScale)
    {
        mScaledTransform = CTransform4f::skIdentity;
        mScaledTransform.Scale(mScale * mTotalScale);
        mScaledTransform.Rotate(mRotation);
        mScaledTransform.Translate(mPosition);
    }
}

void CGizmo::WrapCursor()
{
    QRect Geom = QApplication::desktop()->screenGeometry();
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

// ************ STATIC MEMBER INITIALIZATION ************
bool CGizmo::smModelsLoaded = false;
CGizmo::SModelPart CGizmo::smTranslateModels[CGIZMO_TRANSLATE_NUM];
CGizmo::SModelPart CGizmo::smRotateModels[CGIZMO_ROTATE_NUM];
CGizmo::SModelPart CGizmo::smScaleModels[CGIZMO_SCALE_NUM];
