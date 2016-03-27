#include "MathUtil.h"
#include "CMatrix4f.h"
#include <gtc/matrix_transform.hpp>

namespace Math
{

float Abs(float Value)
{
    return fabs(Value);
}

float Pow(float Base, float Exponent)
{
    return pow(Base, Exponent);
}

float Sqrt(float Value)
{
    return sqrtf(Value);
}

float Distance(const CVector3f& rkA, const CVector3f& rkB)
{
    return sqrtf( Pow(rkB.X - rkA.X, 2.f) +
                  Pow(rkB.Y - rkA.Y, 2.f) +
                  Pow(rkB.Z - rkA.Z, 2.f) );
}

float DegreesToRadians(float Deg)
{
    return Deg * skPi / 180.f;
}

float RadiansToDegrees(float Rad)
{
    return Rad * 180.f / skPi;
}

std::pair<bool,float> RayPlaneIntersecton(const CRay& rkRay, const CPlane& plane)
{
    // Code based on ray/plane intersect code from Ogre
    // https://bitbucket.org/sinbad/ogre/src/197116fd2ac62c57cdeed1666f9866c3dddd4289/OgreMain/src/OgreMath.cpp?at=default#OgreMath.cpp-350

    // Are ray and plane parallel?
    float Denom = plane.Normal().Dot(rkRay.Direction());

    if (Abs(Denom) < FLT_EPSILON)
        return std::pair<bool,float>(false, 0.f);

    // Not parallel
    float nom = plane.Normal().Dot(rkRay.Origin()) + plane.Dist();
    float t = -(nom / Denom);
    return std::pair<bool,float>(t >= 0.f, t);
}

std::pair<bool,float> RayBoxIntersection(const CRay& rkRay, const CAABox& rkBox)
{
    // Code slightly modified from Ogre
    // https://github.com/ehsan/ogre/blob/master/OgreMain/src/OgreMath.cpp
    if (rkBox.IsNull())     return std::pair<bool,float>(false, 0.f);
    if (rkBox.IsInfinite()) return std::pair<bool,float>(true, 0.f);

    float lowt = 0.0f;
    float t;
    bool Hit = false;
    CVector3f HitPoint;
    const CVector3f& RayOrig = rkRay.Origin();
    const CVector3f& RayDir = rkRay.Direction();
    const CVector3f Min = rkBox.Min();
    const CVector3f Max = rkBox.Max();

    // Check origin inside first
    if ( RayOrig > Min && RayOrig < Max )
    {
        return std::pair<bool, float>(true, 0.f);
    }

    // Check each face in turn, only check closest 3
    // Min x
    if (RayOrig.X <= Min.X && RayDir.X > 0)
    {
        t = (Min.X - RayOrig.X) / RayDir.X;
        if (t >= 0)
        {
            // Substitute t back into ray and check bounds and dist
            HitPoint = RayOrig + RayDir * t;
            if (HitPoint.Y >= Min.Y && HitPoint.Y <= Max.Y &&
                HitPoint.Z >= Min.Z && HitPoint.Z <= Max.Z &&
                (!Hit || t < lowt))
            {
                Hit = true;
                lowt = t;
            }
        }
    }
    // Max x
    if (RayOrig.X >= Max.X && RayDir.X < 0)
    {
        t = (Max.X - RayOrig.X) / RayDir.X;
        if (t >= 0)
        {
            // Substitute t back into ray and check bounds and dist
            HitPoint = RayOrig + RayDir * t;
            if (HitPoint.Y >= Min.Y && HitPoint.Y <= Max.Y &&
                HitPoint.Z >= Min.Z && HitPoint.Z <= Max.Z &&
                (!Hit || t < lowt))
            {
                Hit = true;
                lowt = t;
            }
        }
    }
    // Min y
    if (RayOrig.Y <= Min.Y && RayDir.Y > 0)
    {
        t = (Min.Y - RayOrig.Y) / RayDir.Y;
        if (t >= 0)
        {
            // Substitute t back into ray and check bounds and dist
            HitPoint = RayOrig + RayDir * t;
            if (HitPoint.X >= Min.X && HitPoint.X <= Max.X &&
                HitPoint.Z >= Min.Z && HitPoint.Z <= Max.Z &&
                (!Hit || t < lowt))
            {
                Hit = true;
                lowt = t;
            }
        }
    }
    // Max y
    if (RayOrig.Y >= Max.Y && RayDir.Y < 0)
    {
        t = (Max.Y - RayOrig.Y) / RayDir.Y;
        if (t >= 0)
        {
            // Substitute t back into ray and check bounds and dist
            HitPoint = RayOrig + RayDir * t;
            if (HitPoint.X >= Min.X && HitPoint.X <= Max.X &&
                HitPoint.Z >= Min.Z && HitPoint.Z <= Max.Z &&
                (!Hit || t < lowt))
            {
                Hit = true;
                lowt = t;
            }
        }
    }
    // Min z
    if (RayOrig.Z <= Min.Z && RayDir.Z > 0)
    {
        t = (Min.Z - RayOrig.Z) / RayDir.Z;
        if (t >= 0)
        {
            // Substitute t back into ray and check bounds and dist
            HitPoint = RayOrig + RayDir * t;
            if (HitPoint.X >= Min.X && HitPoint.X <= Max.X &&
                HitPoint.Y >= Min.Y && HitPoint.Y <= Max.Y &&
                (!Hit || t < lowt))
            {
                Hit = true;
                lowt = t;
            }
        }
    }
    // Max z
    if (RayOrig.Z >= Max.Z && RayDir.Z < 0)
    {
        t = (Max.Z - RayOrig.Z) / RayDir.Z;
        if (t >= 0)
        {
            // Substitute t back into ray and check bounds and dist
            HitPoint = RayOrig + RayDir * t;
            if (HitPoint.X >= Min.X && HitPoint.X <= Max.X &&
                HitPoint.Y >= Min.Y && HitPoint.Y <= Max.Y &&
                (!Hit || t < lowt))
            {
                Hit = true;
                lowt = t;
            }
        }
    }

    return std::pair<bool,float>(Hit, lowt);
}

std::pair<bool,float> RayLineIntersection(const CRay& rkRay, const CVector3f& rkPointA,
                                          const CVector3f& rkPointB, float Threshold)
{
    // http://geomalgorithms.com/a07-_distance.html
    // http://www.gamedev.net/topic/589705-rayline-intersection-in-3d/
    CVector3f u = rkRay.Direction();
    CVector3f v = rkPointB - rkPointA;
    CVector3f w = rkRay.Origin() - rkPointA;
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
    bool hit = (dP.Magnitude() <= Threshold);
    return std::pair<bool,float>(hit, sc);
}

std::pair<bool,float> RayTriangleIntersection(const CRay& rkRay,
                                              const CVector3f& rkVtxA, const CVector3f& rkVtxB,
                                              const CVector3f& rkVtxC, bool AllowBackfaces)
{
    // Ogre code cuz I'm lazy and bad at math
    // https://github.com/ehsan/ogre/blob/master/OgreMain/src/OgreMath.cpp#L709
    CVector3f FaceNormal = (rkVtxB - rkVtxA).Cross(rkVtxC - rkVtxA);

    //
    // Calculate intersection with plane.
    //
    float t;
    {
        float denom = FaceNormal.Dot(rkRay.Direction());

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

        t = FaceNormal.Dot(rkVtxA - rkRay.Origin()) / denom;

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
        float u1 = rkVtxB[i0] - rkVtxA[i0];
        float v1 = rkVtxB[i1] - rkVtxA[i1];
        float u2 = rkVtxC[i0] - rkVtxA[i0];
        float v2 = rkVtxC[i1] - rkVtxA[i1];
        float u0 = t * rkRay.Direction()[i0] + rkRay.Origin()[i0] - rkVtxA[i0];
        float v0 = t * rkRay.Direction()[i1] + rkRay.Origin()[i1] - rkVtxA[i1];

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

CMatrix4f PerspectiveMatrix(float FOV, float Aspect, float Near, float Far)
{
    // todo: don't use glm
    return CMatrix4f::FromGlmMat4(glm::perspective(FOV, Aspect, Near, Far)).Transpose();
}

CMatrix4f OrthographicMatrix(float Left, float Right, float Bottom, float Top, float Near, float Far)
{
    return CMatrix4f::FromGlmMat4(glm::ortho(Left, Right, Bottom, Top, Near, Far)).Transpose();
}

} // End namespace
