#include "CDoorExtra.h"
#include "Core/Render/CRenderer.h"

CDoorExtra::CDoorExtra(CScriptObject *pInstance, CScene *pScene, CSceneNode *pParent)
    : CScriptExtra(pInstance, pScene, pParent)
    , mpShieldModelProp(nullptr)
    , mpShieldColorProp(nullptr)
    , mpShieldModel(nullptr)
{
    CPropertyStruct *pBaseStruct = pInstance->Properties();

    mpShieldModelProp = TPropCast<TFileProperty>(pBaseStruct->PropertyByID(0xB20CC271));
    if (mpShieldModelProp) PropertyModified(mpShieldModelProp);

    if (mGame >= eEchoes)
    {
        mpShieldColorProp = TPropCast<TColorProperty>(pBaseStruct->PropertyByID(0x47B4E863));
        if (mpShieldColorProp) PropertyModified(mpShieldColorProp);
    }

    else
    {
        mpDisabledProp = TPropCast<TBoolProperty>(pBaseStruct->PropertyByID(0xDEE730F5));
        if (mpDisabledProp) PropertyModified(mpDisabledProp);
    }
}

void CDoorExtra::PropertyModified(IProperty *pProperty)
{
    if (pProperty == mpShieldModelProp)
    {
        mpShieldModel = mpShieldModelProp->Get().Load();

        if (mpShieldModel)
            mLocalAABox = mpShieldModel->AABox();

        else
            mLocalAABox = CAABox::skInfinite;

        MarkTransformChanged();
    }

    else if (pProperty == mpShieldColorProp)
    {
        mShieldColor = mpShieldColorProp->Get();
    }

    else if (pProperty == mpDisabledProp)
    {
        // The Echoes demo doesn't have the shield color property. The color is
        // always cyan if the door is unlocked and always white if the door is locked.
        mShieldColor = CColor::skWhite;

        if (!mpDisabledProp->Get())
            mShieldColor = CColor::skCyan;
    }
}

void CDoorExtra::AddToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo)
{
    if (!mpShieldModel) return;
    if (rkViewInfo.GameMode && !mpInstance->IsActive()) return;
    if (!rkViewInfo.GameMode && ((rkViewInfo.ShowFlags & eShowObjectGeometry) == 0)) return;

    if (mpParent->IsVisible() && rkViewInfo.ViewFrustum.BoxInFrustum(AABox()))
    {
        if (mpShieldModel->HasTransparency(0))
            AddSurfacesToRenderer(pRenderer, mpShieldModel, 0, rkViewInfo);
        else
            pRenderer->AddOpaqueMesh(this, -1, AABox(), eDrawMesh);

        if (mpParent->IsSelected() && !rkViewInfo.GameMode)
            pRenderer->AddOpaqueMesh(this, -1, AABox(), eDrawSelection);
    }
}

void CDoorExtra::Draw(FRenderOptions Options, int ComponentIndex, const SViewInfo& rkViewInfo)
{
    LoadModelMatrix();
    mpParent->LoadLights(rkViewInfo);

    CGraphics::SetupAmbientColor();
    CGraphics::UpdateVertexBlock();

    CColor Tint = mpParent->TintColor(rkViewInfo) * mShieldColor;

    CGraphics::sPixelBlock.TintColor = Tint;
    CGraphics::sPixelBlock.TevColor = CColor::skWhite;
    CGraphics::UpdatePixelBlock();

    if (ComponentIndex < 0)
        mpShieldModel->Draw(Options, 0);
    else
        mpShieldModel->DrawSurface(Options, ComponentIndex, 0);
}

void CDoorExtra::DrawSelection()
{
    LoadModelMatrix();
    glBlendFunc(GL_ONE, GL_ZERO);
    mpShieldModel->DrawWireframe(eNoRenderOptions, mpParent->WireframeColor());
}

void CDoorExtra::RayAABoxIntersectTest(CRayCollisionTester& rTester, const SViewInfo& rkViewInfo)
{
    if (!mpShieldModel) return;
    if (rkViewInfo.GameMode && !mpInstance->IsActive()) return;

    const CRay& Ray = rTester.Ray();
    std::pair<bool,float> BoxResult = AABox().IntersectsRay(Ray);

    if (BoxResult.first)
        rTester.AddNodeModel(this, mpShieldModel);
}

SRayIntersection CDoorExtra::RayNodeIntersectTest(const CRay& rkRay, u32 AssetID, const SViewInfo& rkViewInfo)
{
    FRenderOptions Options = rkViewInfo.pRenderer->RenderOptions();

    SRayIntersection out;
    out.pNode = mpParent;
    out.ComponentIndex = AssetID;

    CRay TransformedRay = rkRay.Transformed(Transform().Inverse());
    std::pair<bool,float> Result = mpShieldModel->GetSurface(AssetID)->IntersectsRay(TransformedRay, ((Options & eEnableBackfaceCull) == 0));

    if (Result.first)
    {
        out.Hit = true;
        CVector3f HitPoint = TransformedRay.PointOnRay(Result.second);
        CVector3f WorldHitPoint = Transform() * HitPoint;
        out.Distance = rkRay.Origin().Distance(WorldHitPoint);
    }

    else out.Hit = false;

    return out;
}
