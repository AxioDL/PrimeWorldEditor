#ifndef CWAYPOINTEXTRA_H
#define CWAYPOINTEXTRA_H

#include "CScriptExtra.h"
#include "CSplinePathExtra.h"
#include <Common/CColor.h>

class CWaypointExtra : public CScriptExtra
{
    // Draw waypoint paths formed by script connections
    CColor mColor{CColor::Black()};
    bool mLinksBuilt = false;
    std::list<CSplinePathExtra*> mPaths;

    struct SWaypointLink
    {
        CScriptNode *pWaypoint;
        CAABox LineAABB;
    };
    std::vector<SWaypointLink> mLinks;

public:
    explicit CWaypointExtra(CScriptObject *pInstance, CScene *pScene, CScriptNode *pParent = nullptr);
    ~CWaypointExtra() override;
    void CheckColor();
    void AddToSplinePath(CSplinePathExtra *pPath);
    void RemoveFromSplinePath(const CSplinePathExtra *pPath);
    void BuildLinks();
    bool IsPathLink(const CLink *pLink) const;
    void GetLinkedWaypoints(std::list<CWaypointExtra*>& rOut);

    void OnTransformed() override;
    void LinksModified() override;
    void AddToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo) override;
    void Draw(FRenderOptions Options, int ComponentIndex, ERenderCommand Command, const SViewInfo& rkViewInfo) override;
    CColor TevColor() override;
};

#endif // CWAYPOINTEXTRA_H
