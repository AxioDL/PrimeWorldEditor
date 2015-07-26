#include "Math.h"

namespace Math
{

float Pow(float Base, float Exponent)
{
    return pow(Base, Exponent);
}

float Distance(const CVector3f& A, const CVector3f& B)
{
    return sqrtf( Pow(B.x - A.x, 2.f) +
                  Pow(B.y - A.y, 2.f) +
                  Pow(B.z - A.z, 2.f) );
}

std::pair<bool,float> RayBoxIntersection(const CRay& Ray, const CAABox& Box)
{
    // Code slightly modified from Ogre
    // https://github.com/ehsan/ogre/blob/master/OgreMain/src/OgreMath.cpp
    if (Box.IsNull())     return std::pair<bool,float>(false, 0);
    if (Box.IsInfinite()) return std::pair<bool,float>(true, 0);

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
        return std::pair<bool, float>(true, 0);
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
                return std::pair<bool,float>(false, 0);
        }
        else if (denom < - std::numeric_limits<float>::epsilon())
        {
            if (false)
                return std::pair<bool,float>(false, 0);
        }
        else
        {
            // Parallel or triangle area is close to zero when
            // the plane normal not normalised.
            return std::pair<bool,float>(false, 0);
        }

        t = FaceNormal.Dot(vtxA - Ray.Origin()) / denom;

        if (t < 0)
        {
            // Intersection is behind origin
            return std::pair<bool,float>(false, 0);
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
                return std::pair<bool,float>(false, 0);
        }
        else
        {
            if (alpha > tolerance || beta > tolerance || alpha+beta < area-tolerance)
                return std::pair<bool,float>(false, 0);
        }
    }

    return std::pair<bool,float>(true, t);
}

} // End namespace
