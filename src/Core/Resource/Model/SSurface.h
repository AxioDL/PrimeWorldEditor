#ifndef SSURFACE_H
#define SSURFACE_H

#include "../CMaterialSet.h"
#include "CVertex.h"
#include <Common/types.h>
#include <Common/CAABox.h>
#include <Common/CRay.h>
#include <Common/CTransform4f.h>
#include <Common/CVector3f.h>
#include <Common/SRayIntersection.h>
#include <OpenGL/GLCommon.h>
#include <vector>

struct SSurface
{
    u32 VertexCount;
    u32 TriangleCount;
    CAABox AABox;
    CVector3f CenterPoint;
    u32 MaterialID;
    CVector3f ReflectionDirection;

    struct SPrimitive
    {
        EGXPrimitiveType Type;
        std::vector<CVertex> Vertices;
    };
    std::vector<SPrimitive> Primitives;

    SSurface() {
        VertexCount = 0;
        TriangleCount = 0;
    }

    std::pair<bool,float> IntersectsRay(const CRay& Ray, bool allowBackfaces = false, float LineThreshold = 0.02f);
};

#endif // SSURFACE_H
