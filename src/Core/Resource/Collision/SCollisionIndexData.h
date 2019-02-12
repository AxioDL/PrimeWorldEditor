#ifndef SCOLLISIONINDEXDATA_H
#define SCOLLISIONINDEXDATA_H

#include "CCollisionMaterial.h"
#include <Common/Math/CVector3f.h>

/** Common index data found in all collision file formats */
struct SCollisionIndexData
{
    std::vector<CCollisionMaterial> Materials;
    std::vector<uint8>              VertexMaterialIndices;
    std::vector<uint8>              EdgeMaterialIndices;
    std::vector<uint8>              TriangleMaterialIndices;
    std::vector<uint16>             EdgeIndices;
    std::vector<uint16>             TriangleIndices;
    std::vector<uint16>             UnknownData;
    std::vector<CVector3f>          Vertices;
};

#endif // SCOLLISIONINDEXDATA_H
