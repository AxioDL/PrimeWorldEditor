#include "Math.h"
#include "CMatrix4f.h"
#include <gtc/matrix_transform.hpp>

namespace Math
{

float Abs(float v)
{
    return fabs(v);
}

float Pow(float Base, float Exponent)
{
    return pow(Base, Exponent);
}

float Sqrt(float v)
{
    return sqrtf(v);
}

float Distance(const CVector3f& A, const CVector3f& B)
{
    return sqrtf( Pow(B.x - A.x, 2.f) +
                  Pow(B.y - A.y, 2.f) +
                  Pow(B.z - A.z, 2.f) );
}

float DegreesToRadians(float deg)
{
    return deg * skPi / 180.f;
}

float RadiansToDegrees(float rad)
{
    return rad * 180.f / skPi;
}

std::pair<bool,float> RayPlaneIntersecton(const CRay& ray, const CPlane& plane)
{
    // Code based on ray/plane intersect code from Ogre
    // https://bitbucket.org/sinbad/ogre/src/197116fd2ac62c57cdeed1666f9866c3dddd4289/OgreMain/src/OgreMath.cpp?at=default#OgreMath.cpp-350

    // Are ray and plane parallel?
    float denom = plane.Normal().Dot(ray.Direction());

    if (Abs(denom) < FLT_EPSILON)
        return std::pair<bool,float>(false, 0.f);

    // Not parallel
    float nom = plane.Normal().Dot(ray.Origin()) + plane.Dist();
    float t = -(nom / denom);
    return std::pair<bool,float>(t >= 0.f, t);
}

std::pair<bool,float> RayBoxIntersection(const CRay& Ray, const CAABox& Box)
{
    // Code slightly modified from Ogre
    // https://github.com/ehsan/ogre/blob/master/OgreMain/src/OgreMath.cpp
    if (Box.IsNull())     return std::pair<bool,float>(false, 0.f);
    if (Box.IsInfinite()) return std::pair<bool,float>(true, 0.f);

    float lowt = 0.0f;
    float t;
    bool Hit = false;
    CVector3f HitPoint;
    const CVector3f& RayOrig = Ray.Origin();
    const CVector3f& RayDir = Ray.Direction();
    const CVector3f Min = Box.Min();
    const CVector3f Max = Box.Max();

    // Check origin inside first
    if ( RayOrig > Min && RayOrig < Max )
    {
        return std::pair<bool, float>(true, 0.f);
    }

    // Check each face in turn, only check closest 3
    // Min x
    if (RayOrig.x <= Min.x && RayDir.x > 0)
    {
        t = (Min.x - RayOrig.x) / RayDir.x;
        if (t >= 0)
        {
            // Substitute t back into ray and check bounds and dist
            HitPoint = RayOrig + RayDir * t;
            if (HitPoint.y >= Min.y && HitPoint.y <= Max.y &&
                HitPoint.z >= Min.z && HitPoint.z <= Max.z &&
                (!Hit || t < lowt))
            {
                Hit = true;
                lowt = t;
            }
        }
    }
    // Max x
    if (RayOrig.x >= Max.x && RayDir.x < 0)
    {
        t = (Max.x - RayOrig.x) / RayDir.x;
        if (t >= 0)
        {
            // Substitute t back into ray and check bounds and dist
            HitPoint = RayOrig + RayDir * t;
            if (HitPoint.y >= Min.y && HitPoint.y <= Max.y &&
                HitPoint.z >= Min.z && HitPoint.z <= Max.z &&
                (!Hit || t < lowt))
            {
                Hit = true;
                lowt = t;
            }
        }
    }
    // Min y
    if (RayOrig.y <= Min.y && RayDir.y > 0)
    {
        t = (Min.y - RayOrig.y) / RayDir.y;
        if (t >= 0)
        {
            // Substitute t back into ray and check bounds and dist
            HitPoint = RayOrig + RayDir * t;
            if (HitPoint.x >= Min.x && HitPoint.x <= Max.x &&
                HitPoint.z >= Min.z && HitPoint.z <= Max.z &&
                (!Hit || t < lowt))
            {
                Hit = true;
                lowt = t;
            }
        }
    }
    // Max y
    if (RayOrig.y >= Max.y && RayDir.y < 0)
    {
        t = (Max.y - RayOrig.y) / RayDir.y;
        if (t >= 0)
        {
            // Substitute t back into ray and check bounds and dist
            HitPoint = RayOrig + RayDir * t;
            if (HitPoint.x >= Min.x && HitPoint.x <= Max.x &&
                HitPoint.z >= Min.z && HitPoint.z <= Max.z &&
                (!Hit || t < lowt))
            {
                Hit = true;
                lowt = t;
            }
        }
    }
    // Min z
    if (RayOrig.z <= Min.z && RayDir.z > 0)
    {
        t = (Min.z - RayOrig.z) / RayDir.z;
        if (t >= 0)
        {
            // Substitute t back into ray and check bounds and dist
            HitPoint = RayOrig + RayDir * t;
            if (HitPoint.x >= Min.x && HitPoint.x <= Max.x &&
                HitPoint.y >= Min.y && HitPoint.y <= Max.y &&
                (!Hit || t < lowt))
            {
                Hit = true;
                lowt = t;
            }
        }
    }
    // Max z
    if (RayOrig.z >= Max.z && RayDir.z < 0)
    {
        t = (Max.z - RayOrig.z) / RayDir.z;
        if (t >= 0)
        {
            // Substitute t back into ray and check bounds and dist
            HitPoint = RayOrig + RayDir * t;
            if (HitPoint.x >= Min.x && HitPoint.x <= Max.x &&
                HitPoint.y >= Min.y && HitPoint.y <= Max.y &&
                (!Hit || t < lowt))
            {
                Hit = true;
                lowt = t;
            }
        }
    }

    return std::pair<bool,float>(Hit, lowt);
}

std::pair<bool,float> RayLineIntersection(const CRay& ray, const CVector3f& pointA,
                                          const CVector3f& pointB, float threshold)
{
    // http://geomalgorithms.com/a07-_distance.html
    // http://www.gamedev.net/topic/589705-rayline-intersection-in-3d/
    CVector3f u = ray.Direction();
    CVector3f v = pointB - pointA;
    CVector3f w = ray.Origin() - pointA;
    float a = u.Dot(u);
    float b = u.Dot(v);
    float c = v.Dot(v);
    float d = u.Dot(w);
    float e = v.Dot(w);
    float D = a * c - b * b;
    float sc, sN, sD = D;
    float tc, tN, tD = D;

    if (D < FLT_EPSILON) {
        sN = 0.f;
        sD = 1.f;
        tN = e;
        tD = c;
    }
    else {
        sN = b * e - c * d;
        tN = a * e - b * d;

        if (sN < 0.f) {
            sN = 0.f;
            tN = e;
            tD = c;
        }
    }

    if (tN < 0.f) {
        tN = 0.f;

        if (-d < 0.f)
            sN = 0.f;
        else {
            sN = -d;
            sD = a;
        }
    }
    else if (tN > tD) {
        tN = tD;

        if (-d + b < 0.f)
            sN = 0.f;
        else {
            sN = -d + b;
            sD = a;
        }
    }

    sc = (fabs(sN) < FLT_EPSILON ? 0.f : sN / sD);
    tc = (fabs(tN) < FLT_EPSILON ? 0.f : tN / tD);

    CVector3f dP = w + (u * sc) - (v * tc);
    bool hit = (dP.Magnitude() <= threshold);
    return std::pair<bool,float>(hit, sc);
}

std::pair<bool,float> RayTriangleIntersection(const CRay& Ray,
                                              const CVector3f& vtxA, const CVector3f& vtxB,
                                              const CVector3f& vtxC, bool AllowBackfaces)
{
    // Ogre code cuz I'm lazy and bad at math
    // https://github.com/ehsan/ogre/blob/master/OgreMain/src/OgreMath.cpp#L709
    CVector3f FaceNormal = (vtxB - vtxA).Cross(vtxC - vtxA);

    //
    // Calculate intersection with plane.
    //
    float t;
    {
        float denom = FaceNormal.Dot(Ray.Direction());

        // Check intersect side
        if (denom > + std::numeric_limits<float>::epsilon())
        {
            if (!AllowBackfaces)
                return std::pair<bool,float>(false, 0.f);
        }
        else if (denom >= - std::numeric_limits<float>::epsilon())
        {
            // Parallel or triangle area is close to zero when
            // the plane normal not normalised.
            return std::pair<bool,float>(false, 0.f);
        }

        t = FaceNormal.Dot(vtxA - Ray.Origin()) / denom;

        if (t < 0)
        {
            // Intersection is behind origin
            return std::pair<bool,float>(false, 0.f);
        }
    }

    //
    // Calculate the largest area projection plane in X, Y or Z.
    //
    size_t i0, i1;
    {
        float n0 = fabs(FaceNormal[0]);
        float n1 = fabs(FaceNormal[1]);
        float n2 = fabs(FaceNormal[2]);

        i0 = 1; i1 = 2;
        if (n1 > n2)
        {
            if (n1 > n0) i0 = 0;
        }
        else
        {
            if (n2 > n0) i1 = 0;
        }
    }

    //
    // Check the intersection point is inside the triangle.
    //
    {
        float u1 = vtxB[i0] - vtxA[i0];
        float v1 = vtxB[i1] - vtxA[i1];
        float u2 = vtxC[i0] - vtxA[i0];
        float v2 = vtxC[i1] - vtxA[i1];
        float u0 = t * Ray.Direction()[i0] + Ray.Origin()[i0] - vtxA[i0];
        float v0 = t * Ray.Direction()[i1] + Ray.Origin()[i1] - vtxA[i1];

        float alpha = u0 * v2 - u2 * v0;
        float beta  = u1 * v0 - u0 * v1;
        float area  = u1 * v2 - u2 * v1;

        // epsilon to avoid float precision error
        const float EPSILON = 1e-6f;

        float tolerance = - EPSILON * area;

        if (area > 0)
        {
            if (alpha < tolerance || beta < tolerance || alpha+beta > area-tolerance)
                return std::pair<bool,float>(false, 0.f);
        }
        else
        {
            if (alpha > tolerance || beta > tolerance || alpha+beta < area-tolerance)
                return std::pair<bool,float>(false, 0.f);
        }
    }

    return std::pair<bool,float>(true, t);
}

CMatrix4f PerspectiveMatrix(float fov, float aspect, float near, float far)
{
    // todo: don't use glm
    return CMatrix4f::FromGlmMat4(glm::perspective(fov, aspect, near, far)).Transpose();
}

CMatrix4f OrthographicMatrix(float left, float right, float bottom, float top, float near, float far)
{
    return CMatrix4f::FromGlmMat4(glm::ortho(left, right, bottom, top, near, far)).Transpose();
}

} // End namespace
