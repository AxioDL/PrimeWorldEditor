#include "CRayCollisionTester.h"
#include <Scene/CSceneNode.h>

CRayCollisionTester::CRayCollisionTester(const CRay& Ray)
    : mRay(Ray)
{
}

CRayCollisionTester::~CRayCollisionTester()
{
}

void CRayCollisionTester::AddNode(CSceneNode *pNode, u32 ComponentIndex, float Distance)
{
    mBoxIntersectList.emplace_back(SRayIntersection());
    SRayIntersection& Intersection = mBoxIntersectList.back();
    Intersection.pNode = pNode;
    Intersection.ComponentIndex = ComponentIndex;
    Intersection.Distance = Distance;
}

void CRayCollisionTester::AddNodeModel(CSceneNode *pNode, CBasicModel *pModel)
{
    // Check each of the model's surfaces and queue them for further testing if they hit
    for (u32 iSurf = 0; iSurf < pModel->GetSurfaceCount(); iSurf++)
    {
        std::pair<bool,float> SurfResult = pModel->GetSurfaceAABox(iSurf).Transformed(pNode->Transform()).IntersectsRay(mRay);

        if (SurfResult.first)
            AddNode(pNode, iSurf, SurfResult.second);
    }
}

SRayIntersection CRayCollisionTester::TestNodes(const SViewInfo& ViewInfo)
{
    // Sort nodes by distance from ray
    mBoxIntersectList.sort(
        [](const SRayIntersection& A, SRayIntersection& B) -> bool
    {
        return (A.Distance < B.Distance);
    });

    // Now do more precise intersection tests on geometry
    SRayIntersection Result;
    Result.Hit = false;

    for (auto iNode = mBoxIntersectList.begin(); iNode != mBoxIntersectList.end(); iNode++)
    {
        SRayIntersection& Intersection = *iNode;

        // If we have a result, and the distance for the bounding box hit is further than the current result distance
        // then we know that every remaining node is further away and there is no chance of finding a closer hit.
        if ((Result.Hit) && (Result.Distance < Intersection.Distance))
            break;

        // Otherwise, more intersection tests...
        CSceneNode *pNode = Intersection.pNode;
        SRayIntersection MidResult = pNode->RayNodeIntersectTest(mRay, Intersection.ComponentIndex, ViewInfo);

        if (MidResult.Hit)
        {
            if ((!Result.Hit) || (MidResult.Distance < Result.Distance))
                Result = MidResult;
        }
    }

    return Result;
}
