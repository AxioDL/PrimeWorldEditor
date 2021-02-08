#include "CWaypointExtra.h"
#include "Core/Resource/Script/CLink.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/Render/CRenderer.h"
#include "Core/Scene/CScene.h"

CWaypointExtra::CWaypointExtra(CScriptObject *pInstance, CScene *pScene, CScriptNode *pParent)
    : CScriptExtra(pInstance, pScene, pParent)
{
    CheckColor();
}

CWaypointExtra::~CWaypointExtra()
{
    for (auto& path : mPaths)
        path->RemoveWaypoint(this);
}

void CWaypointExtra::CheckColor()
{
    // Fetch color from attached SplinePath
    if (!mPaths.empty())
    {
        CSplinePathExtra *pPath = mPaths.front();
        mColor = pPath->PathColor();
    }
    // Fetch color from parent node's model (MP1/2/3)
    else if (mGame < EGame::DKCReturns)
    {
        auto *pScript = static_cast<CScriptNode*>(mpParent);
        CModel *pModel = pScript->ActiveModel();

        if (pModel != nullptr && (pModel->GetMatSetCount() > 0) && (pModel->GetMatCount() > 0))
        {
            const CMaterial *pMat = pModel->GetMaterialByIndex(0, 0);
            mColor = pMat->Konst(0);
        }
    }
    else // Use preset color (DKCR)
    {
        mColor = CColor::Cyan();
    }

    mColor.A = 0;
}

void CWaypointExtra::AddToSplinePath(CSplinePathExtra *pPath)
{
    const auto iter = std::find_if(mPaths.cbegin(), mPaths.cend(),
                                   [pPath](const auto* entry) { return entry == pPath; });

    if (iter != mPaths.cend())
        return;

    mPaths.push_back(pPath);
    if (mPaths.size() == 1)
        CheckColor();
}

void CWaypointExtra::RemoveFromSplinePath(const CSplinePathExtra *pPath)
{
    const auto iter = std::find_if(mPaths.cbegin(), mPaths.cend(),
                                   [pPath](const auto* entry) { return entry == pPath; });

    if (iter == mPaths.cend())
        return;

    mPaths.erase(iter);
    CheckColor();
}

void CWaypointExtra::BuildLinks()
{
    mLinks.clear();

    for (size_t iLink = 0; iLink < mpInstance->NumLinks(ELinkType::Outgoing); iLink++)
    {
        const CLink *pLink = mpInstance->Link(ELinkType::Outgoing, iLink);

        if (IsPathLink(pLink))
        {
            CScriptNode *pNode = mpScene->NodeForInstanceID(pLink->ReceiverID());

            SWaypointLink Link;
            Link.pWaypoint = pNode;
            Link.LineAABB.ExpandBounds(AbsolutePosition());
            Link.LineAABB.ExpandBounds(pNode->AbsolutePosition());
            mLinks.push_back(Link);
        }
    }

    mLinksBuilt = true;
}

bool CWaypointExtra::IsPathLink(const CLink *pLink) const
{
    bool Valid = false;

    if (pLink->State() < 0xFF)
    {
        // Arrived / Next (MP1)
        if (pLink->State() == 0x1 && pLink->Message() == 0x8)
            Valid = true;
    }
    else
    {
        const CFourCC State(pLink->State());
        const CFourCC Message(pLink->Message());

        // Arrived / Next (MP2)
        if (State == FOURCC('ARRV') && Message == FOURCC('NEXT'))
            Valid = true;

        // Next / Attach (MP3/DKCR)
        if (State == FOURCC('NEXT') && Message == FOURCC('ATCH'))
            Valid = true;
    }

    if (Valid)
    {
        const CScriptNode *pNode = mpScene->NodeForInstanceID(pLink->ReceiverID());

        if (pNode != nullptr)
            return pNode->Instance()->ObjectTypeID() == mpInstance->ObjectTypeID();
    }

    return false;
}

void CWaypointExtra::GetLinkedWaypoints(std::list<CWaypointExtra*>& rOut)
{
    if (!mLinksBuilt)
        BuildLinks();

    for (auto& link : mLinks)
    {
        rOut.push_back(static_cast<CWaypointExtra*>(link.pWaypoint->Extra()));
    }
}

void CWaypointExtra::OnTransformed()
{
    for (auto& link : mLinks)
    {
        link.LineAABB = CAABox::Infinite();
        link.LineAABB.ExpandBounds(AbsolutePosition());
        link.LineAABB.ExpandBounds(link.pWaypoint->AbsolutePosition());
    }
}

void CWaypointExtra::LinksModified()
{
    BuildLinks();
}

void CWaypointExtra::AddToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo)
{
    // This call is necessary because if we try to build links in the constructor, it
    // won't work properly because we haven't finished loading the scene yet.
    if (!mLinksBuilt) BuildLinks();

    if (!rkViewInfo.GameMode && ((rkViewInfo.ShowFlags & EShowFlag::ObjectGeometry) != 0) && mpParent->IsVisible() && !mpParent->IsSelected())
    {
        for (uint32 iLink = 0; iLink < mLinks.size(); iLink++)
        {
            CScriptNode *pNode = mLinks[iLink].pWaypoint;

            if (pNode->IsVisible() && !pNode->IsSelected() && rkViewInfo.ViewFrustum.BoxInFrustum(mLinks[iLink].LineAABB))
                pRenderer->AddMesh(this, iLink, mLinks[iLink].LineAABB, false, ERenderCommand::DrawMesh);
        }
    }
}

void CWaypointExtra::Draw(FRenderOptions /*Options*/, int ComponentIndex, ERenderCommand /*Command*/, const SViewInfo& /*rkViewInfo*/)
{
    glBlendFunc(GL_ONE, GL_ZERO);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);

    CGraphics::sMVPBlock.ModelMatrix = CMatrix4f::skIdentity;
    CGraphics::UpdateMVPBlock();
    CDrawUtil::DrawLine(mpParent->AABox().Center(), mLinks[ComponentIndex].pWaypoint->AABox().Center(), mColor);
}

CColor CWaypointExtra::TevColor()
{
    return mGame < EGame::DKCReturns ? CColor::White() : mColor;
}
