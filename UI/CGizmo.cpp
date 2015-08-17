#include "CGizmo.h"
#include <Common/Math.h>
#include <Core/CRenderer.h>
#include <Core/CResCache.h>

CGizmo::CGizmo()
{
    LoadModels();

    SetMode(eTranslate);
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
    SModelPart *pPart = mpCurrentParts;

    // Add all parts to renderer
    for (u32 iPart = 0; iPart < mNumCurrentParts; iPart++)
    {
        CModel *pModel = pPart->pModel;

        // Determine whether to use the mat set for regular (0) or highlight (1)
        bool isHighlighted = (mSelectedAxes & pPart->modelAxes) == pPart->modelAxes;
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
    CGraphics::sMVPBlock.ModelMatrix = mTransform.ToMatrix4f();
    CGraphics::UpdateMVPBlock();

    // Determine which SModelPart array to use
    if (asset >= mNumCurrentParts) return;
    SModelPart *pPart = mpCurrentParts;

    // Draw model
    bool isHighlighted = (mSelectedAxes & pPart[asset].modelAxes) == pPart[asset].modelAxes;
    u32 setID = (isHighlighted ? 1 : 0);
    pPart[asset].pModel->Draw((ERenderOptions) 0, setID);
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

bool CGizmo::IntersectsRay(const CRay &ray)
{
    // todo: fix raycasting for rotate gizmo; currently it can hit the invisible back side of the model
    CRay localRay = ray.Transformed(mTransform.Inverse());
    float threshold = 0.02f * mGizmoSize * mCameraDist;

    // Do raycast on each model
    SModelPart *pPart = mpCurrentParts;

    struct SResult {
        SModelPart *pPart;
        float dist;
    };
    std::list<SResult> results;

    for (u32 iPart = 0; iPart < mNumCurrentParts; iPart++)
    {
        if (!pPart->enableRayCast) continue;
        CModel *pModel = pPart->pModel;

        // Ray/Model AABox test - allow buffer room because lines are small
        CAABox AABox = pModel->AABox();
        AABox.ExpandBy(CVector3f::skOne);
        bool modelBoxCheck = Math::RayBoxIntersection(localRay, AABox).first;

        if (modelBoxCheck)
        {
            bool hit = false;
            float dist;

            for (u32 iSurf = 0; iSurf < pModel->GetSurfaceCount(); iSurf++)
            {
                // Skip surface/box check
                SSurface *pSurf = pModel->GetSurface(iSurf);
                std::pair<bool,float> surfCheck = pSurf->IntersectsRay(localRay, 0.05f);

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

    mSelectedAxes = results.front().pPart->modelAxes;
    return true;
}

CGizmo::EGizmoMode CGizmo::Mode()
{
    return mMode;
}

void CGizmo::SetMode(EGizmoMode mode)
{
    mMode = mode;

    switch (mode)
    {
    case eTranslate:
        mpCurrentParts = smTranslateModels;
        mNumCurrentParts = 9;
        break;

    case eRotate:
        mpCurrentParts = smRotateModels;
        mNumCurrentParts = 4;
        break;

    case eScale:
        mpCurrentParts = smScaleModels;
        mNumCurrentParts = 10;
        break;
    }
}

void CGizmo::SetPosition(const CVector3f& position)
{
    mPosition = position;
}

void CGizmo::ResetSelectedAxes()
{
    mSelectedAxes = eNone;
}

// ************ PRIVATE STATIC ************
void CGizmo::LoadModels()
{
    if (!smModelsLoaded)
    {
        smTranslateModels[CGIZMO_TRANSLATE_X]        = SModelPart(eX,  true,  (CModel*) gResCache.GetResource("../resources/editor/TranslateX.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_Y]        = SModelPart(eY,  true,  (CModel*) gResCache.GetResource("../resources/editor/TranslateY.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_Z]        = SModelPart(eZ,  true,  (CModel*) gResCache.GetResource("../resources/editor/TranslateZ.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_LINES_XY] = SModelPart(eXY, true,  (CModel*) gResCache.GetResource("../resources/editor/TranslateLinesXY.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_LINES_XZ] = SModelPart(eXZ, true,  (CModel*) gResCache.GetResource("../resources/editor/TranslateLinesXZ.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_LINES_YZ] = SModelPart(eYZ, true,  (CModel*) gResCache.GetResource("../resources/editor/TranslateLinesYZ.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_POLY_XY]  = SModelPart(eXY, false, (CModel*) gResCache.GetResource("../resources/editor/TranslatePolyXY.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_POLY_XZ]  = SModelPart(eXZ, false, (CModel*) gResCache.GetResource("../resources/editor/TranslatePolyXZ.CMDL"));
        smTranslateModels[CGIZMO_TRANSLATE_POLY_YZ]  = SModelPart(eYZ, false, (CModel*) gResCache.GetResource("../resources/editor/TranslatePolyYZ.CMDL"));

        smRotateModels[CGIZMO_ROTATE_X]       = SModelPart(eX,    true,  (CModel*) gResCache.GetResource("../resources/editor/RotateX.CMDL"));
        smRotateModels[CGIZMO_ROTATE_Y]       = SModelPart(eY,    true,  (CModel*) gResCache.GetResource("../resources/editor/RotateY.CMDL"));
        smRotateModels[CGIZMO_ROTATE_Z]       = SModelPart(eZ,    true,  (CModel*) gResCache.GetResource("../resources/editor/RotateZ.CMDL"));
        smRotateModels[CGIZMO_ROTATE_XYZ]     = SModelPart(eXYZ,  false, (CModel*) gResCache.GetResource("../resources/editor/RotateXYZ.CMDL"));
        smRotateClipOutline                   = SModelPart(eNone, false, (CModel*) gResCache.GetResource("../resources/editor/RotateClipOutline.CMDL"));

        smScaleModels[CGIZMO_SCALE_X]         = SModelPart(eX,   true,  (CModel*) gResCache.GetResource("../resources/editor/ScaleX.CMDL"));
        smScaleModels[CGIZMO_SCALE_Y]         = SModelPart(eY,   true,  (CModel*) gResCache.GetResource("../resources/editor/ScaleY.CMDL"));
        smScaleModels[CGIZMO_SCALE_Z]         = SModelPart(eZ,   true,  (CModel*) gResCache.GetResource("../resources/editor/ScaleZ.CMDL"));
        smScaleModels[CGIZMO_SCALE_LINES_XY]  = SModelPart(eXY,  true,  (CModel*) gResCache.GetResource("../resources/editor/ScaleLinesXY.CMDL"));
        smScaleModels[CGIZMO_SCALE_LINES_XZ]  = SModelPart(eXZ,  true,  (CModel*) gResCache.GetResource("../resources/editor/ScaleLinesXZ.CMDL"));
        smScaleModels[CGIZMO_SCALE_LINES_YZ]  = SModelPart(eYZ,  true,  (CModel*) gResCache.GetResource("../resources/editor/ScaleLinesYZ.CMDL"));
        smScaleModels[CGIZMO_SCALE_POLY_XY]   = SModelPart(eXY,  false, (CModel*) gResCache.GetResource("../resources/editor/ScalePolyXY.CMDL"));
        smScaleModels[CGIZMO_SCALE_POLY_XZ]   = SModelPart(eXZ,  false, (CModel*) gResCache.GetResource("../resources/editor/ScalePolyXZ.CMDL"));
        smScaleModels[CGIZMO_SCALE_POLY_YZ]   = SModelPart(eYZ,  false, (CModel*) gResCache.GetResource("../resources/editor/ScalePolyYZ.CMDL"));
        smScaleModels[CGIZMO_SCALE_XYZ]       = SModelPart(eXYZ, true,  (CModel*) gResCache.GetResource("../resources/editor/ScaleXYZ.CMDL"));

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
CGizmo::SModelPart CGizmo::smTranslateModels[9];
CGizmo::SModelPart CGizmo::smRotateModels[4];
CGizmo::SModelPart CGizmo::smScaleModels[10];
CGizmo::SModelPart CGizmo::smRotateClipOutline;
