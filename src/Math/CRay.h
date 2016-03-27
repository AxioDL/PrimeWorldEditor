#ifndef CRAY_H
#define CRAY_H

#include "CTransform4f.h"
#include "CVector3f.h"

class CRay
{
    CVector3f mOrigin;
    CVector3f mDirection;

public:
    CRay() {}

    CRay(const CVector3f& rkOrigin, const CVector3f& rkDirection)
        : mOrigin(rkOrigin), mDirection(rkDirection) {}

    const CVector3f& Origin() const                 { return mOrigin; }
    const CVector3f& Direction() const              { return mDirection; }
    void SetOrigin(const CVector3f& rkOrigin)       { mOrigin = rkOrigin; }
    void SetDirection(const CVector3f& rkDirection) { mDirection = rkDirection; }

    CRay Transformed(const CTransform4f& rkMatrix) const
    {
        CRay Out;
        Out.mOrigin = rkMatrix * mOrigin;

        CVector3f Point = rkMatrix * (mOrigin + mDirection);
        Out.mDirection = (Point - Out.mOrigin).Normalized();

        return Out;
    }

    CVector3f PointOnRay(float Distance) const { return mOrigin + (mDirection * Distance); }
};

#endif // CRAY_H
