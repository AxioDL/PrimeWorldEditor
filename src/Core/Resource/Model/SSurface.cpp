#include "SSurface.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/CRayCollisionTester.h"
#include <Common/Math/MathUtil.h>

std::pair<bool,float> SSurface::IntersectsRay(const CRay& rkRay, bool AllowBackfaces, float LineThreshold) const
{
    bool Hit = false;
    float HitDist = 0.0f;

    for (const auto& prim : Primitives)
    {
        const size_t NumVerts = prim.Vertices.size();

        // Triangles
        if (prim.Type == EPrimitiveType::Triangles || prim.Type == EPrimitiveType::TriangleFan || prim.Type == EPrimitiveType::TriangleStrip)
        {
            size_t NumTris;

            if (prim.Type == EPrimitiveType::Triangles)
                NumTris = NumVerts / 3;
            else
                NumTris = NumVerts - 2;

            for (size_t iTri = 0; iTri < NumTris; iTri++)
            {
                CVector3f VtxA, VtxB, VtxC;

                // Get the three vertices that make up the current tri
                if (prim.Type == EPrimitiveType::Triangles)
                {
                    const size_t VertIndex = iTri * 3;
                    VtxA = prim.Vertices[VertIndex + 0].Position;
                    VtxB = prim.Vertices[VertIndex + 1].Position;
                    VtxC = prim.Vertices[VertIndex + 2].Position;
                }
                else if (prim.Type == EPrimitiveType::TriangleFan)
                {
                    VtxA = prim.Vertices[0].Position;
                    VtxB = prim.Vertices[iTri + 1].Position;
                    VtxC = prim.Vertices[iTri + 2].Position;
                }
                else if (prim.Type == EPrimitiveType::TriangleStrip)
                {
                    if ((iTri & 1) != 0)
                    {
                        VtxA = prim.Vertices[iTri + 2].Position;
                        VtxB = prim.Vertices[iTri + 1].Position;
                        VtxC = prim.Vertices[iTri + 0].Position;
                    }
                    else
                    {
                        VtxA = prim.Vertices[iTri + 0].Position;
                        VtxB = prim.Vertices[iTri + 1].Position;
                        VtxC = prim.Vertices[iTri + 2].Position;
                    }
                }

                // Intersection test
                const auto [intersects, distance] = Math::RayTriangleIntersection(rkRay, VtxA, VtxB, VtxC, AllowBackfaces);

                if (intersects)
                {
                    if (!Hit || distance < HitDist)
                    {
                        Hit = true;
                        HitDist = distance;
                    }
                }
            }
        }

        // Lines
        if (prim.Type == EPrimitiveType::Lines || prim.Type == EPrimitiveType::LineStrip)
        {
            size_t NumLines;

            if (prim.Type == EPrimitiveType::Lines)
                NumLines = NumVerts / 2;
            else
                NumLines = NumVerts - 1;

            for (size_t iLine = 0; iLine < NumLines; iLine++)
            {
                CVector3f VtxA, VtxB;

                // Get the two vertices that make up the current line
                const size_t Index = (prim.Type == EPrimitiveType::Lines ? iLine * 2 : iLine);
                VtxA = prim.Vertices[Index + 0].Position;
                VtxB = prim.Vertices[Index + 1].Position;

                // Intersection test
                const auto [intersects, distance] = Math::RayLineIntersection(rkRay, VtxA, VtxB, LineThreshold);

                if (intersects)
                {
                    if (!Hit || distance < HitDist)
                    {
                        Hit = true;
                        HitDist = distance;
                    }
                }
            }
        }
    }

    return {Hit, HitDist};
}
