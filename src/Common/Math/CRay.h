#ifndef CRAY_H
#define CRAY_H

#include "CVector3f.h"

class CRay
{
    CVector3f mOrigin;
    CVector3f mDirection;

public:
    CRay();
    CRay(const CVector3f& Origin, const CVector3f& Direction);
    ~CRay();
    const CVector3f& Origin() const;
    const CVector3f& Direction() const;
    void SetOrigin(const CVector3f& Origin);
    void SetDirection(const CVector3f& Direction);

    CRay Transformed(const CTransform4f& Matrix) const;
    CVector3f PointOnRay(float Distance) const;
};

// ************ INLINE FUNCTIONS ************
inline const CVector3f& CRay::Origin() const
{
    return mOrigin;
}

inline const CVector3f& CRay::Direction() const
{
    return mDirection;
}

#endif // CRAY_H
