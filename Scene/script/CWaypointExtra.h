#ifndef CWAYPOINTEXTRA_H
#define CWAYPOINTEXTRA_H

#include "CScriptExtra.h"
#include <Common/CColor.h>

class CWaypointExtra : public CScriptExtra
{
    // Draw waypoint paths formed by script connections
    CColor mColor;
    bool mLinksBuilt;

    struct SWaypointLink
    {
        CScriptNode *pWaypoint;
        CAABox LineAABB;
    };
    std::vector<SWaypointLink> mLinks;

public:
    explicit CWaypointExtra(CScriptObject *pInstance, CSceneManager *pScene, CSceneNode *pParent = 0);
    void BuildLinks();
    bool IsPathLink(const SLink& rkLink);

    void LinksModified();
    void AddToRenderer(CRenderer *pRenderer, const SViewInfo& ViewInfo);
    void Draw(ERenderOptions Options, int ComponentIndex, const SViewInfo& ViewInfo);
};

#endif // CWAYPOINTEXTRA_H
