#include "CDoorExtra.h"
#include "Core/Render/CRenderer.h"

CDoorExtra::CDoorExtra(CScriptObject* pInstance, CScene* pScene, CScriptNode* pParent)
    : CScriptExtra(pInstance, pScene, pParent)
{
    CStructProperty* pProperties = pInstance->Template()->Properties();

    mShieldModelProp = CAssetRef(pInstance->PropertyData(), pProperties->ChildByID(0xB20CC271));
    if (mShieldModelProp.IsValid())
        PropertyModified(mShieldModelProp.Property());

    if (mGame >= EGame::Echoes)
    {
        mShieldColorProp = CColorRef(pInstance->PropertyData(), pProperties->ChildByID(0x47B4E863));
        if (mShieldColorProp.IsValid())
            PropertyModified(mShieldColorProp.Property());
    }

    else
    {
        mDisabledProp = CBoolRef(pInstance->PropertyData(), pProperties->ChildByID(0xDEE730F5));
        if (mDisabledProp.IsValid())
            PropertyModified(mDisabledProp.Property());
    }
}

void CDoorExtra::PropertyModified(IProperty* pProperty)
{
    if (pProperty == mShieldModelProp)
    {
        mpShieldModel = gpResourceStore->LoadResource<CModel>( mShieldModelProp.Get() );

        if (mpShieldModel)
            mLocalAABox = mpShieldModel->AABox();
        else
            mLocalAABox = CAABox::Infinite();

        MarkTransformChanged();
    }
    else if (pProperty == mShieldColorProp)
    {
        mShieldColor = mShieldColorProp.Get();
    }
    else if (pProperty == mDisabledProp)
    {
        // The Echoes demo doesn't have the shield color property. The color is
        // always cyan if the door is unlocked and always white if the door is locked.
        mShieldColor = CColor::White();

        if (!mDisabledProp)
            mShieldColor = CColor::Cyan();
    }
}

void CDoorExtra::AddToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo)
{
    if (!mpShieldModel)
        return;

    if (rkViewInfo.GameMode && !mpInstance->IsActive())
        return;

    if (!rkViewInfo.GameMode && ((rkViewInfo.ShowFlags & EShowFlag::ObjectGeometry) == 0))
        return;

    if (mpParent->IsVisible() && rkViewInfo.ViewFrustum.BoxInFrustum(AABox()))
    {
        AddModelToRenderer(pRenderer, mpShieldModel, 0);

        if (mpParent->IsSelected() && !rkViewInfo.GameMode)
            pRenderer->AddMesh(this, -1, AABox(), false, ERenderCommand::DrawSelection);
    }
}

void CDoorExtra::Draw(FRenderOptions Options, int /*ComponentIndex*/, ERenderCommand Command, const SViewInfo& rkViewInfo)
{
    LoadModelMatrix();
    mpParent->LoadLights(rkViewInfo);

    CGraphics::SetupAmbientColor();
    CGraphics::UpdateVertexBlock();

    const CColor Tint = mpParent->TintColor(rkViewInfo) * mShieldColor;

    CGraphics::sPixelBlock.TintColor = Tint;
    CGraphics::sPixelBlock.SetAllTevColors(CColor::White());
    CGraphics::UpdatePixelBlock();
    DrawModelParts(mpShieldModel, Options, 0, Command);
}

void CDoorExtra::DrawSelection()
{
    LoadModelMatrix();
    glBlendFunc(GL_ONE, GL_ZERO);
    mpShieldModel->DrawWireframe(ERenderOption::None, mpParent->WireframeColor());
}

void CDoorExtra::RayAABoxIntersectTest(CRayCollisionTester& rTester, const SViewInfo& rkViewInfo)
{
    if (!mpShieldModel)
        return;

    if (rkViewInfo.GameMode && !mpInstance->IsActive())
        return;

    const CRay& Ray = rTester.Ray();
    const std::pair<bool, float> BoxResult = AABox().IntersectsRay(Ray);

    if (BoxResult.first)
        rTester.AddNodeModel(this, mpShieldModel);
}

SRayIntersection CDoorExtra::RayNodeIntersectTest(const CRay& rkRay, uint32 AssetID, const SViewInfo& rkViewInfo)
{
    const FRenderOptions Options = rkViewInfo.pRenderer->RenderOptions();

    SRayIntersection Out;
    Out.pNode = mpParent;
    Out.ComponentIndex = AssetID;

    const CRay TransformedRay = rkRay.Transformed(Transform().Inverse());
    const auto [intersects, distance] = mpShieldModel->GetSurface(AssetID)->IntersectsRay(TransformedRay, ((Options & ERenderOption::EnableBackfaceCull) == 0));

    if (intersects)
    {
        Out.Hit = true;
        const CVector3f HitPoint = TransformedRay.PointOnRay(distance);
        const CVector3f WorldHitPoint = Transform() * HitPoint;
        Out.Distance = rkRay.Origin().Distance(WorldHitPoint);
    }
    else
    {
        Out.Hit = false;
    }

    return Out;
}
