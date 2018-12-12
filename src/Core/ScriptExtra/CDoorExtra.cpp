#include "CDoorExtra.h"
#include "Core/Render/CRenderer.h"

CDoorExtra::CDoorExtra(CScriptObject* pInstance, CScene* pScene, CScriptNode* pParent)
    : CScriptExtra(pInstance, pScene, pParent)
{
    CStructProperty* pProperties = pInstance->Template()->Properties();

    mShieldModelProp = CAssetRef(pInstance->PropertyData(), pProperties->ChildByID(0xB20CC271));
    if (mShieldModelProp.IsValid()) PropertyModified(mShieldModelProp.Property());

    if (mGame >= EGame::Echoes)
    {
        mShieldColorProp = CColorRef(pInstance->PropertyData(), pProperties->ChildByID(0x47B4E863));
        if (mShieldColorProp.IsValid()) PropertyModified(mShieldColorProp.Property());
    }

    else
    {
        mDisabledProp = CBoolRef(pInstance->PropertyData(), pProperties->ChildByID(0xDEE730F5));
        if (mDisabledProp.IsValid()) PropertyModified(mDisabledProp.Property());
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
            mLocalAABox = CAABox::skInfinite;

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
        mShieldColor = CColor::skWhite;

        if (!mDisabledProp)
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
        AddModelToRenderer(pRenderer, mpShieldModel, 0);

        if (mpParent->IsSelected() && !rkViewInfo.GameMode)
            pRenderer->AddMesh(this, -1, AABox(), false, eDrawSelection);
    }
}

void CDoorExtra::Draw(FRenderOptions Options, int /*ComponentIndex*/, ERenderCommand Command, const SViewInfo& rkViewInfo)
{
    LoadModelMatrix();
    mpParent->LoadLights(rkViewInfo);

    CGraphics::SetupAmbientColor();
    CGraphics::UpdateVertexBlock();

    CColor Tint = mpParent->TintColor(rkViewInfo) * mShieldColor;

    CGraphics::sPixelBlock.TintColor = Tint;
    CGraphics::sPixelBlock.TevColor = CColor::skWhite;
    CGraphics::UpdatePixelBlock();
    DrawModelParts(mpShieldModel, Options, 0, Command);
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

SRayIntersection CDoorExtra::RayNodeIntersectTest(const CRay& rkRay, uint32 AssetID, const SViewInfo& rkViewInfo)
{
    FRenderOptions Options = rkViewInfo.pRenderer->RenderOptions();

    SRayIntersection Out;
    Out.pNode = mpParent;
    Out.ComponentIndex = AssetID;

    CRay TransformedRay = rkRay.Transformed(Transform().Inverse());
    std::pair<bool,float> Result = mpShieldModel->GetSurface(AssetID)->IntersectsRay(TransformedRay, ((Options & eEnableBackfaceCull) == 0));

    if (Result.first)
    {
        Out.Hit = true;
        CVector3f HitPoint = TransformedRay.PointOnRay(Result.second);
        CVector3f WorldHitPoint = Transform() * HitPoint;
        Out.Distance = rkRay.Origin().Distance(WorldHitPoint);
    }

    else Out.Hit = false;

    return Out;
}
