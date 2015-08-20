#ifndef MATH
#define MATH

#include "CAABox.h"
#include "CRay.h"
#include "CPlane.h"
#include "CVector3f.h"
#include "SRayIntersection.h"
#include <utility>

namespace Math
{

float Abs(float v);

float Pow(float Base, float Exponent);

float Distance(const CVector3f& A, const CVector3f& B);

std::pair<bool,float> RayPlaneIntersecton(const CRay& ray, const CPlane& plane);

std::pair<bool,float> RayBoxIntersection(const CRay& Ray, const CAABox& Box);

std::pair<bool,float> RayLineIntersection(const CRay& ray, const CVector3f& pointA,
                                          const CVector3f& pointB, float threshold = 0.02f);

std::pair<bool,float> RayTriangleIntersection(const CRay& Ray, const CVector3f& PointA,
                                              const CVector3f& PointB, const CVector3f& PointC,
                                              bool AllowBackfaces = false);
}

#endif // MATH

