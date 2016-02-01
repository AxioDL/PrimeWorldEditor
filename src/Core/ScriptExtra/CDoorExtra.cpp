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

void CDoorExtra::AddToRenderer(CRenderer *pRenderer, const SViewInfo& ViewInfo)
{
    if (!mpShieldModel) return;
    if (ViewInfo.GameMode && !mpInstance->IsActive()) return;
    if (!ViewInfo.GameMode && ((ViewInfo.ShowFlags & eShowObjectGeometry) == 0)) return;

    if (mpParent->IsVisible() && ViewInfo.ViewFrustum.BoxInFrustum(AABox()))
    {
        if (mpShieldModel->HasTransparency(0))
            AddSurfacesToRenderer(pRenderer, mpShieldModel, 0, ViewInfo);
        else
            pRenderer->AddOpaqueMesh(this, -1, AABox(), eDrawMesh);

        if (mpParent->IsSelected() && !ViewInfo.GameMode)
            pRenderer->AddOpaqueMesh(this, -1, AABox(), eDrawSelection);
    }
}

void CDoorExtra::Draw(FRenderOptions Options, int ComponentIndex, const SViewInfo& ViewInfo)
{
    LoadModelMatrix();
    mpParent->LoadLights(ViewInfo);

    CGraphics::SetupAmbientColor();
    CGraphics::UpdateVertexBlock();

    CColor Tint = mpParent->TintColor(ViewInfo) * mShieldColor;

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

void CDoorExtra::RayAABoxIntersectTest(CRayCollisionTester& Tester, const SViewInfo& ViewInfo)
{
    if (!mpShieldModel) return;
    if (ViewInfo.GameMode && !mpInstance->IsActive()) return;

    const CRay& Ray = Tester.Ray();
    std::pair<bool,float> BoxResult = AABox().IntersectsRay(Ray);

    if (BoxResult.first)
        Tester.AddNodeModel(this, mpShieldModel);
}

SRayIntersection CDoorExtra::RayNodeIntersectTest(const CRay &Ray, u32 AssetID, const SViewInfo& ViewInfo)
{
    FRenderOptions Options = ViewInfo.pRenderer->RenderOptions();

    SRayIntersection out;
    out.pNode = mpParent;
    out.ComponentIndex = AssetID;

    CRay TransformedRay = Ray.Transformed(Transform().Inverse());
    std::pair<bool,float> Result = mpShieldModel->GetSurface(AssetID)->IntersectsRay(TransformedRay, ((Options & eEnableBackfaceCull) == 0));

    if (Result.first)
    {
        out.Hit = true;
        CVector3f HitPoint = TransformedRay.PointOnRay(Result.second);
        CVector3f WorldHitPoint = Transform() * HitPoint;
        out.Distance = Ray.Origin().Distance(WorldHitPoint);
    }

    else out.Hit = false;

    return out;
}
