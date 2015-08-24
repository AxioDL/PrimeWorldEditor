#include "CGizmo.h"
#include <Common/Math.h>
#include <Core/CRenderer.h>
#include <Core/CResCache.h>
#include <Core/CDrawUtil.h>
#include <Core/Log.h>

#include <QApplication>
#include <QDesktopWidget>
#include <iostream>

CGizmo::CGizmo()
{
    LoadModels();

    SetMode(eTranslate);
    mSelectedAxes = eNone;
    mTransformSpace = eWorldTransform;
    mGizmoSize = 1.f;
    mCameraDist = 0.f;
    mIsTransforming = false;
    mHasTransformed = false;
    mWrapOffset = 0.f;
    mEnableCursorWrap = true;

    mPosition = CVector3f::skZero;
    mRotation = CQuaternion::skIdentity;
    mScale = CVector3f::skOne;
    mDeltaTranslation = CVector3f::skZero;
    mDeltaRotation = CQuaternion::skIdentity;
    mDeltaScale = CVector3f::skOne;
    mSetOffset = false;
    mFlipScaleX = false;
    mFlipScaleY = false;
    mFlipScaleZ = false;
}

CGizmo::~CGizmo()
{
}

void CGizmo::AddToRenderer(CRenderer *pRenderer)
{
    // Transform is updated every frame even if the user doesn't modify the gizmo
    // in order to account for scale changes based on camera distance
    UpdateTransform();
    SModelPart *pPart = mpCurrentParts;

    // Add all parts to renderer
    for (u32 iPart = 0; iPart < mNumCurrentParts; iPart++)
    {
        CModel *pModel = pPart->pModel;

        // Determine whether to use the mat set for regular (0) or highlight (1)
        EGizmoAxes partAxes = pPart->modelAxes;
        bool isHighlighted = (partAxes != eNone) && ((mSelectedAxes & partAxes) == pPart->modelAxes);
        u32 setID = (isHighlighted ? 1 : 0);

        // Add to renderer...
        if (pModel->HasTransparency(setID))
            pRenderer->AddTransparentMesh(this, iPart, pModel->AABox().Transformed(mTransform), eDrawAsset);
        else
            pRenderer->AddOpaqueMesh(this, iPart, pModel->AABox().Transformed(mTransform), eDrawAsset);

        pPart++;
    }
}

