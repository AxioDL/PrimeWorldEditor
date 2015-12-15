#ifndef CAABOX_H
#define CAABOX_H

#include "CVector3f.h"
#include <FileIO/CInputStream.h>
#include <FileIO/COutputStream.h>
#include <utility>

class CRay;
class CRayIntersection;

class CAABox
{
    CVector3f mMin, mMax;

public:
    CAABox();
    CAABox(const CVector3f& Min, const CVector3f& Max);
    CAABox(CInputStream& input);
    void Write(COutputStream& Output);
    CVector3f Center() const;
    CVector3f Size() const;
    CVector3f Min() const;
    CVector3f Max() const;
    void SetMin(const CVector3f& min);
    void SetMax(const CVector3f& max);
    bool IsNull() const;
    bool IsInfinite() const;

    void ExpandBounds(const CVector3f& vtx);
    void ExpandBounds(const CAABox& AABox);
    void ExpandBy(const CVector3f& amount);
    CAABox Transformed(const CTransform4f& transform) const;

    bool IsPointInBox(const CVector3f& Point) const;
    CVector3f ClosestPointAlongVector(const CVector3f& dir) const;
    CVector3f FurthestPointAlongVector(const CVector3f& dir) const;

    // Intersection Tests
    bool IntersectsAABox(const CAABox& AABox);
    bool IntersectsSphere(const CVector3f& SphereCenter, const float SphereRadius);
    std::pair<bool,float> IntersectsRay(const CRay& Ray) const;

    // Operators
    CAABox operator+(const CVector3f& translate) const;
    CAABox operator*(float scalar) const;
    bool operator==(const CAABox& Other) const;
    bool operator!=(const CAABox& Other) const;

    // Constants
    static const CAABox skInfinite;
    static const CAABox skOne;
    static const CAABox skZero;
};

#endif // CAABOX_H
