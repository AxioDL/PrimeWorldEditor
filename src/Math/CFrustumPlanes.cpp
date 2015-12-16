#include "CFrustumPlanes.h"
#include "MathUtil.h"
#include "Common/types.h"

CFrustumPlanes::CFrustumPlanes()
{

}

CFrustumPlanes::~CFrustumPlanes()
{

}

const CPlane& CFrustumPlanes::GetPlane(EFrustumSide side) const
{
    return mPlanes[side];
}

void CFrustumPlanes::SetPlanes(const CVector3f& position, const CVector3f& direction, float fieldOfView, float aspectRatio, float near, float far)
{
    // Calculate up/right vectors
    CVector3f right = direction.Cross(CVector3f::skUp).Normalized();
    CVector3f up = right.Cross(direction).Normalized();

    // Calculate dimensions of near plane
    float nearHeight = 2 * tanf(Math::DegreesToRadians(fieldOfView) / 2.f) * near;
    float nearWidth = nearHeight * aspectRatio;

    // Define the planes
    CVector3f nearCenter = position + (direction * near);
    mPlanes[eNearPlane].Redefine(direction, nearCenter);

    CVector3f farCenter = position + (direction * far);
    mPlanes[eFarPlane].Redefine(-direction, farCenter);

    CVector3f midRight = nearCenter + (right * (nearWidth / 2.f));
    CVector3f rightNormal = up.Cross((midRight - position).Normalized());
    mPlanes[eRightPlane].Redefine(rightNormal, position);

    CVector3f midLeft = nearCenter - (right * (nearWidth / 2.f));
    CVector3f leftNormal = (midLeft - position).Normalized().Cross(up);
    mPlanes[eLeftPlane].Redefine(leftNormal, position);

    CVector3f midTop = nearCenter + (up * (nearHeight / 2.f));
    CVector3f topNormal = (midTop - position).Normalized().Cross(right);
    mPlanes[eTopPlane].Redefine(topNormal, position);

    CVector3f midBottom = nearCenter - (up * (nearHeight / 2.f));
    CVector3f bottomNormal = right.Cross((midBottom - position).Normalized());
    mPlanes[eBottomPlane].Redefine(bottomNormal, position);
}

bool CFrustumPlanes::PointInFrustum(const CVector3f& point) const
{
    for (u32 iPlane = 0; iPlane < 6; iPlane++)
    {
        const CPlane& plane = mPlanes[iPlane];

        if (plane.Normal().Dot(point) + plane.Dist() < 0.f)
            return false;
    }

    return true;
}

bool CFrustumPlanes::BoxInFrustum(const CAABox& box) const
{
    CVector3f min = box.Min();
    CVector3f max = box.Max();

    CVector3f points[8];
    points[0] = min;
    points[1] = max;
    points[2] = CVector3f(min.x, min.y, max.z);
    points[3] = CVector3f(min.x, max.y, min.z);
    points[4] = CVector3f(min.x, max.y, max.z);
    points[5] = CVector3f(max.x, min.y, max.z);
    points[6] = CVector3f(max.x, max.y, min.z);
    points[7] = CVector3f(max.x, min.y, min.z);

    for (u32 iPlane = 0; iPlane < 6; iPlane++)
    {
        const CPlane& plane = mPlanes[iPlane];
        int numPoints = 0;

        for (u32 iPoint = 0; iPoint < 8; iPoint++)
        {
            if (plane.Normal().Dot(points[iPoint]) + plane.Dist() < 0.f)
                numPoints++;
            else
                break;
        }

        if (numPoints == 8) return false;
    }
    return true;
}
