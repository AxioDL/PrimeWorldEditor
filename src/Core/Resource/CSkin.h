#ifndef CSKIN_H
#define CSKIN_H

#include "CResource.h"
#include "Core/Resource/Model/CVertex.h"

struct SVertexWeights
{
    TBoneIndices Indices;
    TBoneWeights Weights;
};

class CSkin : public CResource
{
    DECLARE_RESOURCE_TYPE(eSkin)
    friend class CSkinLoader;

    struct SVertGroup
    {
        SVertexWeights Weights;
        u32 NumVertices;
    };
    std::vector<SVertGroup> mVertGroups;

    u32 mSkinnedVertexCount;

public:
    CSkin() {}

    const SVertexWeights& WeightsForVertex(u32 VertIdx)
    {
        static const SVertexWeights skNullWeights = {
            { 0, 0, 0, 0 },
            { 0.f, 0.f, 0.f, 0.f }
        };

        u32 Index = 0;

        for (u32 iGrp = 0; iGrp < mVertGroups.size(); iGrp++)
        {
            if (VertIdx < Index + mVertGroups[iGrp].NumVertices)
                return mVertGroups[iGrp].Weights;

            Index += mVertGroups[iGrp].NumVertices;
        }

        return skNullWeights;
    }
};

#endif // CSKIN_H
