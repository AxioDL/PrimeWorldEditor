#ifndef CSPLINEPATHEXTRA_H
#define CSPLINEPATHEXTRA_H

#include "CScriptExtra.h"
#include <Common/CColor.h>
#include <list>
#include <set>

class CWaypointExtra;

class CSplinePathExtra : public CScriptExtra
{
    // Recolor waypoint paths to match the editor color parameter
    CColorRef mPathColor;
    std::list<CWaypointExtra*> mWaypoints;

public:
    explicit CSplinePathExtra(CScriptObject* pInstance, CScene* pScene, CScriptNode* pParent = nullptr);
    ~CSplinePathExtra() override { ClearWaypoints(); }
    CColor PathColor() const { return mPathColor.IsValid() ? mPathColor.Get() : CColor::Black(); }

    void PostLoad() override;
    void PropertyModified(IProperty* pProperty) override;

    void FindAttachedWaypoints(std::set<CWaypointExtra*>& rChecked, CWaypointExtra* pWaypoint);
    void AddWaypoints();
    void RemoveWaypoint(const CWaypointExtra* pWaypoint);
    void ClearWaypoints();
};

#endif // CSPLINEPATHEXTRA_H