void CGizmo::DrawAsset(ERenderOptions options, u32 asset)
{
    // Determine which SModelPart array to use
    if (asset >= mNumCurrentParts) return;
    SModelPart *pPart = mpCurrentParts;

    // Set model matrix
    CGraphics::sMVPBlock.ModelMatrix = (pPart[asset].isBillboard ? mBillboardTransform.ToMatrix4f() : mTransform.ToMatrix4f());
    CGraphics::UpdateMVPBlock();

    // Choose material set
    EGizmoAxes partAxes = pPart[asset].modelAxes;
    bool isHighlighted = (partAxes != eNone) && ((mSelectedAxes & partAxes) == pPart[asset].modelAxes);
    u32 setID = (isHighlighted ? 1 : 0);

    // Draw model
    pPart[asset].pModel->Draw((ERenderOptions) 0, setID);
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

void CGizmo::UpdateForCamera(const CCamera &camera)
{
    CVector3f camPos = camera.Position();
    mFlipScaleX = camPos.x < mPosition.x;
    mFlipScaleY = camPos.y < mPosition.y;
    mFlipScaleZ = camPos.z < mPosition.z;

    if ((!mIsTransforming) || (mMode != eTranslate))
        mCameraDist = mPosition.Distance(camPos);

    // todo: make this cleaner...
    CVector3f billDir = (mPosition - camPos).Normalized();
    CVector3f axis = CVector3f::skForward.Cross(billDir);
    float angle = acosf(CVector3f::skForward.Dot(billDir));
    angle = 180 + (angle * 180 / 3.14159265358979323846f);
    mBillboardRotation = CQuaternion::FromAxisAngle(angle, axis);
}

bool CGizmo::CheckSelectedAxes(const CRay &ray)
{
    // todo: fix raycasting for rotate gizmo; currently it can hit the invisible back side of the model
    CRay localRay = ray.Transformed(mTransform.Inverse());
    CRay billRay = ray.Transformed(mBillboardTransform.Inverse());

    // Do raycast on each model
    SModelPart *pPart = mpCurrentParts;

    struct SResult {
        SModelPart *pPart;
        float dist;
    };
    std::list<SResult> results;

    for (u32 iPart = 0; iPart < mNumCurrentParts; iPart++)
    {
        if (!pPart->enableRayCast)
        {
            pPart++;
            continue;
        }

        CModel *pModel = pPart->pModel;
        CRay& partRay = (pPart->isBillboard ? billRay : localRay);

        // Ray/Model AABox test - allow buffer room because lines are small
        CAABox AABox = pModel->AABox();
        AABox.ExpandBy(CVector3f::skOne);
        bool modelBoxCheck = Math::RayBoxIntersection(partRay, AABox).first;

        if (modelBoxCheck)
        {
            bool hit = false;
            float dist;

            for (u32 iSurf = 0; iSurf < pModel->GetSurfaceCount(); iSurf++)
            {
                // Skip surface/box check - since we use lines the boxes might be too small
                SSurface *pSurf = pModel->GetSurface(iSurf);
                std::pair<bool,float> surfCheck = pSurf->IntersectsRay(partRay, 0.05f);

                if (surfCheck.first)
                {
                    if ((!hit) || (surfCheck.second < dist))
                        dist = surfCheck.second;

                    hit = true;
                }
            }

            if (hit)
            {
                SResult result;
                result.pPart = pPart;
                result.dist = dist;
                results.push_back(result);
            }
        }

        pPart++;
    }

    // Results list empty = no hits
    if (results.empty())
    {
        mSelectedAxes = eNone;
        return false;
    }

    // Otherwise, we have at least one hit - sort results and set selected axes
    results.sort([](const SResult& a, SResult& b) -> bool
            {
                return (a.dist < b.dist);
            });

    CRay& partRay = (pPart->isBillboard ? billRay : localRay);
    mSelectedAxes = results.front().pPart->modelAxes;
    mHitPoint = mTransform * partRay.PointOnRay(results.front().dist);

    return (mSelectedAxes != eNone);
}

u32 CGizmo::NumSelectedAxes()
{
    u32 out = 0;

    for (u32 iAxis = 1; iAxis < 8; iAxis <<= 1)
        if (mSelectedAxes & (EGizmoAxes) iAxis) out++;

    return out;
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

    // Set rotation clockwise direction
    if (mMode == eRotate)
    {
        CVector3f axis;
        if (mSelectedAxes & eX) axis = mRotation.XAxis();
        else if (mSelectedAxes & eY) axis = mRotation.YAxis();
        else axis = mRotation.ZAxis();

        CVector3f gizmoToHit = (mHitPoint - mPosition).Normalized();
        mClockwiseDir = axis.Cross(gizmoToHit);
    }
}

bool CGizmo::TransformFromInput(const CRay& ray, CCamera& camera)
{
    // Wrap cursor (this has no effect until the next time this function is called)
    if (mEnableCursorWrap && (mMode != eTranslate))
        WrapCursor();

    // Calculate normalized cursor position
    QPoint cursorPos = QCursor::pos();
    QRect geom = QApplication::desktop()->screenGeometry();
    CVector2f mouseCoords(
                (((2.f * cursorPos.x()) / geom.width()) - 1.f),
                (1.f - ((2.f * cursorPos.y()) / geom.height()))
            );

    // Translate
    if (mMode == eTranslate)
    {
        // Create translate plane
        CVector3f axisA, axisB;
        u32 numAxes = NumSelectedAxes();

        if (numAxes == 1)
        {
            if (mSelectedAxes & eX) axisB = mRotation.XAxis();
            else if (mSelectedAxes & eY) axisB = mRotation.YAxis();
            else axisB = mRotation.ZAxis();

            CVector3f gizmoToCamera = (mPosition - camera.Position()).Normalized();
            axisA = axisB.Cross(gizmoToCamera);
        }

        else if (numAxes == 2)
        {
            axisA = (mSelectedAxes & eX ? mRotation.XAxis() : mRotation.YAxis());
            axisB = (mSelectedAxes & eZ ? mRotation.ZAxis() : mRotation.YAxis());
        }

        CVector3f planeNormal = axisA.Cross(axisB);
        mTranslatePlane.Redefine(planeNormal, mPosition);

        // Do translate
        std::pair<bool,float> result = Math::RayPlaneIntersecton(ray, mTranslatePlane);

        if (result.first)
        {
            CVector3f hit = ray.PointOnRay(result.second);
            CVector3f localDelta = mRotation.Inverse() * (hit - mPosition);

            // Calculate new position
            CVector3f newPos = mPosition;
            if (mSelectedAxes & eX) newPos += mRotation.XAxis() * localDelta.x;
            if (mSelectedAxes & eY) newPos += mRotation.YAxis() * localDelta.y;
            if (mSelectedAxes & eZ) newPos += mRotation.ZAxis() * localDelta.z;

            // Check relativity of new pos to camera to reduce issue where the gizmo might
            // go flying off into the distance if newPosToCamera is parallel to the plane
            CVector3f newPosToCamera = (newPos - camera.Position()).Normalized();
            float dot = Math::Abs(planeNormal.Dot(newPosToCamera));
            if (dot < 0.02f) return false;

            // Set offset
            if (!mSetOffset)
            {
                mTranslateOffset = mPosition - newPos;
                mDeltaTranslation = CVector3f::skZero;
                mSetOffset = true;
                return false;
            }

            // Apply translation
            else
            {
                mDeltaTranslation = mRotation.Inverse() * (newPos - mPosition + mTranslateOffset);
                if (!(mSelectedAxes & eX)) mDeltaTranslation.x = 0.f;
                if (!(mSelectedAxes & eY)) mDeltaTranslation.y = 0.f;
                if (!(mSelectedAxes & eZ)) mDeltaTranslation.z = 0.f;

                mTotalTranslation += mDeltaTranslation;
                mPosition += mRotation * mDeltaTranslation;

                if (!mHasTransformed && (mDeltaTranslation != CVector3f::skZero))
                    mHasTransformed = true;

                return true;
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
        CVector3f axis;
        if (mSelectedAxes & eX) axis = CVector3f::skUnitX;
        else if (mSelectedAxes & eY) axis = CVector3f::skUnitY;
        else axis = CVector3f::skUnitZ;

        // Convert hit point + clockwise direction into a line in screen space
        // Clockwise direction is set in StartTransform(). Is there a cleaner way to calculate the direction?
        CMatrix4f VP = camera.ViewMatrix().Transpose() * camera.ProjectionMatrix().Transpose();
        CVector2f lineOrigin = (mHitPoint * VP).xy();
        CVector2f lineDir = (((mHitPoint + mClockwiseDir) * VP).xy() - lineOrigin).Normalized();
        float rotAmount = lineDir.Dot(mouseCoords + mWrapOffset - lineOrigin) * 180.f;

        // Set offset
        if (!mSetOffset)
        {
            mRotateOffset = -rotAmount;
            mDeltaRotation = CQuaternion::skIdentity;
            mSetOffset = true;
            return false;
        }

        // Apply rotation
        rotAmount += mRotateOffset;
        CQuaternion oldRot = mCurrentRotation;
        mCurrentRotation = CQuaternion::FromAxisAngle(rotAmount, axis);
        mDeltaRotation = mCurrentRotation * oldRot.Inverse();

        if (mTransformSpace == eLocalTransform)
            mRotation *= mDeltaRotation;

        // Add to total
        if (mSelectedAxes & eX)      mTotalRotation.x = rotAmount;
        else if (mSelectedAxes & eY) mTotalRotation.y = rotAmount;
        else                         mTotalRotation.z = rotAmount;

        if (!mHasTransformed && (rotAmount != 0.f))
            mHasTransformed = true;

        return true;
    }

    return false;
}

void CGizmo::EndTransform()
{
    mIsTransforming = false;
}

bool CGizmo::HasTransformed()
{
    return mHasTransformed;
}

CGizmo::EGizmoMode CGizmo::Mode()
{
    return mMode;
}

CVector3f CGizmo::Position()
{
    return mPosition;
}

CVector3f CGizmo::DeltaTranslation()
{
    return mDeltaTranslation;
}

CVector3f CGizmo::TotalTranslation()
{
    return mTotalTranslation;
}

CQuaternion CGizmo::Rotation()
{
    return mRotation;
}

CQuaternion CGizmo::DeltaRotation()
{
    return mDeltaRotation;
}

CVector3f CGizmo::TotalRotation()
{
    return mTotalRotation;
}

CVector3f CGizmo::Scale()
{
    return mScale;
}

CVector3f CGizmo::DeltaScale()
{
    return mDeltaScale;
}

CVector3f CGizmo::TotalScale()
{
    return mTotalScale;
}

void CGizmo::SetMode(EGizmoMode mode)
{
    mMode = mode;

    switch (mode)
    {
    case eTranslate:
        mpCurrentParts = smTranslateModels;
        mNumCurrentParts = 9;
        mDeltaRotation = CQuaternion::skIdentity;
        mDeltaScale = CVector3f::skOne;
        break;

    case eRotate:
        mpCurrentParts = smRotateModels;
        mNumCurrentParts = 5;
        mDeltaTranslation = CVector3f::skZero;
        mDeltaScale = CVector3f::skOne;
        break;

    case eScale:
        mpCurrentParts = smScaleModels;
        mNumCurrentParts = 10;
        mDeltaTranslation = CVector3f::skZero;
        mDeltaRotation = CQuaternion::skIdentity;
        break;
    }
}

void CGizmo::SetTransformSpace(ETransformSpace space)
{
    mTransformSpace = space;
}

void CGizmo::SetPosition(const CVector3f& position)
{
    mPosition = position;
}

void CGizmo::SetRotation(const CQuaternion& orientation)
{
    mRotation = orientation;
}

void CGizmo::EnableCursorWrap(bool wrap)
{
    mEnableCursorWrap = wrap;
}

// ************ PRIVATE STATIC ************
void CGizmo::LoadModels()
{
    if (!smModelsLoaded)
    {
        Log::Write("Loading transform gizmo models");

        smTranslateModels[CGIZMO_TRANSLATE_X]        = SModelPart(eX,  true,  false, (CModel*) gResCache.GetResource("../resources/editor/TranslateX.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_Y]        = SModelPart(eY,  true,  false, (CModel*) gResCache.GetResource("../resources/editor/TranslateY.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_Z]        = SModelPart(eZ,  true,  false, (CModel*) gResCache.GetResource("../resources/editor/TranslateZ.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_LINES_XY] = SModelPart(eXY, true,  false, (CModel*) gResCache.GetResource("../resources/editor/TranslateLinesXY.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_LINES_XZ] = SModelPart(eXZ, true,  false, (CModel*) gResCache.GetResource("../resources/editor/TranslateLinesXZ.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_LINES_YZ] = SModelPart(eYZ, true,  false, (CModel*) gResCache.GetResource("../resources/editor/TranslateLinesYZ.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_POLY_XY]  = SModelPart(eXY, false, false, (CModel*) gResCache.GetResource("../resources/editor/TranslatePolyXY.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_POLY_XZ]  = SModelPart(eXZ, false, false, (CModel*) gResCache.GetResource("../resources/editor/TranslatePolyXZ.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_POLY_YZ]  = SModelPart(eYZ, false, false, (CModel*) gResCache.GetResource("../resources/editor/TranslatePolyYZ.CMDL"));

        smRotateModels[CGIZMO_ROTATE_OUTLINE] = SModelPart(eNone, true,  true,  (CModel*) gResCache.GetResource("../resources/editor/RotateClipOutline.CMDL"));
        smRotateModels[CGIZMO_ROTATE_X]       = SModelPart(eX,    true,  false, (CModel*) gResCache.GetResource("../resources/editor/RotateX.CMDL"));
        smRotateModels[CGIZMO_ROTATE_Y]       = SModelPart(eY,    true,  false, (CModel*) gResCache.GetResource("../resources/editor/RotateY.CMDL"));
        smRotateModels[CGIZMO_ROTATE_Z]       = SModelPart(eZ,    true,  false, (CModel*) gResCache.GetResource("../resources/editor/RotateZ.CMDL"));
        smRotateModels[CGIZMO_ROTATE_XYZ]     = SModelPart(eXYZ,  false, false, (CModel*) gResCache.GetResource("../resources/editor/RotateXYZ.CMDL"));

        smScaleModels[CGIZMO_SCALE_X]         = SModelPart(eX,   true,  false, (CModel*) gResCache.GetResource("../resources/editor/ScaleX.CMDL"));
        smScaleModels[CGIZMO_SCALE_Y]         = SModelPart(eY,   true,  false, (CModel*) gResCache.GetResource("../resources/editor/ScaleY.CMDL"));
        smScaleModels[CGIZMO_SCALE_Z]         = SModelPart(eZ,   true,  false, (CModel*) gResCache.GetResource("../resources/editor/ScaleZ.CMDL"));
        smScaleModels[CGIZMO_SCALE_LINES_XY]  = SModelPart(eXY,  true,  false, (CModel*) gResCache.GetResource("../resources/editor/ScaleLinesXY.CMDL"));
        smScaleModels[CGIZMO_SCALE_LINES_XZ]  = SModelPart(eXZ,  true,  false, (CModel*) gResCache.GetResource("../resources/editor/ScaleLinesXZ.CMDL"));
        smScaleModels[CGIZMO_SCALE_LINES_YZ]  = SModelPart(eYZ,  true,  false, (CModel*) gResCache.GetResource("../resources/editor/ScaleLinesYZ.CMDL"));
        smScaleModels[CGIZMO_SCALE_POLY_XY]   = SModelPart(eXY,  false, false, (CModel*) gResCache.GetResource("../resources/editor/ScalePolyXY.CMDL"));
        smScaleModels[CGIZMO_SCALE_POLY_XZ]   = SModelPart(eXZ,  false, false, (CModel*) gResCache.GetResource("../resources/editor/ScalePolyXZ.CMDL"));
        smScaleModels[CGIZMO_SCALE_POLY_YZ]   = SModelPart(eYZ,  false, false, (CModel*) gResCache.GetResource("../resources/editor/ScalePolyYZ.CMDL"));
        smScaleModels[CGIZMO_SCALE_XYZ]       = SModelPart(eXYZ, true,  false, (CModel*) gResCache.GetResource("../resources/editor/ScaleXYZ.CMDL"));

        smModelsLoaded = true;
    }
}

// ************ PROTECTED ************
void CGizmo::UpdateTransform()
{
    // Scale is recalculated every frame because it changes frequently, based on camera distance
    // Rotation and position values are just saved directly
    mScale = mGizmoSize * (mCameraDist / 10.f);

    // Scale also factors in total scale + axis flip if mode is Scale.
    if (mMode == eScale)
    {
        mScale *= mTotalScale;

        if (mFlipScaleX) mScale.x = -mScale.x;
        if (mFlipScaleY) mScale.y = -mScale.y;
        if (mFlipScaleZ) mScale.z = -mScale.z;
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
}

void CGizmo::WrapCursor()
{
    QRect geom = QApplication::desktop()->screenGeometry();
    QPoint cursorPos = QCursor::pos();

    // Horizontal
    if (cursorPos.x() == geom.width() - 1)
    {
        QCursor::setPos(1, cursorPos.y());
        mWrapOffset.x += 2.f;
    }
    else if (cursorPos.x() == 0)
    {
        QCursor::setPos(geom.width() - 2, cursorPos.y());
        mWrapOffset.x -= 2.f;
    }

    // Vertical
    cursorPos = QCursor::pos(); // Grab again to account for changes on horizontal wrap

    if (cursorPos.y() == geom.height() - 1)
    {
        QCursor::setPos(cursorPos.x(), 1);
        mWrapOffset.y -= 2.f;
    }
    else if (cursorPos.y() == 0)
    {
        QCursor::setPos(cursorPos.x(), geom.height() - 2);
        mWrapOffset.y += 2.f;
    }
}

// ************ STATIC MEMBER INITIALIZATION ************
bool CGizmo::smModelsLoaded = false;
CGizmo::SModelPart CGizmo::smTranslateModels[9];
CGizmo::SModelPart CGizmo::smRotateModels[5];
CGizmo::SModelPart CGizmo::smScaleModels[10];
