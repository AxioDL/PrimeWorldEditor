#ifndef CRAYCOLLISIONHELPER_H
#define CRAYCOLLISIONHELPER_H

#include "SRayIntersection.h"
#include "Core/Render/SViewInfo.h"
#include "Core/Resource/Model/CBasicModel.h"
#include <Common/BasicTypes.h>
#include <Common/Math/CAABox.h>
#include <Common/Math/CRay.h>
#include <Common/Math/CVector3f.h>

#include <list>

class CSceneNode;

class CRayCollisionTester
{
    CRay mRay;
    std::list<SRayIntersection> mBoxIntersectList;

public:
    CRayCollisionTester(const CRay& rkRay);
    ~CRayCollisionTester();
    const CRay& Ray() const { return mRay; }

    void AddNode(CSceneNode *pNode, uint32 AssetIndex, float Distance);
    void AddNodeModel(CSceneNode *pNode, CBasicModel *pModel);
    SRayIntersection TestNodes(const SViewInfo& rkViewInfo);
};

#endif // CRAYCOLLISIONHELPER_H
