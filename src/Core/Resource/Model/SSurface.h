#ifndef SSURFACE_H
#define SSURFACE_H

#include "CVertex.h"
#include "Core/Resource/CMaterialSet.h"
#include "Core/OpenGL/GLCommon.h"
#include "Core/SRayIntersection.h"
#include <Common/BasicTypes.h>
#include <Common/Math/CAABox.h>
#include <Common/Math/CRay.h>
#include <Common/Math/CTransform4f.h>
#include <Common/Math/CVector3f.h>
#include <vector>

// Should prolly be a class
struct SSurface
{
    uint32 VertexCount;
    uint32 TriangleCount;
    CAABox AABox;
    CVector3f CenterPoint;
    uint32 MaterialID;
    CVector3f ReflectionDirection;
    uint16 MeshID;

    struct SPrimitive
    {
        EGXPrimitiveType Type;
        std::vector<CVertex> Vertices;
    };
    std::vector<SPrimitive> Primitives;

    SSurface()
    {
        VertexCount = 0;
        TriangleCount = 0;
    }

    std::pair<bool,float> IntersectsRay(const CRay& rkRay, bool AllowBackfaces = false, float LineThreshold = 0.02f);
};

#endif // SSURFACE_H
