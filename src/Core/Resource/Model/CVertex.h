#ifndef CVERTEX_H
#define CVERTEX_H

#include <Common/Math/CVector2f.h>
#include <Common/Math/CVector3f.h>
#include <Common/CColor.h>

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

    CVertex(CVector3f& Pos)
    {
        Position = Pos;
    }

    bool operator==(const CVertex& other) {
        return ((Position == other.Position) &&
                (Normal == other.Normal) &&
                (Color[0] == other.Color[0]) &&
                (Color[1] == other.Color[1]) &&
                (Tex[0] == other.Tex[0]) &&
                (Tex[1] == other.Tex[1]) &&
                (Tex[2] == other.Tex[2]) &&
                (Tex[3] == other.Tex[3]) &&
                (Tex[4] == other.Tex[4]) &&
                (Tex[5] == other.Tex[5]) &&
                (Tex[6] == other.Tex[6]) &&
                (Tex[7] == other.Tex[7]));
    }
};

#endif // CVERTEX_H
