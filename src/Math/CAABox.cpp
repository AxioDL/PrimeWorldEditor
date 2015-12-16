#include "CAABox.h"
#include "CRay.h"
#include "CTransform4f.h"
#include "MathUtil.h"
#include <float.h>

CAABox::CAABox()
{
    mMin = CVector3f::skInfinite;
    mMax = -CVector3f::skInfinite;
}

CAABox::CAABox(const CVector3f& Min, const CVector3f& Max)
{
    mMin = Min;
    mMax = Max;
}

CAABox::CAABox(IInputStream& input)
{
    mMin = CVector3f(input);
    mMax = CVector3f(input);
}

void CAABox::Write(IOutputStream& Output)
{
    mMin.Write(Output);
    mMax.Write(Output);
}

CVector3f CAABox::Center() const
{
    return (mMax - ((mMax - mMin) * 0.5f));
}

CVector3f CAABox::Size() const
{
    return (mMax - mMin);
}

CVector3f CAABox::Min() const
{
    return mMin;
}

CVector3f CAABox::Max() const
{
    return mMax;
}

void CAABox::SetMin(const CVector3f& min)
{
    mMin = min;
}

void CAABox::SetMax(const CVector3f& max)
{
    mMax = max;
}

bool CAABox::IsNull() const
{
    return (Size() == CVector3f::skZero);
}

bool CAABox::IsInfinite() const
{
    return (Size() == CVector3f::skInfinite);
}

void CAABox::ExpandBounds(const CVector3f& vtx)
{
    // take an input vertex coordinate and expand the bounding box to fit it, if necessary
    if (vtx.x < mMin.x) mMin.x = vtx.x;
    if (vtx.x > mMax.x) mMax.x = vtx.x;
    if (vtx.y < mMin.y) mMin.y = vtx.y;
    if (vtx.y > mMax.y) mMax.y = vtx.y;
    if (vtx.z < mMin.z) mMin.z = vtx.z;
    if (vtx.z > mMax.z) mMax.z = vtx.z;
}

void CAABox::ExpandBounds(const CAABox& AABox)
{
    ExpandBounds(AABox.mMin);
    ExpandBounds(AABox.mMax);
}

void CAABox::ExpandBy(const CVector3f& amount)
{
    CVector3f halfAmount = amount / 2.f;
    mMin -= halfAmount;
    mMax += halfAmount;
}

CAABox CAABox::Transformed(const CTransform4f& transform) const
{
    CAABox AABox;

    AABox.ExpandBounds(transform * CVector3f(mMin.x, mMin.y, mMin.z));
    AABox.ExpandBounds(transform * CVector3f(mMin.x, mMin.y, mMax.z));
    AABox.ExpandBounds(transform * CVector3f(mMin.x, mMax.y, mMax.z));
    AABox.ExpandBounds(transform * CVector3f(mMin.x, mMax.y, mMin.z));
    AABox.ExpandBounds(transform * CVector3f(mMax.x, mMin.y, mMin.z));
    AABox.ExpandBounds(transform * CVector3f(mMax.x, mMin.y, mMax.z));
    AABox.ExpandBounds(transform * CVector3f(mMax.x, mMax.y, mMax.z));
    AABox.ExpandBounds(transform * CVector3f(mMax.x, mMax.y, mMin.z));

    return AABox;
}

bool CAABox::IsPointInBox(const CVector3f& Point) const
{
    return ( ((Point.x >= mMin.x) && (Point.x <= mMax.x)) &&
             ((Point.y >= mMin.y) && (Point.y <= mMax.y)) &&
             ((Point.z >= mMin.z) && (Point.z <= mMax.z)) );
}

CVector3f CAABox::ClosestPointAlongVector(const CVector3f& dir) const
{
    CVector3f out;
    out.x = (dir.x >= 0.f) ? mMin.x : mMax.x;
    out.y = (dir.y >= 0.f) ? mMin.y : mMax.y;
    out.z = (dir.z >= 0.f) ? mMin.z : mMax.z;
    return out;
}

CVector3f CAABox::FurthestPointAlongVector(const CVector3f& dir) const
{
    CVector3f out;
    out.x = (dir.x >= 0.f) ? mMax.x : mMin.x;
    out.y = (dir.y >= 0.f) ? mMax.y : mMin.y;
    out.z = (dir.z >= 0.f) ? mMax.z : mMin.z;
    return out;
}

// ************ INTERSECTION TESTS ************
// These tests are kinda bad and probably inaccurate, they need rewrites
bool CAABox::IntersectsAABox(const CAABox& AABox)
{
    return ((mMax > AABox.mMin) && (mMin < AABox.mMax));
}

bool CAABox::IntersectsSphere(const CVector3f& SphereCenter, const float SphereRadius)
{
    // Placeholder for proper sphere intersection test
    // Generate an AABox for the sphere and do an AABox/AABox intersection test instead
    return IntersectsAABox(CAABox(SphereCenter - SphereRadius, SphereCenter + SphereRadius));
}

std::pair<bool,float> CAABox::IntersectsRay(const CRay &Ray) const
{
    return Math::RayBoxIntersection(Ray, *this);
}

// ************ OPERATORS ************
CAABox CAABox::operator+(const CVector3f& translate) const
{
    return CAABox(mMin + translate, mMax + translate);
}

CAABox CAABox::operator*(float scalar) const
{
    return CAABox(mMin * scalar, mMax * scalar);
}

bool CAABox::operator==(const CAABox& Other) const
{
    return ((mMin == Other.mMin) && (mMax == Other.mMax));
}

bool CAABox::operator!=(const CAABox& Other) const
{
    return (!(*this == Other));
}

// ************ CONSTANTS ************

// min set to float maximum, max set to float minimum; ideal for creating an AABox from scratch
const CAABox CAABox::skInfinite = CAABox(CVector3f(FLT_MAX), CVector3f(-FLT_MAX));

// a 1x1x1 bounding box
const CAABox CAABox::skOne = CAABox( CVector3f(-0.5f, -0.5f, -0.5f), CVector3f(0.5f, 0.5f, 0.5f) );

// a 0x0x0 bounding box
const CAABox CAABox::skZero = CAABox(CVector3f(0), CVector3f(0));
