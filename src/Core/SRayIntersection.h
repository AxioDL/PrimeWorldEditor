#ifndef SRAYINTERSECTION
#define SRAYINTERSECTION

#include <Common/types.h>
#include <Math/CVector3f.h>

class CSceneNode;

struct SRayIntersection
{
    bool Hit;
    float Distance;
    CVector3f HitPoint;
    CSceneNode *pNode;
    u32 ComponentIndex;

    SRayIntersection()
        : Hit(false), Distance(0.f), HitPoint(CVector3f::skZero), pNode(nullptr), ComponentIndex(-1) {}

    SRayIntersection(bool _Hit, float _Distance, CVector3f _HitPoint, CSceneNode *_pNode, u32 _ComponentIndex)
        : Hit(_Hit), Distance(_Distance), HitPoint(_HitPoint), pNode(_pNode), ComponentIndex(_ComponentIndex) {}
};

#endif // SRAYINTERSECTION

