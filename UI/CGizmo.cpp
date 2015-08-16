#include "CGizmo.h"
#include <Core/CRenderer.h>
#include <Core/CResCache.h>

CGizmo::CGizmo()
{
    LoadModels();

    mMode = eRotate;
    mSelectedAxes = eNone;
    mGizmoSize = 1.f;
    mCameraDist = 0.f;

    mPosition = CVector3f::skZero;
    mRotation = CQuaternion::skIdentity;
    mScale = CVector3f::skOne;
    mDeltaPosition = CVector3f::skZero;
    mDeltaRotation = CQuaternion::skIdentity;
    mDeltaScale = CVector3f::skOne;
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

    // Determine which SModelPart array to use
    SModelPart *pParts;
    u32 numParts;

    if (mMode == eTranslate) {
        pParts = smTranslateModels;
        numParts = 6;
    } else if (mMode == eRotate) {
        pParts = smRotateModels;
        numParts = 4;
    } else if (mMode == eScale) {
        pParts = smScaleModels;
        numParts = 7;
    }

    // Add all parts to renderer
    for (u32 iPart = 0; iPart < numParts; iPart++)
    {
        CModel *pModel = pParts->pModel;

        // Determine whether to use the mat set for regular (0) or highlight (1)
        bool isHighlighted = (mSelectedAxes & pParts->modelAxes) == pParts->modelAxes;
        u32 setID = (isHighlighted ? 1 : 0);

        // Add to renderer...
        if (pModel->HasTransparency(setID))
            pRenderer->AddTransparentMesh(this, iPart, pModel->AABox().Transformed(mTransform), eDrawAsset);
        else
            pRenderer->AddOpaqueMesh(this, iPart, pModel->AABox().Transformed(mTransform), eDrawAsset);

        pParts++;
    }
}

void CGizmo::DrawAsset(ERenderOptions options, u32 asset)
{
    CGraphics::sMVPBlock.ModelMatrix = mTransform.ToMatrix4f();
    CGraphics::UpdateMVPBlock();

    // Determine which SModelPart array to use
    SModelPart *pParts;
    u32 numParts;

    if (mMode == eTranslate) {
        pParts = smTranslateModels;
        numParts = 6;
    } else if (mMode == eRotate) {
        pParts = smRotateModels;
        numParts = 4;
    } else if (mMode == eScale) {
        pParts = smScaleModels;
        numParts = 7;
    }

    if (asset >= numParts) return;

    // Draw model
    bool isHighlighted = (mSelectedAxes & pParts[asset].modelAxes) == pParts[asset].modelAxes;
    u32 setID = (isHighlighted ? 1 : 0);
    pParts[asset].pModel->Draw((ERenderOptions) 0, setID);
}

void CGizmo::DrawRotationOutline()
{
    CGraphics::sMVPBlock.ModelMatrix = mBillboardTransform.ToMatrix4f();
    CGraphics::UpdateMVPBlock();
    smRotateClipOutline.pModel->Draw((ERenderOptions) 0, 0);
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
    mCameraDist = mPosition.Distance(camPos);
    mFlipScaleX = camPos.x < mPosition.x;
    mFlipScaleY = camPos.y < mPosition.y;
    mFlipScaleZ = camPos.z < mPosition.z;

    // todo: make this cleaner...
    CVector3f billDir = (mPosition - camPos).Normalized();
    CVector3f axis = CVector3f::skForward.Cross(billDir);
    float angle = acos(CVector3f::skForward.Dot(billDir));
    angle = 180 + (angle * 180 / 3.14159265358979323846f);
    mBillboardRotation = CQuaternion::FromAxisAngle(angle, axis);
}

CGizmo::EGizmoMode CGizmo::Mode()
{
    return mMode;
}

void CGizmo::SetMode(EGizmoMode mode)
{
    mMode = mode;
}

void CGizmo::SetPosition(const CVector3f& position)
{
    mPosition = position;
}

