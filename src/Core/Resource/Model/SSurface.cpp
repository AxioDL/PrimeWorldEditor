#include "SSurface.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/CRayCollisionTester.h"
#include <Common/Math/MathUtil.h>

std::pair<bool,float> SSurface::IntersectsRay(const CRay& rkRay, bool AllowBackfaces, float LineThreshold)
{
    bool Hit = false;
    float HitDist;

    for (uint32 iPrim = 0; iPrim < Primitives.size(); iPrim++)
    {
        SPrimitive *pPrim = &Primitives[iPrim];
        uint32 NumVerts = pPrim->Vertices.size();

        // Triangles
        if ((pPrim->Type == EPrimitiveType::Triangles) || (pPrim->Type == EPrimitiveType::TriangleFan) || (pPrim->Type == EPrimitiveType::TriangleStrip))
        {
            uint32 NumTris;

            if (pPrim->Type == EPrimitiveType::Triangles)
                NumTris = NumVerts / 3;
            else
                NumTris = NumVerts - 2;

            for (uint32 iTri = 0; iTri < NumTris; iTri++)
            {
                CVector3f VtxA, VtxB, VtxC;

                // Get the three vertices that make up the current tri
                if (pPrim->Type == EPrimitiveType::Triangles)
                {
                    uint32 VertIndex = iTri * 3;
                    VtxA = pPrim->Vertices[VertIndex].Position;
                    VtxB = pPrim->Vertices[VertIndex+1].Position;
                    VtxC = pPrim->Vertices[VertIndex+2].Position;
                }

                else if (pPrim->Type == EPrimitiveType::TriangleFan)
                {
                    VtxA = pPrim->Vertices[0].Position;
                    VtxB = pPrim->Vertices[iTri+1].Position;
                    VtxC = pPrim->Vertices[iTri+2].Position;
                }

                else if (pPrim->Type == EPrimitiveType::TriangleStrip)
                {
                    if (iTri & 0x1)
                    {
                        VtxA = pPrim->Vertices[iTri+2].Position;
                        VtxB = pPrim->Vertices[iTri+1].Position;
                        VtxC = pPrim->Vertices[iTri].Position;
                    }

                    else
                    {
                        VtxA = pPrim->Vertices[iTri].Position;
                        VtxB = pPrim->Vertices[iTri+1].Position;
                        VtxC = pPrim->Vertices[iTri+2].Position;
                    }
                }

                // Intersection test
                std::pair<bool,float> TriResult = Math::RayTriangleIntersection(rkRay, VtxA, VtxB, VtxC, AllowBackfaces);

                if (TriResult.first)
                {
                    if ((!Hit) || (TriResult.second < HitDist))
                    {
                        Hit = true;
                        HitDist = TriResult.second;
                    }
                }
            }
        }

        // Lines
        if ((pPrim->Type == EPrimitiveType::Lines) || (pPrim->Type == EPrimitiveType::LineStrip))
        {
            uint32 NumLines;

            if (pPrim->Type == EPrimitiveType::Lines)
                NumLines = NumVerts / 2;
            else
                NumLines = NumVerts - 1;

            for (uint32 iLine = 0; iLine < NumLines; iLine++)
            {
                CVector3f VtxA, VtxB;

                // Get the two vertices that make up the current line
                uint32 Index = (pPrim->Type == EPrimitiveType::Lines ? iLine * 2 : iLine);
                VtxA = pPrim->Vertices[Index].Position;
                VtxB = pPrim->Vertices[Index+1].Position;

                // Intersection test
                std::pair<bool,float> Result = Math::RayLineIntersection(rkRay, VtxA, VtxB, LineThreshold);

                if (Result.first)
                {
                    if ((!Hit) || (Result.second < HitDist))
                    {
                        Hit = true;
                        HitDist = Result.second;
                    }
                }
            }
        }
    }

    return std::pair<bool,float>(Hit, HitDist);
}
