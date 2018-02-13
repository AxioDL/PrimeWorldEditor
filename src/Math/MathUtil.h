#ifndef MATH_H
#define MATH_H

#include "CAABox.h"
#include "CRay.h"
#include "CPlane.h"
#include "CVector3f.h"
#include <utility>

namespace Math
{

float Abs(float Value);

float Pow(float Base, float Exponent);

float Sqrt(float Value);

float Distance(const CVector3f& rkA, const CVector3f& rkB);

float DegreesToRadians(float Deg);

float RadiansToDegrees(float Rad);

template<typename Type>
Type Clamp(const Type& rkMin, const Type& rkMax, const Type& rkVal)
{
    return (rkVal < rkMin) ? rkMin :
           (rkVal > rkMax) ? rkMax :
                             rkVal;
}

template<typename Type>
Type Max(const Type& rkA, const Type& rkB)
{
    return (rkA > rkB ? rkA : rkB);
}

template<typename Type>
Type Min(const Type& rkA, const Type& rkB)
{
    return (rkA < rkB ? rkA : rkB);
}

template<typename Type>
Type Lerp(const Type& rkA, const Type& rkB, float t)
{
    Type Diff = rkB - rkA;
    return rkA + Type(Diff * t);
}

std::pair<bool,float> RayPlaneIntersection(const CRay& rkRay, const CPlane& rkPlane);

std::pair<bool,float> RayBoxIntersection(const CRay& rkRay, const CAABox& rkBox);

std::pair<bool,float> RayLineIntersection(const CRay& rkRay, const CVector3f& rkPointA,
                                          const CVector3f& rkPointB, float Threshold = 0.02f);

std::pair<bool,float> RaySphereIntersection(const CRay& rkRay, const CVector3f& rkSpherePos,
                                            float SphereRadius, bool AllowBackfaces = false);

std::pair<bool,float> RayTriangleIntersection(const CRay& rkRay, const CVector3f& rkPointA,
                                              const CVector3f& rkPointB, const CVector3f& rkPointC,
                                              bool AllowBackfaces = false);

CMatrix4f PerspectiveMatrix(float FOV, float Aspect, float Near, float Far);

CMatrix4f OrthographicMatrix(float Left, float Right, float Bottom, float Top, float Near, float Far);

// Constants
static const float skPi = 3.14159265358979323846f;
static const float skHalfPi = skPi / 2.f;
}

#endif // MATH_H

