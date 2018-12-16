#include "CWaypointExtra.h"
#include "Core/Resource/Script/CLink.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/Render/CRenderer.h"
#include "Core/Scene/CScene.h"

CWaypointExtra::CWaypointExtra(CScriptObject *pInstance, CScene *pScene, CScriptNode *pParent)
    : CScriptExtra(pInstance, pScene, pParent)
    , mColor(CColor::skBlack)
    , mLinksBuilt(false)
{
    CheckColor();
}

CWaypointExtra::~CWaypointExtra()
{
    for (auto it = mPaths.begin(); it != mPaths.end(); it++)
        (*it)->RemoveWaypoint(this);
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
        CScriptNode *pScript = static_cast<CScriptNode*>(mpParent);
        CModel *pModel = pScript->ActiveModel();

        if (pModel && (pModel->GetMatSetCount() > 0) && (pModel->GetMatCount() > 0))
        {
            CMaterial *pMat = pModel->GetMaterialByIndex(0, 0);
            mColor = pMat->Konst(0);
        }
    }

    // Use preset color (DKCR)
    else
    {
        mColor = CColor::skCyan;
    }

    mColor.A = 0;
}

void CWaypointExtra::AddToSplinePath(CSplinePathExtra *pPath)
{
    for (auto it = mPaths.begin(); it != mPaths.end(); it++)
    {
        if (*it == pPath)
            return;
    }

    mPaths.push_back(pPath);
    if (mPaths.size() == 1)
        CheckColor();
}

void CWaypointExtra::RemoveFromSplinePath(CSplinePathExtra *pPath)
{
    for (auto it = mPaths.begin(); it != mPaths.end(); it++)
    {
        if (*it == pPath)
        {
            mPaths.erase(it);
            CheckColor();
            break;
        }
    }
}

void CWaypointExtra::BuildLinks()
{
    mLinks.clear();

    for (uint32 iLink = 0; iLink < mpInstance->NumLinks(ELinkType::Outgoing); iLink++)
    {
        CLink *pLink = mpInstance->Link(ELinkType::Outgoing, iLink);

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

bool CWaypointExtra::IsPathLink(CLink *pLink)
{
    bool Valid = false;

    if (pLink->State() < 0xFF)
    {
        if (pLink->State() == 0x1 && pLink->Message() == 0x8) Valid = true; // Arrived / Next (MP1)
    }

    else
    {
        CFourCC State(pLink->State());
        CFourCC Message(pLink->Message());
        if (State == FOURCC('ARRV') && Message == FOURCC('NEXT')) Valid = true; // Arrived / Next (MP2)
        if (State == FOURCC('NEXT') && Message == FOURCC('ATCH')) Valid = true; // Next / Attach (MP3/DKCR)
    }

    if (Valid)
    {
        CScriptNode *pNode = mpScene->NodeForInstanceID(pLink->ReceiverID());

        if (pNode)
            return pNode->Instance()->ObjectTypeID() == mpInstance->ObjectTypeID();
    }

    return false;
}

void CWaypointExtra::GetLinkedWaypoints(std::list<CWaypointExtra*>& rOut)
{
    if (!mLinksBuilt) BuildLinks();

    for (uint32 iLink = 0; iLink < mLinks.size(); iLink++)
    {
        const SWaypointLink& rkLink = mLinks[iLink];
        CWaypointExtra *pExtra = static_cast<CWaypointExtra*>(rkLink.pWaypoint->Extra());
        rOut.push_back(pExtra);
    }
}

void CWaypointExtra::OnTransformed()
{
    for (uint32 iLink = 0; iLink < mLinks.size(); iLink++)
    {
        SWaypointLink& rLink = mLinks[iLink];
        rLink.LineAABB = CAABox::skInfinite;
        rLink.LineAABB.ExpandBounds(AbsolutePosition());
        rLink.LineAABB.ExpandBounds(rLink.pWaypoint->AbsolutePosition());
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

    if (!rkViewInfo.GameMode && (rkViewInfo.ShowFlags & EShowFlag::ObjectGeometry) && mpParent->IsVisible() && !mpParent->IsSelected())
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
    return (mGame < EGame::DKCReturns ? CColor::skWhite : mColor);
}
