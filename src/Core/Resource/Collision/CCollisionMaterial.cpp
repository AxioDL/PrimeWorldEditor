#include "CCollisionMaterial.h"
#include <Common/Macros.h>
#include <unordered_map>
#include <array>

ECollisionFlag CCollisionMaterial::SurfaceType(EGame Game) const
{
    // Arrays determining the type hierarchy for each game.
    // Flags earlier in the list take priority over flags later in the list.
    static constexpr std::array skPrimeTypeHierarchy{
        eCF_Organic, eCF_Wood, eCF_Sand, eCF_Shield, eCF_Glass, eCF_Mud, eCF_SlowMud,
        eCF_Snow, eCF_Lava, eCF_Dirt, eCF_Phazon, eCF_MetalGrating, eCF_Ice, eCF_Grass,
        eCF_Metal, eCF_Stone
    };
    static constexpr std::array skEchoesTypeHierarchy{
        eCF_Rubber, eCF_Organic, eCF_Wood, eCF_Web, eCF_MothSeedOrganics, eCF_Sand,
        eCF_Shield, eCF_Fabric, eCF_Snow, eCF_Glass, eCF_AltMetal, eCF_Dirt, eCF_Phazon,
        eCF_MetalGrating, eCF_Ice, eCF_Grass, eCF_Metal, eCF_Stone
    };

    // Determine which list we should use.
    const ECollisionFlag* pkFlagArray;
    size_t Num;

    if (Game <= EGame::Prime)
    {
        pkFlagArray = skPrimeTypeHierarchy.data();
        Num = skPrimeTypeHierarchy.size();
    }
    else
    {
        pkFlagArray = skEchoesTypeHierarchy.data();
        Num = skEchoesTypeHierarchy.size();
    }

    // Locate type.
    for (size_t iType = 0; iType < Num; iType++)
    {
        if (*this & pkFlagArray[iType])
            return pkFlagArray[iType];
    }
    return eCF_Unknown;
}

// Type-to-color mappings
const std::unordered_map<ECollisionFlag, CColor> gkTypeToColor = {
    { eCF_Stone,            CColor::Integral(220, 215, 160) }, // Brownish/greenish
    { eCF_Metal,            CColor::Integral(143, 143, 143) }, // Gray
    { eCF_Grass,            CColor::Integral( 90, 150,  70) }, // Green
    { eCF_Ice,              CColor::Integral(200, 255, 255) }, // Light blue
    { eCF_MetalGrating,     CColor::Integral(180, 180, 180) }, // Gray
    { eCF_Phazon,           CColor::Integral(  0, 128, 255) }, // Blue
    { eCF_Dirt,             CColor::Integral(107,  84,  40) }, // Brown
    { eCF_Lava,             CColor::Integral(200,  30,  30) }, // Red
    { eCF_AltMetal,         CColor::Integral(100, 100, 100) }, // Gray
    { eCF_Snow,             CColor::Integral(230, 255, 255) }, // *Very* light blue
    { eCF_Fabric,           CColor::Integral( 64, 133, 236) }, // Blue
    { eCF_SlowMud,          CColor::Integral(109,  91,  66) }, // Brown
    { eCF_Mud,              CColor::Integral(121, 106,  84) }, // Brown
    { eCF_Glass,            CColor::Integral(172, 209, 254) }, // Greenish blue
    { eCF_Shield,           CColor::Integral(230, 250,  60) }, // Yellow
    { eCF_Sand,             CColor::Integral(225, 188, 133) }, // Light brown
    { eCF_MothSeedOrganics, CColor::Integral(172, 170, 202) }, // Light purple
    { eCF_Web,              CColor::Integral(191, 176, 162) }, // Light grayish/brown
    { eCF_Wood,             CColor::Integral(139,  90,  43) }, // Brown
    { eCF_Organic,          CColor::Integral(130, 130, 250) }, // Purple
    { eCF_Rubber,           CColor::Integral( 58,  58,  58) }  // Black
};
CColor CCollisionMaterial::SurfaceColor(EGame Game) const
{
    const ECollisionFlag SurfType = SurfaceType(Game);
    const auto FindColor = gkTypeToColor.find(SurfType);
    return (FindColor == gkTypeToColor.end() ? CColor::White() : FindColor->second);
}

bool CCollisionMaterial::IsFloor() const
{
    return HasFlag(eCF_Floor) && !HasFlag(eCF_JumpNotAllowed);
}

bool CCollisionMaterial::IsUnstandable(EGame Game) const
{
    return HasFlag(eCF_JumpNotAllowed) || (Game >= EGame::CorruptionProto && !HasFlag(eCF_Floor) && HasAnyFlags(eCF_Wall | eCF_Ceiling));
}
