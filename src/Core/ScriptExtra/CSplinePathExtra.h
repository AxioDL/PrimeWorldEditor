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
    TColorProperty *mpPathColor;
    std::list<CWaypointExtra*> mWaypoints;

public:
    explicit CSplinePathExtra(CScriptObject *pInstance, CScene *pScene, CSceneNode *pParent = 0);
    ~CSplinePathExtra()             { ClearWaypoints(); }
    inline CColor PathColor() const { return (mpPathColor ? mpPathColor->Get() : CColor::skBlack); }

    void PostLoad();
    void PropertyModified(IProperty *pProperty);

    void FindAttachedWaypoints(std::set<CWaypointExtra*>& rChecked, CWaypointExtra *pWaypoint);
    void AddWaypoints();
    void RemoveWaypoint(CWaypointExtra *pWaypoint);
    void ClearWaypoints();
};

#endif // CSPLINEPATHEXTRA_H
