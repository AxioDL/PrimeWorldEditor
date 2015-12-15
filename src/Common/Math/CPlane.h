#ifndef CPLANE_H
#define CPLANE_H

#include "CVector3f.h"

class CPlane
{
    CVector3f mNormal;
    float mDist;

public:
    CPlane();
    CPlane(const CVector3f& normal, float dist);
    CPlane(const CVector3f& normal, const CVector3f& origin);

    void Redefine(const CVector3f& normal, const CVector3f& origin);
    CVector3f Normal() const;
    float Dist() const;
    void SetNormal(const CVector3f& normal);
    void SetDist(float dist);
};

#endif // CPLANE_H
