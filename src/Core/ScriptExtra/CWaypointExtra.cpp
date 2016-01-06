#include "CWaypointExtra.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/Render/CRenderer.h"
#include "Core/Scene/CScene.h"

CWaypointExtra::CWaypointExtra(CScriptObject *pInstance, CScene *pScene, CSceneNode *pParent)
    : CScriptExtra(pInstance, pScene, pParent)
    , mColor(CColor::skBlack)
    , mLinksBuilt(false)
{
    // Fetch color from parent node's model
    CScriptNode *pScript = static_cast<CScriptNode*>(pParent);
    CModel *pModel = pScript->ActiveModel();

    if (pModel && (pModel->GetMatSetCount() > 0) && (pModel->GetMatCount() > 0))
    {
        CMaterial *pMat = pModel->GetMaterialByIndex(0, 0);
        mColor = pMat->Konst(0);
        mColor.a = 0;
    }
}

void CWaypointExtra::BuildLinks()
{
    mLinks.clear();

    for (u32 iLink = 0; iLink < mpInstance->NumOutLinks(); iLink++)
    {
        const SLink& rkLink = mpInstance->OutLink(iLink);

        if (IsPathLink(rkLink))
        {
            CScriptNode *pNode = mpScene->ScriptNodeByID(rkLink.ObjectID);

            SWaypointLink Link;
            Link.pWaypoint = pNode;
            Link.LineAABB.ExpandBounds(AbsolutePosition());
            Link.LineAABB.ExpandBounds(pNode->AbsolutePosition());
            mLinks.push_back(Link);
        }
    }

    mLinksBuilt = true;
}

bool CWaypointExtra::IsPathLink(const SLink& rkLink)
{
    bool Valid = false;

    if (rkLink.State < 0xFF)
    {
        if (rkLink.State == 0x1 && rkLink.Message == 0x8) Valid = true; // Arrived / Next (MP1)
    }

    else
    {
        CFourCC State(rkLink.State);
        CFourCC Message(rkLink.Message);
        if (State == "ARRV" && Message == "NEXT") Valid = true; // Arrived / Next (MP2)
        if (State == "NEXT" && Message == "ATCH") Valid = true; // Next / Attach (MP3/DKCR)
    }

    if (Valid)
    {
        CScriptNode *pNode = mpScene->ScriptNodeByID(rkLink.ObjectID);

        if (pNode)
            return pNode->Object()->ObjectTypeID() == mpInstance->ObjectTypeID();
    }

    return false;
}

void CWaypointExtra::LinksModified()
{
    BuildLinks();
}

void CWaypointExtra::AddToRenderer(CRenderer *pRenderer, const SViewInfo& ViewInfo)
{
    // This call is necessary because if we try to build links in the constructor, it
    // won't work properly because we haven't finished loading the scene yet.
    if (!mLinksBuilt) BuildLinks();

    if (!ViewInfo.GameMode && mpParent->IsVisible() && !mpParent->IsSelected())
    {
        for (u32 iLink = 0; iLink < mLinks.size(); iLink++)
        {
            CScriptNode *pNode = mLinks[iLink].pWaypoint;

            if (pNode->IsVisible() && !pNode->IsSelected() && ViewInfo.ViewFrustum.BoxInFrustum(mLinks[iLink].LineAABB))
                pRenderer->AddOpaqueMesh(this, iLink, mLinks[iLink].LineAABB, eDrawMesh);
        }
    }
}

void CWaypointExtra::Draw(FRenderOptions /*Options*/, int ComponentIndex, const SViewInfo& /*ViewInfo*/)
{
    glBlendFunc(GL_ONE, GL_ZERO);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);

    CGraphics::sMVPBlock.ModelMatrix = CMatrix4f::skIdentity;
    CGraphics::UpdateMVPBlock();
    CDrawUtil::DrawLine(mpParent->AABox().Center(), mLinks[ComponentIndex].pWaypoint->AABox().Center(), mColor);
}
