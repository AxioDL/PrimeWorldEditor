#include "CSplinePathExtra.h"
#include "CWaypointExtra.h"
#include "Core/Resource/Script/CLink.h"
#include "Core/Scene/CScene.h"

CSplinePathExtra::CSplinePathExtra(CScriptObject* pInstance, CScene* pScene, CScriptNode* pParent)
    : CScriptExtra(pInstance, pScene, pParent)
{
    mPathColor = CColorRef(pInstance->PropertyData(), pInstance->Template()->Properties()->ChildByID(0x00DD86E2));
}

void CSplinePathExtra::PropertyModified(IProperty* pProperty)
{
    if (pProperty == mPathColor.Property())
    {
        for (auto it = mWaypoints.begin(); it != mWaypoints.end(); it++)
            (*it)->CheckColor();
    }
}

void CSplinePathExtra::PostLoad()
{
    AddWaypoints();
}

void CSplinePathExtra::FindAttachedWaypoints(std::set<CWaypointExtra*>& rChecked, CWaypointExtra* pWaypoint)
{
    if (rChecked.find(pWaypoint) != rChecked.end())
        return;

    rChecked.insert(pWaypoint);
    pWaypoint->AddToSplinePath(this);
    mWaypoints.push_back(pWaypoint);

    std::list<CWaypointExtra*> Attached;
    pWaypoint->GetLinkedWaypoints(Attached);

    for (auto it = Attached.begin(); it != Attached.end(); it++)
        FindAttachedWaypoints(rChecked, *it);
}

void CSplinePathExtra::AddWaypoints()
{
    if (mGame != EGame::DKCReturns)
        return;

    std::set<CWaypointExtra*> CheckedWaypoints;

    for (uint32 LinkIdx = 0; LinkIdx < mpInstance->NumLinks(ELinkType::Outgoing); LinkIdx++)
    {
        CLink* pLink = mpInstance->Link(ELinkType::Outgoing, LinkIdx);

        if ( (pLink->State() == FOURCC('IS00') && pLink->Message() == FOURCC('ATCH')) || // InternalState00/Attach
             (pLink->State() == FOURCC('MOTP') && pLink->Message() == FOURCC('ATCH')) )  // MotionPath/Attach
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

void CSplinePathExtra::RemoveWaypoint(CWaypointExtra *pWaypoint)
{
    for (auto it = mWaypoints.begin(); it != mWaypoints.end(); it++)
    {
        if (*it == pWaypoint)
        {
            mWaypoints.erase(it);
            break;
        }
    }
}

void CSplinePathExtra::ClearWaypoints()
{
    for (auto it = mWaypoints.begin(); it != mWaypoints.end(); it++)
        (*it)->RemoveFromSplinePath(this);

    mWaypoints.clear();
}
