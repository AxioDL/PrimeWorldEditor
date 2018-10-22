#include "CAABox.h"
#include "CRay.h"
#include "CTransform4f.h"
#include "MathUtil.h"
#include <float.h>

CAABox::CAABox()
    : mMin(CVector3f::skInfinite)
    , mMax(-CVector3f::skInfinite)
{
}

CAABox::CAABox(const CVector3f& rkMin, const CVector3f& rkMax)
    : mMin(rkMin)
    , mMax(rkMax)
{
}

CAABox::CAABox(IInputStream& rInput)
    : mMin(rInput)
    , mMax(rInput)
{
}

void CAABox::Serialize(IArchive& rArc)
{
    rArc << SerialParameter("Min", mMin)
         << SerialParameter("Max", mMax);
}

void CAABox::Write(IOutputStream& rOutput)
{
    mMin.Write(rOutput);
    mMax.Write(rOutput);
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

void CAABox::SetMin(const CVector3f& rkMin)
{
    mMin = rkMin;
}

void CAABox::SetMax(const CVector3f& rkMax)
{
    mMax = rkMax;
}

bool CAABox::IsNull() const
{
    return (Size() == CVector3f::skZero);
}

bool CAABox::IsInfinite() const
{
    return (Size() == CVector3f::skInfinite);
}

void CAABox::ExpandBounds(const CVector3f& rkVtx)
{
    // take an input vertex coordinate and expand the bounding box to fit it, if necessary
    if (rkVtx.X < mMin.X) mMin.X = rkVtx.X;
    if (rkVtx.X > mMax.X) mMax.X = rkVtx.X;
    if (rkVtx.Y < mMin.Y) mMin.Y = rkVtx.Y;
    if (rkVtx.Y > mMax.Y) mMax.Y = rkVtx.Y;
    if (rkVtx.Z < mMin.Z) mMin.Z = rkVtx.Z;
    if (rkVtx.Z > mMax.Z) mMax.Z = rkVtx.Z;
}

void CAABox::ExpandBounds(const CAABox& rkAABox)
{
    ExpandBounds(rkAABox.mMin);
    ExpandBounds(rkAABox.mMax);
}

void CAABox::ExpandBy(const CVector3f& rkAmount)
{
    CVector3f halfAmount = rkAmount / 2.f;
    mMin -= halfAmount;
    mMax += halfAmount;
}

CAABox CAABox::Transformed(const CTransform4f& rkTransform) const
{
    CAABox AABox;

    AABox.ExpandBounds(rkTransform * CVector3f(mMin.X, mMin.Y, mMin.Z));
    AABox.ExpandBounds(rkTransform * CVector3f(mMin.X, mMin.Y, mMax.Z));
    AABox.ExpandBounds(rkTransform * CVector3f(mMin.X, mMax.Y, mMax.Z));
    AABox.ExpandBounds(rkTransform * CVector3f(mMin.X, mMax.Y, mMin.Z));
    AABox.ExpandBounds(rkTransform * CVector3f(mMax.X, mMin.Y, mMin.Z));
    AABox.ExpandBounds(rkTransform * CVector3f(mMax.X, mMin.Y, mMax.Z));
    AABox.ExpandBounds(rkTransform * CVector3f(mMax.X, mMax.Y, mMax.Z));
    AABox.ExpandBounds(rkTransform * CVector3f(mMax.X, mMax.Y, mMin.Z));

    return AABox;
}

bool CAABox::IsPointInBox(const CVector3f& rkPoint) const
{
    return ( ((rkPoint.X >= mMin.X) && (rkPoint.X <= mMax.X)) &&
             ((rkPoint.Y >= mMin.Y) && (rkPoint.Y <= mMax.Y)) &&
             ((rkPoint.Z >= mMin.Z) && (rkPoint.Z <= mMax.Z)) );
}

CVector3f CAABox::ClosestPointAlongVector(const CVector3f& rkDir) const
{
    CVector3f out;
    out.X = (rkDir.X >= 0.f) ? mMin.X : mMax.X;
    out.Y = (rkDir.Y >= 0.f) ? mMin.Y : mMax.Y;
    out.Z = (rkDir.Z >= 0.f) ? mMin.Z : mMax.Z;
    return out;
}

CVector3f CAABox::FurthestPointAlongVector(const CVector3f& rkDir) const
{
    CVector3f out;
    out.X = (rkDir.X >= 0.f) ? mMax.X : mMin.X;
    out.Y = (rkDir.Y >= 0.f) ? mMax.Y : mMin.Y;
    out.Z = (rkDir.Z >= 0.f) ? mMax.Z : mMin.Z;
    return out;
}

// ************ INTERSECTION TESTS ************
// These tests are kinda bad and probably inaccurate, they need rewrites
bool CAABox::IntersectsAABox(const CAABox& rkAABox)
{
    return ((mMax > rkAABox.mMin) && (mMin < rkAABox.mMax));
}

bool CAABox::IntersectsSphere(const CVector3f& rkSphereCenter, float SphereRadius)
{
    // Placeholder for proper sphere intersection test
    // Generate an AABox for the sphere and do an AABox/AABox intersection test instead
    return IntersectsAABox(CAABox(rkSphereCenter - SphereRadius, rkSphereCenter + SphereRadius));
}

std::pair<bool,float> CAABox::IntersectsRay(const CRay& rkRay) const
{
    return Math::RayBoxIntersection(rkRay, *this);
}

// ************ OPERATORS ************
CAABox CAABox::operator+(const CVector3f& rkTranslate) const
{
    return CAABox(mMin + rkTranslate, mMax + rkTranslate);
}

void CAABox::operator+=(const CVector3f& rkTranslate)
{
    *this = *this + rkTranslate;
}

CAABox CAABox::operator*(float Scalar) const
{
    return CAABox(mMin * Scalar, mMax * Scalar);
}

void CAABox::operator*=(float Scalar)
{
    *this = *this * Scalar;
}

bool CAABox::operator==(const CAABox& rkOther) const
{
    return ((mMin == rkOther.mMin) && (mMax == rkOther.mMax));
}

bool CAABox::operator!=(const CAABox& rkOther) const
{
    return (!(*this == rkOther));
}

// ************ CONSTANTS ************

// min set to float maximum, max set to float minimum; ideal for creating an AABox from scratch
const CAABox CAABox::skInfinite = CAABox(CVector3f(FLT_MAX), CVector3f(-FLT_MAX));

// a 1x1x1 bounding box
const CAABox CAABox::skOne = CAABox( CVector3f(-0.5f, -0.5f, -0.5f), CVector3f(0.5f, 0.5f, 0.5f) );

// a 0x0x0 bounding box
const CAABox CAABox::skZero = CAABox(CVector3f(0), CVector3f(0));
