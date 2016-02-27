#include "CSplinePathExtra.h"
#include "CWaypointExtra.h"
#include "Core/Scene/CScene.h"

CSplinePathExtra::CSplinePathExtra(CScriptObject *pInstance, CScene *pScene, CSceneNode *pParent)
    : CScriptExtra(pInstance, pScene, pParent)
{
    mpPathColor = TPropCast<TColorProperty>(pInstance->Properties()->PropertyByID(0x00DD86E2));
}

void CSplinePathExtra::PropertyModified(IProperty *pProperty)
{
    if (pProperty == mpPathColor)
    {
        for (auto it = mWaypoints.begin(); it != mWaypoints.end(); it++)
            (*it)->CheckColor();
    }
}

void CSplinePathExtra::PostLoad()
{
    AddWaypoints();
}

void CSplinePathExtra::FindAttachedWaypoints(std::set<CWaypointExtra*>& rChecked, CWaypointExtra *pWaypoint)
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
    if (mGame != eReturns)
        return;

    std::set<CWaypointExtra*> CheckedWaypoints;

    for (u32 iLink = 0; iLink < mpInstance->NumOutLinks(); iLink++)
    {
        const SLink& rkLink = mpInstance->OutLink(iLink);

        if ( (rkLink.State == 0x49533030 && rkLink.Message == 0x41544348) || // InternalState00/Attach
             (rkLink.State == 0x4D4F5450 && rkLink.Message == 0x41544348) )  // MotionPath/Attach
        {
            CScriptNode *pNode = mpScene->ScriptNodeByID(rkLink.ObjectID);

            if (pNode && pNode->Object()->ObjectTypeID() == 0x57415950) // Waypoint
            {
                CWaypointExtra *pWaypoint = static_cast<CWaypointExtra*>(pNode->Extra());
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
