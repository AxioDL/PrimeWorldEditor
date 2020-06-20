#include "CSplinePathExtra.h"
#include "CWaypointExtra.h"
#include "Core/Resource/Script/CLink.h"
#include "Core/Scene/CScene.h"

#include <algorithm>

CSplinePathExtra::CSplinePathExtra(CScriptObject* pInstance, CScene* pScene, CScriptNode* pParent)
    : CScriptExtra(pInstance, pScene, pParent)
{
    mPathColor = CColorRef(pInstance->PropertyData(), pInstance->Template()->Properties()->ChildByID(0x00DD86E2));
}

void CSplinePathExtra::PropertyModified(IProperty* pProperty)
{
    if (pProperty == mPathColor.Property())
    {
        for (auto* extra : mWaypoints)
            extra->CheckColor();
    }
}

void CSplinePathExtra::PostLoad()
{
    AddWaypoints();
}

void CSplinePathExtra::FindAttachedWaypoints(std::set<CWaypointExtra*>& rChecked, CWaypointExtra* pWaypoint)
{
    if (rChecked.find(pWaypoint) != rChecked.cend())
        return;

    rChecked.insert(pWaypoint);
    pWaypoint->AddToSplinePath(this);
    mWaypoints.push_back(pWaypoint);

    std::list<CWaypointExtra*> Attached;
    pWaypoint->GetLinkedWaypoints(Attached);

    for (auto* extra : Attached)
        FindAttachedWaypoints(rChecked, extra);
}

void CSplinePathExtra::AddWaypoints()
{
    if (mGame != EGame::DKCReturns)
        return;

    std::set<CWaypointExtra*> CheckedWaypoints;

    for (size_t LinkIdx = 0; LinkIdx < mpInstance->NumLinks(ELinkType::Outgoing); LinkIdx++)
    {
        const CLink* pLink = mpInstance->Link(ELinkType::Outgoing, LinkIdx);

        if ((pLink->State() == FOURCC('IS00') && pLink->Message() == FOURCC('ATCH')) || // InternalState00/Attach
            (pLink->State() == FOURCC('MOTP') && pLink->Message() == FOURCC('ATCH')))   // MotionPath/Attach
        {
            CScriptNode* pNode = mpScene->NodeForInstanceID(pLink->ReceiverID());

            if (pNode && pNode->Instance()->ObjectTypeID() == FOURCC('WAYP')) // Waypoint
            {
                CWaypointExtra* pWaypoint = static_cast<CWaypointExtra*>(pNode->Extra());
                FindAttachedWaypoints(CheckedWaypoints, pWaypoint);
            }
        }
    }
}

void CSplinePathExtra::RemoveWaypoint(const CWaypointExtra *pWaypoint)
{
    const auto iter = std::find_if(mWaypoints.cbegin(), mWaypoints.cend(),
                                   [pWaypoint](const auto* entry) { return entry == pWaypoint; });

    if (iter == mWaypoints.cend())
        return;

    mWaypoints.erase(iter);
}

void CSplinePathExtra::ClearWaypoints()
{
    for (auto* extra : mWaypoints)
        extra->RemoveFromSplinePath(this);

    mWaypoints.clear();
}
