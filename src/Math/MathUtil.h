#ifndef MATH_H
#define MATH_H

#include "CAABox.h"
#include "CRay.h"
#include "CPlane.h"
#include "CVector3f.h"
#include <utility>

namespace Math
{

float Abs(float v);

float Pow(float Base, float Exponent);

float Sqrt(float v);

float Distance(const CVector3f& A, const CVector3f& B);

float DegreesToRadians(float deg);

float RadiansToDegrees(float rad);

std::pair<bool,float> RayPlaneIntersecton(const CRay& ray, const CPlane& plane);

std::pair<bool,float> RayBoxIntersection(const CRay& Ray, const CAABox& Box);

std::pair<bool,float> RayLineIntersection(const CRay& ray, const CVector3f& pointA,
                                          const CVector3f& pointB, float threshold = 0.02f);

std::pair<bool,float> RayTriangleIntersection(const CRay& Ray, const CVector3f& PointA,
                                              const CVector3f& PointB, const CVector3f& PointC,
                                              bool AllowBackfaces = false);

CMatrix4f PerspectiveMatrix(float fov, float aspect, float near, float far);

CMatrix4f OrthographicMatrix(float left, float right, float bottom, float top, float near, float far);

// Constants
static const float skPi = 3.14159265358979323846f;
static const float skHalfPi = skPi / 2.f;
}

#endif // MATH_H

