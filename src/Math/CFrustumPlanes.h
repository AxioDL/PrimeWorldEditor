#ifndef CFRUSTUMPLANES_H
#define CFRUSTUMPLANES_H

#include "CAABox.h"
#include "CPlane.h"
#include "CVector3f.h"

class CFrustumPlanes
{
public:
    enum EFrustumSide
    {
        eNearPlane = 0, eFarPlane    = 1,
        eTopPlane  = 2, eBottomPlane = 3,
        eLeftPlane = 4, eRightPlane  = 5
    };

private:
    CPlane mPlanes[6];

public:
    CFrustumPlanes();
    ~CFrustumPlanes();
    const CPlane& GetPlane(EFrustumSide side) const;
    void SetPlanes(const CVector3f& position, const CVector3f& direction, float fieldOfView, float aspectRatio, float near, float far);
    bool PointInFrustum(const CVector3f& point) const;
    bool BoxInFrustum(const CAABox& box) const;
};

#endif // CFRUSTUMPLANES_H
