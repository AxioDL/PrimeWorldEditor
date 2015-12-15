#include "CRay.h"
#include <Common/CVector4f.h>
#include <Common/CTransform4f.h>
#include <iostream>

CRay::CRay()
{
}

CRay::CRay(const CVector3f& Origin, const CVector3f& Direction)
        : mOrigin(Origin), mDirection(Direction)
{
}

CRay::~CRay()
{
}

void CRay::SetOrigin(const CVector3f& Origin)
{
    mOrigin = Origin;
}

void CRay::SetDirection(const CVector3f& Direction)
{
    mDirection = Direction;
}

CRay CRay::Transformed(const CTransform4f &Matrix) const
{
    CRay out;
    out.mOrigin = Matrix * mOrigin;

    CVector3f Point = Matrix * (mOrigin + mDirection);
    out.mDirection = (Point - out.mOrigin).Normalized();

    return out;
}

CVector3f CRay::PointOnRay(float Distance) const
{
    return mOrigin + (mDirection * Distance);
}
