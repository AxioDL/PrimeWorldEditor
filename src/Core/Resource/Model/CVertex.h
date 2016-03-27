#ifndef CVERTEX_H
#define CVERTEX_H

#include <Common/CColor.h>
#include <Math/CVector2f.h>
#include <Math/CVector3f.h>

class CVertex
{
public:
    u32 ArrayPosition; // Position of this vertex in the input model file.
                       // This is needed to resave without breaking rigging.
    CVector3f Position;
    CVector3f Normal;
    CColor Color[2];
    CVector2f Tex[8];
    u8 MatrixIndices[8];

    CVertex() {}

    CVertex(CVector3f& rPos)
    {
        Position = rPos;
    }

    bool operator==(const CVertex& rkOther) {
        return ((Position == rkOther.Position) &&
                (Normal == rkOther.Normal) &&
                (Color[0] == rkOther.Color[0]) &&
                (Color[1] == rkOther.Color[1]) &&
                (Tex[0] == rkOther.Tex[0]) &&
                (Tex[1] == rkOther.Tex[1]) &&
                (Tex[2] == rkOther.Tex[2]) &&
                (Tex[3] == rkOther.Tex[3]) &&
                (Tex[4] == rkOther.Tex[4]) &&
                (Tex[5] == rkOther.Tex[5]) &&
                (Tex[6] == rkOther.Tex[6]) &&
                (Tex[7] == rkOther.Tex[7]));
    }
};

#endif // CVERTEX_H
