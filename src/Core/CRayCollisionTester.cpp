#include "CRayCollisionTester.h"
#include "Core/Scene/CSceneNode.h"

CRayCollisionTester::CRayCollisionTester(const CRay& rkRay)
    : mRay(rkRay)
{
}

CRayCollisionTester::~CRayCollisionTester()
{
}

void CRayCollisionTester::AddNode(CSceneNode *pNode, uint32 ComponentIndex, float Distance)
{
    mBoxIntersectList.emplace_back(SRayIntersection());
    SRayIntersection& rIntersection = mBoxIntersectList.back();
    rIntersection.pNode = pNode;
    rIntersection.ComponentIndex = ComponentIndex;
    rIntersection.Distance = Distance;
}

void CRayCollisionTester::AddNodeModel(CSceneNode *pNode, CBasicModel *pModel)
{
    // Check each of the model's surfaces and queue them for further testing if they hit
    for (uint32 iSurf = 0; iSurf < pModel->GetSurfaceCount(); iSurf++)
    {
        std::pair<bool,float> SurfResult = pModel->GetSurfaceAABox(iSurf).Transformed(pNode->Transform()).IntersectsRay(mRay);

        if (SurfResult.first)
            AddNode(pNode, iSurf, SurfResult.second);
    }
}

SRayIntersection CRayCollisionTester::TestNodes(const SViewInfo& rkViewInfo)
{
    // Sort nodes by distance from ray
    mBoxIntersectList.sort(
        [](const SRayIntersection& rkLeft, const SRayIntersection& rkRight) -> bool
    {
        return (rkLeft.Distance < rkRight.Distance);
    });

    // Now do more precise intersection tests on geometry
    SRayIntersection Result;
    Result.Hit = false;

    for (auto iNode = mBoxIntersectList.begin(); iNode != mBoxIntersectList.end(); iNode++)
    {
        SRayIntersection& rIntersection = *iNode;

        // If we have a result, and the distance for the bounding box hit is further than the current result distance
        // then we know that every remaining node is further away and there is no chance of finding a closer hit.
        if ((Result.Hit) && (Result.Distance < rIntersection.Distance))
            break;

        // Otherwise, more intersection tests...
        CSceneNode *pNode = rIntersection.pNode;
        SRayIntersection MidResult = pNode->RayNodeIntersectTest(mRay, rIntersection.ComponentIndex, rkViewInfo);

        if (MidResult.Hit)
        {
            if ((!Result.Hit) || (MidResult.Distance <= Result.Distance))
                Result = MidResult;
        }
    }

    if (Result.Hit) Result.HitPoint = mRay.PointOnRay(Result.Distance);
    return Result;
}
