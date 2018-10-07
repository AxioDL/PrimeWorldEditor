#include "CScriptExtra.h"

#include "CWaypointExtra.h"
#include "CDamageableTriggerExtra.h"
#include "CSpacePirateExtra.h"
#include "CPointOfInterestExtra.h"
#include "CDoorExtra.h"
#include "CRadiusSphereExtra.h"
#include "CSplinePathExtra.h"
#include "CSandwormExtra.h"

CScriptExtra* CScriptExtra::CreateExtra(CScriptNode *pNode)
{
    CScriptExtra *pExtra = nullptr;
    CScriptObject *pObj = pNode->Instance();

    if (pObj)
    {
        switch (pObj->ObjectTypeID())
        {
        case 0x02:       // Waypoint (MP1)
        case 0x0D:       // CameraWaypoint (MP1)
        case 0x2C:       // SpiderBallWaypoint (MP1)
        case 0x32:       // DebugCameraWaypoint(MP1)
        case 0x41495750: // "AIWP" AIWaypoint (MP2/MP3/DKCR)
        case 0x42414C57: // "BALW" SpiderBallWaypoint (MP2/MP3)
        case 0x43414D57: // "CAMW" CameraWaypoint (MP2)
        case 0x57415950: // "WAYP" Waypoint (MP2/MP3/DKCR)
            pExtra = new CWaypointExtra(pObj, pNode->Scene(), pNode);
            break;

        case 0x1A:  // DamageableTrigger (MP1)
            pExtra = new CDamageableTriggerExtra(pObj, pNode->Scene(), pNode);
            break;

        case 0x24: // SpacePirate (MP1)
            pExtra = new CSpacePirateExtra(pObj, pNode->Scene(), pNode);
            break;

        case 0x42:       // PointOfInterest (MP1)
        case 0x504F494E: // "POIN" PointOfInterest (MP2/MP3)
            pExtra = new CPointOfInterestExtra(pObj, pNode->Scene(), pNode);
            break;

        case 0x444F4F52: // "DOOR" Door (MP2/MP3)
            pExtra = new CDoorExtra(pObj, pNode->Scene(), pNode);
            break;

        case 0x63:       // Repulsor (MP1)
        case 0x68:       // RadialDamage (MP1)
        case 0x5245504C: // "REPL" Repulsor (MP2/MP3)
        case 0x52414444: // "RADD" RadialDamage (MP2/MP3/DKCR)
            pExtra = new CRadiusSphereExtra(pObj, pNode->Scene(), pNode);
            break;

        case 0x53505041: // "SPPA" SplinePath (DKCR)
        case 0x5043544C: // "PCTL" PathControl (DKCR)
        case 0x434C5043: // "CLPC" ClingPathControl (DKCR)
            if (pNode->Instance()->Area()->Game() == EGame::DKCReturns)
                pExtra = new CSplinePathExtra(pObj, pNode->Scene(), pNode);
            break;

        case 0x574F524D: // "WORM" Sandworm (MP2)
            pExtra = new CSandwormExtra(pObj, pNode->Scene(), pNode);
            break;
        }
    }

    return pExtra;
}