// ************ PRIVATE STATIC ************
void CGizmo::LoadModels()
{
    if (!smModelsLoaded)
    {
        smTranslateModels[CGIZMO_TRANSLATE_X]  = SModelPart(eX,  (CModel*) gResCache.GetResource("../resources/editor/TranslateGizmoX.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_Y]  = SModelPart(eY,  (CModel*) gResCache.GetResource("../resources/editor/TranslateGizmoY.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_Z]  = SModelPart(eZ,  (CModel*) gResCache.GetResource("../resources/editor/TranslateGizmoZ.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_XY] = SModelPart(eXY, (CModel*) gResCache.GetResource("../resources/editor/TranslateGizmoXY.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_XZ] = SModelPart(eXZ, (CModel*) gResCache.GetResource("../resources/editor/TranslateGizmoXZ.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_YZ] = SModelPart(eYZ, (CModel*) gResCache.GetResource("../resources/editor/TranslateGizmoYZ.CMDL"));

        smRotateModels[CGIZMO_ROTATE_X]       = SModelPart(eX,    (CModel*) gResCache.GetResource("../resources/editor/RotateGizmoX.CMDL"));
        smRotateModels[CGIZMO_ROTATE_Y]       = SModelPart(eY,    (CModel*) gResCache.GetResource("../resources/editor/RotateGizmoY.CMDL"));
        smRotateModels[CGIZMO_ROTATE_Z]       = SModelPart(eZ,    (CModel*) gResCache.GetResource("../resources/editor/RotateGizmoZ.CMDL"));
        smRotateModels[CGIZMO_ROTATE_XYZ]     = SModelPart(eXYZ,  (CModel*) gResCache.GetResource("../resources/editor/RotateGizmoXYZ.CMDL"));
        smRotateClipOutline                   = SModelPart(eNone, (CModel*) gResCache.GetResource("../resources/editor/RotateGizmoClipOutline.CMDL"));

        smScaleModels[CGIZMO_SCALE_X]   = SModelPart(eX,   (CModel*) gResCache.GetResource("../resources/editor/ScaleGizmoX.CMDL"));
        smScaleModels[CGIZMO_SCALE_Y]   = SModelPart(eY,   (CModel*) gResCache.GetResource("../resources/editor/ScaleGizmoY.CMDL"));
        smScaleModels[CGIZMO_SCALE_Z]   = SModelPart(eZ,   (CModel*) gResCache.GetResource("../resources/editor/ScaleGizmoZ.CMDL"));
        smScaleModels[CGIZMO_SCALE_XY]  = SModelPart(eXY,  (CModel*) gResCache.GetResource("../resources/editor/ScaleGizmoXY.CMDL"));
        smScaleModels[CGIZMO_SCALE_XZ]  = SModelPart(eXZ,  (CModel*) gResCache.GetResource("../resources/editor/ScaleGizmoXZ.CMDL"));
        smScaleModels[CGIZMO_SCALE_YZ]  = SModelPart(eYZ,  (CModel*) gResCache.GetResource("../resources/editor/ScaleGizmoYZ.CMDL"));
        smScaleModels[CGIZMO_SCALE_XYZ] = SModelPart(eXYZ, (CModel*) gResCache.GetResource("../resources/editor/ScaleGizmoXYZ.CMDL"));

        smModelsLoaded = true;
    }
}

// ************ PROTECTED ************
void CGizmo::UpdateTransform()
{
    // Scale is recalculated every frame because it changes frequently, based on camera distance
    // Rotation and position values are just saved directly
    mScale = mGizmoSize * (mCameraDist / 10.f);

    // Scale also factors in delta scale + axis flip if mode is Scale.
    if (mMode == eScale)
    {
        mScale *= mDeltaScale;

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

// ************ STATIC MEMBER INITIALIZATION ************
bool CGizmo::smModelsLoaded = false;
CGizmo::SModelPart CGizmo::smTranslateModels[6];
CGizmo::SModelPart CGizmo::smRotateModels[4];
CGizmo::SModelPart CGizmo::smScaleModels[7];
CGizmo::SModelPart CGizmo::smRotateClipOutline;
