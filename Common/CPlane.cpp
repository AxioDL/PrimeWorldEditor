#include "CPlane.h"

CPlane::CPlane()
{
    mNormal = CVector3f::skUp;
    mDist = 0.f;
}

CPlane::CPlane(const CVector3f& normal, float dist)
{
    mNormal = normal;
    mDist = dist;
}

CPlane::CPlane(const CVector3f& normal, const CVector3f& origin)
{
    Redefine(normal, origin);
}

void CPlane::Redefine(const CVector3f& normal, const CVector3f& origin)
{
    mNormal = normal;
    mDist = -normal.Dot(origin);
}

CVector3f CPlane::Normal() const
{
    return mNormal;
}

float CPlane::Dist() const
{
    return mDist;
}

void CPlane::SetNormal(const CVector3f& normal)
{
    mNormal = normal;
}

void CPlane::SetDist(float dist)
{
    mDist = dist;
}
