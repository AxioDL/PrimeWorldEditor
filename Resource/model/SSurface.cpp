#include "SSurface.h"
#include <Common/CRayCollisionTester.h>
#include <Common/Math.h>
#include <Core/CDrawUtil.h>

std::pair<bool,float> SSurface::IntersectsRay(const CRay& Ray, const CTransform4f& Transform)
{
    //CDrawUtil::DrawWireCube(AABox.Transformed(Transform), CColor::skRed);
    bool Hit = false;
    float HitDist;

    for (u32 iPrim = 0; iPrim < Primitives.size(); iPrim++)
    {
        SPrimitive *pPrim = &Primitives[iPrim];
        u32 NumVerts = pPrim->Vertices.size();

        if ((pPrim->Type == eGX_Triangles) || (pPrim->Type == eGX_TriangleFan) || (pPrim->Type == eGX_TriangleStrip))
        {
            u32 NumTris;

            if (pPrim->Type == eGX_Triangles)
                NumTris = NumVerts / 3;
            else
                NumTris = NumVerts - 2;

            CColor LineColor;
            if (pPrim->Type == eGX_Triangles)
                LineColor = CColor::skRed;
            else if (pPrim->Type == eGX_TriangleStrip)
                LineColor = CColor::skYellow;
            else if (pPrim->Type == eGX_TriangleFan)
                LineColor = CColor::skGreen;

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
                std::pair<bool,float> TriResult = Math::RayTriangleIntersection(Ray, vtxA, vtxB, vtxC);

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

        // todo: Intersection tests for line primitives
    }

    return std::pair<bool,float>(Hit, HitDist);
}
