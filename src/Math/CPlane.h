#ifndef CPLANE_H
#define CPLANE_H

#include "CVector3f.h"

class CPlane
{
    CVector3f mNormal;
    float mDist;

public:
    CPlane()
        : mNormal(CVector3f::skUp)
        , mDist(0.f)
    {}

    CPlane(const CVector3f& rkNormal, float Dist)
        : mNormal(rkNormal)
        , mDist(Dist)
    {}

    CPlane(const CVector3f& rkNormal, const CVector3f& rkOrigin)
    {
        Redefine(rkNormal, rkOrigin);
    }

    void Redefine(const CVector3f& rkNormal, const CVector3f& rkOrigin)
    {
        mNormal = rkNormal;
        mDist = -rkNormal.Dot(rkOrigin);
    }

    CVector3f Normal() const                    { return mNormal; }
    float Dist() const                          { return mDist; }
    void SetNormal(const CVector3f& rkNormal)   { mNormal = rkNormal; }
    void SetDist(float Dist)                    { mDist = Dist; }
};

#endif // CPLANE_H
