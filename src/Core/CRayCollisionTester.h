#ifndef CRAYCOLLISIONHELPER_H
#define CRAYCOLLISIONHELPER_H

#include "SRayIntersection.h"
#include "Core/Render/SViewInfo.h"
#include "Core/Resource/Model/CBasicModel.h"
#include <Common/Math/CAABox.h>
#include <Common/Math/CRay.h>
#include <Common/Math/CVector3f.h>
#include <Common/types.h>

#include <list>

class CSceneNode;

class CRayCollisionTester
{
    CRay mRay;
    std::list<SRayIntersection> mBoxIntersectList;

public:
    CRayCollisionTester(const CRay& Ray);
    ~CRayCollisionTester();
    const CRay& Ray() const;
    void AddNode(CSceneNode *pNode, u32 AssetIndex, float Distance);
    void AddNodeModel(CSceneNode *pNode, CBasicModel *pModel);
    SRayIntersection TestNodes(const SViewInfo& ViewInfo);
};

inline const CRay& CRayCollisionTester::Ray() const
{
    return mRay;
}

#endif // CRAYCOLLISIONHELPER_H
