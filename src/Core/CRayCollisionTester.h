#ifndef CRAYCOLLISIONHELPER_H
#define CRAYCOLLISIONHELPER_H

#include "CAABox.h"
#include "CRay.h"
#include "CVector3f.h"
#include "SRayIntersection.h"
#include "types.h"
#include <Core/SViewInfo.h>
#include <Resource/model/CBasicModel.h>

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
