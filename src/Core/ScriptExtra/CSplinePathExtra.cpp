#include "Core/ScriptExtra/CSplinePathExtra.h"

#include "Core/Resource/Script/CLink.h"
#include "Core/Scene/CScene.h"
#include "Core/Scene/CScriptNode.h"
#include "Core/ScriptExtra/CWaypointExtra.h"

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
    if (rChecked.contains(pWaypoint))
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

    for (const auto* link : mpInstance->Links(ELinkType::Outgoing))
    {
        const CFourCC State(link->State());
        const CFourCC Message(link->Message());

        if ((State == CFourCC("IS00") && Message == CFourCC("ATCH")) || // InternalState00/Attach
            (State == CFourCC("MOTP") && Message == CFourCC("ATCH")))   // MotionPath/Attach
        {
            CScriptNode* pNode = mpScene->NodeForInstanceID(link->ReceiverID());

            if (pNode && pNode->Instance()->ObjectTypeID() == CFourCC("WAYP").ToU32()) // Waypoint
            {
                auto* pWaypoint = static_cast<CWaypointExtra*>(pNode->Extra());
                FindAttachedWaypoints(CheckedWaypoints, pWaypoint);
            }
        }
    }
}

void CSplinePathExtra::RemoveWaypoint(const CWaypointExtra* pWaypoint)
{
    const auto iter = std::ranges::find(mWaypoints, pWaypoint);
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
