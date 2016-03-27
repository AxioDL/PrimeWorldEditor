#ifndef CAABOX_H
#define CAABOX_H

#include "CVector3f.h"
#include <FileIO/IInputStream.h>
#include <FileIO/IOutputStream.h>
#include <utility>

class CRay;
class CRayIntersection;

class CAABox
{
    CVector3f mMin, mMax;

public:
    CAABox();
    CAABox(const CVector3f& rkMin, const CVector3f& rkMax);
    CAABox(IInputStream& rInput);
    void Write(IOutputStream& rOutput);
    CVector3f Center() const;
    CVector3f Size() const;
    CVector3f Min() const;
    CVector3f Max() const;
    void SetMin(const CVector3f& rkMin);
    void SetMax(const CVector3f& rkMax);
    bool IsNull() const;
    bool IsInfinite() const;

    void ExpandBounds(const CVector3f& rkVtx);
    void ExpandBounds(const CAABox& rkAABox);
    void ExpandBy(const CVector3f& rkAmount);
    CAABox Transformed(const CTransform4f& rkTransform) const;

    bool IsPointInBox(const CVector3f& rkPoint) const;
    CVector3f ClosestPointAlongVector(const CVector3f& rkDir) const;
    CVector3f FurthestPointAlongVector(const CVector3f& rkDir) const;

    // Intersection Tests
    bool IntersectsAABox(const CAABox& rkAABox);
    bool IntersectsSphere(const CVector3f& rkSphereCenter, float SphereRadius);
    std::pair<bool,float> IntersectsRay(const CRay& rkRay) const;

    // Operators
    CAABox operator+(const CVector3f& rkTranslate) const;
    void operator+=(const CVector3f& rkTranslate);
    CAABox operator*(float Scalar) const;
    void operator*=(float Scalar);
    bool operator==(const CAABox& rkOther) const;
    bool operator!=(const CAABox& rkOther) const;

    // Constants
    static const CAABox skInfinite;
    static const CAABox skOne;
    static const CAABox skZero;
};

#endif // CAABOX_H
