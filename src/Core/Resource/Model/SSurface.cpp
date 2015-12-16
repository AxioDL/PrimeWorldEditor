#include "SSurface.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/CRayCollisionTester.h"
#include <Math/MathUtil.h>

std::pair<bool,float> SSurface::IntersectsRay(const CRay& Ray, bool allowBackfaces, float LineThreshold)
{
    bool Hit = false;
    float HitDist;

    for (u32 iPrim = 0; iPrim < Primitives.size(); iPrim++)
    {
        SPrimitive *pPrim = &Primitives[iPrim];
        u32 NumVerts = pPrim->Vertices.size();

        // Triangles
        if ((pPrim->Type == eGX_Triangles) || (pPrim->Type == eGX_TriangleFan) || (pPrim->Type == eGX_TriangleStrip))
        {
            u32 NumTris;

            if (pPrim->Type == eGX_Triangles)
                NumTris = NumVerts / 3;
            else
                NumTris = NumVerts - 2;

            for (u32 iTri = 0; iTri < NumTris; iTri++)
            {
                CVector3f vtxA, vtxB, vtxC;

                // Get the three vertices that make up the current tri
                if (pPrim->Type == eGX_Triangles)
                {
                    u32 VertIndex = iTri * 3;
                    vtxA = pPrim->Vertices[VertIndex].Position;
                    vtxB = pPrim->Vertices[VertIndex+1].Position;
                    vtxC = pPrim->Vertices[VertIndex+2].Position;
                }

                else if (pPrim->Type == eGX_TriangleFan)
                {
                    vtxA = pPrim->Vertices[0].Position;
                    vtxB = pPrim->Vertices[iTri+1].Position;
                    vtxC = pPrim->Vertices[iTri+2].Position;
                }

                else if (pPrim->Type = eGX_TriangleStrip)
                {
                    if (iTri & 0x1)
                    {
                        vtxA = pPrim->Vertices[iTri+2].Position;
                        vtxB = pPrim->Vertices[iTri+1].Position;
                        vtxC = pPrim->Vertices[iTri].Position;
                    }

                    else
                    {
                        vtxA = pPrim->Vertices[iTri].Position;
                        vtxB = pPrim->Vertices[iTri+1].Position;
                        vtxC = pPrim->Vertices[iTri+2].Position;
                    }
                }

                // Intersection test
                std::pair<bool,float> TriResult = Math::RayTriangleIntersection(Ray, vtxA, vtxB, vtxC, allowBackfaces);

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
        if ((pPrim->Type == eGX_Lines) || (pPrim->Type == eGX_LineStrip))
        {
            u32 NumLines;

            if (pPrim->Type == eGX_Lines)
                NumLines = NumVerts / 2;
            else
                NumLines = NumVerts - 1;

            for (u32 iLine = 0; iLine < NumLines; iLine++)
            {
                CVector3f vtxA, vtxB;

                // Get the two vertices that make up the current line
                u32 index = (pPrim->Type == eGX_Lines ? iLine * 2 : iLine);
                vtxA = pPrim->Vertices[index].Position;
                vtxB = pPrim->Vertices[index+1].Position;

                // Intersection test
                std::pair<bool,float> result = Math::RayLineIntersection(Ray, vtxA, vtxB, LineThreshold);

                if (result.first)
                {
                    if ((!Hit) || (result.second < HitDist))
                    {
                        Hit = true;
                        HitDist = result.second;
                    }
                }
            }
        }
    }

    return std::pair<bool,float>(Hit, HitDist);
}
