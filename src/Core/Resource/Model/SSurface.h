#ifndef SSURFACE_H
#define SSURFACE_H

#include "CVertex.h"
#include "Core/Resource/CMaterialSet.h"
#include "Core/OpenGL/GLCommon.h"
#include "Core/SRayIntersection.h"
#include <Common/types.h>
#include <Math/CAABox.h>
#include <Math/CRay.h>
#include <Math/CTransform4f.h>
#include <Math/CVector3f.h>
#include <vector>

// Should prolly be a class
struct SSurface
{
    u32 VertexCount;
    u32 TriangleCount;
    CAABox AABox;
    CVector3f CenterPoint;
    u32 MaterialID;
    CVector3f ReflectionDirection;
    u16 MeshID;

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
