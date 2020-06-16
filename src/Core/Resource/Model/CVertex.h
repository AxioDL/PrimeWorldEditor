#ifndef CVERTEX_H
#define CVERTEX_H

#include <Common/CColor.h>
#include <Common/Math/CVector2f.h>
#include <Common/Math/CVector3f.h>
#include <array>

using TBoneIndices = std::array<uint8, 4>;
using TBoneWeights = std::array<float, 4>;

class CVertex
{
public:
    uint32 ArrayPosition = 0; // Position of this vertex in the input model file.
                              // This is needed to resave without breaking rigging.
    CVector3f Position;
    CVector3f Normal;
    std::array<CColor, 2> Color;
    std::array<CVector2f, 8> Tex;
    TBoneIndices BoneIndices{};
    TBoneWeights BoneWeights{};
    std::array<uint8, 8> MatrixIndices{};

    constexpr CVertex() = default;
    constexpr CVertex(const CVector3f& pos) : Position{pos}
    {
    }

    bool operator==(const CVertex& other) const {
        return Position == other.Position &&
                Normal == other.Normal &&
                Color == other.Color &&
                Tex == other.Tex &&
                BoneIndices == other.BoneIndices &&
                BoneWeights == other.BoneWeights;
    }

    bool operator!=(const CVertex& other) const
    {
        return !operator==(other);
    }
};

#endif // CVERTEX_H
